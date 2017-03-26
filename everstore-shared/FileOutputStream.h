//
// Created by Per on 2015-07-08.
//

#ifndef _EVERSTORE_FILE_OUTPUT_STREAM_H_
#define _EVERSTORE_FILE_OUTPUT_STREAM_H_

#include "es_config.h"
#include "Event.h"
#include "Bytes.h"


struct FileOutputStream
{
	friend struct Journal;

	// Destructor
	~FileOutputStream();

	//
	// Write the supplied event to the supplied associated file
	//
	// \param t Timestamp for when the event is saved to the HDD
	// \param events The events we want to save
	uint32_t writeEvents(const Timestamp* t, IntrusiveBytesString events);

	// Replace the character at the given position with a newline
	void replaceWithNL(uint32_t pos);

	void close();

protected:
	//
	// \param fileName 
	// \param bytesOffset
	FileOutputStream(const string& fileName, uint32_t byteOffset);

private:
	uint32_t mByteOffset;
	FILE* mFileHandle;
};

#endif
