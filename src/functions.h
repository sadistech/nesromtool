#include "types.h" /* for u32 */

#ifndef _FUNCTIONS_H_
#define _FUCNTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

void hr_filesize(char *buf, double filesize);

void debug_print_argv(char **argv);

bool check_is_range(char *val);
int str_to_range(Range *r, char *val);

bool write_data_to_file(char *data, u32 length, char *path);
bool append_data_to_file(char *data, u32 length, char *path);

#ifdef __cplusplus
};
#endif

#endif  /* _FUNCTIONS_H_ */
