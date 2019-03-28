#include "FileOutputStream.h"
#include "FileUtils.h"
#include "../Database/Timestamp.h"
#include "../Database/Journal.h"

FileOutputStream::FileOutputStream(FILE* file, uint32_t byteOffset)
	: mFileHandle(file), mByteOffset(byteOffset) {
	if (mByteOffset > 0) {
		fseek(mFileHandle, mByteOffset, SEEK_SET);
	}
}

uint32_t FileOutputStream::writeEvents(const Timestamp* t, MutableString events) {
	const auto offset = Timestamp::BytesLength + FileUtils::SPACE_SIZE;

	char tmp[1024];
	strncpy(tmp, t->value, Timestamp::BytesLength);
	tmp[Timestamp::BytesLength] = FileUtils::SPACE;

	uint32_t newLines = 0;
	uint32_t writeLen = offset;
	const char* str = events.str;
	const char* end = events.str + events.length;
	for (; str != end; ++str) {
		tmp[writeLen++] = *str;
		if (*str == FileUtils::NL) {
			fwrite(tmp, writeLen, 1, mFileHandle);
			writeLen = offset;
			newLines++;
		}
	}

	// Write the last line (if exists)
	if (writeLen > offset) {
		fwrite(tmp, writeLen, 1, mFileHandle);
		newLines++;
	}

	// Add a EOF-marker
	fwrite(&Journal::JournalEof, 1, 1, mFileHandle);

	// Flush the data before marking the commit as "comitted".
	fflush(mFileHandle);

	// If any bytes where already written then make sure to remove the previous EOF-marker
	if (mByteOffset > 0) {
		replaceWithNL(mByteOffset - 1);
	}

	// Flush the data so that the commit is solidified
	fflush(mFileHandle);

	return events.length + offset * newLines + Journal::JournalEofLen;
}

void FileOutputStream::replaceWithNL(uint32_t pos) {
	// Replace a character somewhere on the stream with a new-line character
	auto currentPos = ftell(mFileHandle) + 1;
	fseek(mFileHandle, pos, SEEK_SET);
	fwrite(&FileUtils::NL, FileUtils::NL_SIZE, 1, mFileHandle);
	fseek(mFileHandle, currentPos, SEEK_SET);
}
