/**
 * XString (c) 2010 by rofl0r.
 * 
 * licensed under GNU LGPL 2.1+
 * 
 * use at your own risk.
 */


#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define ASSERT_NOT_NULL

#ifdef ASSERT_NOT_NULL
#define USE_ASSERT
#endif

#ifdef USE_ASSERT
#include <assert.h>
#endif

#include "xstring.h"

static const uchar _XSTR_WHITESPACE[4] = " \r\n\t";
static const xint _XSTR_WHITESPACE_SIZE = sizeof(_XSTR_WHITESPACE);

static inline xint xstr_rshift(xstr* str) {
	return (str->mallocaddr == NULL || str->data == NULL ) ? 0 : (xint) str->data - (xint) str->mallocaddr;
}

static inline bool xstr_is_shortstring(xstr* str) {
	return ((xuint) str->data == (xuint) &str->data + sizeof(void*));
}

/* use like this
 * xshortstr512 temp; // let it be allocated on the stack
 * xstr* myStr = xshortstr_init((void*)&temp, 512); // work with myStr from now on.
 */
xstr* xshortstr_init(void* str, xuint bufsize) {
	// it doesnt matter which shortstring type we cast to here, since mem layout is the same.
	xshortstr512* it = (xshortstr512*) str;
	it->data = &it->buffer[0];
	it->mallocaddr = it->data;
	it->size = 0;
	it->capacity = bufsize - 1;
	return (xstr*) str;
}

/* create a new string from a literal. if size passed is 0, strlen will be called instead */
xstr* xstr_fromliteral(const char* literal, xuint size) {
	assert(literal != NULL);
	if(size==0) size = strlen((char*) literal);
	xstr* self = xstr_create();
	self->data = (uchar*) literal;
	self->size = size;
	return self;
}

xstr* xstr_create(void) {
	return (xstr*) calloc(1,sizeof(xstr));
}

xstr* xstr_new(xuint size) {
	xstr* self = xstr_create();
	return xstr_init(self, size);
}

/* free's a xstr - call only on heap alloced ones */
void xstr_free(xstr* self) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	if(self->mallocaddr && (void*) self->mallocaddr != (void*) self) 
		free(self->mallocaddr);
	free(self);
}

/*  shifts data pointer to the right count bytes if possible
    if count is bigger as possible shifts right maximum possible
    size and capacity is decreased accordingly  */
// remark: can be called with negative value (done by leftShift)
void xstr_shiftright (xstr* self, xint count) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	if (count == 0 || self->size == 0) return;
    xint c = count;
    xuint rshift = xstr_rshift(self);
    if (c > (xint) self->size) c = self->size;
    else if (c < 0 && abs(c) > rshift) c = rshift *-1;
    self->data += c;
    self->size -= c;
}

/* shifts back count bytes, only possible if shifted right before */
void xstr_shiftleft(xstr* self, xint count) {
    if (count) xstr_shiftright (self, -count); // it can be so easy
}

/**
 * Adjust this buffer's capacity, reallocate memory if needed.
 *
 * @param newCapacity The number of bytes (not characters) this buffer
 * should be capable of storing
 * returns false if no more memory could be alloc'd
 */
bool xstr_setcapacity(xstr* self, xuint newcapacity) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	xuint _rshift = xstr_rshift(self);
    xuint min = newcapacity + 1 + _rshift;

    if(min >= self->capacity) {
    	if (xstr_is_shortstring(self)) return false;
        // allocate 20% + 10 bytes more than needed - just in case
        self->capacity = (min * 120) / 100 + 10;

        // align at 8 byte boundary for performance
        xuint al = 8 - (self->capacity % 8);
        if (al < 8) self->capacity += al;

        xuint rs = _rshift;
        if (rs) xstr_shiftleft(self, rs);

        unsigned char* tmp = (unsigned char*) realloc(self->mallocaddr, self->capacity);
        if (!tmp) return false;

        // if we were coming from a string literal, copy the original data as well (gc_realloc cant work on text segment)
        if(self->size > 0 && self->mallocaddr == NULL) {
        	rs = 0;
        	memcpy(tmp, self->data, self->size);
        }

        self->mallocaddr = tmp;
        self->data = tmp;
        if (rs) xstr_shiftright(self, rs);
    }
    // just to be sure to be always zero terminated
    self->data[newcapacity] = '\0';
    return true;
}

xstr* xstr_init(xstr* self, xuint capacity) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	self->capacity = 0;
	self->data = NULL;
	self->mallocaddr = NULL;
	self->size = 0;
	if(capacity) xstr_setcapacity(self, capacity);
	return self;
}

// sets capacity and size flag, and a zero termination */
bool xstr_setlength(xstr* self, xuint new_length) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	bool ret = true;
    if(new_length != self->size) {
        if(new_length > self->capacity) {
            ret = xstr_setcapacity(self, new_length);
        }
        if (ret) {
        	self->size = new_length;
        	self->data[self->size] = '\0';
        }
    }
    return ret;
}

// @return true if the string is empty */
bool xstr_empty(xstr* self){
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	return self->size == 0;
}

// compare *length* characters of *this* with *other*, starting at *start*.
// Return true if the two strings are equal, return false if they are not. */
bool xstr_compare (xstr* self, xstr* other, xint start, xint length) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
    //"comparing %s and %s, start = %zd, length = %zd, size = %zd, start + length = %zd" printfln(data, other data, start, length, size, start + length)
    if (self->size < (start + length) || other->size < length) return false;

    xint i;
    for(i = 0; i < length; ++i) {
        if(self->data[start + i] != other->data[i]) {
            return false;
        }
    }
    return true;
}

// return true if *other* and *this* are equal (in terms of being null / having same size and content). */
bool xstr_equals (xstr* self, xstr* other)  {
    //"equaling %s and %s, size = %zd, other size = %zd" printfln(data, other data, size, other size)
    if ((self == NULL) && (other != NULL)) return false;
    if ((other == NULL) && (self != NULL)) return false;
    if ((other == NULL) && (self == NULL)) return true;
    if(other->size != self->size) return false;
    return !memcmp(self->data, other->data, self->size);
}

// @return true if the first characters of *this* are equal to *s*. */
bool xstr_startswith (xstr* self, xstr* s) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
    if (self->size < s->size) return false;
    return xstr_compare(self, s, 0, s->size);
}

// @return true if the first character of *this* is equal to *c*. */
bool xstr_startswith_char (xstr* self, uchar c) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	return (self->size > 0) && (self->data[0] == c);
}

// @return true if the last characters of *this* are equal to *s*. */
bool xstr_endswith (xstr* self, xstr* s) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
    if (self->size < s->size) return false;
    return xstr_compare(self, s, self->size - s->size, s->size);
}

// @return true if the last character of *this* is equal to *c*. */
bool xstr_endswith_char(xstr* self, uchar c) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	return (self->size > 0) && self->data[self->size] == c;
}

bool xstr_contains_pointer(xstr* self, uchar* what, xuint whatsize) {
	return(xstr_find_pointer(self, what, whatsize, 0, true) != -1);
}

bool xstr_contains_char(xstr* self, uchar what) {
	return(xstr_contains_pointer(self, &what, 1));
}

bool xstr_contains_xstr(xstr* self, xstr* what) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	return(xstr_contains_pointer(self, what->data, what->size));
}

// return the number of *what*'s occurences in *this*. */
xuint xstr_count_char(xstr* self, uchar what, bool searchCaseSensitive) {
	return xstr_count_pointer(self, &what, 1, searchCaseSensitive);
}

// return the number of *what*'s non-overlapping occurences in *this*. */
xuint xstr_count_xstr(xstr* self, xstr* what, bool searchCaseSensitive){
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	return xstr_count_pointer(self, what->data, what->size, searchCaseSensitive);
}

// return the number of *what*'s non-overlapping occurences in *this*. */
xuint xstr_count_pointer(xstr* self, uchar* what, xuint whatSize, bool searchCaseSensitive){
    xuint result = 0;
    xint offset  = (whatSize) * -1;
    while (((offset = xstr_find_pointer(self, what, whatSize, offset + whatSize, searchCaseSensitive)) != -1)) result++;
	return result;
}

/**
    returns -1 when not found, otherwise the position of the first occurence of "what"
    use offset 0 for a new search, then increase it by the last found position +1
    look at implementation of findAll() for an example
*/
xint xstr_find (xstr* self, xstr* what, xint offset, bool searchCaseSensitive) {
    return xstr_find_pointer(self, what->data, what->size, offset, searchCaseSensitive);
}

xint xstr_find_char(xstr* self, uchar what, xint offset, bool searchCaseSensitive)  {
    return xstr_find_pointer (self, &what, 1, offset, searchCaseSensitive);
}

xint xstr_find_pointer (xstr* self, uchar* what, xuint whatSize, xint offset, bool searchCaseSensitive) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	if (offset >= self->size || offset < 0 || what == NULL || whatSize == 0) return -1;

    xint maxpos = self->size - whatSize; // need a signed type here
    if (maxpos < 0) return -1;

    bool found;
    xint sstart = offset;

    for (sstart = offset; sstart < (maxpos + 1); ++sstart) {
        found = true;
        xint j;
        for (j = 0; j < whatSize; ++j) {
            if (searchCaseSensitive) {
                if (self->data[sstart + j] != what[j] ) {
                    found = false;
                    break;
                }
            } else {
                if (toupper(self->data[sstart + j]) != toupper(what[j])) {
                    found = false;
                    break;
                }
            }
        }
        if (found) return sstart;
    }
    return -1;
}

// Converts all of the characters in this Buffer to lower case.
void xstr_tolower(xstr* self) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	xint i;
    for(i = 0; i < self->size; ++i) {
        self->data[i] = tolower(self->data[i]);
    }
}

//Converts all of the characters in this Buffer to lower case.
void xstr_toupper(xstr* self) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	xint i;
	for(i=0; i < self->size; ++i) {
        self->data[i] = toupper(self->data[i]);
    }
}

void xstr_trim_pointer(xstr* self, uchar* s, xuint sLength) {
    xstr_trimright_pointer(self, s, sLength);
    xstr_trimleft_pointer(self, s, sLength);
}

void xstr_trim_xstr(xstr* self, xstr* s) {
    xstr_trim_pointer(self, s->data, s->size);
}

// *c* characters stripped at both ends. */
void xstr_trim_char(xstr* self, uchar c) {
    xstr_trim_pointer(self, &c, 1);
}

// whitespace characters (space, CR, LF, tab) stripped at both ends. */
void xstr_trim_whitespace(xstr* self) {
    xstr_trim_pointer(self, (uchar*) _XSTR_WHITESPACE, _XSTR_WHITESPACE_SIZE);
}

void xstr_trim_space(xstr* self) {
	xstr_trim_char(self, ' ');
}

// *c* character stripped from the left side. */
void xstr_trimleft_char(xstr* self, uchar c) {
    xstr_trimleft_pointer(self, &c, 1);
}

// all characters _contained in_ *s* stripped from the left side. either from *this* or a clone */
void xstr_trimleft_xstr(xstr* self, xstr* s) {
    xstr_trimleft_pointer(self, s->data, s->size);
}

// all characters _contained in_ *s* stripped from the left side. either from *this* or a clone */
void xstr_trimleft_pointer(xstr* self, uchar* s, xuint sLength) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
    if (self->size == 0 || sLength == 0) return;
    xuint start  = 0;
    while (start < self->size && xchar_containedin_pointer(*(self->data + start), s, sLength)) start += 1;
    if(start == 0) return;
    xstr_shiftright(self, start);
}

// whitespace characters (space, CR, LF, tab) stripped at left end. */
void xstr_trimleft_whitespace(xstr* self) {
    xstr_trimleft_pointer(self, (uchar*) _XSTR_WHITESPACE, _XSTR_WHITESPACE_SIZE);
}

void xstr_trimleft_space(xstr* self) {
	xstr_trimleft_char(self, ' ');
}

// *c* characters stripped from the right side. */
void xstr_trimright_char(xstr* self, uchar c) {
    xstr_trimright_pointer(self, &c, 1);
}

// *this* with all characters contained by *s* stripped from the right side.
void xstr_trimright_xstr(xstr* self, xstr* s) {
    xstr_trimright_pointer(self, s->data, s->size);
}

// @return (a copy of) *this* with all characters contained by *s* stripped from the right side
void xstr_trimright_pointer(xstr* self, uchar* s, xuint sLength) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	xint end = self->size;
	while(end > 0 && xchar_containedin_pointer(self->data[end - 1], s, sLength)) 
		end--;
	if (end != self->size)
		xstr_setlength(self, end);
}

// whitespace characters (space, CR, LF, tab) stripped at right end. */
void xstr_trimright_whitespace(xstr* self) {
	xstr_trimright_pointer(self, (uchar*) _XSTR_WHITESPACE, _XSTR_WHITESPACE_SIZE);
}

// space characters (ASCII 32) stripped from the right side. */
void xstr_trimright_space(xstr* self) {
	xstr_trimright_char(self, ' ');
}


// reverses self. "ABBA" -> "ABBA" .no. joke. "ABC" -> "CBA" */
void xstr_reverse(xstr* self) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	xuint bytesLeft = self->size;
	xint i = 0;
	while (bytesLeft > 1) {
		uchar c = self->data[i];
		self->data[i] = self->data[self->size - 1 - i];
		self->data[self->size - 1 - i] = c;
		bytesLeft -= 2;
		i += 1;
	}
}

// use like this: xstr_append(str1, str2, str3, NULL);
void xstr_append(xstr* self, ...) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	xuint totalsize = self->size;
	xstr* candidate = NULL;
	xint save;

	va_list ap;
	va_start(ap, self);
	while(1) {
		candidate = va_arg(ap, xstr*);
		if(candidate == NULL) break;
		totalsize += candidate->size;
	}
	va_end(ap);
	save = self->size;
	if (xstr_setlength(self, totalsize)) {
    	uchar* dest = self->data + save;
    	va_start(ap, self);
    	while(1) {
    		candidate = va_arg(ap, xstr*);
    		if(candidate == NULL) break;
    		memcpy(dest, candidate->data, candidate->size);
    		dest += candidate->size;
    	}
    	va_end(ap);
    }
}

int xstr_puts(xstr* self) {
#ifdef ASSERT_NOT_NULL
	assert(self != NULL);
#endif
	return puts((char*) self->data);
}
