/*
 *   IRC - Internet Relay Chat, nocolorumode.c
 *   (C) 2003 Dominick Meglio
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "config.h"
#include "struct.h"
#include "common.h"
#include "sys.h"
#include "numeric.h"
#include "msg.h"
#include "channel.h"
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#endif
#include <fcntl.h>
#include "h.h"
#include "proto.h"
#ifdef STRIPBADWORDS
#include "badwords.h"
#endif
#ifdef _WIN32
#include "version.h"
#endif

ModuleHeader MOD_HEADER(nocolorumode)
  = {
	"nocolorumode",
	"1.0",
	"color stripping usermode", 
	"3.2-b8-1",
	NULL 
    };

long UMODE_STRIPCOLOR = 0L;

char *h_nocolor_usermsg(aClient *cptr, aClient *sptr, aClient *acptr, char *text, int notice);
char *h_nocodes_chanmsg(aClient *cptr, aClient *sptr, aChannel *acptr, char *text, int notice);

DLLFUNC int MOD_INIT(nocolorumode)(ModuleInfo *modinfo)
{
	UmodeAdd(modinfo->handle, 'c', UMODE_GLOBAL, umode_allow_all, &UMODE_STRIPCOLOR);
	HookAddPCharEx(modinfo->handle, HOOKTYPE_USERMSG, h_nocolor_usermsg);
	return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(nocolorumode)(int module_load)
{
	return MOD_SUCCESS;
}

DLLFUNC int MOD_UNLOAD(nocolorumode)(int module_unload)
{
	return MOD_SUCCESS;
}

char *h_nocolor_usermsg(aClient *cptr, aClient *sptr, aClient *acptr, char *text, int notice)
{
	static char retbuf[4096];
        if (IsULine(sptr) || IsServer(sptr)) {
                return text;
        }
	if (acptr->umodes & UMODE_STRIPCOLOR) {
                strncpyzt(retbuf, StripControlCodes(text), sizeof(retbuf));
                return retbuf;
        }
        else {
	        return text;
        }
}
