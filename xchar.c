#include "xchar.h"

bool xchar_containedin_pointer(uchar self, uchar* data, xuint datasize) {
	xint i;
	for (i=0; i<datasize; ++i) {
		if(data[i] == self) return true;
	}
	return false;
}
