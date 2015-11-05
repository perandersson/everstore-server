#include "Authenticator.h"

Authenticator::Authenticator()
{

}

Authenticator::~Authenticator()
{

}

bool Authenticator::required() const {
	return true;
}

bool Authenticator::login(const string& username, const string& password) {
	// TODO: Add support for encryption, user- and passwords
	return username == string("admin") && password == string("passwd");
}
