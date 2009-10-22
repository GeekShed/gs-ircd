/*
 * JumpServer: This module can redirect clients to another server.
 * (C) Copyright 2004, Bram Matthys (Syzop).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: m_jumpserver.c,v 1.6 2004/05/16 20:18:14 syzop Exp $
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

DLLFUNC int m_jumpserver(aClient *cptr, aClient *sptr, int parc, char *parv[]);

#define MSG_JUMPSERVER 	"JUMPSERVER"
#define TOK_JUMPSERVER 	"Js"

ModuleHeader MOD_HEADER(m_jumpserver)
  = {
	"m_jumpserver",
	"v0.0.2",
	"/jumpserver command",
	"3.2-b8-1",
	NULL 
    };

static int js_in_progress = 0;
static char *js_server = NULL;
static char *js_reason = NULL;
static int js_port = 6667;
static char *js_ssl_server = NULL;
static int js_ssl_port = 6697;

DLLFUNC int jumpserver_preconnect(aClient *);

#define ircstrdup(x,y) do { if (x) MyFree(x); if (!y) x = NULL; else x = strdup(y); } while(0)

DLLFUNC int MOD_INIT(m_jumpserver)(ModuleInfo *modinfo)
{
	ModuleSetOptions(modinfo->handle, MOD_OPT_PERM);
	CommandAdd(modinfo->handle, MSG_JUMPSERVER, TOK_JUMPSERVER, m_jumpserver, 3, M_USER);
	HookAddEx(modinfo->handle, HOOKTYPE_PRE_LOCAL_CONNECT, jumpserver_preconnect);
	return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(m_jumpserver)(int module_load)
{
	return MOD_SUCCESS;
}

DLLFUNC int MOD_UNLOAD(m_jumpserver)(int module_unload)
{
	/* Done automatically */
	return MOD_SUCCESS;
}

static int do_jumpserver_exit_client(aClient *sptr)
{
	if (js_server && js_reason)
	{
		if (IsSecure(sptr) && js_ssl_server)
			sendto_one(sptr, rpl_str(RPL_REDIR), me.name,
				BadPtr(sptr->name) ? "*" : sptr->name,
				js_ssl_server, js_ssl_port);
		else
			sendto_one(sptr, rpl_str(RPL_REDIR), me.name,
				BadPtr(sptr->name) ? "*" : sptr->name,
				js_server, js_port);
	}
 	return exit_client(sptr, sptr, sptr, js_reason);
}

static void redirect_all_clients(void)
{
int i, count = 0;
aClient *acptr;

	for (i = LastSlot; i >= 0; i--)
	{
		if ((acptr = local[i]) && IsPerson(acptr) && !IsAnOper(acptr))
		{
			do_jumpserver_exit_client(acptr);
			count++;
		}
	}
	sendto_realops("JUMPSERVER: Redirected %d client%s",
		count, count == 1 ? "" : "s"); /* Language fun... ;p */
}

DLLFUNC int jumpserver_preconnect(aClient *sptr)
{
	if (js_in_progress)
		return do_jumpserver_exit_client(sptr);
	return 0;
}

DLLFUNC int m_jumpserver(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
char *serv, *sslserv=NULL, *reason, *p, *p2;
int all=0, port=6667, sslport=6697;
char logbuf[512];

	if (!IsCoAdmin(sptr) && !IsAdmin(sptr) && !IsNetAdmin(sptr) && !IsSAdmin(sptr))
	{
		sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
		return 0;
	}

	if ((parc < 2) || BadPtr(parv[1]))
	{
		if (js_ssl_server)
			sendnotice(sptr, "JumpServer is \002ENABLED\002 to %s:%d (SSL: %s:%d) with reason '%s'",
				js_server, js_port, js_ssl_server, js_ssl_port, js_reason);
		else if (js_server)
			sendnotice(sptr, "JumpServer is \002ENABLED\002 to %s:%d with reason '%s'",
				js_server, js_port, js_reason);
		else
			sendnotice(sptr, "JumpServer is \002DISABLED\002");
		return 0;
	}

	if ((parc > 1) && (!strcasecmp(parv[1], "OFF") || !strcasecmp(parv[1], "STOP")))
	{
		if (!js_in_progress)
		{
			sendnotice(sptr, "JUMPSERVER: No redirect active");
			return 0;
		}
		js_in_progress = 0;
		snprintf(logbuf, sizeof(logbuf), "%s (%s@%s) turned JUMPSERVER OFF",
			sptr->name, sptr->user->username, sptr->user->realhost);
		sendto_realops("%s", logbuf);
		ircd_log(LOG_ERROR, "%s", logbuf);
		return 0;
	}

	if (parc < 4)
	{
		/* Waah, pretty verbose usage info ;) */
		sendnotice(sptr, "Use: /JUMPSERVER <server>[:port] <NEW|ALL> <reason>");
#ifdef USE_SSL
		sendnotice(sptr, " Or: /JUMPSERVER <server>[:port]/<sslserver>[:port] <NEW|ALL> <reason>");
#endif
		sendnotice(sptr, "if 'NEW' is chosen then only new (incoming) connections will be redirected");
		sendnotice(sptr, "if 'ALL' is chosen then all clients except opers will be redirected immediately (+incoming connections)");
		sendnotice(sptr, "Example: /JUMPSERVER irc2.test.net INCOMING This server will be upgraded, please use irc2.test.net for now");
		sendnotice(sptr, "And then for example 10 minutes later...");
		sendnotice(sptr, "         /JUMPSERVER irc2.test.net ALL This server will be upgraded, please use irc2.test.net for now");
		sendnotice(sptr, "Use: '/JUMPSERVER OFF' to turn off any redirects");
		return 0;
	}

	serv = parv[1];
	
	p = strchr(serv, '/');
	if (p)
	{
		*p = '\0';
		sslserv = p+1;
	}
	
	p = strchr(serv, ':');
	if (p)
	{
		*p++ = '\0';
		port = atoi(p);
		if ((port < 1) || (port > 65535))
		{
			sendnotice(sptr, "Invalid serverport specified (%d)", port);
			return 0;
		}
	}
	if (sslserv)
	{
		p = strchr(sslserv, ':');
		if (p)
		{
			*p++ = '\0';
			sslport = atoi(p);
			if ((sslport < 1) || (sslport > 65535))
			{
				sendnotice(sptr, "Invalid SSL serverport specified (%d)", sslport);
				return 0;
			}
		}
		if (!*sslserv)
			sslserv = NULL;
	}
	if (!strcasecmp(parv[2], "new"))
		all = 0;
	else if (!strcasecmp(parv[2], "all"))
		all = 1;
	else {
		sendnotice(sptr, "ERROR: Invalid action '%s', should be 'NEW' or 'ALL' (see /jumpserver help for usage)", parv[2]);
		return 0;
	}

	reason = parv[3];

	/* Set it */
	ircstrdup(js_server, serv);
	js_port = port;
	if (sslserv)
	{
		ircstrdup(js_ssl_server, sslserv);
		js_ssl_port = sslport;
	} else {
		if (js_ssl_server)
		{
			MyFree(js_ssl_server);
			js_ssl_server = NULL;
		}
	}
	ircstrdup(js_reason, reason);

	/* Broadcast/log */
	if (sslserv)
		snprintf(logbuf, sizeof(logbuf), "%s (%s@%s) added JUMPSERVER redirect for %s to %s:%d [SSL: %s:%d] with reason '%s'",
			sptr->name, sptr->user->username, sptr->user->realhost,
			all ? "ALL CLIENTS" : "all new clients",
			js_server, js_port, js_ssl_server, js_ssl_port, js_reason);
	else
		snprintf(logbuf, sizeof(logbuf), "%s (%s@%s) added JUMPSERVER redirect for %s to %s:%d with reason '%s'",
			sptr->name, sptr->user->username, sptr->user->realhost,
			all ? "ALL CLIENTS" : "all new clients",
			js_server, js_port, js_reason);

	sendto_realops("%s", logbuf);
	ircd_log(LOG_ERROR, "%s", logbuf);

	js_in_progress = 1;

	if (all)
		redirect_all_clients();

	return 0;
}
