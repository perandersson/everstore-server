#include "Journals.h"


Journals::Journals(ChildProcessID childProcessId, uint32_t maxJournalLifeTime)
		: mChildProcessId(childProcessId), mMaxJournalLifeTime(maxJournalLifeTime),
		  mJournalsToBeRemoved(offsetof(Journal, link)) {
	mTimeSinceLastGC = chrono::system_clock::now();
}

Journals::~Journals() {
	for (auto& pair : mJournals) {
		delete pair.second;
	}
	mJournals.clear();
}

Journal* Journals::getOrCreate(const Path& path) {
	auto it = mJournals.find(path);
	Journal* journal = nullptr;
	if (it == mJournals.end()) {
		journal = new Journal(path, mChildProcessId);
		mJournals[path] = journal;
		mJournalsToBeRemoved.addLast(journal);
		gc();
	} else {
		journal = it->second;
		mJournalsToBeRemoved.moveToLast(journal);
	}
	journal->refresh();
	return journal;
}

Journal* Journals::getOrNull(const Path& path) {
	auto it = mJournals.find(path);
	if (it == mJournals.end()) {
		return nullptr;
	}

	auto journal = it->second;
	journal->refresh();
	mJournalsToBeRemoved.moveToLast(journal);
	return journal;
}

void Journals::gc() {
	Log::Write(Log::Debug, "Garbage collecting journals");
	// Ignore if nothing is removable
	if (mJournalsToBeRemoved.empty()) return;

	// Ignore if not over one minute since last GC
	const auto now = chrono::system_clock::now();
	const auto timeSinceLastGC = chrono::duration_cast<chrono::seconds>(now - mTimeSinceLastGC).count();
	if (timeSinceLastGC < mMaxJournalLifeTime) return;
	mTimeSinceLastGC = now;

	Journal* journal = mJournalsToBeRemoved.first();
	while (journal != nullptr) {
		Journal* next = journal->link.tail;
		const auto duration = chrono::duration_cast<chrono::seconds>(now - journal->timeSinceLastUsed()).count();
		if (duration < mMaxJournalLifeTime)
			break;

		Log::Write(Log::Debug, "Destroing journal");
		auto it = mJournals.find(journal->path());
		mJournals.erase(it);
		delete journal;
		journal = next;
	}
}
