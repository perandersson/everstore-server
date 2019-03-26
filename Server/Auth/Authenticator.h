#ifndef _EVERSTORE_AUTHENTICATOR_H_
#define _EVERSTORE_AUTHENTICATOR_H_

#include "../../Shared/everstore.h"

class Authenticator
{
public:
	virtual ~Authenticator() = default;

	/**
	 * Is authentication required?
	 *
	 * @return <code>true</code> if the current implementation requires authentication
	 */
	virtual bool required() const = 0;

	/**
	 * Try to login using the supplied username and password
	 *
	 * @param username
	 * @param password
	 * @return
	 */
	virtual bool login(const string& username, const string& password) = 0;
};

#endif
