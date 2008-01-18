/*
**	actions.h
**	nesromtool
**
**	commandline action functions and related
**	
**	spike grobstein
**	spike@sadistech.com
**	1/10/2007
*/

#ifndef _ACTIONS_H_
#define _ACTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

//command parsing functions
void parse_cli_info(char **argv);
void parse_cli_title(char **argv);
void parse_cli_extract(char **argv);
void parse_cli_inject(char **argv);
void parse_cli_patch(char **argv);

#ifdef __cplusplus
};
#endif

#endif /* _ACTIONS_H_ */
