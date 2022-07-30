/*base64.h*/
#ifndef _BASE64_H
#define _BASE64_H

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int base64_decode(const char *base64, unsigned char *bindata);

char *base64_encode(const unsigned char *bindata, char *base64, int binlength);

#ifdef __cplusplus
}
#endif
#endif