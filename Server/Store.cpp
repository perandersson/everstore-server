#include "Store.h"
#include "Auth/FixedUserAuthenticator.hpp"
#include "../Shared/File/Path.hpp"
#include "../Shared/Socket/Socket.hpp"

Store::Store(const Config& config)
		: mConfig(config), mRunning(false), mHost(nullptr), mServer(nullptr), mAuthenticator(nullptr) {
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
	prepareDirectories();

	// Make sure that we are not already running the everstore
	const auto applicationLockPath = "everstore.lock";
	if (FileLock::exists(applicationLockPath)) {
		return ESERR_STORE_ALREADY_RUNNING;
	}

	// Make sure that the journals are consistent
	if (!performConsistencyCheck()) {
		return ESERR_STORE_CONSISTENCY_CHECK_FAILED;
	}

	// Initialize the event store
	ESErrorCode err = initialize();
	if (isError(err)) {
		return err;
	}

	// Lock the application
	FileLock lock(applicationLockPath);
	lock.addRef();

	// Listen for any incomming connections
	while (mRunning && !isErrorCodeFatal(err)) {
		err = mServer->acceptClient();

		if (IsErrorButNotFatal(err)) {
			// Only log this as an error if the server is actually running. An error is otherwise expected
			if (mRunning) {
				Log::Write(Log::Error, "Failed to accept a new client: %s (%d)", parseErrorCode(err), err);
			}
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
	// Initialize sockets
	if (!Socket::Initialize()) {
		return ESERR_SOCKET_INIT;
	}

	// Create a new authenticator
	mAuthenticator = new FixedUserAuthenticator(string("admin"), string("passwd"));

	// Create host
	mHost = new IpcHost(mConfig.rootDir, mConfig.configPath, mConfig.maxBufferSize);

	// Create worker processes
	ESErrorCode err;
	for (uint32_t i = 0; i < mConfig.numWorkers; ++i) {
		err = mHost->addWorker();
		if (isError(err)) return err;
	}

	// Listen for incoming database connections
	mServer = new StoreServer(mConfig.port, mConfig.maxConnections, mConfig.maxBufferSize, mHost, mAuthenticator);
	err = mServer->listen();
	if (isError(err))
		return err;

	mRunning = true;
	return ESERR_NO_ERROR;
}

void Store::stop() {
	if (mServer != nullptr) {
		mServer->close();
	}

	mRunning = false;
}

void Store::release() {
	if (mHost != nullptr) {
		mHost->close();
	}
	Socket::Shutdown();
}

bool Store::performConsistencyCheck() {
	const string lockSuffix(".lock");
	auto files = FileUtils::findFilesEndingWith((mConfig.rootDir + mConfig.journalDir).value, lockSuffix);
	for (auto& file : files) {
		const uint32_t del = file.find_last_of('.');
		const uint32_t del2 = file.find_last_of('.', del - 1);
		const Path journalFile(file.substr(0, del2));

		Log::Write(Log::Error, "Validating consistency for journal: %s", journalFile.value.c_str());
		Journal j(journalFile);
		if (!j.performConsistencyCheck()) {
			Log::Write(Log::Error, "Consistency check failed for journal: %s", journalFile.value.c_str());
			return false;
		}
	}
	return true;
}

void Store::prepareDirectories() {
	FileUtils::createFolder((mConfig.rootDir + mConfig.journalDir).value);
	FileUtils::createFolder(FileUtils::getTempDirectory());
}
