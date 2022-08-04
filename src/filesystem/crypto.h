#ifndef FFS_CRYPTO_H
#define FFS_CRYPTO_H

#include <iostream>

namespace FFS::Crypto {
	void* encrypt(const void*, size_t);
	void* decrypt(const void*, size_t);
}

#endif