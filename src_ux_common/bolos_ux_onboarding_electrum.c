/* @BANNER@ */ 

#ifdef HAVE_ELECTRUM

#include "os.h"
#include "cx.h"

#include "bolos_ux_common.h"

int cx_math_shiftr_11(unsigned char  *r,  unsigned int len) {
  unsigned int j,b11;
  b11 = r[len-1] | ((r[len-2]&7)<<8);

  for (j = len-2; j>0; j--) {
    r[j+1] = (r[j]>>3) | (r[j-1]<<5);
  }
  r[1]  = r[0]>>3;
  r[0]  = 0;
 
  return b11;
}

static unsigned int bolos_ux_electrum_mnemonic_encode(const uint8_t *seed17, uint8_t *out, size_t outLength) {
  unsigned char tmp[17];
  unsigned int i;
  unsigned int offset = 0;
  memcpy(tmp, seed17, sizeof(tmp));
  for (i=0; i<12; i++) {
    unsigned char wordLength;
    unsigned int idx = cx_math_shiftr_11(tmp, sizeof(tmp));
    wordLength = BIP39_WORDLIST_OFFSETS[idx + 1] - BIP39_WORDLIST_OFFSETS[idx];
    if ((offset + wordLength) > outLength) {
      THROW (INVALID_PARAMETER);      
    }
    memcpy(out + offset, BIP39_WORDLIST + BIP39_WORDLIST_OFFSETS[idx], wordLength);
    offset += wordLength;
    if (i < 11) {
      if (offset > outLength) {
          THROW (INVALID_PARAMETER);
      }
      out[offset++] = ' ';
    }
  }
  return offset;
}

unsigned int bolos_ux_electrum_new_mnemonic(unsigned int version, unsigned char *out, unsigned int outLength) {
  unsigned char seed[17];
  unsigned int nonce;
  unsigned int offset;
  // Initialize a proper seed <= 132 bits
  for (;;) {
    cx_rng(seed, sizeof(seed));
    if (seed[0] < 0x10) {
      break;
    }
  }
  nonce = (seed[sizeof(seed) - 4] << 24) | (seed[sizeof(seed) - 3] << 16) | (seed[sizeof(seed) - 2] << 8) | (seed[sizeof(seed) - 1]);
  // Find a nonce that matches the version
  for (;;) {
    nonce++;
    seed[sizeof(seed) - 4] = (nonce >> 24);
    seed[sizeof(seed) - 3] = (nonce >> 16);
    seed[sizeof(seed) - 2] = (nonce >> 8);
    seed[sizeof(seed) - 1] = nonce;
    offset = bolos_ux_electrum_mnemonic_encode(seed, out, outLength);
    if (bolos_ux_electrum_mnemonic_check(version, out, offset)) {
      break;
    }
  }
  return offset;
}

unsigned int bolos_ux_electrum_mnemonic_check(unsigned int version, unsigned char *mnemonic, unsigned int mnemonicLength) {
  unsigned char tmp[64];
  cx_hmac_sha512(ELECTRUM_SEED_VERSION, ELECTRUM_SEED_VERSION_LENGTH, mnemonic, mnemonicLength, tmp, 64);
  return (tmp[0] == version);
}

#endif
