#ifndef _IRCD_DOG3_FDLIST
#define _IRCD_DOG3_FDLIST

/* $Id: fdlist.h,v 1.1.1.1.2.24 2009/04/13 11:03:57 syzop Exp $ */

typedef struct fdstruct {
	int  entry[MAXCONNECTIONS + 2];
	int  last_entry;
} fdlist;

void addto_fdlist(int a, fdlist * b);
void delfrom_fdlist(int a, fdlist * b);
void init_fdlist(fdlist * b);

#ifndef NO_FDLIST
extern MODVAR fdlist oper_fdlist;
#endif


#ifndef TRUE
#define TRUE 1
#endif

#define LOADCFREQ 5
#define LOADRECV 35
#define FDLISTCHKFREQ  2

#endif /*
        * _IRCD_DOG3_FDLIST 
        */
