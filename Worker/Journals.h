#ifndef _EVERSTORE_JOURNALS_H_
#define _EVERSTORE_JOURNALS_H_

#include "../Shared/everstore.h"
#include "../Shared/File/Path.hpp"

//
// Map managing any open journals
class Journals
{
public:
	Journals(ProcessID id, uint32_t maxJournalLifeTime);

	~Journals();

	//
	// Retrieves a journal with the supplied name
	Journal* getOrCreate(const Path& path);

	// Retrieves the journal if found; NULL otherwise.
	Journal* getOrNull(const Path& path);

	//
	// Look for journals that's reacently been closed and remove them if they are old enough
	void gc();

private:
	const ProcessID mChildProcessId;
	const uint32_t mMaxJournalLifeTime;
	unordered_map<Path, Journal*> mJournals;

	// GC
	LinkedList<Journal> mJournalsToBeRemoved;
	chrono::system_clock::time_point mTimeSinceLastGC;

};

#endif
