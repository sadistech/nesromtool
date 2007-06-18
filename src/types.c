#include "types.h"

int range_length(Range *r) {
	/*
	**	returns the length (end - start)
	*/
	
	return (r->end - r->start);
}