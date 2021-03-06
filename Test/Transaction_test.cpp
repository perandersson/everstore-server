#include "../Shared/everstore.h"
#include "test/Test.h"

TEST_SUITE(Transaction)
{
	const string logSuffix(".log");

	UNIT_TEST(transactionIdsInSequence) {
		const Path journalPath(FileUtils::getTempFile() + logSuffix);
		Journal j(journalPath);
		const auto transaction1 = j.openTransaction();
		const auto transaction2 = j.openTransaction();
		const auto transaction3 = j.openTransaction();

		assertEquals(1U, transaction1.value);
		assertEquals(2U, transaction2.value);
		assertEquals(3U, transaction3.value);
	}

	UNIT_TEST(commitSuccessfulOnOneTranscation) {
		const Path journalPath(FileUtils::getTempFile() + logSuffix);
		Journal j(journalPath);
		const auto transaction = j.openTransaction();

		assertEquals(1U, transaction.value);

		const string data("data123");
		ByteBuffer bytes(32);
		memcpy(bytes.allocate(data.length()), data.c_str(), data.length());
		bytes.reset();
		MutableString events(data.length(), &bytes);

		auto err = j.tryCommit(transaction, Bits::All, events);
		assertEquals((ESErrorCode) ESERR_NO_ERROR, err);

		bytes.reset();
		auto stream = j.inputStream(0);
		err = stream->readBytes(&bytes);
		bytes.reset();
		assertEquals((ESErrorCode) ESERR_NO_ERROR, err);
		stream->close();

		bytes.moveForward(Timestamp::BytesLength + 1);
		const string endsWith = bytes.allocate(data.length());
		assertEquals(data, endsWith);
	}

	//UNIT_TEST(secondCommitFailedOnSameType) {

	//}

	//UNIT_TEST(commitSuccessfullForDifferentTypes) {

	//}
}
