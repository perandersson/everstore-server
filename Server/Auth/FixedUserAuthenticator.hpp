//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_FIXEDUSERAUTHENTICATOR_HPP
#define EVERSTORE_FIXEDUSERAUTHENTICATOR_HPP

#include "Authenticator.h"

class FixedUserAuthenticator : public Authenticator
{
public:
	FixedUserAuthenticator(const string& username, const string& password);

	bool required() const override;

	bool login(const string& username, const string& password) override;

private:
	string mUsername;
	string mPassword;
};

#endif //EVERSTORE_FIXEDUSERAUTHENTICATOR_HPP
