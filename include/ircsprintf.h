/*
 * sprintf_irc.h
 *
 * $Id: ircsprintf.h,v 1.1.6.24 2009/04/13 11:03:57 syzop Exp $
 */

#ifndef IRCSPRINTF_H
#define IRCSPRINTF_H
#include <stdarg.h>
#include <stdio.h>


/*
 * Proto types
 */

/* You do want it to work in debug mode yes ? --DrBin */

/* ugly hack GRR */
#if !defined(__GNUC__) && !defined(__common_include__)
#define __attribute__(x) /* nothing */
#endif

extern char *ircvsprintf(char *str, const char *format, va_list);
extern char *ircsprintf(char *str, const char *format, ...) __attribute__((format(printf,2,3)));

extern const char atoi_tab[4000];

#endif
