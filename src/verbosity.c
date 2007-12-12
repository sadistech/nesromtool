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
		
		if (verbose_level > 2) {
			fprintf(stderr, "\033[1m\033[31m*\033[m");
			int i = 0;
			for (i = 0; i < verbose_level - 2; i++) {
				fprintf(stderr, "  ");
			}
		}
		
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

void set_verbosity(int v) {
	verbosity = v;
}
