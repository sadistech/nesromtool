/*
 *  pathfunc.c
 *  nesromtool (borrowed and modified from gcmtool)
 *
 *  Created by spike on Fri May 21 2004.
 *  Copyright (c) 2004 Sadistech. All rights reserved.
 *
 */

#include "pathfunc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR '/'
#endif

void appendPathComponent(char *buf, char *source, char *append) {
	/*
	**	add a path component to the end of buf
	**	buf MUST have enough space allocated to hold append
	*/
		
	// create a new string... this is where we're going to temporarily store our new string...
	//	need room for \0 and for a path separator, if necessary
	char *new_string = (char*)malloc(strlen(source) + strlen(append) + 2);
	
	//copy our source string to our new string...
	strcpy(new_string, source);

	while(*new_string++); //get to the end of source
	 
	new_string -= 2; //back up 2 spots (one for the \0 and one more so we're pointing to the last character)

	//check if source already has a path separator on the end
	//if not, then put one there...
	if (*new_string != PATH_SEPARATOR) {
		new_string++;
		*new_string = PATH_SEPARATOR;
	}
	new_string++; //move on to \0 so we overwrite it (and not the path separator)

	while(*new_string++ = *append++); //do the appending

	strcpy(buf, new_string);
}

char *lastPathComponent(char *source) {
	/*
	**  returns a pointer to the start of the last path component...
	**  lastPathComponent("this/path/is/cool") returns a pointer to "cool"
	*/
	
	char *last = rindex(source, PATH_SEPARATOR);
	
	if (last && last != '\0') {
		*last++;
		return last;
	} else { 
		return source;
	}
}

char *nthPathComponent(char *buf, char *source, int n) {
	/*
	**	returns a pointer to buf
	**	sets buf to the nth path component of source.
	**	nthPathComponent("this/path/is/cool", 2) returns "is"
	*/
	
	if (!source || !buf) return NULL;
	
	int i = 0;
	char *pathItem = (char*)malloc(strlen(source) + 1);
	char *os1 = pathItem;
	char *s = source;
	
	for (i = 0; (i < n) && (s = index(s, PATH_SEPARATOR)); i++, *s++);

	//printf("- %s\n", s);

	//there was an error! (not enough path components!)
	if (!s) {
		printf("not enough path components!!!!!\n");
		free(pathItem);
		return NULL;
	}

	while (*s != PATH_SEPARATOR && (*os1++ = *s++));	
	
	*os1++ = 0;
	
	strcpy(buf, pathItem);
	
	//printf("%d of %s = %s (%s)\n", n, source, buf, os1);
	free(pathItem);

	return buf;
}

int pathComponentCount(char *source) {
	/*
	**	returns the number of path components in source
	**	ie: pathComponentCount("path/to/something/cool") --> 4
	*/
	
	int count = 0;
	
	while(*source) {
		if (*source++ == PATH_SEPARATOR) {
			count++;
		}
	}
	
	return count;
}
