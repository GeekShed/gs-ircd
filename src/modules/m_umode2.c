/*
 *   IRC - Internet Relay Chat, src/modules/m_umode2.c
 *   (C) 2004 The UnrealIRCd Team
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers.
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
#include "proto.h"
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
#ifdef STRIPBADWORDS
#include "badwords.h"
#endif
#ifdef _WIN32
#include "version.h"
#endif

DLLFUNC int m_umode2(aClient *cptr, aClient *sptr, int parc, char *parv[]);

#define MSG_UMODE2 	"UMODE2"	
#define TOK_UMODE2 	"|"	

ModuleHeader MOD_HEADER(m_umode2)
  = {
	"m_umode2",
	"$Id$",
	"command /umode2", 
	"3.2-b8-1",
	NULL 
    };

DLLFUNC int MOD_INIT(m_umode2)(ModuleInfo *modinfo)
{
	add_Command(MSG_UMODE2, TOK_UMODE2, m_umode2, MAXPARA);
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(m_umode2)(int module_load)
{
	return MOD_SUCCESS;
}

DLLFUNC int MOD_UNLOAD(m_umode2)(int module_unload)
{
	if (del_Command(MSG_UMODE2, TOK_UMODE2, m_umode2) < 0)
	{
		sendto_realops("Failed to delete commands when unloading %s",
			MOD_HEADER(m_umode2).name);
	}
	return MOD_SUCCESS;
}

/*
    m_umode2 added by Stskeeps
    parv[0] - sender
    parv[1] - modes to change

    Small wrapper to bandwidth save
*/

CMD_FUNC(m_umode2)
{
	char *xparv[5] = {
		parv[0],
		parv[0],
		parv[1],
		(parc > 3) ? parv[3] : NULL,
		NULL
	};

	if (!parv[1])
		return 0;
	return m_umode(cptr, sptr, (parc > 3) ? 4 : 3, xparv);
}
