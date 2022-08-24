#include "crypto.h"

#include "../secret.h"

#include <iostream>

#include <cryptlib.h>
#include <aes.h>
#include <sha.h>
#include <hkdf.h>
#include <modes.h>
#include <osrng.h>
#include <files.h>
#include <cstdlib>

// Will work even if access tokens needs renewal. Will only have to change if app tokens are revoked/re-generated
// i.e. will work for any user who authenticates with FFS Flickr app
#define UNDERIVED_KEY getenv("FFS_PASSWORD")

#define KEY_LEN CryptoPP::AES::DEFAULT_KEYLENGTH

// Some public data
const char* salt = "FFS_ULTIMATE_FILESYSTEM_SALT";

CryptoPP::AutoSeededRandomPool rng;

CryptoPP::byte* derive_key() {
	CryptoPP::byte* password = (unsigned char*) UNDERIVED_KEY;

	CryptoPP::byte* key = new CryptoPP::byte[KEY_LEN];

	CryptoPP::HKDF<CryptoPP::SHA256> key_derivation_func;

	auto salt_bytes = (const CryptoPP::byte*) salt;

	key_derivation_func.DeriveKey(key, KEY_LEN, password, strlen(UNDERIVED_KEY), salt_bytes, strlen(salt), nullptr, 0);

	return key;
}

FFS::Crypto::crypt_t FFS::Crypto::encrypt(const void* in_ptr, size_t len) {
	CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption e;

	size_t iv_len = e.IVSize();
	CryptoPP::byte iv[iv_len];

	rng.GenerateBlock(iv, iv_len);

	auto key = derive_key();
	
	e.SetKeyWithIV(key, KEY_LEN, iv, iv_len);

	// Cipher string
	std::string cipher;
	CryptoPP::StringSource s((unsigned char*) in_ptr, len, true, 
            new CryptoPP::StreamTransformationFilter(e,
                new CryptoPP::StringSink(cipher)
            )
        );
	
	size_t iv_cipher_len = e.IVSize() + cipher.size();
	// Create char ptr with iv + cipher
	char* out_ptr = new char[iv_cipher_len];

	// Copy iv to beginning
	memcpy(out_ptr, iv, e.IVSize());
	// Copy cipher to after cipher
	memcpy(out_ptr + e.IVSize(), cipher.c_str(), cipher.size());

	crypt_t crypt;
	crypt.ptr = (void*) out_ptr;
	crypt.len = iv_cipher_len;

	return crypt;
}

FFS::Crypto::crypt_t FFS::Crypto::decrypt(const void* ptr, size_t len) {
	CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption d;

	CryptoPP::byte iv[d.IVSize()];
	memcpy(iv, ptr, d.IVSize());

	auto cipher_len = len - d.IVSize();

	auto key = derive_key();
	d.SetKeyWithIV(key, CryptoPP::AES::DEFAULT_KEYLENGTH, iv);

	std::string recovered;
	CryptoPP::StringSource s(((unsigned char*) ptr) + d.IVSize(), cipher_len, true, 
		new CryptoPP::StreamTransformationFilter(d,
			new CryptoPP::StringSink(recovered)
		)
	);

	char* out_ptr = new char[recovered.size()];
	memcpy(out_ptr, recovered.c_str(), recovered.size());

	crypt_t crypt;
	crypt.ptr = out_ptr;
	crypt.len = recovered.size();

	return crypt;
}

uint8_t FFS::Crypto::random_c() {
	return rng.GenerateByte();
}