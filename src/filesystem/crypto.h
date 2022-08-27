#ifndef FFS_CRYPTO_H
#define FFS_CRYPTO_H

#include "../helpers/constants.h"

#include <iostream>

namespace FFS::Crypto {

	struct crypt_t {
		void* ptr;
		size_t len;
	};

	bool has_secret();

	crypt_t encrypt(const void*, size_t);
	crypt_t decrypt(const void*, size_t);
	
	uint8_t random_c();
}

#endif