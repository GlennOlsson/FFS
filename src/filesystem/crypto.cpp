#include "crypto.h"

#include "../secret.h"

#include <iostream>

#include <cryptlib.h>
#include <aes.h>
#include <sha.h>
#include <hkdf.h>

// Will work even if access tokens needs renewal. Will only have to change if app tokens are revoked/re-generated
// i.e. will work for any user who authenticates with FFS Flickr app
#define UNDERIVED_KEY FFS_FLICKR_APP_CONSUMER_SECRET

#define KEY_LEN CryptoPP::AES::DEFAULT_KEYLENGTH

// Some public data
const char* salt = "FFS_ULTIMATE_FILESYSTEM_SALT";

CryptoPP::byte* derive_key() {
	CryptoPP::byte* password = (unsigned char*) UNDERIVED_KEY;

	CryptoPP::byte* key = new CryptoPP::byte[KEY_LEN];

	CryptoPP::HKDF<CryptoPP::SHA256> key_derivation_func;

	auto salt_bytes = (const CryptoPP::byte*) salt;

	key_derivation_func.DeriveKey(key, KEY_LEN, password, strlen(UNDERIVED_KEY), salt_bytes, strlen(salt), nullptr, 0);

	return key;
}

void* FFS::Crypto::encrypt(const void* ptr, size_t len) {
	char* ret_p = new char[len];
	memcpy(ret_p, ptr, len);
	return ret_p;
}

void* FFS::Crypto::decrypt(const void* ptr, size_t len) {
	char* ret_p = new char[len];
	memcpy(ret_p, ptr, len);
	return ret_p;
}
