//
// Created by Per on 2015-07-08.
//

#ifndef _EVERSTORE_FILE_INPUT_STREAM_H_
#define _EVERSTORE_FILE_INPUT_STREAM_H_

#include "../es_config.h"
#include "../ESErrorCodes.h"
#include "../Memory/ByteBuffer.h"
#include "../Properties.h"

struct FileInputStream {
	friend struct Journal;

	// Destructor
	~FileInputStream();

	// Read the entire bytes into the supplied memory
	inline ESErrorCode readBytes(ByteBuffer* memory) {
		return readBytes(memory, mFileSize);
	}

	// Read the amount of bytes from this file stream. The requested size will be clamped to the file size.
	ESErrorCode readBytes(ByteBuffer* memory, uint32_t size);

	// Read the journal-specific bytes from this file stream. The actual size will be clamped to the file size.
	ESErrorCode readJournalBytes(ByteBuffer* memory, uint32_t size, _OUT uint32_t* journalDataSize);

	// Close the input stream
	void close();

	const inline uint32_t bytesLeft() const { return mFileSize - mByteOffset; }

protected:

	// 
	// \param fileName The path to where the file is located
	// \param fileSize the size of the file
	// \param bytesOffset Offset, in bytes, where the stream should start read data
	FileInputStream(const string& fileName, uint32_t fileSize, uint32_t byteOffset);

private:
	FILE* mFile;
	uint32_t mFileSize;
	uint32_t mByteOffset;
	
	uint32_t mSeekAfterRead;
};


#endif //EVENTSTORE_INPUT_STREAM_H
