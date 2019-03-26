#include "Store.h"
#include "Auth/FixedUserAuthenticator.hpp"

Store::Store(const Properties& props) : mProperties(props), mHost(nullptr), mServer(nullptr), mAuthenticator(nullptr) {
	mRunning.store(true, memory_order_relaxed);
}

Store::~Store() {
	if (mServer != nullptr) {
		delete mServer;
		mServer = nullptr;
	}

	if (mHost != nullptr) {
		delete mHost;
		mHost = nullptr;
	}

	if (mAuthenticator != nullptr) {
		delete mAuthenticator;
		mAuthenticator = nullptr;
	}
}

ESErrorCode Store::start() {
	// Create the root directory if missing
	FileUtils::createFolder(mProperties.rootDir + string(FileUtils::PATH_DELIM) + mProperties.journalDir);

	// Make sure that we are not already running the everstore
	const auto applicationLockPath = "everstore.lock";
	if (FileLock::exists(applicationLockPath)) {
		return ESERR_STORE_ALREADY_RUNNING;
	}

	// Make sure that the journals are consistent
	if (!performConsistencyCheck()) return ESERR_STORE_CONSISTENCY_CHECK_FAILED;

	// Initialize the event store
	ESErrorCode err = initialize();
	if (isError(err)) return err;

	// Lock the application
	FileLock lock(applicationLockPath);
	lock.addRef();

	// Listen for any incomming connections
	while (mRunning.load() && !isErrorCodeFatal(err)) {
		err = mServer->acceptClient();

		if (IsErrorButNotFatal(err)) {
			mHost->error(err);
			err = ESERR_NO_ERROR;
		}
	}

	// Mark the store as non-running
	lock.release();

	// Release the event store
	release();

	return err;
}

ESErrorCode Store::initialize() {
	ESErrorCode err;

	// Create temp folder if it does not exists
	FileUtils::createFolder(FileUtils::getTempDirectory());

	// Initialize sockets
	err = socket_init();
	if (isError(err))
		return err;

	// Create a new authenticator
	mAuthenticator = new FixedUserAuthenticator(string("admin"), string("passwd"));

	// Create host
	mHost = new IpcHost(mProperties.rootDir, mProperties.configFilename, mProperties.numWorkers);

	// Create worker processes
	const uint32_t numWorkers = mProperties.numWorkers;
	for (uint32_t i = 0; i < numWorkers; ++i) {
		err = mHost->addWorker();
		if (isError(err)) return err;
	}

	// Listen for incomming database connections
	mServer = new StoreServer(mProperties.port, mProperties.maxConnections, mHost, mAuthenticator);
	err = mServer->listen();
	if (isError(err))
		return err;

	return err;
}

void Store::stop() {
	if (mServer != nullptr) {
		mServer->close();
	}

	mRunning.store(false, memory_order_relaxed);
}

void Store::release() {
	if (mHost != nullptr) {
		mHost->close();
	}
	socket_cleanup();
}

bool Store::performConsistencyCheck() {
	const string lockSufix(".lock");
	auto files = FileUtils::findFilesEndingWith(mProperties.rootDir + string(FileUtils::PATH_DELIM) + mProperties.journalDir, lockSufix);
	for (auto& file : files) {
		const uint32_t del = file.find_last_of('.');
		const uint32_t del2 = file.find_last_of('.', del - 1);
		const string journalFile = file.substr(0, del2);

		mHost->log("Validating consistency for journal: %s", journalFile.c_str());
		Journal j(journalFile);
		if (!j.performConsistencyCheck()) {
			mHost->error("Consistency check failed for file: %s", journalFile.c_str());
			return false;
		}
	}
	return true;
}
