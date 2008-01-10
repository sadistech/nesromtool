#include "types.h"

int range_count(Range *r) {
	/*
	**	returns the number of elements in the range (end - start + 1)
	*/
	
	return (r->end - r->start + 1);
}
