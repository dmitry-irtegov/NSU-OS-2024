#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "shell.h"
static char* blankskip(register char*);

int parseline(char* line) {
	printf("start pars\n");
	int nargs, ncmds, rval;
	register char* s;
	char aflg = 0;
	register int i;
	char isdbl = 0, whstr = 0;
	static char delim[] = " \"2\t|&<>;\n";
	static char delimNoDigit[] = " \"\t|&<>;\n";
	unsigned char curConv = 0;

	/*init*/
	nargs = ncmds = rval = 0;
	s = line;
	for (int i = 0; i < MAXCONV; i++) {
		conv[i].cntcommands = 0;
		conv[i].in.file = conv[i].out.file = conv[i].err.file = NULL;
		conv[i].in.flags = conv[i].out.flags = conv[i].err.flags = 0;
		conv[i].flag = 0;
	}

	cmds[0].cmdargs[0] = NULL;
	for (i = 0; i < MAXCMDS; i++) cmds[i].cmdflag = 0;

	while (*s) {
		printf("%s\n", s);
		s = blankskip(s);
		if (!*s) break;

		switch (*s) {
		case '&':
			if (*s && *(s + 1) == '>') {
				isdbl = 1;
			}
			else {
				conv[curConv].flag |= BKGRND;
			}
			*s++ = '\0';
			break;
		case '>':

			if (*(s + 1) == '>') {
				++aflg;
				*s++ = '\0';
			}
			*s++ = '\0';
			s = blankskip(s);
			if (!*s) {
				fprintf(stderr, "syntax error\n");
				return(-1);
			}

			if (isdbl) {
				conv[curConv].out.file = s;
				conv[curConv].err.file = s;
				if (aflg) {
					conv[curConv].out.flags |= 3;
					conv[curConv].err.flags |= 3;
				}
			}
			else {
				if (whstr) {
					conv[curConv].err.file = s;
					if (aflg) {
						conv[curConv].err.flags |= 3;
					}
					else {
						conv[curConv].err.flags |= 1;
					}
				}
				else {
					conv[curConv].out.file = s;
					if (aflg) {
						conv[curConv].out.flags |= 3;
					}
					else {
						conv[curConv].out.flags |= 1;
					}
				}
			}
			s = strpbrk(s, delim);
			if (isspace(*s)) *s++ = '\0';
			isdbl = 0;
			break;

		case '<':
			*s++ = '\0';
			s = blankskip(s);
			if (!*s) {
				fprintf(stderr, "syntax error\n");
				return(-1);
			}

			conv[curConv].in.file = s;
			conv[curConv].in.flags = 1;
			s = strpbrk(s, delim);
			if (isspace(*s)) *s++ = '\0';
			break;

		case '|':
			if (nargs == 0) {
				fprintf(stderr, "syntax error\n");
				return(-1);
			}

			cmds[ncmds++].cmdflag |= OUTPIP;
			cmds[ncmds].cmdflag |= INPIP;
			*s++ = '\0';
			nargs = 0;
			break;

		case ';':
			*s++ = '\0';
			++ncmds;
			curConv++;
			nargs = 0;
			break;

		case '2':
			if (*(s + 1) && *(s + 1) == '>') {
				*s++ = '\0';
				whstr = 1;
			}
			else {
				if (nargs == 0) {
					rval++;
					conv[curConv].cntcommands++;
				}
				cmds[ncmds].cmdargs[nargs++] = s;
				cmds[ncmds].cmdargs[nargs] = NULL;
				s = strpbrk(s, delimNoDigit);
				if (isspace(*s)) *s++ = '\0';
			}
			break;

		case '\"':
			*s++ = '\0';
			cmds[ncmds].cmdargs[nargs] = s;
			s = strpbrk(s, "\"");
			*s++ = '\0';
			s = blankskip(s);
			break;
		default:
			/*a command argument*/
			if (nargs == 0) {
				rval++;
				conv[curConv].cntcommands++;
			}
			cmds[ncmds].cmdargs[nargs++] = s;
			cmds[ncmds].cmdargs[nargs] = NULL;
			s = strpbrk(s, delim);
			if (isspace(*s)) *s++ = '\0';
			break;
		}
	}

	if (cmds[ncmds - 1].cmdflag & OUTPIP) {
		if (nargs == 0) {
			fprintf(stderr, "syntax error\n");
			return(-1);
		}
	}

	return rval;
}

static char* blankskip(register char* s) {
	while (isspace(*s) && *s) ++s;
	return s;
}


void clearcmds() {
	for (int i = 0; i < MAXCMDS && cmds[i].cmdargs[0]; i++) {
		for (int j = 0; j < MAXARGS && cmds[i].cmdargs[j]; j++) {
			cmds[i].cmdargs[j] = NULL;
		}

		cmds[i].cmdflag = 0;
	}
}

void clearconvs() {

	for (int i = 0; i < MAXCONV && conv[i].cntcommands != 0; i++) {
		conv[i].cntcommands = 0;
		conv[i].flag = 0;
		conv[i].in.file = NULL;
		conv[i].out.file = NULL;
		conv[i].err.file = NULL;
		conv[i].in.flags = 0;
		conv[i].err.flags = 0;
		conv[i].out.flags = 0;
	}
}

void clearPars() {
	clearcmds();
	clearconvs();
}