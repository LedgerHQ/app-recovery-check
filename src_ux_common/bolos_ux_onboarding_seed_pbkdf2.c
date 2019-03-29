/* @BANNER@ */ 

#include "os.h"
#include "bolos_ux_common.h"

#define HMAC_LENGTH 64

#if 0

#define hashctx (((bolos_ux_bip39_buffers_t*)G_bolos_ux_context.screen_volatile_elements)->hashctx)
#define pbkdf2_f (((bolos_ux_bip39_buffers_t*)G_bolos_ux_context.screen_volatile_elements)->pbkdf2_f)
#define pbkdf2_g (((bolos_ux_bip39_buffers_t*)G_bolos_ux_context.screen_volatile_elements)->pbkdf2_g)


// ok working
void bolos_ux_pbkdf2(
unsigned char* password, unsigned int passwordlen,         
unsigned char* salt,  unsigned int saltlen,
unsigned int iterations,
unsigned char* out, unsigned int outLength) {
  unsigned long int i;
  unsigned int j;  
  unsigned int blocks = outLength / HMAC_LENGTH;
  if (outLength & (HMAC_LENGTH - 1)) {
    blocks++;
  }
  for (i=1; i<=blocks; i++) {
    salt[saltlen - 4] = (i >> 24) & 0xff;
    salt[saltlen - 3] = (i >> 16) & 0xff;
    salt[saltlen - 2] = (i >> 8) & 0xff;
    salt[saltlen - 1] = i & 0xff;
    cx_hmac_sha512_init(&hashctx, password, passwordlen);    
    cx_hmac((cx_hmac_t *)&hashctx, CX_LAST | CX_NO_REINIT, salt, saltlen, pbkdf2_g);    
    os_memmove(pbkdf2_f, pbkdf2_g, HMAC_LENGTH);
    for (j=1; j<iterations; j++) {
      cx_hmac_sha512_init(&hashctx, password, passwordlen);    
      cx_hmac((cx_hmac_t *)&hashctx, CX_LAST | CX_NO_REINIT, pbkdf2_g, HMAC_LENGTH, pbkdf2_g);    
      os_xor(pbkdf2_f, pbkdf2_f, pbkdf2_g, HMAC_LENGTH);
    }
    // here outLength is nor respected
    if (i == (blocks - 1) && (outLength & (HMAC_LENGTH - 1))) {
      os_memmove(out + HMAC_LENGTH * (i - 1), pbkdf2_f, outLength & (HMAC_LENGTH - 1));
    }
    else {
      os_memmove(out + HMAC_LENGTH * (i - 1), pbkdf2_f, HMAC_LENGTH);
    }
  }
}

#endif
