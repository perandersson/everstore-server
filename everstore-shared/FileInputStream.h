#pragma once

#include "es_config.h"
#include "ESErrorCodes.h"
#include "Bytes.h"
#include "Properties.h"

struct FileInputStream
{
	friend struct Journal;

	/**
	 * Read the entire bytes into the supplied memory
	 *
	 * @param memory Where to put the file content
	 * @return
	 */
	inline ESErrorCode readBytes(Bytes* memory) const {
		return readBytes(memory, mFileSize);
	}

	/**
	 * Read the amount of bytes from this file stream. The requested size will be clamped to the file size.
	 *
	 * @param memory
	 * @param size
	 * @return
	 */
	ESErrorCode readBytes(Bytes* memory, uint32_t size) const;

	/**
	 * Read the journal-specific bytes from this file stream. The actual size will be clamped to the file size.
	 *
	 * @param memory
	 * @param size
	 * @param journalDataSize
	 * @return
	 */
	ESErrorCode readJournalBytes(Bytes* memory, uint32_t size, uint32_t* journalDataSize);

	/**
	 * Close the input stream
	 */
	void close();

	/**
	 * How many bytes are left to read until we are at the assumed EOF
	 *
	 * @return
	 */
	inline uint32_t bytesLeft() const { return mFileSize - mByteOffset; }

protected:

	/**
	 * @param filePath The path to where the file is located
	 * @param fileSize the assumed size of the file
	 * @param byteOffset Offset, in bytes, where the stream should start read data
	 */
	FileInputStream(const string& filePath, uint32_t fileSize, uint32_t byteOffset);

	~FileInputStream();

private:
	FILE* mFile;
	uint32_t mFileSize;
	uint32_t mByteOffset;
	uint32_t mSeekAfterRead;
};
