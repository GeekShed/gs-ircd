/*
 *   IRC - Internet Relay Chat, src/modules/m_setident.c
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
#include "macros.h"
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

#define MSG_SETIDENT 	"SETIDENT"	/* set ident */
#define	TOK_SETIDENT	"AD"	/* good old BASIC ;P */

DLLFUNC int m_setident(aClient *cptr, aClient *sptr, int parc, char *parv[]);

ModuleHeader MOD_HEADER(m_setident)
  = {
	"setident",	/* Name of module */
	"$Id$", /* Version */
	"/setident", /* Short description of module */
	"3.2-b8-1",
	NULL 
    };

DLLFUNC int MOD_INIT(m_setident)(ModuleInfo *modinfo)
{
	/*
	 * We call our add_Command crap here
	*/
	add_Command(MSG_SETIDENT, TOK_SETIDENT, m_setident, MAXPARA);
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(m_setident)(int module_load)
{
	return MOD_SUCCESS;
}

DLLFUNC int MOD_UNLOAD(m_setident)(int module_unload)
{
	if (del_Command(MSG_SETIDENT, TOK_SETIDENT, m_setident) < 0)
	{
		sendto_realops("Failed to delete commands when unloading %s",
				MOD_HEADER(m_setident).name);
	}
	return MOD_SUCCESS;
}

/* m_setident - 12/05/1999 - Stskeeps
 *  :prefix SETIDENT newident
 *  parv[0] - sender
 *  parv[1] - newident
 *  D: This will set your username to be <x> (like (/setident Root))
 *     (if you are IRCop) **efg*
 *     Very experimental currently
 * 	   Cloning of m_sethost at some points - so same authors ;P
*/

DLLFUNC int m_setident(aClient *cptr, aClient *sptr, int parc, char *parv[])
{

	char *vident, *s;
#ifndef DISABLE_USERMOD
	int  permit = 0;	/* 0 = opers(glob/locop) 1 = global oper */
#else
	int  permit = 2;
#endif
	int  legalident = 1;	/* is legal characters? */
	if (!MyConnect(sptr))
		goto permit_2;
	switch (permit)
	{
	  case 0:
		  if (!IsAnOper(sptr))
		  {
			  sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name,
			      parv[0]);
			  return 0;
		  }
		  break;
	  case 1:
		  if (!IsOper(sptr))
		  {
			  sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name,
			      parv[0]);
			  return 0;
		  }
		  break;
	  case 2:
		  if (MyConnect(sptr))
		  {
			  sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name,
			      parv[0]);
			  return 0;
		  }
		  break;
	  default:
		  sendto_ops_butone(IsServer(cptr) ? cptr : NULL, sptr,
		      ":%s WALLOPS :[SETIDENT] Somebody fixing this corrupted server? !(0|1) !!!",
		      me.name);
		  break;
	}
      permit_2:
	if (parc < 2)
		vident = NULL;
	else
		vident = parv[1];

	/* bad bad bad boys .. ;p */
	if (vident == NULL)
	{
		if (MyConnect(sptr))
		{
			sendto_one(sptr,
			    ":%s NOTICE %s :*** Syntax: /SetIdent <new ident>",
			    me.name, parv[0]);
		}
		return 1;
	}
	if (strlen(parv[1]) < 1)
	{
		if (MyConnect(sptr))
			sendto_one(sptr,
			    ":%s NOTICE %s :*** /SetIdent Error: Atleast write SOMETHING that makes sense (':' string)",
			    me.name, sptr->name);
		return 0;
	}

	/* too large huh? */
	if (strlen(vident) > (USERLEN))
	{
		/* ignore us as well if we're not a child of 3k */
		if (MyConnect(sptr))
			sendto_one(sptr,
			    ":%s NOTICE %s :*** /SetIdent Error: Usernames are limited to %i characters.",
			    me.name, sptr->name, USERLEN);
		return 0;
	}

	/* illegal?! */
	for (s = vident; *s; s++)
	{
		if ((*s == '~') && (s == vident))
			continue;
		if (!isallowed(*s))
		{
			legalident = 0;
			break;
		}
	}

	if (legalident == 0)
	{
		sendto_one(sptr,
		    ":%s NOTICE %s :*** /SetIdent Error: A username may contain a-z, A-Z, 0-9, '-', '~' & '.' - Please only use them",
		    me.name, parv[0]);
		return 0;
	}

	{
		DYN_LOCAL(char, did_parts, sptr->user->joined);
		switch (UHOST_ALLOWED)
		{
			case UHALLOW_ALWAYS:
				break;
			case UHALLOW_NEVER:
				if (MyClient(sptr))
				{
					sendto_one(sptr, ":%s NOTICE %s :*** /SetIdent is disabled", me.name, sptr->name);
					DYN_FREE(did_parts);
					return 0;
				}
				break;
			case UHALLOW_NOCHANS:
				if (MyClient(sptr) && sptr->user->joined)
				{
					sendto_one(sptr, ":%s NOTICE %s :*** /SetIdent can not be used while you are on a channel", me.name, sptr->name);
					DYN_FREE(did_parts);
					return 0;
				}
				break;
			case UHALLOW_REJOIN:
				rejoin_doparts(sptr, did_parts);
				break;
		}

		/* get it in */
		ircsprintf(sptr->user->username, "%s", vident);
		/* spread it out */
		sendto_serv_butone_token(cptr, sptr->name,
		    MSG_SETIDENT, TOK_SETIDENT, "%s", parv[1]);

		if (UHOST_ALLOWED == UHALLOW_REJOIN)
			rejoin_dojoinandmode(sptr, did_parts);
		DYN_FREE(did_parts);
	}

	if (MyConnect(sptr))
	{
		sendto_one(sptr,
		    ":%s NOTICE %s :Your nick!user@host-mask is now (%s!%s@%s) - To disable ident set change it manually by /setident'ing again",
		    me.name, parv[0], parv[0], sptr->user->username, GetHost(sptr));
	}
	return 0;
}
