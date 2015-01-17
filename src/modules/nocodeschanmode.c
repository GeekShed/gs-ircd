/*
 *   IRC - Internet Relay Chat, nocodeschanmode.c
 *   by Dylan Cochran 
 *   based on: 
 *   nocolorumode.c (C) 2003 Dominick Meglio
 *   m_nocodes.c (C) Syzop
 *   noquit.c (C) penna@clanintern-irc.de
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

#define FLAG_NOCODES		'U' 
#define IsNoCodes(x)		((x)->mode.extmode & MODE_NOCODES)
Cmode_t				MODE_NOCODES = 0L;
Cmode				*ModeNoCodes;

static Hook *CheckMsg;

ModuleHeader MOD_HEADER(nocodeschmode)
  = {
	"nocodeschmode",
	"1.4",
	"codes stripping channelmode v1.4", 
	"3.2-b8-1",
	NULL 
    };

long CHMODE_NOCODES = 0L;

char *h_nocodes_chanmsg(aClient *cptr, aClient *sptr, aChannel *acptr, char *text, int notice);

DLLFUNC int MOD_INIT(nocodeschmode)(ModuleInfo *modinfo)
{
	CmodeInfo ModeNC;

#ifndef STATIC_LINKING
	ModuleSetOptions(modinfo->handle, MOD_OPT_PERM);
#endif
	memset(&ModeNoCodes, 0, sizeof ModeNoCodes);
	ModeNC.paracount	= 0;
	ModeNC.is_ok		= extcmode_default_requirechop;
	ModeNC.flag		= FLAG_NOCODES;
	ModeNoCodes		= CmodeAdd(modinfo->handle, ModeNC, &MODE_NOCODES);
#ifndef STATIC_LINKING
        if (ModuleGetError(modinfo->handle) != MODERR_NOERROR || !ModeNoCodes)
        {
            config_error("Error adding channel mode +%c when loading module %s: %s",
                ModeNC.flag,MOD_HEADER(nocodeschmode).name,ModuleGetErrorStr(modinfo->handle));
        }
#else
        if (!ModeNoCodes)
        {
            config_error("Error adding channel mode +%c when loading module %s:",
                ModeNC.flag,MOD_HEADER(nocodeschmode).name);
        }
#endif


        HookAddPCharEx(modinfo->handle, HOOKTYPE_CHANMSG, h_nocodes_chanmsg);
	return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(nocodeschmode)(int module_load)
{
	return MOD_SUCCESS;
}

DLLFUNC int MOD_UNLOAD(nocodeschmode)(int module_unload)
{
	return MOD_SUCCESS;
}

char *h_nocodes_chanmsg(aClient *cptr, aClient *sptr, aChannel *acptr, char *text, int notice)
{
static char retbuf[4096];
        if (IsULine(sptr) || IsServer(sptr)) {
                return text;
        }
        if (IsNoCodes(acptr)) {
		strncpyzt(retbuf, StripControlCodes(text), sizeof(retbuf));
		return retbuf;
        }
        else {
        return text;
        }
}

