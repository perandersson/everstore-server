#ifndef _EVERSTORE_AUTHENTICATOR_H_
#define _EVERSTORE_AUTHENTICATOR_H_

#include <everstore.h>

struct Authenticator {

	Authenticator();

	~Authenticator();

	// Is authentication required?
	bool required() const;

	// Try to login using the supplied username and password
	bool login(const string& username, const string& password);

};

#endif
