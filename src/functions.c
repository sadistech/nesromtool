#include "functions.h"
#include <stdio.h>

void hr_filesize(char *buf, double filesize) {
	/*
	**	sets the contents of buf to a human-readable string of filesize
	**	filesize is bytes, prints out the size in B, KB, MB, GB or TB
	**	ie:	1024 --> 1KB
	**			
	*/
	
	if (!buf) return;
	
	char *sizes[] = {"B", "KB", "MB", "GB", "TB"};
	
	int i = 0;
	
	for (i = 0; i < 4 && filesize >= 1024; i++) {
		filesize /= 1024;
	}
	
	sprintf(buf, "%.02f %s", filesize, sizes[i]);
}

void debug_print_argv(char **argv) {
	/*
	**	debug output...
	**	prints the argv values
	*/
	
	printf("ARGV:\n");
	
	for(; *argv != NULL; *argv++) {
		printf("  %s\n", *argv);
	}
	printf("--\n\n");
}