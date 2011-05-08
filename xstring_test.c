#include "xstring.h"

int main() {
        xstr* str = xstr_new(1);
        xstr_append(str, xstr_fromliteral("HAHAHAA", 0), xstr_fromliteral("HAHAHAA", 0), xstr_fromliteral("HAHAHAA", 0), 0);
        xstr_reverse(str);
        xstr_puts(str);
        xstr_free(str);
        shortstr(1024) z;
        xstr* s = xshortstr_init((void*) &z, 1024);
        //z.buffer[1023] = '\0';
        xstr_append(s, xstr_fromliteral("LOLOLOL", 0), str, 0);
        xstr_puts(s);
	return 0;
}
