/*
 *   IRC - Internet Relay Chat, src/modules/m_protoctl.c
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

DLLFUNC int m_protoctl(aClient *cptr, aClient *sptr, int parc, char *parv[]);

#define MSG_PROTOCTL 	"PROTOCTL"	
#define TOK_PROTOCTL 	"_"	

ModuleHeader MOD_HEADER(m_protoctl)
  = {
	"m_protoctl",
	"$Id: m_protoctl.c,v 1.1.6.10 2009/04/13 11:04:37 syzop Exp $",
	"command /protoctl", 
	"3.2-b8-1",
	NULL 
    };

DLLFUNC int MOD_INIT(m_protoctl)(ModuleInfo *modinfo)
{
	add_CommandX(MSG_PROTOCTL, TOK_PROTOCTL, m_protoctl, MAXPARA, M_UNREGISTERED|M_SERVER|M_USER);
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(m_protoctl)(int module_load)
{
	return MOD_SUCCESS;
}

DLLFUNC int MOD_UNLOAD(m_protoctl)(int module_unload)
{
	if (del_Command(MSG_PROTOCTL, TOK_PROTOCTL, m_protoctl) < 0)
	{
		sendto_realops("Failed to delete commands when unloading %s",
			MOD_HEADER(m_protoctl).name);
	}
	return MOD_SUCCESS;
}

/*
 * m_protoctl
 *	parv[0] = Sender prefix
 *	parv[1+] = Options
 */
CMD_FUNC(m_protoctl)
{
	int  i;
#ifndef PROTOCTL_MADNESS
	int  remove = 0;
#endif
	char proto[128], *s;
/*	static char *dummyblank = "";	Yes, it is kind of ugly */

#ifdef PROTOCTL_MADNESS
	if (GotProtoctl(sptr))
	{
		/*
		 * But we already GOT a protoctl msg!
		 */
		if (!IsServer(sptr))
			sendto_one(cptr,
			    "ERROR :Already got a PROTOCTL from you.");
		return 0;
	}
#endif
	cptr->flags |= FLAGS_PROTOCTL;
	/* parv[parc - 1] */
	for (i = 1; i < parc; i++)
	{
		strncpyzt(proto, parv[i], sizeof proto);
		s = proto;
#ifndef PROTOCTL_MADNESS
		if (*s == '-')
		{
			s++;
			remove = 1;
		}
		else
			remove = 0;
#endif
/*		equal = (char *)index(proto, '=');
		if (equal == NULL)
			options = dummyblank;
		else
		{
			options = &equal[1];
			equal[0] = '\0';
		}
*/
		if (!strcmp(s, "NAMESX"))
		{
			Debug((DEBUG_ERROR, "Chose protocol %s for link %s", proto, cptr->name));
			SetNAMESX(cptr);
		}
		if (!strcmp(s, "UHNAMES"))
		{
			Debug((DEBUG_ERROR, "Chose protocol %s for link %s", proto, cptr->name));
			SetUHNAMES(cptr);
		}
		else if (strcmp(s, "NOQUIT") == 0)
		{
#ifndef PROTOCTL_MADNESS
			if (remove)
			{
				ClearNoQuit(cptr);
				continue;
			}
#endif
			Debug((DEBUG_ERROR, "Chose protocol %s for link %s",
			    proto, cptr->name));
			SetNoQuit(cptr);

		}
		else if (strcmp(s, "TOKEN") == 0)
		{
#ifndef PROTOCTL_MADNESS
			if (remove)
			{
				ClearToken(cptr);
				continue;
			}
#endif
			Debug((DEBUG_ERROR, "Chose protocol %s for link %s",
			    proto, cptr->name));
			SetToken(cptr);
		}
		else if (strcmp(s, "HCN") == 0)
		{
#ifndef PROTOCTL_MADNESS
			if (remove)
			{
				ClearHybNotice(cptr);
				continue;
			}
#endif
			Debug((DEBUG_ERROR, "Chose protocol %s for link %s",
			    proto, cptr->name));
			SetHybNotice(cptr);
		}
		else if (strcmp(s, "SJOIN") == 0)
		{
#ifndef PROTOCTL_MADNESS
			if (remove)
			{
				ClearSJOIN(cptr);
				continue;
			}
#endif
			Debug((DEBUG_ERROR, "Chose protocol %s for link %s",
			    proto, cptr->name));
			SetSJOIN(cptr);
		}
		else if (strcmp(s, "SJOIN2") == 0)
		{
#ifndef PROTOCTL_MADNESS
			if (remove)
			{
				ClearSJOIN2(cptr);
				continue;
			}
#endif
			Debug((DEBUG_ERROR, "Chose protocol %s for link %s",
			    proto, cptr->name));
			SetSJOIN2(cptr);
		}
		else if (strcmp(s, "NICKv2") == 0)
		{
#ifndef PROTOCTL_MADNESS
			if (remove)
			{
				ClearNICKv2(cptr);
				continue;
			}
#endif
			Debug((DEBUG_ERROR, "Chose protocol %s for link %s",
			    proto, cptr->name));
			SetNICKv2(cptr);
		}
		else if (strcmp(s, "UMODE2") == 0)
		{
#ifndef PROTOCTL_MADNESS
			if (remove)
			{
				ClearUMODE2(cptr);
				continue;
			}
#endif
			Debug((DEBUG_ERROR,
			    "Chose protocol %s for link %s",
			    proto, cptr->name));
			SetUMODE2(cptr);
		}
		else if (strcmp(s, "NS") == 0)
		{
#ifdef PROTOCTL_MADNESS
			if (remove)
			{
				ClearNS(cptr);
				continue;
			}
#endif
			Debug((DEBUG_ERROR,
			    "Chose protocol %s for link %s",
			    proto, cptr->name));
			SetNS(cptr);
		}
		else if (strcmp(s, "VL") == 0)
		{
#ifndef PROTOCTL_MADNESS
			if (remove)
			{
				ClearVL(cptr);
				continue;
			}
#endif
			Debug((DEBUG_ERROR,
			    "Chose protocol %s for link %s",
			    proto, cptr->name));
			SetVL(cptr);
		}
		else if (strcmp(s, "VHP") == 0)
		{
#ifndef PROTOCTL_MADNESS
			if (remove)
			{
				ClearVHP(cptr);
				continue;
			}
#endif
			Debug((DEBUG_ERROR,
			    "Chose protocol %s for link %s",
			    proto, cptr->name));
			SetVHP(cptr);
		}
		else if (strcmp(s, "CLK") == 0)
		{
			Debug((DEBUG_ERROR,
			    "Chose protocol %s for link %s",
			    proto, cptr->name));
			SetCLK(cptr);
		}
		else if (strcmp(s, "SJ3") == 0)
		{
#ifndef PROTOCTL_MADNESS
			if (remove)
			{
				ClearSJ3(cptr);
				continue;
			}
#endif
			Debug((DEBUG_ERROR,
			    "Chose protocol %s for link %s",
			    proto, cptr->name));
			SetSJ3(cptr);
		}
		else if (strcmp(s, "SJB64") == 0)
		{
#ifndef PROTOCTL_MADNESS
			if (remove)
			{
				cptr->proto &= ~PROTO_SJB64;
				continue;
			}
#endif
			Debug((DEBUG_ERROR,
			    "Chose protocol %s for link %s",
			    proto, cptr->name));
			cptr->proto |= PROTO_SJB64;
		}
		else if (strcmp(s, "ZIP") == 0)
		{
			if (remove)
			{
				cptr->proto &= ~PROTO_ZIP;
				continue;
			}
			Debug((DEBUG_ERROR,
				"Chose protocol %s for link %s",
				proto, cptr->name));
			cptr->proto |= PROTO_ZIP;
		}
		else if (strcmp(s, "TKLEXT") == 0)
		{
			Debug((DEBUG_ERROR, "Chose protocol %s for link %s", proto, cptr->name));
			SetTKLEXT(cptr);
		}
		else if (strcmp(s, "NICKIP") == 0)
		{
			Debug((DEBUG_ERROR, "Chose protocol %s for link %s", proto, cptr->name));
			cptr->proto |= PROTO_NICKIP;
		}
		else if (strncmp(s, "NICKCHARS=", 10) == 0)
		{
			/* Compare... */
			if (strcmp(s+10, langsinuse))
			{
				sendto_realops("\002WARNING!!!!\002 Link %s does not have the same set::allowed-nickchars settings (or is "
							"a different UnrealIRCd version), this MAY cause display issues. Our charset: '%s', theirs: '%s'",
					get_client_name(cptr, FALSE), langsinuse, s+10);
				/* return exit_client(cptr, cptr, &me, "Nick charset mismatch"); */
			}
		}
		/*
		 * Add other protocol extensions here, with proto
		 * containing the base option, and options containing
		 * what it equals, if anything.
		 *
		 * DO NOT error or warn on unknown proto; we just don't
		 * support it.
		 */
	}

	return 0;
}
