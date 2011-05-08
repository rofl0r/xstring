/*
 * xstring.h
 *
 *  Created on: 15.10.2010
 *      Author: rofl
 */

#ifndef XSTRING_H_
#define XSTRING_H_

#include "xtypes.h"
#include "xchar.h"

#include <stdlib.h>
#include <string.h>

// heap alloced string.
typedef struct xstr_struct {
	xuint size;
	xuint capacity;
	// stores the pointer to the original allocation. used for shifting
	// and to keep always a reference to keep a GC in tune, if one is used.
	unsigned char* mallocaddr;
	// points to the beginning of our string.
	unsigned char* data;

} xstr;

// for stackalloc. size limited.
typedef struct xshortstr512_struct {
	xuint size;
	xuint capacity;
	// stores the pointer to the original allocation. used for shifting
	// and to keep always a reference to keep a GC in tune, if one is used.
	unsigned char* mallocaddr;
	// points to the beginning of our string.
	unsigned char* data;
	unsigned char buffer[512];

} xshortstr512;

#define shortstr(X) struct xshortstr##X##_struct { \
	xuint size; \
	xuint capacity; \
	uchar* mallocaddr; \
	uchar* data; \
	uchar buffer[X]; \
}

// ctor, dtor and stuff to play with the internals.
xuint rshift(xstr* str);
xstr* xshortstr_init(void* str, xuint size);
xstr* xstr_new(xuint size);
xstr* xstr_create(void);
xstr* xstr_fromliteral(const char* literal, xuint size);
void xstr_free(xstr* self);
void xstr_shiftright (xstr* self, xint count);
void xstr_shiftleft(xstr* self, xint count);
bool xstr_setcapacity(xstr* self, xuint newcapacity);
xstr* xstr_init(xstr* self, xuint capacity);
bool xstr_setlength(xstr* self, xuint new_length);

// test functions
bool xstr_empty(xstr* self);
bool xstr_compare (xstr* self, xstr* other, xint start, xint length);
bool xstr_equals (xstr* self, xstr* other);
bool xstr_startswith (xstr* self, xstr* s);
bool xstr_startswith_char (xstr* self, uchar c);
bool xstr_endswith (xstr* self, xstr* s);
bool xstr_endswith_char(xstr* self, uchar c);
bool xstr_contains_pointer(xstr* self, uchar* what, xuint whatsize);
bool xstr_contains_char(xstr* self, uchar what);
bool xstr_contains_xstr(xstr* self, xstr* what);
xuint xstr_count_char(xstr* self, uchar what, bool searchCaseSensitive);
xuint xstr_count_xstr(xstr* self, xstr* what, bool searchCaseSensitive);
xuint xstr_count_pointer(xstr* self, uchar* what, xuint whatSize, bool searchCaseSensitive);


// helper functions
xint xstr_find (xstr* self, xstr* what, xint offset, bool searchCaseSensitive);
xint xstr_find_char(xstr* self, uchar what, xint offset, bool searchCaseSensitive);
xint xstr_find_pointer (xstr* self, uchar* what, xuint whatSize, xint offset, bool searchCaseSensitive);

// conversion function
void xstr_tolower(xstr* self);
void xstr_toupper(xstr* self);

void xstr_trim_pointer(xstr* self, uchar* s, xuint sLength);
void xstr_trim_xstr(xstr* self, xstr* s);
void xstr_trim_char(xstr* self, uchar c);
void xstr_trim_whitespace(xstr* self);
void xstr_trim_space(xstr* self);

void xstr_trimleft_char(xstr* self, uchar c);
void xstr_trimleft_xstr(xstr* self, xstr* s);
void xstr_trimleft_pointer(xstr* self, uchar* s, xuint sLength);
void xstr_trimleft_whitespace(xstr* self);
void xstr_trimleft_space(xstr* self);

void xstr_trimright_char(xstr* self, uchar c);
void xstr_trimright_xstr(xstr* self, xstr* s);
void xstr_trimright_pointer(xstr* self, uchar* s, xuint sLength);
void xstr_trimright_whitespace(xstr* self);
void xstr_trimright_space(xstr* self);

void xstr_reverse(xstr* self);
void xstr_append(xstr* self, ...);

int xstr_puts(xstr* self);





#endif /* XSTRING_H_ */
