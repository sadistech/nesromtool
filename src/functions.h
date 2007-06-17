#include "types.h" /* for u32 */

#ifndef _FUNCTIONS_H_
#define _FUCNTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

void hr_filesize(char *buf, double filesize);

void debug_print_argv(char **argv);

#ifdef __cplusplus
};
#endif

#endif  /* _FUNCTIONS_H_ */