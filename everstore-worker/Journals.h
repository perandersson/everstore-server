#ifndef _EVERSTORE_JOURNALS_H_
#define _EVERSTORE_JOURNALS_H_

#include <everstore.h>

//
// Map managing any open journals
struct Journals : unordered_map<string, Journal*> {

	Journals(ChildProcessId childProcessId, uint32_t maxJournalLifeTime);

	~Journals();

	//
	// Retrieves a journal with the supplied name
	Journal* getOrCreate(const string& name);

	// Retrieves the journal if found; NULL otherwise.
	Journal* getOrNull(const string& name);

	//
	// Look for journals that's reacently been closed and remove them if they are old enough
	void gc();

private:
	ChildProcessId mChildProcessId;
	uint32_t mMaxJournalLifeTime;

	// GC
	LinkedList<Journal> mJournalsToBeRemoved;
	chrono::system_clock::time_point mTimeSinceLastGC;
};

#endif
