#ifndef _IPSEC_SHA1_H_
#define _IPSEC_SHA1_H_

#include <stdio.h>    
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


typedef struct
{
	uint32_t state[5];
	uint32_t count[2];
	uint8_t  buffer[64];
} SHA1_CTX;

#define SHA1_HASH_SIZE 20

#if defined(rol)
#undef rol
#endif

#define SHA1HANDSOFF

#ifndef SHA_DIGESTSIZE
#define SHA_DIGESTSIZE 20
#endif

#ifndef SHA_BLOCKSIZE
#define SHA_BLOCKSIZE 64
#endif

//#define __LITTLE_ENDIAN 

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#ifdef __LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);


/* Hash a single 512-bit block. This is the core of the algorithm. */

void SHA1Transform(uint32_t state[5], const uint8_t buffer[64]);
void SHA1Init(SHA1_CTX *context);
void SHA1Update(SHA1_CTX *context, const unsigned char *data, uint32_t len);
void SHA1Final(SHA1_CTX *context, unsigned char digest[SHA1_HASH_SIZE]);
//void hmac_sha1(unsigned char *to_mac,unsigned int to_mac_length, unsigned char *key,unsigned int key_length, unsigned char *out_mac);
 

#endif /* _IPSEC_SHA1_H_ */
