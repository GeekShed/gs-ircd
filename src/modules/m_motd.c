/*
 *   IRC - Internet Relay Chat, src/modules/m_motd.c
 *   (C) 2005 The UnrealIRCd Team
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

DLLFUNC CMD_FUNC(m_motd);

#define MSG_MOTD 	"MOTD"	
#define TOK_MOTD 	"F"	

ModuleHeader MOD_HEADER(m_motd)
  = {
	"m_motd",
	"$Id$",
	"command /motd", 
	"3.2-b8-1",
	NULL 
    };

DLLFUNC int MOD_INIT(m_motd)(ModuleInfo *modinfo)
{
	CommandAdd(modinfo->handle, MSG_MOTD, TOK_MOTD, m_motd, MAXPARA, M_USER|M_SERVER);
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(m_motd)(int module_load)
{
	return MOD_SUCCESS;
}

DLLFUNC int MOD_UNLOAD(m_motd)(int module_unload)
{
	return MOD_SUCCESS;
}

/*
 * Heavily modified from the ircu m_motd by codemastr
 * Also svsmotd support added
 */
DLLFUNC CMD_FUNC(m_motd)
{
	ConfigItem_tld *ptr;
	aMotdFile *themotd;
	aMotdLine *motdline;
	int  svsnofile = 0;
	char userhost[HOSTLEN + USERLEN + 6];


	if (IsServer(sptr))
		return 0;
	if (hunt_server_token(cptr, sptr, MSG_MOTD, TOK_MOTD, ":%s", 1, parc, parv) !=
HUNTED_ISME)
		return 0;
#ifndef TLINE_Remote
	if (!MyConnect(sptr))
	{
		themotd = &motd;
		goto playmotd;
	}
#endif
	strlcpy(userhost, make_user_host(cptr->user->username, cptr->user->realhost), sizeof(userhost));
	ptr = Find_tld(sptr, userhost);

	if (ptr)
		themotd = &ptr->motd;
	else
		themotd = &motd;

      playmotd:
	if (themotd == NULL || themotd->lines == NULL)
	{
		sendto_one(sptr, err_str(ERR_NOMOTD), me.name, parv[0]);
		svsnofile = 1;
		goto svsmotd;
	}

	sendto_one(sptr, rpl_str(RPL_MOTDSTART), me.name, parv[0],
		   me.name);

	/* tm_year should be zero only if the struct is zero-ed */
	if (themotd && themotd->lines && themotd->last_modified.tm_year)
	{
		sendto_one(sptr, ":%s %d %s :- %d/%d/%d %d:%02d",
			me.name, RPL_MOTD, parv[0],
			themotd->last_modified.tm_mday,
			themotd->last_modified.tm_mon + 1,
			themotd->last_modified.tm_year + 1900,
			themotd->last_modified.tm_hour,
			themotd->last_modified.tm_min);
	}

	motdline = NULL;
	if(themotd)
		motdline = themotd->lines;
	while (motdline)
	{
		sendto_one(sptr, rpl_str(RPL_MOTD), me.name, parv[0],
		    motdline->line);
		motdline = motdline->next;
	}
      svsmotd:

	motdline = svsmotd.lines;
	while (motdline)
	{
		sendto_one(sptr, rpl_str(RPL_MOTD), me.name, parv[0],
		    motdline->line);
		motdline = motdline->next;
	}
	if (svsnofile == 0)
		sendto_one(sptr, rpl_str(RPL_ENDOFMOTD), me.name, parv[0]);
	return 0;
}
