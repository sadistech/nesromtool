#ifndef _NESROMTOOL_H_
#define _NESROMTOOL_H_

//check if we have autoheader config.h, and include if necessary
#ifdef HAVE_CONFIG
#include <config.h>
#else
#define VERSION "1.0b"
#define PACKAGE "nesromtool"
#endif

//globals
//********************
extern char *program_name;

#endif /* _NESROMTOOL_H_ */
