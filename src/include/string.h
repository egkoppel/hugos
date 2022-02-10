#ifndef _HUG_STRING_H
#define _HUG_STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* memcpy(void *dest, const void *src, size_t num);
void* memset(void *ptr, int value, size_t n);
size_t strlen(const char *str);

#ifdef __cplusplus
}
#endif

#endif