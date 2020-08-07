#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t ret = 0;
  while (s[ret] != '\0') {
    ret++;
  }
  return ret;
}

char *strcpy(char *dst,const char *src) {
  size_t i = 0;
  for (i = 0; src[i] != '\0'; ++i) {
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i = 0;
  for (i = 0; i < n && src[i] != '\0'; ++i) {
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t dst_len = strlen(dst);
  size_t i = 0;
  for (i = 0; src[i] != '\0'; ++i) {
    dst[dst_len + i] = src[i];
  }
  dst[dst_len + i] = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  size_t i = 0;
  for (i = 0; s1[i] != '\0' && s2[i] != '\0'; ++i) {
    if (s1[i] != s2[i]) {
      return s1[i] < s2[i] ? -1 : 1;
    } 
  }
  return s1[i] == s2[i] ? 0 : (s1[i] < s2[i] ? -1 : 1);
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i = 0;
  for (i = 0; i < n && s1[i] != '\0' && s2[i] != '\0'; ++i) {
    if (s1[i] != s2[i]) {
      return s1[i] < s2[i] ? -1 : 1;
    } 
  }
  return i == n ? 0 : (s1[i] < s2[i] ? -1 : 1);
}

void *memset(void *v, int c, size_t n) {
  uint8_t c8 = (uint8_t) c & 0xff;
  uint32_t c32 = (uint32_t) c8 | ((uint32_t) c8 << 8) | ((uint32_t) c8 << 16) | ((uint32_t) c8 << 24);

  uint8_t *pv = (uint8_t *) v;

  int i = 0, loops = (n / sizeof(uint32_t));
  for (i = 0; i < loops; ++i) {
    *((uint32_t *) pv) = c32;
    pv += sizeof(uint32_t);
  }

  loops = (n % sizeof(uint32_t));
  for (i = 0; i < loops; ++i) {
    *pv = c8;
    pv++;
  }
  return v;
}

void *memcpy(void *out, const void *in, size_t n) {
  int8_t *pout = (int8_t *) out;
  int8_t *pin = (int8_t *) in;

  int i = 0, loops = (n / sizeof(int32_t));
  for (i = 0; i < loops; ++i) {
    *((int32_t *) pout) = *((int32_t *) pin);
    pout += sizeof(int32_t);
    pin += sizeof(int32_t);
  }

  loops = (n % sizeof(int32_t));
  for (i = 0; i < loops; ++i) {
    *pout = *pin;
    pout++;
    pin++;
  }
  return NULL;
}

void *memmove(void *dest, const void *src, size_t n) {
  if (src + n < dest || src > dest + n) {
    // no overlap
    memcpy(dest, src, n);
  } else if (src >= dest) {
    // front to end
    int8_t *pin = (int8_t *) src;
    int8_t *pout = (int8_t *) dest;
    for (int i = 0; i < n; ++i) {
      *pout = *pin;
      ++pout, ++pin;
    }
  } else {
    // end to front
    int8_t *pin = (int8_t *) (src + n);
    int8_t *pout = (int8_t *) (dest + n);
    for (int i = 0; i < n; ++i) {
      *pout = *pin;
      --pout, --pin;
    }
  }
  return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  int8_t *p1 = (int8_t *) s1;
  int8_t *p2 = (int8_t *) s2;

  int i = 0, loops = (n / sizeof(int32_t));
  for (i = 0; i < loops; ++i) {
    if (*((int32_t *) p1) != *((int32_t *) p2)) {
      return *((int32_t *) p1) < *((int32_t *) p2) ? -1 : 1;
    } 
    p1 += sizeof(int32_t);
    p2 += sizeof(int32_t);
  }

  loops = (n % sizeof(int32_t));
  for (i = 0; i < loops; ++i) {
    if (*p1 != *p2) {
      return *p1 < *p2 ? -1 : 1;
    }
    p1++;
    p2++;
  }
  return 0;
}

#endif