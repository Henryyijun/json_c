#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>

#include "xtest.h"

int usage()
{
	fprintf(stderr, "xtest(a simple c unit test framework) by wxc@sangfor.com\n"
					"\n"
					"Usage: \n"
					"    1. compile your code to be a shared library(gcc -shared)\n"
					"       use \"xtest --so <yourlib.so>\" to execute unit test.\n"
					"    2. normal program(with <main> entry) should\n"
					"       call \"xtest_start_test\" to execute unit test.\n"
					"\n"
					"[options]\n"
					"    --xml <output file>       export xml output\n"
					"    --sort <file|f|name|n>    method for sorting testcases\n"
					"                              file,f - sort by filenames, line no.\n"
					"                              name,n - sort by testcase names, [default]\n"
					"    --filter <pattern>        filtering testcase names, wildcard matching\n"
					"                              * - match any char, ? - match 1 char\n"
					"    --help                    show this message\n"
					"    --nofork                  run unit test in main process\n"
			);
	return 1;
}

/* test case info struct */
typedef struct xtest_entry_info_st {
	const char 			*name;
	const char 			*file;
	int 				lineno;
	xtest_entry_func_t	init;
	xtest_entry_func_t	entry;
	xtest_entry_func_t	fini;
} xtest_entry_info_st;

/* test case entry set for collecting entries */
typedef struct entry_infos_st {
	xtest_entry_info_st **infos;
	int 				ninfo;
	int					sizeinfo;
} entry_infos_st;


/* test cases info */
static entry_infos_st s_infos;

static xtest_entry_info_st *make_entry(const char *name, 
									   const char *file, int lineno,
									   xtest_entry_func_t init, 
									   xtest_entry_func_t entry, 
									   xtest_entry_func_t fini)
{
	xtest_entry_info_st *ret = malloc(sizeof(xtest_entry_info_st));
	assert(ret);
	
	ret->name 		= strdup(name);
	if (!ret->name) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	ret->file	 	= file;
	ret->lineno		= lineno;
	ret->init		= init;
	ret->entry 		= entry;
	ret->fini 		= fini;
	return ret;
}

void xtest_register(const char *catagory, const char *name, 
					const char *file, int lineno,
					xtest_entry_func_t init, 
					xtest_entry_func_t entry, 
					xtest_entry_func_t fini)
{
	char buf[1024];
	
	if (s_infos.ninfo >= s_infos.sizeinfo) {
		s_infos.sizeinfo = s_infos.sizeinfo ? s_infos.sizeinfo * 2 : 32;
		s_infos.infos = realloc(s_infos.infos, s_infos.sizeinfo * sizeof(xtest_entry_info_st *));
		assert(s_infos.infos);
	}
	
	snprintf(buf, sizeof(buf), "%s.%s", catagory, name);
	s_infos.infos[s_infos.ninfo++] = make_entry(buf, file, lineno, init, entry, fini);
}

/******************************************\
 * 测试案例运行时的API，运行于子进程      *
\******************************************/

typedef struct fail_msg_st {
	char	*file;
	int 	lineno;
	char	*msg;
} fail_msg_st;

static fail_msg_st 	**s_failmsgs;
static int 			s_nfailmsg;
static int			s_sizefailmsg;

static fail_msg_st *make_fail_msg(const char *file, int lineno, const char *msg)
{
	fail_msg_st *ret = malloc(sizeof(fail_msg_st));
	assert(ret);
	
	ret->file = strdup(file);
	ret->lineno = lineno;
	ret->msg = strdup(msg);
	
	assert(ret->file && ret->msg);
	return ret;
}

void xtest_fail_message(const char *file, int lineno, const char *fmt, ...)
{
	char msg[2048];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg) - 1, fmt, ap);
	va_end(ap);
	
	if (s_nfailmsg >= s_sizefailmsg) {
		s_sizefailmsg = s_sizefailmsg ? s_sizefailmsg * 2 : 32;
		s_failmsgs = realloc(s_failmsgs, s_sizefailmsg * sizeof(fail_msg_st *));
		assert(s_failmsgs);
	}
	
	s_failmsgs[s_nfailmsg++] = make_fail_msg(file, lineno, msg);
}


/******************************************\
 * 命令行处理及输出                       *
\******************************************/

#define EXP_MODE_NORMAL		0
#define EXP_MODE_XML		1

#define SORT_MODE_NAME		0
#define SORT_MODE_FILE		1

static char			*s_filter;
static int			s_mode;
static FILE			*s_exportfp;
static int			s_sortmode;
static int			s_fork;

static int parse_cmdline(int argc, char **argv)
{
	int i;
	
	s_exportfp = stdout;
	
	for (i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--xml") == 0) {
			++i;
			s_mode = EXP_MODE_XML;
			if (i < argc) {
				s_exportfp = fopen(argv[i], "w");
				if (!s_exportfp) {
					LOG_RED("Can not open %s for output.\n", argv[i]);
					exit(1);
				}
				setbuf(s_exportfp, NULL);
			}
		} else if (strcmp(argv[i], "--sort") == 0) {
			++i;
			if (i < argc) {
				if (strcmp(argv[i], "file") == 0 || strcmp(argv[i], "f") == 0) {
					s_sortmode = SORT_MODE_FILE;
				} else if (strcmp(argv[i], "name") == 0 || strcmp(argv[i], "n") == 0) {
					s_sortmode = SORT_MODE_NAME;
				} else return usage();
			} else return usage();
		} else if (strcmp(argv[i], "--filter") == 0) {
			++i;
			if (i < argc) {
				s_filter = strdup(argv[i]);
			} else return usage();
		} else if (strcmp(argv[i], "--help") == 0) {
			return usage();
		} else if (strcmp(argv[i], "--fork") == 0) {
			s_fork = 1;
		}
	}
	
	return 0;
}

static void export_header(const entry_infos_st *infos)
{
	if (s_mode == EXP_MODE_XML) {
		fprintf(s_exportfp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(s_exportfp, "<testcases tests=\"%d\">\n", infos->ninfo);
	} else {
		LOG_GREEN(   "[INFO     ]: Total %d case(s) found.\n", infos->ninfo);
	}
}

static void export_tailer()
{
	if (s_mode == EXP_MODE_XML) {
		fprintf(s_exportfp, "</testcases>\n");
	}
}

static void export_case_header(const char *name, 
							   const char *file, int lineno)
{
	if (s_mode == EXP_MODE_XML) {
		fprintf(s_exportfp, "\t<testcase name=\"%s\" file=\"%s\" lineno=\"%d\">\n",
				name, file, lineno);
	} else {
		LOG_GREEN(   "[START    ]: Testing %s in %s:%d\n", name, file, lineno);
	}
}

static void export_case_tailer()
{
	if (s_mode == EXP_MODE_XML) {
		fprintf(s_exportfp, "\t</testcase>\n");
	}
}

static int xml_escape(unsigned char* out, int *outlen, 
					  const unsigned char* in, int *inlen) {
	unsigned char* outstart = out;
	const unsigned char* base = in;
	unsigned char* outend = out + *outlen;
	const unsigned char* inend;

	inend = in + (*inlen);

	while ((in < inend) && (out < outend)) {
		if (*in == '<') {
			if (outend - out < 4) break;
			*out++ = '&';
			*out++ = 'l';
			*out++ = 't';
			*out++ = ';';
		} else if (*in == '>') {
			if (outend - out < 4) break;
			*out++ = '&';
			*out++ = 'g';
			*out++ = 't';
			*out++ = ';';
		} else if (*in == '&') {
			if (outend - out < 5) break;
			*out++ = '&';
			*out++ = 'a';
			*out++ = 'm';
			*out++ = 'p';
			*out++ = ';';
		} else if (*in == '\r') {
			if (outend - out < 5) break;
			*out++ = '&';
			*out++ = '#';
			*out++ = '1';
			*out++ = '3';
			*out++ = ';';
		} else {
			*out++ = (unsigned char) *in;
		}
		++in;
	}
	*outlen = out - outstart;
	*inlen = in - base;
	return 0;
}

static void export_failure(const char *file, int lineno, const char *msg)
{
	if (s_mode == EXP_MODE_XML) {
		unsigned char out[2048];
		int outlen = sizeof(out) - 1;
		int inlen = strlen(msg);
		
		xml_escape(out, &outlen, (const unsigned char *)msg, &inlen);
		out[outlen] = 0;
		
		fprintf(s_exportfp, "\t\t<failure message=\"%s\" file=\"%s\" lineno=\"%d\"/>\n",
				out, file, lineno);
	} else {
		LOG_RED("[     FAIL]: %s (%s:%d)\n", msg, file, lineno);
	}
}

static void export_msg_and_clean()
{
	if (s_exportfp == NULL)
		s_exportfp = stdout;
	
	if (s_nfailmsg) {
		int i;
		
		if (s_mode == EXP_MODE_XML) {
			fprintf(s_exportfp, "\t\t<result>FAILED</result>\n");
		}
		
		for (i = 0; i < s_nfailmsg; ++i) {
			export_failure(s_failmsgs[i]->file, s_failmsgs[i]->lineno, s_failmsgs[i]->msg);
			free(s_failmsgs[i]->file);
			free(s_failmsgs[i]->msg);
			free(s_failmsgs[i]);
		}
		free(s_failmsgs);
		s_nfailmsg = 0;
		s_sizefailmsg = 0;
	} else {
		if (s_mode == EXP_MODE_XML) {
			fprintf(s_exportfp, "\t\t<result>PASS</result>\n");
		} else {
			LOG_GREEN(  "[       OK]: Pass\n");
		}
	}
}

void xtest_exit(int code)
{
	export_msg_and_clean();
	exit(code);
}

/******************************************\
 * 案例查找并执行，主进程                 *
\******************************************/

static int casecmp(const void *a, const void *b)
{
	xtest_entry_info_st *ea = *(xtest_entry_info_st **)a;
	xtest_entry_info_st *eb = *(xtest_entry_info_st **)b;
	
	if (s_sortmode == SORT_MODE_FILE) {
		int r = strcmp(ea->file, eb->file);
		if (r)
			return r;
		
		return ea->lineno - eb->lineno;
	} else {
		return strcmp(ea->name, eb->name);
	}
}

static int wild_match(const char *pat, const char *str)
{
	switch (*pat) {
		case '\0':
			return !(*str);
		case '*':
			return wild_match(pat + 1, str) || ((*str) && wild_match(pat, str + 1));
		case '?':
			return (*str) && wild_match(pat + 1, str + 1);
		default:
			return *pat == *str && wild_match(pat + 1, str + 1);
	}
}

/* parse the argument and execute test case */
static int infoset_execute(entry_infos_st *infos, int argc, char **argv)
{
	int i;
	
	if (parse_cmdline(argc, argv))
		return 1;
	
	qsort(infos->infos, infos->ninfo, sizeof(xtest_entry_info_st *), casecmp);
	
	export_header(infos);
	
	for (i = 0; i < infos->ninfo; ++i) {
		xtest_entry_info_st *ent = infos->infos[i];
		pid_t pid;
		
		if (s_filter) {
			if (!wild_match(s_filter, ent->name))
				continue;
		}
		
		export_case_header(ent->name, ent->file, ent->lineno);
	
		if (!s_fork) {
			if ((void *)ent->init)
				ent->init();
				
			if ((void *)ent->entry)
				ent->entry();
			
			if ((void *)ent->fini)
				ent->fini();
			
			export_msg_and_clean();
		} else {
			if ((pid = fork()) < 0) {
				export_failure(__FILE__, __LINE__, "Can not fork");
				continue;
			}
			
			if (pid == 0) {
				if ((void *)ent->init)
					ent->init();
				
				if ((void *)ent->entry)
					ent->entry();
				
				if ((void *)ent->fini)
					ent->fini();
				
				xtest_exit(0);
			} else {
				int status = 0;
				pid_t wpid;
				int code = 0, sig = 0;
				
				if ((wpid = waitpid(pid, &status, 0)) < 0) {
					LOG_RED( "[     FAIL]: wait fail: %s\n", strerror(errno));
				}
				
				if (status) {
					char msg[1024];
					
					if (WIFEXITED(status))
						code = WEXITSTATUS(status);
					
					if (WIFSIGNALED(status))
						sig = WTERMSIG(status);
					
					snprintf(msg, sizeof(msg), "Exit with code: %d, signal: %d", code, sig);
					export_failure(__FILE__, __LINE__, msg);
				}
			}
		}
		export_case_tailer();
	}
	export_tailer();
	return 0;
}

int xtest_start_test(int argc, char **argv)
{
	return infoset_execute(&s_infos, argc, argv);
}
