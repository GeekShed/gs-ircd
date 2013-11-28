/*
 *   IRC - Internet Relay Chat, src/modules/m_cap.c
 *   (C) 2012 The UnrealIRCd Team
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

typedef int (*bqcmp)(const void *, const void *);

DLLFUNC int m_cap(aClient *cptr, aClient *sptr, int parc, char *parv[]);

#define MSG_CAP 	"CAP"
#define TOK_CAP 	"CA"

ModuleHeader MOD_HEADER(m_cap)
  = {
	"m_cap",	/* Name of module */
	"$Id$", /* Version */
	"command /cap", /* Short description of module */
	"3.2-b8-1",
	NULL 
    };

struct clicap {
	const char *name;
	int cap;
	int flags;
};

#define CLICAP_FLAGS_NONE		0x0
#define CLICAP_FLAGS_STICKY		0x1
#define CLICAP_FLAGS_CLIACK		0x2

static struct clicap clicap_table[] = {
	{"account-notify", PROTO_ACCOUNT_NOTIFY, CLICAP_FLAGS_NONE},
	{"away-notify", PROTO_AWAY_NOTIFY, CLICAP_FLAGS_NONE},
	{"multi-prefix", PROTO_NAMESX, CLICAP_FLAGS_NONE},
	{"sasl", PROTO_SASL, CLICAP_FLAGS_NONE},
#ifdef USE_SSL
	{"tls", PROTO_STARTTLS, CLICAP_FLAGS_NONE},
#endif
	{"userhost-in-names", PROTO_UHNAMES, CLICAP_FLAGS_NONE},
};

#define CLICAP_TABLE_SIZE (sizeof(clicap_table) / sizeof(struct clicap))

static int clicap_compare(const char *name, struct clicap *cap)
{
	return strcmp(name, cap->name);
}

static struct clicap *clicap_find(const char *data, int *negate, int *finished)
{
	static char buf[BUFSIZE];
	static char *p;
	struct clicap *cap;
	char *s;

	*negate = 0;

	if (data)
        {
		strlcpy(buf, data, sizeof(buf));
		p = buf;
	}

	if (*finished)
		return NULL;

	/* skip any whitespace */
	while(*p && isspace(*p))
		p++;

	if (BadPtr(p))
	{
		*finished = 1;
		return NULL;
	}

	if(*p == '-')
	{
		*negate = 1;
		p++;

		/* someone sent a '-' without a parameter.. */
		if(*p == '\0')
			return NULL;
	}

	if((s = strchr(p, ' ')))
		*s++ = '\0';

	if (!strcmp(p, "sasl") && (!SASL_SERVER || !find_server(SASL_SERVER, NULL)))
		return NULL; /* hack: if SASL is disabled or server not online, then pretend it does not exist. -- Syzop */

	if((cap = bsearch(p, clicap_table, CLICAP_TABLE_SIZE,
			  sizeof(struct clicap), (bqcmp) clicap_compare)))
	{
		if(s)
			p = s;
		else
			*finished = 1;
	}

	return cap;
}

static void clicap_generate(aClient *sptr, const char *subcmd, int flags, int clear)
{
	char buf[BUFSIZE];
	char capbuf[BUFSIZE];
	char *p;
	int buflen = 0;
	int curlen, mlen;
	size_t i;

	mlen = snprintf(buf, BUFSIZE, ":%s CAP %s %s", me.name,	BadPtr(sptr->name) ? "*" : sptr->name, subcmd);

	p = capbuf;
	buflen = mlen;

	if (flags == -1)
	{
		sendto_one(sptr, "%s :", buf);
		return;
	}

	for (i = 0; i < CLICAP_TABLE_SIZE; i++)
	{
		if ((clicap_table[i].cap == PROTO_SASL) && (!SASL_SERVER || !find_server(SASL_SERVER, NULL)))
			continue; /* if SASL is disabled or server not online, then pretend it does not exist. -- Syzop */

		if (flags)
		{
			if (!CHECKPROTO(sptr, clicap_table[i].cap))
				continue;
			else if (clear && clicap_table[i].flags & CLICAP_FLAGS_STICKY)
				continue;
		}

		/* \r\n\0, possible "-~=", space, " *" */
		if (buflen + strlen(clicap_table[i].name) >= BUFSIZE - 10)
		{
			if (buflen != mlen)
				*(p - 1) = '\0';
			else
				*p = '\0';

			sendto_one(sptr, "%s * :%s", buf, capbuf);
			p = capbuf;
			buflen = mlen;
		}

		if (clear)
		{
			*p++ = '-';
			buflen++;

			if (clicap_table[i].flags & CLICAP_FLAGS_CLIACK &&
			    CHECKPROTO(sptr, clicap_table[i].cap))
			{
				*p++ = '~';
				buflen++;
			}
		}
		else
		{
			if (clicap_table[i].flags & CLICAP_FLAGS_STICKY)
			{
				*p++ = '=';
				buflen++;
			}

			if (clicap_table[i].flags & CLICAP_FLAGS_CLIACK &&
			    !CHECKPROTO(sptr, clicap_table[i].cap))
			{
				*p++ = '~';
				buflen++;
			}
		}

		curlen = snprintf(p, (capbuf + BUFSIZE) - p, "%s ", clicap_table[i].name);
		p += curlen;
		buflen += curlen;
	}

	if (buflen != mlen)
		*(p - 1) = '\0';
	else
		*p = '\0';

	sendto_one(sptr, "%s :%s", buf, capbuf);
}

static int cap_ack(aClient *sptr, const char *arg)
{
	struct clicap *cap;
	int capadd = 0, capdel = 0;
	int finished = 0, negate;

	if (BadPtr(arg))
		return 0;

	for(cap = clicap_find(arg, &negate, &finished); cap;
	    cap = clicap_find(NULL, &negate, &finished))
	{
		/* sent an ACK for something they havent REQd */
		if(!CHECKPROTO(sptr, cap->cap))
			continue;

		if(negate)
		{
			/* dont let them ack something sticky off */
			if(cap->flags & CLICAP_FLAGS_STICKY)
				continue;

			capdel |= cap->cap;
		}
		else
			capadd |= cap->cap;
	}

	sptr->proto |= capadd;
	sptr->proto &= ~capdel;
        return 0;
}

static int cap_clear(aClient *sptr, const char *arg)
{
        clicap_generate(sptr, "ACK", sptr->proto ? sptr->proto : -1, 1);

     	sptr->proto = 0;
     	return 0;
}

static int cap_end(aClient *sptr, const char *arg)
{
	if (IsRegisteredUser(sptr))
		return 0;

	sptr->proto &= ~PROTO_CLICAP;

	if (sptr->name[0] && sptr->user != NULL)
		return register_user(sptr, sptr, sptr->name, sptr->user->username, NULL, NULL, NULL);
        
        return 0;
}

static int cap_list(aClient *sptr, const char *arg)
{
        clicap_generate(sptr, "LIST", sptr->proto ? sptr->proto : -1, 0);
        return 0;
}

static int cap_ls(aClient *sptr, const char *arg)
{
	if (!IsRegisteredUser(sptr))
		sptr->proto |= PROTO_CLICAP;

       	clicap_generate(sptr, "LS", 0, 0);

       	return 0;
}

static int cap_req(aClient *sptr, const char *arg)
{
	char buf[BUFSIZE];
	char pbuf[2][BUFSIZE];
	struct clicap *cap;
	int buflen, plen;
	int i = 0;
	int capadd = 0, capdel = 0;
	int finished = 0, negate;

	if (!IsRegisteredUser(sptr))
		sptr->proto |= PROTO_CLICAP;

	if (BadPtr(arg))
		return 0;

	buflen = snprintf(buf, sizeof(buf), ":%s CAP %s ACK",
			  me.name, BadPtr(sptr->name) ? "*" : sptr->name);

	pbuf[0][0] = '\0';
	plen = 0;

	for(cap = clicap_find(arg, &negate, &finished); cap;
	    cap = clicap_find(NULL, &negate, &finished))
	{
		/* filled the first array, but cant send it in case the
		 * request fails.  one REQ should never fill more than two
		 * buffers --fl
		 */
		if (buflen + plen + strlen(cap->name) + 6 >= BUFSIZE)
		{
			pbuf[1][0] = '\0';
			plen = 0;
			i = 1;
		}

		if (negate)
		{
			if (cap->flags & CLICAP_FLAGS_STICKY)
			{
				finished = 0;
				break;
			}

			strcat(pbuf[i], "-");
			plen++;

			capdel |= cap->cap;
		}
		else
		{
			if (cap->flags & CLICAP_FLAGS_STICKY)
			{
				strcat(pbuf[i], "=");
				plen++;
			}

			capadd |= cap->cap;
		}

		if (cap->flags & CLICAP_FLAGS_CLIACK)
		{
			strcat(pbuf[i], "~");
			plen++;
		}

		strcat(pbuf[i], cap->name);
		strcat(pbuf[i], " ");
		plen += (strlen(cap->name) + 1);
	}

	if (!finished)
	{
		sendto_one(sptr, ":%s CAP %s NAK :%s", me.name, BadPtr(sptr->name) ? "*" : sptr->name, arg);
		return 0;
	}

	if (i)
	{
		sendto_one(sptr, "%s * :%s", buf, pbuf[0]);
		sendto_one(sptr, "%s :%s", buf, pbuf[1]);
	}
	else
		sendto_one(sptr, "%s :%s", buf, pbuf[0]);

	sptr->proto |= capadd;
	sptr->proto &= ~capdel;
	return 0;
}

struct clicap_cmd {
	const char *cmd;
	int (*func)(struct Client *source_p, const char *arg);
};

static struct clicap_cmd clicap_cmdtable[] = {
	{ "ACK",	cap_ack		},
	{ "CLEAR",	cap_clear	},
	{ "END",	cap_end		},
	{ "LIST",	cap_list	},
	{ "LS",		cap_ls		},
	{ "REQ",	cap_req		},
};

static int clicap_cmd_search(const char *command, struct clicap_cmd *entry)
{
        return strcmp(command, entry->cmd);
}

DLLFUNC int m_cap(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
	struct clicap_cmd *cmd;

	if (DISABLE_CAP)
	{
		/* I know nothing! */
		if (IsPerson(sptr))
			sendto_one(sptr, err_str(ERR_UNKNOWNCOMMAND), me.name, parv[0], "CAP");
		else
			sendto_one(sptr, err_str(ERR_NOTREGISTERED), me.name, "CAP");
		return 0;
	}

	if (parc < 2)
	{
		sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS),
			   me.name, BadPtr(sptr->name) ? "*" : sptr->name,
			   "CAP");

		return 0;
	}

	if(!(cmd = bsearch(parv[1], clicap_cmdtable,
			   sizeof(clicap_cmdtable) / sizeof(struct clicap_cmd),
			   sizeof(struct clicap_cmd), (bqcmp) clicap_cmd_search)))
        {
		sendto_one(sptr, err_str(ERR_INVALIDCAPCMD),
			   me.name, BadPtr(sptr->name) ? "*" : sptr->name,
			   parv[1]);

		return 0;
	}

	return (cmd->func)(sptr, parv[2]);
}

/* This is called on module init, before Server Ready */
DLLFUNC int MOD_INIT(m_cap)(ModuleInfo *modinfo)
{
	MARK_AS_OFFICIAL_MODULE(modinfo);
	CommandAdd(modinfo->handle, MSG_CAP, TOK_CAP, m_cap, MAXPARA, M_UNREGISTERED|M_USER);

	return MOD_SUCCESS;
}

/* Is first run when server is 100% ready */
DLLFUNC int MOD_LOAD(m_cap)(int module_load)
{
	return MOD_SUCCESS;
}


/* Called when module is unloaded */
DLLFUNC int MOD_UNLOAD(m_cap)(int module_unload)
{
	return MOD_SUCCESS;
}
