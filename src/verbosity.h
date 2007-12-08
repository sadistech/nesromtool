#ifndef _VERBOSITY_H_
#define _VERBOSITY_H_

#ifdef __cplusplus
extern "C" {
#endif

static int verbosity = 0;

void v_printf(int verbose_level, char *fmt, ...);
int get_verbosity();
int increment_verbosity();

#ifdef __cplusplus
};
#endif

#endif /* _VERBOSITY_H_ */
