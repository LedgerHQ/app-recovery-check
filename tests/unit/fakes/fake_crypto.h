#pragma once

#include <stdint.h>

// BIP32 master key: 32 bytes of private key, then 32 bytes of chain code
#define BIP32_ROOT_KEY_SIZE 64

// BIP32 master key of the default Speculos mnemonic, see fake_crypto.c
extern const uint8_t REFERENCE_ROOT_KEY[BIP32_ROOT_KEY_SIZE];

// Same mnemonic with a BIP39 passphrase
extern const char REFERENCE_PASSPHRASE[];
extern const uint8_t REFERENCE_PASSPHRASE_ROOT_KEY[BIP32_ROOT_KEY_SIZE];

/*
 * Plugs the libsodium implementations of SHA-512, HMAC-SHA512, PBKDF2 and
 * os_secure_memcmp into the crypto mocks, and sets the master key the device
 * holds. To call after the mocks Init().
 */
void fake_crypto_setup(const uint8_t *device_root_key);
