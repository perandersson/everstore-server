//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_UNIXMUTEX_HPP
#define EVERSTORE_UNIXMUTEX_HPP


struct OsMutex
{
	inline static bool IsInvalid(const OsMutex* mutex) { return mutex == nullptr; }
};


#endif //EVERSTORE_UNIXMUTEX_HPP
