#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "verbosity.h"

void v_printf(int verbose_level, char *fmt, ...) {
	/*
	**	prints a string like printf() to stderr
	**	only if verbose_level == verbosity
	*/
			
	if (verbose_level <= verbosity) {
		va_list arg;
		va_start(arg, fmt);
		
		//new_fmt is fmt + \n
		char *new_fmt = (char*)malloc(strlen(fmt) + 2);
		strcpy(new_fmt, fmt);
		strcat(new_fmt, "\n");
		
		vfprintf(stderr, new_fmt, arg);
		
		free(new_fmt);
		va_end(arg);
	}
}

int increment_verbosity() {
	/*
	**	increments verbosity
	*/
	
	verbosity++;
}

int get_verbosity() {
	return verbosity;
}
