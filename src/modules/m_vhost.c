/*
 *   Unreal Internet Relay Chat Daemon, src/modules/m_vhost.c
 *   (C) 2000-2001 Carsten V. Munk and the UnrealIRCd Team
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
#include <sys/timeb.h>
#include <fcntl.h>
#include "h.h"
#include "proto.h"
#ifdef STRIPBADWORDS
#include "badwords.h"
#endif
#ifdef _WIN32
#include "version.h"
#endif

DLLFUNC int m_vhost(aClient *cptr, aClient *sptr, int parc, char *parv[]);

/* Place includes here */
#define MSG_VHOST       "VHOST"
#define TOK_VHOST       "BE"

ModuleHeader MOD_HEADER(m_vhost)
  = {
	"vhost",	/* Name of module */
	"$Id$", /* Version */
	"command /vhost", /* Short description of module */
	"3.2-b8-1",
	NULL 
    };

/* This is called on module init, before Server Ready */
DLLFUNC int MOD_INIT(m_vhost)(ModuleInfo *modinfo)
{
	/*
	 * We call our add_Command crap here
	*/
	add_Command(MSG_VHOST, TOK_VHOST, m_vhost, MAXPARA);
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

/* Is first run when server is 100% ready */
DLLFUNC int MOD_LOAD(m_vhost)(int module_load)
{
	return MOD_SUCCESS;
}


/* Called when module is unloaded */
DLLFUNC int MOD_UNLOAD(m_vhost)(int module_unload)
{
	if (del_Command(MSG_VHOST, TOK_VHOST, m_vhost) < 0)
	{
		sendto_realops("Failed to delete commands when unloading %s",
				MOD_HEADER(m_vhost).name);
	}
	return MOD_SUCCESS;	
}

int  m_vhost(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
	ConfigItem_vhost *vhost;
	ConfigItem_oper_from *from;
	char *user, *pwd, host[NICKLEN+USERLEN+HOSTLEN+6], host2[NICKLEN+USERLEN+HOSTLEN+6];
	int	len, length;
	int 	i;
	if (parc < 3)
	{
		sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS),
		    me.name, parv[0], "VHOST");
		return 0;

	}
	if (!MyClient(sptr))
		return 0;

	user = parv[1];
	pwd = parv[2];
	if (strlen(user) > HOSTLEN)
		*(user + HOSTLEN) = '\0';

	if (!(vhost = Find_vhost(user))) {
		sendto_snomask(SNO_VHOST,
		    "[\2vhost\2] Failed login for vhost %s by %s!%s@%s - incorrect password",
		    user, sptr->name,
		    sptr->user->username,
		    sptr->user->realhost);
		sendto_one(sptr,
		    ":%s NOTICE %s :*** [\2vhost\2] Login for %s failed - password incorrect",
		    me.name, sptr->name, user);
		return 0;
	}
	strlcpy(host, make_user_host(sptr->user->username, sptr->user->realhost), sizeof host);
	strlcpy(host2, make_user_host(sptr->user->username, (char *)Inet_ia2p(&sptr->ip)), sizeof host2);
	for (from = (ConfigItem_oper_from *)vhost->from; from; from = (ConfigItem_oper_from *)from->next) {
		if (!match(from->name, host) || !match(from->name, host2))
			break;
	}
	if (!from) {
		sendto_snomask(SNO_VHOST,
		    "[\2vhost\2] Failed login for vhost %s by %s!%s@%s - host does not match",
		    user, sptr->name, sptr->user->username, sptr->user->realhost);
		sendto_one(sptr,
		    ":%s NOTICE %s :*** No vHost lines available for your host",
		    me.name, sptr->name);
		return 0;
	}
	i = Auth_Check(cptr, vhost->auth, pwd);
	if (i > 0)
	{
		char olduser[USERLEN+1];
		DYN_LOCAL(char, did_parts, sptr->user->joined);
		
		switch (UHOST_ALLOWED)
		{
			case UHALLOW_NEVER:
				if (MyClient(sptr))
				{
					sendto_one(sptr, ":%s NOTICE %s :*** /vhost is disabled", me.name, sptr->name);
					DYN_FREE(did_parts);
					return 0;
				}
				break;
			case UHALLOW_ALWAYS:
				break;
			case UHALLOW_NOCHANS:
				if (MyClient(sptr) && sptr->user->joined)
				{
					sendto_one(sptr, ":%s NOTICE %s :*** /vhost can not be used while you are on a channel", me.name, sptr->name);
					DYN_FREE(did_parts);
					return 0;
				}
				break;
			case UHALLOW_REJOIN:
				rejoin_doparts(sptr, did_parts);
				/* join sent later when the host has been changed */
				break;
		}
		if (sptr->user->virthost)
		{
			MyFree(sptr->user->virthost);
			sptr->user->virthost = NULL;
		}
		len = strlen(vhost->virthost);
		length =  len > HOSTLEN ? HOSTLEN : len;
		sptr->user->virthost = MyMalloc(length + 1);
		strlcpy(sptr->user->virthost, vhost->virthost, length + 1);
		if (vhost->virtuser) {
			strcpy(olduser, sptr->user->username);
			strlcpy(sptr->user->username, vhost->virtuser, USERLEN);
			sendto_serv_butone_token(cptr, sptr->name, MSG_SETIDENT, TOK_SETIDENT,
						 "%s", sptr->user->username);
		}
		sptr->umodes |= UMODE_HIDE;
		sptr->umodes |= UMODE_SETHOST;
		sendto_serv_butone_token(cptr, sptr->name,
			MSG_SETHOST, TOK_SETHOST,
			"%s", sptr->user->virthost);
		sendto_one(sptr, ":%s MODE %s :+tx",
		    sptr->name, sptr->name);
		if (vhost->swhois) {
			if (sptr->user->swhois)
				MyFree(sptr->user->swhois);
			sptr->user->swhois = MyMalloc(strlen(vhost->swhois) +1);
			strcpy(sptr->user->swhois, vhost->swhois);
			sendto_serv_butone_token(cptr, me.name,
				MSG_SWHOIS, TOK_SWHOIS, "%s :%s", sptr->name, vhost->swhois);
		}
		sendto_one(sptr,
		    ":%s NOTICE %s :*** Your vhost is now %s%s%s",
		    me.name, sptr->name, vhost->virtuser ? vhost->virtuser : "", 
			vhost->virtuser ? "@" : "", vhost->virthost);
		sendto_snomask(SNO_VHOST,
		    "[\2vhost\2] %s (%s!%s@%s) is now using vhost %s%s%s",
		    user, sptr->name,
		    vhost->virtuser ? olduser : sptr->user->username,
		    sptr->user->realhost, vhost->virtuser ? vhost->virtuser : "", 
		    	vhost->virtuser ? "@" : "", vhost->virthost);
		if (UHOST_ALLOWED == UHALLOW_REJOIN)
			rejoin_dojoinandmode(sptr, did_parts);
		DYN_FREE(did_parts);
		return 0;
	}
	if (i == -1)
	{
		sendto_snomask(SNO_VHOST,
		    "[\2vhost\2] Failed login for vhost %s by %s!%s@%s - incorrect password",
		    user, sptr->name,
		    sptr->user->username,
		    sptr->user->realhost);
		sendto_one(sptr,
		    ":%s NOTICE %s :*** [\2vhost\2] Login for %s failed - password incorrect",
		    me.name, sptr->name, user);
		return 0;
	}
	/* Belay that order, Lt. (upon -2)*/
	
	return 0;	
}
