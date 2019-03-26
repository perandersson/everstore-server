#include "Journals.h"


Journals::Journals(ChildProcessId childProcessId, uint32_t maxJournalLifeTime)
		: mChildProcessId(childProcessId), mMaxJournalLifeTime(maxJournalLifeTime),
		  mJournalsToBeRemoved(offsetof(Journal, link)) {
	mTimeSinceLastGC = chrono::system_clock::now();
}

Journals::~Journals() {

}

Journal* Journals::getOrCreate(const string& name) {
	auto it = mJournals.find(name);
	Journal* journal = nullptr;
	if (it == mJournals.end()) {
		journal = new Journal(name, mChildProcessId);
		mJournals.insert(make_pair(name, journal));
		mJournalsToBeRemoved.addLast(journal);
		gc();
	} else {
		journal = it->second;
		mJournalsToBeRemoved.moveToLast(journal);
	}
	journal->refresh();
	return journal;
}

Journal* Journals::getOrNull(const string& name) {
	auto it = mJournals.find(name);
	if (it == mJournals.end()) return nullptr;

	auto journal = it->second;
	journal->refresh();
	mJournalsToBeRemoved.moveToLast(journal);
	return journal;
}

void Journals::gc() {
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

		auto it = mJournals.find(journal->name());
		mJournals.erase(it);
		delete journal;
		journal = next;
	}
}
