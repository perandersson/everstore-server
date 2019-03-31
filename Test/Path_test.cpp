//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "../Shared/everstore.h"
#include "../Shared/File/Path.hpp"
#include "test/Test.h"

TEST_SUITE(Path)
{
	UNIT_TEST(EmptyPath) {
		const Path path;
		assertEquals(0u, path.hash);
		assertEquals(string(""), path.value);
	}

	UNIT_TEST(SimplePath) {
		const Path path("/test");
		assertTrue(path.hash > 0u);
		assertEquals(Path::StrPathDelim + string("test"), path.value);
	}

	UNIT_TEST(AddingTwoPaths) {
		const Path path("/test");
		const Path path2("/asdf");
		const Path result = path + path2;
		assertTrue(result.hash > 0u);
		assertEquals(Path::StrPathDelim + string("test") + Path::StrPathDelim + string("asdf"), result.value);
	}

	UNIT_TEST(AddingOneStringTwoPath) {
		const Path path("/test");
		const string path2("/asdf");
		const Path result = path + path2;
		assertTrue(result.hash > 0u);
		assertEquals(Path::StrPathDelim + string("test") + Path::StrPathDelim + string("asdf"), result.value);
	}

	UNIT_TEST(DirectoryFromFileAtRoot) {
		const Path path("/path");
		const Path result = path.GetDirectory();
		assertEquals(Path("/"), result);
	}

	UNIT_TEST(DirectoryFromFileAtDepth2) {
		const Path path("/path/to/file");
		const Path result = path.GetDirectory();
		assertEquals(Path("/path/to"), result);
	}

	UNIT_TEST(DirectoryFromDepth2) {
		const Path path("/path/to/");
		const Path result = path.GetDirectory();
		assertEquals(Path("/path/to"), result);
	}

	UNIT_TEST(DirectoryFromRelativePath) {
		const Path path("path");
		const Path result = path.GetDirectory();
		assertEquals(Path::GetWorkingDirectory(), result);
	}
}