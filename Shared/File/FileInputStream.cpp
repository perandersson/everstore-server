#include "FileInputStream.h"
#include "FileUtils.h"
#include "../Database/Timestamp.h"

static const uint32_t TEMP_READ_BLOCK_SIZE = 4096;
static const uint32_t TIMESTAMP_AND_SPACE_LEN = Timestamp::BytesLength + 1;

FileInputStream::FileInputStream(FILE* file, uint32_t fileSize, uint32_t byteOffset)
		: mFile(file), mFileSize(fileSize), mByteOffset(byteOffset), mSeekAfterRead(TIMESTAMP_AND_SPACE_LEN) {
	assert(file != nullptr);
	if (mByteOffset > mFileSize) {
		mByteOffset = mFileSize;
	}
	fseek(mFile, mByteOffset, SEEK_SET);
}

ESErrorCode FileInputStream::readBytes(ByteBuffer* memory, uint32_t size) {
	assert(memory != nullptr);
	if (size == 0) {
		return ESERR_NO_ERROR;
	}

	// Clamp to file size
	const auto readBytes = size > mFileSize ? mFileSize : size;

	// Read the file into the supplied memory block
	const auto ret = fread(memory->allocate(readBytes), readBytes, 1, mFile);
	if (ret != 1) return ESERR_JOURNAL_READ;
	return ESERR_NO_ERROR;
}

ESErrorCode FileInputStream::readJournalBytes(ByteBuffer* memory, uint32_t size, uint32_t* journalDataSize) {
	assert(memory != nullptr);
	assert(journalDataSize != nullptr);

	// Initialize the size to 0
	*journalDataSize = 0;

	// Do nothing if we don't want to read anything
	if (size == 0) {
		return ESERR_NO_ERROR;
	}

	// Do nothing if no bytes are to be read
	auto bytesLeft = mFileSize - mByteOffset;
	if (bytesLeft == 0) {
		return ESERR_NO_ERROR;
	}

	// How many bytes are written to the memory block
	auto bytesWritten = 0u;

	// Read journal data and put it into the memory bytes block as long as there are bytes left to be read
	while (bytesLeft > 0 && bytesWritten < size) {

		// Ignore bytes if neccessary
		if (mSeekAfterRead > 0) {
			if (fseek(mFile, mSeekAfterRead, SEEK_CUR) != 0) {
				return ESERR_JOURNAL_READ;
			}
			mByteOffset += mSeekAfterRead;
			bytesLeft -= mSeekAfterRead;
		}

		// Clamp read size
		const auto readBytes = size > bytesLeft ? bytesLeft : size;
		const auto clampedReadBytes = TEMP_READ_BLOCK_SIZE > readBytes ? readBytes : TEMP_READ_BLOCK_SIZE;

		// Read the file into the supplied memory block
		char* current = memory->allocate(clampedReadBytes);
		const auto end = memory->end();
		const auto ret = fread(current, 1, clampedReadBytes, mFile);
		if (ret != clampedReadBytes) {
			return ESERR_JOURNAL_READ;
		}
		mByteOffset += clampedReadBytes;
		bytesLeft -= clampedReadBytes;

		// Where to move the data
		char* moveDataTo = current;
		mSeekAfterRead = 0;

		// Move the data around up to the maximum size
		uint32_t totalBytes = 0;
		while (current != end) {
			// Move data if not NL, otherwise make sure to ignore the timestamp
			if (*current != FileUtils::NL) {
				*moveDataTo++ = *current++;
				totalBytes++;

				// If we've read to much data, then stop parsing and make sure to move the file handle to the correct position
				if ((bytesWritten + totalBytes) == size) {
					const uint32_t bytesLeftInBuffer = (size_t) (end) - (size_t) (current);
					const int offset = -(int) bytesLeftInBuffer;
					if (fseek(mFile, offset, SEEK_CUR) != 0) {
						return ESERR_JOURNAL_READ;
					}
					bytesLeft += bytesLeftInBuffer;
					mByteOffset -= bytesLeftInBuffer;
					break;
				}
			} else {
				// Copy NL
				*moveDataTo++ = *current++;
				totalBytes++;

				// If we've read to much data, then stop parsing and make sure to move the file handle to the correct position
				if ((bytesWritten + totalBytes) == size) {
					const uint32_t bytesLeftInBuffer = (size_t) (end) - (size_t) (current);
					const int offset = -(int) bytesLeftInBuffer;
					if (fseek(mFile, offset, SEEK_CUR) != 0) {
						return ESERR_JOURNAL_READ;
					}
					bytesLeft += bytesLeftInBuffer;
					mByteOffset -= bytesLeftInBuffer;

					// Ignore the upcomming timestamp and whitespace
					mSeekAfterRead = TIMESTAMP_AND_SPACE_LEN;
					break;
				}

				// Ignore timestamp and space
				const uint32_t bytesLeftInBuffer = (size_t) (end) - (size_t) (current);
				const uint32_t seek = TIMESTAMP_AND_SPACE_LEN > bytesLeftInBuffer
				                      ? bytesLeftInBuffer : TIMESTAMP_AND_SPACE_LEN;

				mSeekAfterRead = TIMESTAMP_AND_SPACE_LEN - seek;
				current += seek;

			}
		}

		// Move backwards in the buffer to the points where we are allowed to data again
		memory->moveBackwards(clampedReadBytes - totalBytes);
		bytesWritten += totalBytes;
	}

	*journalDataSize = bytesWritten;
	return ESERR_NO_ERROR;
}

void FileInputStream::close() {
	delete this;
}
