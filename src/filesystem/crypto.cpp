#include "crypto.h"

#include "../secret.h"

#include "../exceptions/exceptions.h"

#include "../helpers/logger.h"

#include "../helpers/constants.h"

#include <iostream>

#include <cryptlib.h>
#include <aes.h>
#include <sha.h>
#include <hkdf.h>
#include <modes.h>
#include <osrng.h>
#include <files.h>
#include <cstdlib>
#include <gcm.h>
#include <filters.h>

#define KEY_LEN CryptoPP::AES::DEFAULT_KEYLENGTH

#define TAG_LEN 16 // In bytes == 128 bits 

#define SALT_LEN 64 // 16 bytes salt

CryptoPP::AutoSeededRandomPool rng;

char* get_secret() {
	return getenv(FFS_ENCRYPTION_SECRET_KEY);
}

bool FFS::Crypto::has_secret() {
	return get_secret() != nullptr;
}

const CryptoPP::SecByteBlock generate_salt() {
	CryptoPP::SecByteBlock salt(SALT_LEN);
	rng.GenerateBlock(salt, SALT_LEN);

	return salt;
}

const CryptoPP::SecByteBlock derive_key(const CryptoPP::SecByteBlock salt) {
	CryptoPP::byte* password = (unsigned char*) get_secret();
	auto pass_len = strlen((const char*) password);

	CryptoPP::SecByteBlock key(KEY_LEN);

	CryptoPP::HKDF<CryptoPP::SHA256> key_derivation_func;

	key_derivation_func.DeriveKey(key, KEY_LEN, password, pass_len, salt, SALT_LEN, nullptr, 0);

	return key;
}

FFS::Crypto::crypt_t FFS::Crypto::encrypt(const void* in_ptr, size_t len) {
	if(!has_secret())
		throw FFS::NoEncryptionSecret();

	CryptoPP::GCM<CryptoPP::AES>::Encryption e;

	size_t iv_len = e.IVSize();
	CryptoPP::SecByteBlock iv(iv_len);
	e.GetNextIV(rng, iv);

	auto salt = generate_salt();
	auto key = derive_key(salt);
	
	e.SetKeyWithIV(key, KEY_LEN, iv, iv_len);

	// Cipher string
	std::string cipher;
	CryptoPP::StringSource s((unsigned char*) in_ptr, len, true, 
            new CryptoPP::AuthenticatedEncryptionFilter(e,
                new CryptoPP::StringSink(cipher),
				false,
				TAG_LEN
            )
        );
	
	
	size_t output_encryption_length = SALT_LEN + e.IVSize() + cipher.size();
	// Create char ptr with iv + cipher
	char* out_ptr = new char[output_encryption_length];

	auto ptr_offset = 0;

	// Copy salt to beginning
	memcpy(out_ptr + ptr_offset, salt, SALT_LEN);
	ptr_offset += SALT_LEN;

	// Copy iv to after salt
	memcpy(out_ptr + ptr_offset, iv, e.IVSize());
	ptr_offset += e.IVSize();

	// Copy cipher to after iv
	memcpy(out_ptr + ptr_offset, cipher.c_str(), cipher.size());

	crypt_t crypt;
	crypt.ptr = (void*) out_ptr;
	crypt.len = output_encryption_length;

	return crypt;
}

FFS::Crypto::crypt_t FFS::Crypto::decrypt(const void* i_ptr, size_t len) {
	if(!has_secret())
		throw FFS::NoEncryptionSecret();
		
	CryptoPP::GCM<CryptoPP::AES>::Decryption d;

	// re-define type of ptr to handle bytes
	const CryptoPP::byte* ptr = (const unsigned char*) i_ptr;

	int ptr_offset = 0;

	CryptoPP::SecByteBlock salt(SALT_LEN);
	memcpy(salt, ptr + ptr_offset, SALT_LEN);
	ptr_offset += SALT_LEN;

	CryptoPP::SecByteBlock iv(d.IVSize());
	memcpy(iv, ptr + ptr_offset, d.IVSize());
	ptr_offset += d.IVSize();

	auto cipher_len = len - d.IVSize() - SALT_LEN;

	auto key = derive_key(salt);
	d.SetKeyWithIV(key, CryptoPP::AES::DEFAULT_KEYLENGTH, iv);

	std::string recovered;

	auto auth_dec_filt = CryptoPP::AuthenticatedDecryptionFilter(d,
		new CryptoPP::StringSink(recovered),
		CryptoPP::AuthenticatedDecryptionFilter::DEFAULT_FLAGS,
		TAG_LEN
	);

	CryptoPP::StringSource s(ptr + ptr_offset, cipher_len, true, 
		new CryptoPP::Redirector(auth_dec_filt)
	);

	auto auth_success = auth_dec_filt.GetLastResult();
	if(!auth_success)
		throw FFS::CipherIntegrityCompromised();

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