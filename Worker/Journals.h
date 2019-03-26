#ifndef _EVERSTORE_JOURNALS_H_
#define _EVERSTORE_JOURNALS_H_

#include "../Shared/everstore.h"

//
// Map managing any open journals
class Journals
{
public:
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
	const ChildProcessId mChildProcessId;
	const uint32_t mMaxJournalLifeTime;
	unordered_map<string, Journal*> mJournals;

	// GC
	LinkedList<Journal> mJournalsToBeRemoved;
	chrono::system_clock::time_point mTimeSinceLastGC;

};

#endif
