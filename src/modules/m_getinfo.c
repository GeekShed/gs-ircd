/*
 * =================================================================
 * Filename:          m_getinfo.c
 * Description:       Command /getinfo
 * Author:            AngryWolf <angrywolf@flashmail.com>
 * Documentation:     m_getinfo.txt (comes with the package)
 * =================================================================
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
#ifdef STRIPBADWORDS
#include "badwords.h"
#endif
#ifdef _WIN32
#include "version.h"
#endif

extern void             sendto_one(aClient *to, char *pattern, ...);
extern char		*get_mode_str(aClient *acptr);

#define MSG_GETINFO 	"GETINFO"
#define TOK_GETINFO 	"GI"
#define DelCommand(x)	if (x) CommandDel(x); x = NULL
#define MaxSize(x)	(sizeof(x) - strlen(x) - 1)

static Command		*AddCommand(Module *module, char *msg, char *token, iFP func);
static int		m_getinfo(aClient *cptr, aClient *sptr, int parc, char *parv[]);

static Command		*CmdGetinfo;
static char		mybuf[BUFSIZE+1];

#ifndef HOOKTYPE_REHASH_COMPLETE /* 3.2-RC2 */
static char		modebuf[MAXMODEPARAMS*2+1], parabuf[504];
#endif

ModuleHeader MOD_HEADER(m_getinfo)
  = {
	"getinfo",
	"$Id: m_getinfo.c,v 3.5 2004/07/12 18:11:38 angrywolf Exp $",
	"command /getinfo",
	"3.2-b8-1",
	NULL 
    };

DLLFUNC int MOD_INIT(m_getinfo)(ModuleInfo *modinfo)
{
	CmdGetinfo = AddCommand(modinfo->handle, MSG_GETINFO, TOK_GETINFO, m_getinfo);

	if (!CmdGetinfo)
		return MOD_FAILED;

	return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(m_getinfo)(int module_load)
{
	return MOD_SUCCESS;
}

DLLFUNC int MOD_UNLOAD(m_getinfo)(int module_unload)
{
	DelCommand(CmdGetinfo);
	return MOD_SUCCESS;
}

// =================================================================
// Structure type definitions
// =================================================================

typedef struct {
	short	number;
	char	*name;
} ShortNumStruct;

typedef struct {
	long	number;
	char	*name;
} LongNumStruct;

typedef struct {
	unsigned long	sendK;
	unsigned long	recvK;
	unsigned short	sendB;
	unsigned short	recvB;
} MessageStats;

// =================================================================
// Client status table
// =================================================================

static ShortNumStruct _ClientStatusTable[] =
{
    { -7,	"LOG"			},
    { -6,	"CONNECTING"		},
    { -5,	"SSL_CONNECT_HANDSHAKE"	},
    { -4,	"SSL_ACCEPT_HANDSHAKE"	},
    { -3,	"HANDSHAKE"		},
    { -2,	"ME"			},
    { -1,	"UNKNOWN"		},
    { 0,	"SERVER"		},
    { 1,	"CLIENT"		},
};

#define CS_TABLE_SIZE sizeof(_ClientFlagsTable)/sizeof(_ClientFlagsTable[0])-1

// =================================================================
// List of supported protos
// =================================================================

static ShortNumStruct _ProtoctlTable[] =
{
    { PROTO_NOQUIT,	"NOQUIT"	},
    { PROTO_TOKEN,	"TOKEN"		},
    { PROTO_SJOIN,	"SJOIN"		},
    { PROTO_NICKv2,	"NICKv2"	},
    { PROTO_SJOIN2,	"SJOIN2"	},
    { PROTO_UMODE2,	"UMODE2"	},
    { PROTO_NS,		"NS"		},
    { PROTO_ZIP,	"ZIP"		},
    { PROTO_VL,		"VL"		},
    { PROTO_SJ3,	"SJ3"		},
    { PROTO_VHP,	"VHP"		},
    { PROTO_SJB64,	"SJB64"		},
#ifdef PROTO_TKLEXT /* 3.2-RC2 */
    { PROTO_TKLEXT,	"TKLEXT"	},
#endif
#ifdef PROTO_NICKIP /* 3.2.1 */
    { PROTO_NICKIP,	"NICKIP"	},
#endif
};

#define PROTOCTL_TABLE_SIZE sizeof(_ProtoctlTable)/sizeof(_ProtoctlTable[0])-1

// =================================================================
// List of flags (useful for debugging)
// =================================================================

static LongNumStruct _ClientFlagsTable[] =
{
    { FLAGS_PINGSENT,	"PINGSENT"	},
    { FLAGS_DEADSOCKET,	"DEADSOCKET"	},
    { FLAGS_KILLED,	"KILLED"	},
    { FLAGS_BLOCKED,	"BLOCKED"	},
#ifdef FLAGS_OUTGOING /* 3.2.1 */
    { FLAGS_OUTGOING,	"OUTGOING"	},
#endif
    { FLAGS_CLOSING,	"CLOSING"	},
    { FLAGS_LISTEN,	"LISTEN"	},
    { FLAGS_CHKACCESS,	"CHKACCESS"	},
    { FLAGS_DOINGDNS,	"DOINGDNS"	},
    { FLAGS_AUTH,	"AUTH"		},
    { FLAGS_WRAUTH,	"WRAUTH"	},
    { FLAGS_LOCAL,	"LOCAL"		},
    { FLAGS_DOID,	"DOID"		},
    { FLAGS_GOTID,	"GOTID"		},
    { FLAGS_NONL,	"NONL"		},
    { FLAGS_ULINE,	"ULINE"		},
    { FLAGS_SQUIT,	"SQUIT"		},
    { FLAGS_PROTOCTL,	"PROTOCTL"	},
    { FLAGS_PING,	"PING"		},
    { FLAGS_ASKEDPING,	"ASKEDPING"	},
    { FLAGS_NETINFO,	"NETINFO"	},
    { FLAGS_HYBNOTICE,	"HYBNOTICE"	},
    { FLAGS_QUARANTINE,	"QUARANTINE"	},
#ifdef ZIP_LINKS
    { FLAGS_ZIP,	"ZIP"		},
#endif
#ifdef FLAGS_DCCNOTICE /* 3.2.1 */
    { FLAGS_DCCNOTICE,	"DCCNOTICE"	},
#endif
    { FLAGS_SHUNNED,	"SHUNNED"	},
#ifdef FLAGS_VIRUS /* 3.2-RC2 */
    { FLAGS_VIRUS,	"VIRUS"		},
#endif
#ifdef USE_SSL
    { FLAGS_SSL,	"SSL"		},
#endif
    { FLAGS_DCCBLOCK,	"DCCBLOCK"	},
    { FLAGS_MAP,	"MAP"		},
};

#define FLAGS_TABLE_SIZE sizeof(_ClientStatusTable)/sizeof(_ClientStatusTable[0])-1

// =================================================================
// find_client_status: Converts from status number to name
// =================================================================

ShortNumStruct *find_client_status(int sn)
{
	u_int	i;

	for (i = 0; i <= FLAGS_TABLE_SIZE; i++)
        	if (sn == _ClientStatusTable[i].number)
            		return &_ClientStatusTable[i];

        return NULL;
}

// =================================================================
// get_proto_names: Sends back the protos supported by the client
// =================================================================

char *get_proto_names(short proto)
{
	u_int	i, found = 0;

	memset(&mybuf, 0, sizeof mybuf);

	for (i = 0; i <= PROTOCTL_TABLE_SIZE; i++)
        	if (proto & _ProtoctlTable[i].number)
		{
			if (found)
				strncat(mybuf, ", ", MaxSize(mybuf));
			else
				found = 1;

			strncat(mybuf, _ProtoctlTable[i].name,
				MaxSize(mybuf));
		}

	if (!strlen(mybuf))
		strcpy(mybuf, "<None>");

	return mybuf;
}

// =================================================================
// get_flag_names: Sends back flagnames
// =================================================================

char *get_flag_names(long flags)
{
	u_int	i, found = 0;

	memset(&mybuf, 0, sizeof mybuf);

	for (i = 0; i <= CS_TABLE_SIZE; i++)
        	if (flags & _ClientFlagsTable[i].number)
		{
			if (found)
				strncat(mybuf, ", ", MaxSize(mybuf));
			else
				found = 1;

			strncat(mybuf, _ClientFlagsTable[i].name,
				MaxSize(mybuf));
		}

	if (!strlen(mybuf))
		strcpy(mybuf, "<None>");

	return mybuf;
}

// =================================================================
// TS to full date conversion
// =================================================================

/*
 * strftime on Windows doesn't support format parameters %F and %T
 * (and some other parameters as well). More details at
 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/
 *        vclib/html/_crt_strftime.2c_.wcsftime.asp
 */

static char *FullDate(TS time_in)
{
        struct tm *tp = localtime(&time_in);

        if (!tp)
                return NULL;

	memset(&mybuf, 0, sizeof mybuf);
        strftime(mybuf, sizeof mybuf, "%Y-%m-%d %H:%M:%S", tp);

        return mybuf;
}

// =================================================================
// messagestats: Convert statistics sent to/received from aClient
//     to a readable one
// =================================================================

void messagestats(aClient *cptr, MessageStats *ms)
{
	ms->sendB	= cptr->sendB;
	ms->recvB	= cptr->receiveB;
	ms->sendK	= cptr->sendK;
	ms->recvK	= cptr->receiveK;

	if (ms->sendB > 1023)
	{
		ms->sendK += (ms->sendB >> 10);
		ms->sendB &= 0x3ff;
	}
	if (ms->recvB > 1023)
	{
		ms->recvK += (ms->recvB >> 10);
		ms->recvB &= 0x3ff;
	}
}

// =================================================================
// cFlagTab[] (referring to src/channel.c)
// =================================================================

#ifndef HOOKTYPE_REHASH_COMPLETE /* 3.2-RC2 */
typedef struct {
        long mode;
        char flag;
        unsigned  halfop : 1;           /* 1 = yes 0 = no */
        unsigned  parameters : 1;
} aCtab;

extern aCtab cFlagTab[];
#endif

// =================================================================
// I needed a FULL channel mode string,
// so hacked up channel_modes() from src/channel.c
// =================================================================

void full_channel_modes(char *mbuf, char *pbuf, aChannel *chptr)
{
	aCtab	*tab = &cFlagTab[0];
	char	bcbuf[1024];
#ifdef EXTCMODE
	int	i;
#endif

	*mbuf++ = '+';
	/* Paramless first */
	while (tab->mode != 0x0)
	{
		if ((chptr->mode.mode & tab->mode))
			if (!tab->parameters)
				*mbuf++ = tab->flag;
		tab++;
	}
#ifdef EXTCMODE
	for (i=0; i <= Channelmode_highest; i++)
	{
		if (Channelmode_Table[i].flag && !Channelmode_Table[i].paracount &&
		    (chptr->mode.extmode & Channelmode_Table[i].mode))
			*mbuf++ = Channelmode_Table[i].flag;
	}
#endif
	if (chptr->mode.limit)
	{
		*mbuf++ = 'l';
		(void)ircsprintf(pbuf, "%d ", chptr->mode.limit);
	}
	if (*chptr->mode.key)
	{
		*mbuf++ = 'k';
		/* FIXME: hope pbuf is long enough */
		(void)snprintf(bcbuf, sizeof bcbuf, "%s ", chptr->mode.key);
		(void)strcat(pbuf, bcbuf);
	}
	if (*chptr->mode.link)
	{
		*mbuf++ = 'L';
		/* FIXME: is pbuf long enough?  */
		(void)snprintf(bcbuf, sizeof bcbuf, "%s ", chptr->mode.link);
		(void)strcat(pbuf, bcbuf);
	}
	/* if we add more parameter modes, add a space to the strings here --Stskeeps */
#ifdef NEWCHFLOODPROT
	if (chptr->mode.floodprot)
#else
	if (chptr->mode.per)
#endif
	{
		*mbuf++ = 'f';
#ifdef NEWCHFLOODPROT
		ircsprintf(bcbuf, "%s ", channel_modef_string(chptr->mode.floodprot));
#else
		if (chptr->mode.kmode == 1)
			ircsprintf(bcbuf, "*%i:%i ", chptr->mode.msgs, chptr->mode.per);
		else
			ircsprintf(bcbuf, "%i:%i ", chptr->mode.msgs, chptr->mode.per);
#endif
		(void)strcat(pbuf, bcbuf);
	}

#ifdef EXTCMODE
	for (i=0; i <= Channelmode_highest; i++)
	{
		if (Channelmode_Table[i].flag && Channelmode_Table[i].paracount &&
		    (chptr->mode.extmode & Channelmode_Table[i].mode))
		{
			*mbuf++ = Channelmode_Table[i].flag;
			strcat(pbuf, Channelmode_Table[i].get_param(extcmode_get_struct(chptr->mode.extmodeparam, Channelmode_Table[i].flag)));
			strcat(pbuf, " ");
		}
	}
#endif

	/* Remove the trailing space from the parameters -- codemastr */
	if (*pbuf)
		pbuf[strlen(pbuf)-1]=0;

	*mbuf++ = 0;
	return;
}

static Command *AddCommand(Module *module, char *msg, char *token, iFP func)
{
	Command *cmd;

	if (CommandExists(msg))
    	{
		config_error("Command %s already exists", msg);
		return NULL;
    	}
    	if (CommandExists(token))
	{
		config_error("Token %s already exists", token);
		return NULL;
    	}

	cmd = CommandAdd(module, msg, token, func, MAXPARA, 0);

#ifndef STATIC_LINKING
	if (ModuleGetError(module) != MODERR_NOERROR || !cmd)
#else
	if (!cmd)
#endif
	{
#ifndef STATIC_LINKING
		config_error("Error adding command %s: %s", msg,
			ModuleGetErrorStr(module));
#else
		config_error("Error adding command %s", msg);
#endif
		return NULL;
	}

	return cmd;
}

/*
** m_getinfo
**      parv[0] = sender prefix
**      parv[1] = nick/server/channel
*/

static int m_getinfo(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
	enum		ClientType { CT_User, CT_Server, CT_Channel, CT_None };
	int		ct = CT_None;
	aClient		*acptr = NULL;
	aChannel	*chptr = NULL;
	MessageStats	ms;

        if (!IsPerson(sptr) || !IsAnOper(sptr))
	{
        	sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
		return -1;
	}

        if (parc < 2)
	{
	        sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS),
			me.name, parv[0], "GETINFO");
		return -1;
	}

        if (parc > 2)
	{
		if (hunt_server_token(cptr, sptr, MSG_GETINFO, TOK_GETINFO,
		    "%s :%s", 1, parc, parv) != HUNTED_ISME)
			return 0;
		parv[1] = parv[2];
	}

	*modebuf = 0;
	*parabuf = 0;

	/* Find out what we are going to get info from */

        if (*parv[1] == '#')
	{
		if ((chptr = find_channel(parv[1], NullChn)) != NullChn)
			ct = CT_Channel;
	}
	else if ((acptr = find_server_quick(parv[1])))
		ct = CT_Server;
        else if ((acptr = find_person(parv[1], NULL)))
		ct = CT_User;

	/* We can't get info about nothing */

	if (ct == CT_None)
	{
    	    if (!IsServer(sptr))
	        sendto_one(sptr, err_str(ERR_NOSUCHNICK),
		    me.name, sptr->name, parv[1]);
	        return -1;
	}

	/* Informing eyes users */

	if (!(sptr->umodes & UMODE_BOT)) {
		if (ct == CT_Channel || strcasecmp(sptr->name, acptr->name))
			sendto_snomask(SNO_EYES, "*** %s (%s@%s) did a /getinfo on %s",
				sptr->name, sptr->user->username, GetHost(sptr),
				( ct == CT_Channel ? chptr->chname : acptr->name ));
	}

	sendto_one(sptr, ":%s 339 %s :===================================",
		me.name, sptr->name);

	/* FOR CHANNELS */

	if (ct == CT_Channel)
	{
		full_channel_modes(modebuf, parabuf, chptr);

		sendto_one(sptr, ":%s 339 %s :Info on: %s",
			me.name, sptr->name, chptr->chname);
		sendto_one(sptr, ":%s 339 %s :Modes: %s %s",
			me.name, sptr->name, modebuf, parabuf);
		sendto_one(sptr, ":%s 339 %s :Users: %d",
			me.name, sptr->name, chptr->users);
		sendto_one(sptr, ":%s 339 %s :Creation time: %s",
			me.name, sptr->name, FullDate(chptr->creationtime));

		if (chptr->topic)
		{
			sendto_one(sptr, ":%s 339 %s :Topic: %s",
				me.name, sptr->name, chptr->topic);
			sendto_one(sptr, ":%s 339 %s :Topic set by: %s",
				me.name, sptr->name, chptr->topic_nick);
			sendto_one(sptr, ":%s 339 %s :Topic set on: %s",
				me.name, sptr->name, FullDate(chptr->topic_time));
		}

	}

	/* FOR BOTH USERS AND SERVERS */

	if (ct == CT_User || ct == CT_Server)
	{
		ShortNumStruct *status = find_client_status(acptr->status);

		sendto_one(sptr, ":%s 339 %s :Info on: %s (%s)",
			me.name, sptr->name, acptr->name, acptr->info);
		sendto_one(sptr, ":%s 339 %s :Status: %s",
			me.name, sptr->name,
			( status ? status->name : "<None>" ));

		/*
		 * Sends general connection information
		 * It's a bit complex...
		 */

		if (!IsMe(acptr))
		{
#ifdef GetIP /* 3.2.1 */
			char *ipaddr = GetIP(acptr);
#endif

			if (MyConnect(acptr))
			{
#ifdef USE_SSL
				if ((acptr->flags & FLAGS_SSL) && acptr->ssl)
					sendto_one(sptr, ":%s 339 %s :Connected to %s on port %d [%s] with %s",
						me.name, sptr->name,
						( IsClient(acptr) ? acptr->user->server : acptr->serv->up ),
						acptr->listener->port,
						( acptr->class ? acptr->class->name : "<no class>" ),
						ssl_get_cipher(acptr->ssl));
				else
#endif
					sendto_one(sptr, ":%s 339 %s :Connected to %s on port %d [%s]",
						me.name, sptr->name,
						( IsClient(acptr) ? acptr->user->server : acptr->serv->up ),
						acptr->listener->port,
						( acptr->class ? acptr->class->name : "<no class>" ));
			}
			else
				sendto_one(sptr, ":%s 339 %s :Connected to %s",
					me.name, sptr->name,
					( IsClient(acptr) ? acptr->user->server : acptr->serv->up ));

			if (MyConnect(acptr))
#ifdef GetIP /* 3.2.1 */
				sendto_one(sptr, ":%s 339 %s :Remote IP: %s [port: %d]",
					me.name, sptr->name,
					( ipaddr ? ipaddr : "<unknown>" ),
					acptr->port);
			else if (ct != CT_Server)
				sendto_one(sptr, ":%s 339 %s :Remote IP: %s",
					me.name, sptr->name,
					( ipaddr ? ipaddr : "<unknown>" ));
#else
                                sendto_one(sptr, ":%s 339 %s :Remote IP: %s [port: %d]",
                                        me.name, sptr->name, Inet_ia2p(&acptr->ip),
                                        acptr->port);
#endif
		}

		/* For local connections only */

		if (MyConnect(acptr))
		{
			sendto_one(sptr, ":%s 339 %s :Flags: %s",
				me.name, sptr->name,
				get_flag_names(acptr->flags));

			if (!IsMe(acptr))
			{
				sendto_one(sptr, ":%s 339 %s :Protoctl: %s",
					me.name, sptr->name,
					get_proto_names(acptr->proto));

			}

			messagestats(acptr, &ms);
			sendto_one(sptr, ":%s 339 %s :Messages sent: %ld (%ld.%u kB), received: %ld (%ld.%u kB)",
				me.name, sptr->name,
				acptr->sendM, ms.sendK, ms.sendB, acptr->receiveM, ms.recvK, ms.recvB);

			sendto_one(sptr, ":%s 339 %s :Creation time: %s",
				me.name, sptr->name, FullDate(acptr->firsttime));

			if (!IsMe(acptr))
				sendto_one(sptr, ":%s 339 %s :Last use time: %s",
					me.name, sptr->name, FullDate(acptr->lasttime));
			if (acptr->nexttarget)
				sendto_one(sptr, ":%s 339 %s :Next target time: %s",
					me.name, sptr->name, FullDate(acptr->nexttarget));
			if (acptr->nextnick)
			        sendto_one(sptr, ":%s 339 %s :Next nick time: %s",
					me.name, sptr->name, FullDate(acptr->nextnick));
		}
	}

	/* FOR SERVERS */

	if (ct == CT_Server)
	{
		sendto_one(sptr, ":%s 339 %s :Numeric: %d",
			me.name, sptr->name, acptr->serv->numeric);

		if (IsMe(acptr))
			sendto_one(sptr, ":%s 339 %s :Users: %d",
				me.name, sptr->name, IRCstats.me_clients);
		else
			sendto_one(sptr, ":%s 339 %s :Users: %ld",
				me.name, sptr->name, acptr->serv->users);

		if (!IsMe(acptr))
		{
		        sendto_one(sptr, ":%s 339 %s :Synced: %s",
				me.name, sptr->name, acptr->serv->flags.synced ? "yes" : "no");
			sendto_one(sptr, ":%s 339 %s :Link is up by: %s",
				me.name, sptr->name,
				*acptr->serv->by ? acptr->serv->by : "<None>");
		}

#ifdef ZIP_LINKS
		if (MyConnect(acptr) && IsZipped(acptr))
			sendto_one(sptr, ":%s 339 %s :Zipstats (out): %01lu -> %lu bytes (%3.1f%%), "
				"compression level: %d",
				me.name, sptr->name, acptr->zip->out->total_in, acptr->zip->out->total_out,
				(100.0*(float)acptr->zip->out->total_out) /(float)acptr->zip->out->total_in,
				acptr->serv->conf->compression_level ? acptr->serv->conf->compression_level : ZIP_DEFAULT_LEVEL);
#endif
	}

	/* FOR USERS */

	if (ct == CT_User)
	{
		sendto_one(sptr, ":%s 339 %s :Time last nick was set: %s",
			me.name, sptr->name, FullDate(acptr->lastnick));

		if (IsHidden(acptr))
			sendto_one(sptr, ":%s 339 %s :Userhost: %s@%s [VHOST %s]",
				me.name, sptr->name,
				acptr->user->username, acptr->user->realhost,
				acptr->user->virthost);
		else		    	 
			sendto_one(sptr, ":%s 339 %s :Userhost: %s@%s",
				me.name, sptr->name,
				acptr->user->username, acptr->user->realhost);

		sendto_one(sptr, ":%s 339 %s :Userflags: %s",
			me.name, sptr->name, get_mode_str(acptr));

		if (MyConnect(acptr))
		{
			sendto_one(sptr, ":%s 339 %s :Snomasks: %s",
				me.name, sptr->name, get_sno_str(acptr));
			sendto_one(sptr, ":%s 339 %s :Operflags: %s",
				me.name, sptr->name, oflagstr(acptr->oflag));
		}

#ifdef MARK_AS_OFFICIAL_MODULE /* 3.2-beta19 */
/*
 * I hope nobody uses devel CVS with date between 2003-11-11 and 2003-11-20.
 * Had to find a macro that determines the best whether we have operlogin.
 */
		if (acptr->user->operlogin)
			sendto_one(sptr, ":%s 339 %s :Last used oper login: %s",
				me.name, sptr->name, acptr->user->operlogin);
#endif
		if (!BadPtr(acptr->user->swhois))
			sendto_one(sptr, ":%s 339 %s :Special info: %s",
				me.name, sptr->name, acptr->user->swhois);
 		if (!BadPtr(acptr->user->away))
			sendto_one(sptr, ":%s 339 %s :Away message: %s",
				me.name, sptr->name, acptr->user->away);
		if (MyClient(acptr))
			sendto_one(sptr, ":%s 339 %s :Watches: %d",
				me.name, sptr->name, acptr->watches);

		sendto_one(sptr, ":%s 339 %s :Joined %d channels",
			me.name, sptr->name, acptr->user->joined);
	}

	sendto_one(sptr, ":%s 339 %s :===================================",
		me.name, sptr->name);
	sendto_one(sptr, ":%s 339 %s :End of /GETINFO",
		me.name, sptr->name);

	return 0;
}
