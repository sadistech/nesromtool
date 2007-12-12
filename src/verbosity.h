#ifndef _VERBOSITY_H_
#define _VERBOSITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#define VERBOSE_NOTICE		1	/* standard application verbosity */
#define VERBOSE_DEBUG		2	/* extended application verbosity */
#define VERBOSE_TRACE		3	/* library-level verbosity */
#define VERBOSE_TRACE_1		3	/* same as above */
#define VERBOSE_TRACE_2		4	/* library-level extended verbosity */
#define VERBOSE_TRACE_3		5	/* library-level super-extended verbosity... show EVERYTHING */ 
#define VERBOSE_TRACE_ALL	5	/* same as VERBOSE_TRACE_3 */

static int verbosity = 0;

void v_printf(int verbose_level, char *fmt, ...);
int get_verbosity();
int increment_verbosity();
void set_verbosity(int v);

#ifdef __cplusplus
};
#endif

#endif /* _VERBOSITY_H_ */
