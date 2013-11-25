/*
 *   IRC - Internet Relay Chat, src/modules/m_sdesc.c
 *   (C) 1999-2001 Carsten Munk (Techie/Stskeeps) <stskeeps@tspre.org>
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

DLLFUNC int m_sdesc(aClient *cptr, aClient *sptr, int parc, char *parv[]);

/* Place includes here */
#define MSG_SDESC 	"SDESC"	/* sdesc */
#define TOK_SDESC 	"AG"	/* 127 4ever !;) */

ModuleHeader MOD_HEADER(m_sdesc)
  = {
	"sdesc",	/* Name of module */
	"$Id$", /* Version */
	"command /sdesc", /* Short description of module */
	"3.2-b8-1",
	NULL 
    };

DLLFUNC int MOD_INIT(m_sdesc)(ModuleInfo *modinfo)
{
	/*
	 * We call our add_Command crap here
	*/
	add_Command(MSG_SDESC, TOK_SDESC, m_sdesc, 1);
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(m_sdesc)(int module_load)
{
	return MOD_SUCCESS;
}

DLLFUNC int MOD_UNLOAD(m_sdesc)(int module_unload)
{
	if (del_Command(MSG_SDESC, TOK_SDESC, m_sdesc) < 0)
	{
		sendto_realops("Failed to delete commands when unloading %s",
				MOD_HEADER(m_sdesc).name);
	}
	return MOD_SUCCESS;
}

/* m_sdesc - 15/05/1999 - Stskeeps
 *  :prefix SDESC
 *  parv[0] - sender
 *  parv[1] - description
 *  D: Sets server info if you are Server Admin (ONLINE)
*/

int m_sdesc(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
	if (!IsAdmin(sptr) && !IsCoAdmin(sptr))
	{
		sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, sptr->name);
		return 0;
	}
	
	if (parc < 2)
	{
		sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS), me.name, sptr->name, "SDESC");
		return 0;
	}

	if (strlen(parv[1]) < 1)
		if (MyConnect(sptr))
		{
			sendto_one(sptr,
			    ":%s NOTICE %s :*** Nothing to change to (SDESC)",
			    me.name, sptr->name);
			return 0;
		}
	if (strlen(parv[1]) > (REALLEN))
	{
		if (MyConnect(sptr))
		{
			sendto_one(sptr,
			    ":%s NOTICE %s :*** /SDESC Error: \"Server info\" may maximum be %i characters of length",
			    me.name, sptr->name, REALLEN);
		}
		return 0;
	}

	ircsprintf(sptr->srvptr->info, "%s", parv[1]);

	sendto_serv_butone_token(cptr, sptr->name, MSG_SDESC, TOK_SDESC, ":%s",
	    parv[1]);

	if (MyConnect(sptr))
		sendto_one(sptr,
			":%s NOTICE %s :Your \"server description\" is now set to be %s - you have to set it manually to undo it",
			me.name, parv[0], parv[1]);

	sendto_ops("Server description for %s is now '%s' changed by %s",
		sptr->srvptr->name, sptr->srvptr->info, parv[0]);
	return 0;
}
