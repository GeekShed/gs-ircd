/* $Id: ares_version.h,v 1.1.4.7 2009/04/13 11:03:58 syzop Exp $ */

#ifndef ARES__VERSION_H
#define ARES__VERSION_H

#define ARES_VERSION_MAJOR 1
#define ARES_VERSION_MINOR 6
#define ARES_VERSION_PATCH 0
#define ARES_VERSION ((ARES_VERSION_MAJOR<<16)|\
                       (ARES_VERSION_MINOR<<8)|\
                       (ARES_VERSION_PATCH))
#define ARES_VERSION_STR "1.6.0"

#ifdef  __cplusplus
extern "C" {
#endif

const char *ares_version(int *version);

#ifdef  __cplusplus
}
#endif

#endif

