#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include "types.h"

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

bool check_is_range(char *val) {
	/*
	**	checks if a string represents a range
	**	a range is a string like: "5-10" or "1-3"
	**	looks for presence of a '-' character in the string... returns true if one is found
	*/
	
	for(; val[0]; *val++) {
		if (val[0] == '-') {
			return true;
		}
	}
	
	return false;
}

int str_to_range(Range *r, char *val) {
	/*
	**	sets r to the range contained in val
	**	returns the number of elements in the range
	*/
	
	//error detection...
	if (!r || !val) return 0;
	
	char buf[10];
	char *buf_start = buf;
	
	int i = 0;
	
	for (i = 0; i < 10 && val[0] != '-'; i++) {
		buf_start[0] = val[0];
		*val++;
		*buf_start++;
	}
	
	*val++; //skip the '-'
	
	r->start = atoi(buf); //set the start
	r->end = atoi(val); //set the end
	
	return ((r->end) - (r->start) + 1);
}

bool write_data_to_file(char *data, u32 length, char *path) {
	/*
	**	writes data to the file at path
	**	returns false if an error occurs
	*/
	
	//check to make sure that data and path arent' null
	if (!data || !path) return false;
	
	FILE *ofile = NULL;
	
	//create and open file for writing... delete file if it exists
	if (!(ofile = fopen(path, "w"))) {
		perror(path);
		return false;
	}
	
	//write data
	if (fwrite(data, length, 1, ofile) != 1) {
		perror(path);
		fclose(ofile);
		return false;
	}
	
	fclose(ofile);
	
	return true;
}


bool append_data_to_file(char *data, u32 length, char *path) {
	/*
	**	append data to a file
	**	if file doesn't exist, then create it
	*/
	
	//error check
	if (!data || !path) return false;
	
	FILE *ofile = NULL;
	
	//open file for appending... create if it doesn't exist...
	if (!(ofile = fopen(path, "a"))) {
		perror(path);
		return false;
	}
	
	//write data...
	if (fwrite(data, length, 1, ofile) != 1) {
		perror(path);
		fclose(ofile);
		return false;
	}
	
	fclose(ofile);
	
	return true;
}
