//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "FixedUserAuthenticator.hpp"

FixedUserAuthenticator::FixedUserAuthenticator(const string& username, const string& password)
		: mUsername(username), mPassword(password) {
}

bool FixedUserAuthenticator::required() const {
	return !mUsername.empty();
}

bool FixedUserAuthenticator::login(const string& username, const string& password) {
	return mUsername == username && mPassword == password;
}
