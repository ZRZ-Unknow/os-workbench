/*
Copyright (C) 2009-2010 DeSmuME team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "emufile.h"

#ifdef __NO_FILE_SYSTEM__

#include "roms.h" // from $(AM_HOME)/share/games/nes/gen/

void EMUFILE_FILE::open(const char* fname, const char* mode) {
  struct rom *cur = &roms[0];
  int found = 0;
  for (int i = 1; i < nroms; i++) {
    if (strcmp(roms[i].name, fname) == 0) {
      cur = &roms[i];
      found = 1;
    }
  }

  if (found) { printf("Found ROM '%s'\n", fname); }
  else { printf("ROM '%s' not found, using default ROM '%s'\n", fname, cur->name); }

  this->data = (u8 *)cur->body;
  this->filesize = (int)*(cur->size);
  this->curpos = 0;
	this->fname = cur->name;
	strcpy(this->mode,mode);
  this->failbit = false;
}

#else

void EMUFILE_FILE::open(const char* fname, const char* mode)
{
	fp = fopen(fname,mode);
	if(!fp)
	{
    failbit = true;
	}
	this->fname = fname;
	strcpy(this->mode,mode);
}

#endif
