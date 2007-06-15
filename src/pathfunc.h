/*
 *  pathfunc.h
 *  various functions for working with paths (splitting and appending and whatnot)
 *  (borrowed and modified from gcmtool project)
 *
 *  Created by spike on Fri May 21 2004.
 *  Copyright (c) 2004 Sadistech. All rights reserved.
 *
 */

#ifndef _PATHFUNC_H_
#define _PATHFUNC_H_

#define PATH_SEPARATOR '/'

//for windows, use this instead
//#define kpathSeparator '\\'

#ifdef __cplusplus
extern "C" {
#endif

void appendPathComponent(char *buf, char *source, char *append);
char *lastPathComponent(char *source);
char *nthPathComponent(char *buf, char *source, int n);
int pathComponentCount(char *source);

#ifdef __cplusplus
};
#endif

#endif  /* _PATHFUNC_H_ */

