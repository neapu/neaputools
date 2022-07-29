#ifndef sha256_h
#define sha256_h
 
#include<string.h>
#include<stdio.h>
#include<stdint.h>
#ifdef __cplusplus
extern "C"{
#endif
void sha256(const unsigned char *data, size_t len, unsigned char *out);
#ifdef __cplusplus
}
#endif
 
#endif /* sha256_h */