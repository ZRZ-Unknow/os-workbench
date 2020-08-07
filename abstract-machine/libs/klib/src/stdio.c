#include "klib.h"
#include <stdarg.h>
#include <stdbool.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define VBUF_MAX_SIZE 128
#define PBUF_MAX_SIZE 1024

/* arg register for arguments */
// TODO: add more types of arguments!
union arg {
  int intarg;
  char chararg;
  char *pchararg;
} uarg;

char vbuf[VBUF_MAX_SIZE];
char pbuf[PBUF_MAX_SIZE];

/* print an integer to vbuffer zone 
 * and return its start bias */

int vprintf_int(int src, int len, char phchar, const int base) {
  vbuf[VBUF_MAX_SIZE - 1] = '\0';
  if (src == 0) {
    vbuf[VBUF_MAX_SIZE - 2] = '0';
    return VBUF_MAX_SIZE - 2;
  } else {
    int pos = VBUF_MAX_SIZE - 2, cur = 0;
    for ( ; src != 0 && pos >= 0; src /= base, --pos, --len) {
      cur = src % base;
      if (cur < 0) cur = -cur; // bug of INT_MIN solved!
      vbuf[pos] = cur < 10 ? cur + '0' : cur - 10 + 'a'; 
    }
    for ( ; len > 0 && pos >= 0; --pos, --len) {
      vbuf[pos] = phchar;
    }
    return pos + 1;
  }
}

int printf(const char *fmt, ...) {
  int ret = 0;
  va_list ap;

  va_start(ap, fmt);
  ret = vsprintf(pbuf, fmt, ap);
  va_end(ap);

  for (char *s = pbuf; *s; ++s) {
    _putc(*s);
  }
  return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int  ret    = 0;     // character counter
  int  len    = 0;     // length of a signle token
  int  bias   = 0;     // bias of vbuf array
  char phchar = ' ';   // place holder character
  int  width  = 0;     // width from format
  //int  prec   = 0;     // precision from format
  bool done   = false; // done scannning an token 

  char *pfmt = (char *) fmt, *pout = out; // pointers
  while (*pfmt != '\0') {
    for ( ; *pfmt != '\0' && *pfmt != '%'; ++pfmt, ++ret, ++pout) {
      *pout = *pfmt;
    }
    *pout = '\0'; // mark the end of normal string

    if (*pfmt == '\0') {
      break; // done
    } else {
      width = 0;
      phchar = ' ';
      done = false;

      while (!done) {
        pfmt++;
        done = true; // default syntax is one-character long
        switch (*pfmt) {
          case 'c':
            uarg.chararg = va_arg(ap, int);
            len = 1;
            *pout = uarg.chararg; 
            break;
          case 's':
            uarg.pchararg = va_arg(ap, char*);
            len = strlen(uarg.pchararg);
            strcat(pout, uarg.pchararg);
            break;
          case '0':
            done = false;
            if (width == 0) {
              phchar = '0';
              break;
            }
            // no break if width != 0
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            done = false;
            width = width * 10 + (int) (*pfmt - '0');
            break;
          case 'p': // pointer, continue to 'x'&'d'
            phchar = '0';
            width = 8;
          case 'x':
          case 'd':
            uarg.intarg = va_arg(ap, int);
            if (uarg.intarg < 0) {
              strcat(pout, "-");
              ret++;
              pout++;
            }
            bias = vprintf_int(uarg.intarg, width, phchar, (*pfmt == 'd' ? 10 : 16));
            len = (int) VBUF_MAX_SIZE - bias - 1;
            strcat(pout, vbuf + bias);
            break;
          case '\0':
            assert(0);
          default:
            len = 30;
            strcat(pout, "implement me at vsprintf \0");
            break;
        }
      }
      pfmt++; // omit the last syntax
      ret += len;
      pout += len;  
    }
  }

  return ret;
}

int sprintf(char *out, const char *fmt, ...) {
  int ret = 0;
  va_list ap;

  va_start(ap, fmt);
  ret = vsprintf(out, fmt, ap);
  va_end(ap);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  int ret = 0;
  va_list ap;

  va_start(ap, fmt);
  ret = vsprintf(pbuf, fmt, ap);
  va_end(ap);

  // move n bytes from pbuf to out
  // if ret < n, move ret instead
  if (ret > n) ret = n;
  assert(ret < PBUF_MAX_SIZE);
  strncpy(out, pbuf, ret);
  out[ret] = '\0'; 

  return ret;
}

#endif