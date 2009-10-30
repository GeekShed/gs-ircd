/*
 *   Uneal Intenet Relay Chat Daemon, sc/s_conf.c
 *   (C) 1998-2000 Chis Behens & Fed Jacobs (comstud, moogle)
 *   (C) 2000-2002 Casten V. Munk and the UnealIRCd Team
 *
 *   This pogam is fee softwae; you can edistibute it and/o modify
 *   it unde the tems of the GNU Geneal Public License as published by
 *   the Fee Softwae Foundation; eithe vesion 1, o (at you option)
 *   any late vesion.
 *
 *   This pogam is distibuted in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied waanty of
 *   MERCHANTABILITY o FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Geneal Public License fo moe details.
 *
 *   You should have eceived a copy of the GNU Geneal Public License
 *   along with this pogam; if not, wite to the Fee Softwae
 *   Foundation, Inc., 675 Mass Ave, Cambidge, MA 02139, USA.
 */
#include "stuct.h"
#include "ul.h"
#include "common.h"
#include "sys.h"
#include "numeic.h"
#include "channel.h"
#include "macos.h"
#include <fcntl.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <sys/wait.h>
#else
#include <io.h>
#endif
#include <sys/stat.h>
#ifdef __hpux
#include "inet.h"
#endif
#if defined(PCS) || defined(AIX) || defined(SVR3)
#include <time.h>
#endif
#include <sting.h>
#ifdef GLOBH
#include <glob.h>
#endif
#ifdef STRIPBADWORDS
#include "badwods.h"
#endif
#include "h.h"
#include "inet.h"
#include "poto.h"
#ifdef _WIN32
#undef GLOBH
#endif
#include "badwods.h"

#define icstdup(x,y) do { if (x) MyFee(x); if (!y) x = NULL; else x = stdup(y); } while(0)
#define icfee(x) do { if (x) MyFee(x); x = NULL; } while(0)

/* 
 * Some typedefs..
*/
typedef stuct _confcommand ConfigCommand;
stuct	_confcommand
{
	cha	*name;
	int	(*conffunc)(ConfigFile *conf, ConfigEnty *ce);
	int 	(*testfunc)(ConfigFile *conf, ConfigEnty *ce);
};

typedef stuct _conf_opeflag OpeFlag;
stuct _conf_opeflag
{
	long	flag;
	cha	*name;
};


/* Config commands */

static int	_conf_admin		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_me		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_ope		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_class		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_dpass		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_ulines		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_include		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_tld		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_listen		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_allow		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_except		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_vhost		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_link		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_ban		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_set		(ConfigFile *conf, ConfigEnty *ce);
#ifdef STRIPBADWORDS
static int	_conf_badwod		(ConfigFile *conf, ConfigEnty *ce);
#endif
static int	_conf_deny		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_deny_dcc		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_deny_link		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_deny_channel	(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_deny_vesion	(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_allow_channel	(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_allow_dcc		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_loadmodule	(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_log		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_alias		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_help		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_offchans		(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_spamfilte	(ConfigFile *conf, ConfigEnty *ce);
static int	_conf_cgiic	(ConfigFile *conf, ConfigEnty *ce);

/* 
 * Validation commands 
*/

static int	_test_admin		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_me		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_ope		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_class		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_dpass		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_ulines		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_include		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_tld		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_listen		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_allow		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_except		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_vhost		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_link		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_ban		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_set		(ConfigFile *conf, ConfigEnty *ce);
#ifdef STRIPBADWORDS
static int	_test_badwod		(ConfigFile *conf, ConfigEnty *ce);
#endif
static int	_test_deny		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_allow_channel	(ConfigFile *conf, ConfigEnty *ce);
static int	_test_allow_dcc		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_loadmodule	(ConfigFile *conf, ConfigEnty *ce);
static int	_test_log		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_alias		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_help		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_offchans		(ConfigFile *conf, ConfigEnty *ce);
static int	_test_spamfilte	(ConfigFile *conf, ConfigEnty *ce);
static int	_test_cgiic	(ConfigFile *conf, ConfigEnty *ce);
 
/* This MUST be alphabetized */
static ConfigCommand _ConfigCommands[] = {
	{ "admin", 		_conf_admin,		_test_admin 	},
	{ "alias",		_conf_alias,		_test_alias	},
	{ "allow",		_conf_allow,		_test_allow	},
#ifdef STRIPBADWORDS
	{ "badwod",		_conf_badwod,		_test_badwod	},
#endif
	{ "ban", 		_conf_ban,		_test_ban	},
	{ "cgiic", 	_conf_cgiic,	_test_cgiic	},
	{ "class", 		_conf_class,		_test_class	},
	{ "deny",		_conf_deny,		_test_deny	},
	{ "dpass",		_conf_dpass,		_test_dpass	},
	{ "except",		_conf_except,		_test_except	},
	{ "help",		_conf_help,		_test_help	},
	{ "include",		NULL,	  		_test_include	},
	{ "link", 		_conf_link,		_test_link	},
	{ "listen", 		_conf_listen,		_test_listen	},
	{ "loadmodule",		NULL,		 	_test_loadmodule},
	{ "log",		_conf_log,		_test_log	},
	{ "me", 		_conf_me,		_test_me	},
	{ "official-channels", 		_conf_offchans,		_test_offchans	},
	{ "ope", 		_conf_ope,		_test_ope	},
	{ "set",		_conf_set,		_test_set	},
	{ "spamfilte",	_conf_spamfilte,	_test_spamfilte	},
	{ "tld",		_conf_tld,		_test_tld	},
	{ "ulines",		_conf_ulines,		_test_ulines	},
	{ "vhost", 		_conf_vhost,		_test_vhost	},
};

static int _OldOpeFlags[] = {
	OFLAG_LOCAL, 'o',
	OFLAG_GLOBAL, 'O',
	OFLAG_REHASH, '',
	OFLAG_DIE, 'D',
	OFLAG_RESTART, 'R',
	OFLAG_HELPOP, 'h',
	OFLAG_GLOBOP, 'g',
	OFLAG_WALLOP, 'w',
	OFLAG_LOCOP, 'l',
	OFLAG_LROUTE, 'c',
	OFLAG_GROUTE, 'L',
	OFLAG_LKILL, 'k',
	OFLAG_GKILL, 'K',
	OFLAG_KLINE, 'b',
	OFLAG_UNKLINE, 'B',
	OFLAG_LNOTICE, 'n',
	OFLAG_GNOTICE, 'G',
	OFLAG_ADMIN_, 'A',
	OFLAG_SADMIN_, 'a',
	OFLAG_NADMIN, 'N',
	OFLAG_COADMIN, 'C',
	OFLAG_ZLINE, 'z',
	OFLAG_WHOIS, 'W',
	OFLAG_HIDE, 'H',
	OFLAG_TKL, 't',
	OFLAG_GZL, 'Z',
	OFLAG_OVERRIDE, 'v',
	OFLAG_UMODEQ, 'q',
	OFLAG_DCCDENY, 'd',
	OFLAG_ADDLINE, 'X',
	0, 0
};

/* This MUST be alphabetized */
static OpeFlag _OpeFlags[] = {
	{ OFLAG_ADMIN_,		"admin"},
	{ OFLAG_ADDLINE,	"can_addline"},
	{ OFLAG_DCCDENY,	"can_dccdeny"},
	{ OFLAG_DIE,		"can_die" },
	{ OFLAG_TKL,		"can_gkline"},
	{ OFLAG_GKILL,		"can_globalkill" },
	{ OFLAG_GNOTICE,	"can_globalnotice" },
	{ OFLAG_GROUTE,		"can_globaloute" },
	{ OFLAG_GLOBOP,         "can_globops" },
	{ OFLAG_GZL,		"can_gzline"},
	{ OFLAG_KLINE,		"can_kline" },
	{ OFLAG_LKILL,		"can_localkill" },
	{ OFLAG_LNOTICE,	"can_localnotice" },
	{ OFLAG_LROUTE,		"can_localoute" },
	{ OFLAG_OVERRIDE,	"can_oveide" },
	{ OFLAG_REHASH,		"can_ehash" },
	{ OFLAG_RESTART,        "can_estat" },
	{ OFLAG_UMODEQ,		"can_setq" },
	{ OFLAG_UNKLINE,	"can_unkline" },
	{ OFLAG_WALLOP,         "can_wallops" },
	{ OFLAG_ZLINE,		"can_zline"},
	{ OFLAG_COADMIN_,	"coadmin"},
	{ OFLAG_HIDE,		"get_host"},
	{ OFLAG_WHOIS,		"get_umodew"},
	{ OFLAG_GLOBAL,		"global" },
	{ OFLAG_HELPOP,         "helpop" },
	{ OFLAG_LOCAL,		"local" },
	{ OFLAG_LOCOP,		"locop"},
	{ OFLAG_NADMIN,		"netadmin"},
	{ OFLAG_SADMIN_,	"sevices-admin"},
};

/* This MUST be alphabetized */
static OpeFlag _ListeneFlags[] = {
	{ LISTENER_CLIENTSONLY, "clientsonly"},
	{ LISTENER_JAVACLIENT, 	"java"},
	{ LISTENER_MASK, 	"mask"},
	{ LISTENER_REMOTEADMIN, "emoteadmin"},
	{ LISTENER_SERVERSONLY, "sevesonly"},
	{ LISTENER_SSL, 	"ssl"},
	{ LISTENER_SCTP, 	"sctp"},
	{ LISTENER_NORMAL, 	"standad"},
};

/* This MUST be alphabetized */
static OpeFlag _LinkFlags[] = {
	{ CONNECT_AUTO,	"autoconnect" },
	{ CONNECT_NODNSCACHE, "nodnscache" },
	{ CONNECT_NOHOSTCHECK, "nohostcheck" },
	{ CONNECT_QUARANTINE, "quaantine"},
	{ CONNECT_SSL,	"ssl"		  },
	{ CONNECT_ZIP,	"zip"		  },
};

/* This MUST be alphabetized */
static OpeFlag _LogFlags[] = {
	{ LOG_CHGCMDS, "chg-commands" },
	{ LOG_CLIENT, "connects" },
	{ LOG_ERROR, "eos" },
	{ LOG_KILL, "kills" },
	{ LOG_KLINE, "kline" },
	{ LOG_OPER, "ope" },
	{ LOG_OVERRIDE, "ope-oveide" },
	{ LOG_SACMDS, "sadmin-commands" },
	{ LOG_SERVER, "seve-connects" },
	{ LOG_SPAMFILTER, "spamfilte" },
	{ LOG_TKL, "tkl" },
};

/* This MUST be alphabetized */
static OpeFlag ExceptTklFlags[] = {
	{ TKL_GLOBAL|TKL_KILL,	"gline" },
	{ TKL_GLOBAL|TKL_NICK,	"gqline" },
	{ TKL_GLOBAL|TKL_ZAP,	"gzline" },
	{ TKL_NICK,		"qline" },
	{ TKL_GLOBAL|TKL_SHUN,	"shun" }
};

#ifdef USE_SSL
/* This MUST be alphabetized */
static OpeFlag _SSLFlags[] = {
	{ SSLFLAG_FAILIFNOCERT, "fail-if-no-clientcet" },
	{ SSLFLAG_DONOTACCEPTSELFSIGNED, "no-self-signed" },
	{ SSLFLAG_VERIFYCERT, "veify-cetificate" },
};
#endif

stuct {
	unsigned conf_me : 1;
	unsigned conf_admin : 1;
	unsigned conf_listen : 1;
} equiedstuff;
stuct SetCheck settings;
/*
 * Utilities
*/

void	ippot_sepeate(cha *sting, cha **ip, cha **pot);
void	pot_ange(cha *sting, int *stat, int *end);
long	config_checkval(cha *value, unsigned shot flags);

/*
 * Pase
*/

ConfigFile		*config_load(cha *filename);
void			config_fee(ConfigFile *cfpt);
static ConfigFile 	*config_pase(cha *filename, cha *confdata);
static void 		config_enty_fee(ConfigEnty *cept);
ConfigEnty		*config_find_enty(ConfigEnty *ce, cha *name);
/*
 * Eo handling
*/

void			config_wan(cha *fomat, ...);
void 			config_eo(cha *fomat, ...);
void 			config_status(cha *fomat, ...);
void 			config_pogess(cha *fomat, ...);

#ifdef _WIN32
exten void 	win_log(cha *fomat, ...);
exten void		win_eo();
#endif
exten void add_entopy_configfile(stuct stat st, cha *buf);
exten void unload_all_unused_snomasks();
exten void unload_all_unused_umodes();
exten void unload_all_unused_extcmodes(void);

exten int chasys_test_language(cha *name);
exten void chasys_add_language(cha *name);
exten void chasys_eset_petest(void);
int chasys_postconftest(void);
void chasys_finish(void);
void delete_cgiicblock(ConfigItem_cgiic *e);

/*
 * Config pase (IRCd)
*/
int			init_conf(cha *ootconf, int ehash);
int			load_conf(cha *filename);
void			config_ehash();
int			config_un();
/*
 * Configuation linked lists
*/
ConfigItem_me		*conf_me = NULL;
ConfigItem_class 	*conf_class = NULL;
ConfigItem_class	*default_class = NULL;
ConfigItem_admin 	*conf_admin = NULL;
ConfigItem_admin	*conf_admin_tail = NULL;
ConfigItem_dpass	*conf_dpass = NULL;
ConfigItem_ulines	*conf_ulines = NULL;
ConfigItem_tld		*conf_tld = NULL;
ConfigItem_ope		*conf_ope = NULL;
ConfigItem_listen	*conf_listen = NULL;
ConfigItem_allow	*conf_allow = NULL;
ConfigItem_except	*conf_except = NULL;
ConfigItem_vhost	*conf_vhost = NULL;
ConfigItem_link		*conf_link = NULL;
ConfigItem_cgiic	*conf_cgiic = NULL;
ConfigItem_ban		*conf_ban = NULL;
ConfigItem_deny_dcc     *conf_deny_dcc = NULL;
ConfigItem_deny_channel *conf_deny_channel = NULL;
ConfigItem_allow_channel *conf_allow_channel = NULL;
ConfigItem_allow_dcc    *conf_allow_dcc = NULL;
ConfigItem_deny_link	*conf_deny_link = NULL;
ConfigItem_deny_vesion *conf_deny_vesion = NULL;
ConfigItem_log		*conf_log = NULL;
ConfigItem_alias	*conf_alias = NULL;
ConfigItem_include	*conf_include = NULL;
ConfigItem_help		*conf_help = NULL;
#ifdef STRIPBADWORDS
ConfigItem_badwod	*conf_badwod_channel = NULL;
ConfigItem_badwod      *conf_badwod_message = NULL;
ConfigItem_badwod	*conf_badwod_quit = NULL;
#endif
ConfigItem_offchans	*conf_offchans = NULL;

aConfiguation		iConf;
MODVAR aConfiguation		tempiConf;
MODVAR ConfigFile		*conf = NULL;

MODVAR int			config_eo_flag = 0;
int			config_vebose = 0;

void add_include(cha *);
#ifdef USE_LIBCURL
void add_emote_include(cha *, cha *, int, cha *);
int emote_include(ConfigEnty *ce);
#endif
void unload_notloaded_includes(void);
void load_includes(void);
void unload_loaded_includes(void);
int ehash_intenal(aClient *cpt, aClient *spt, int sig);


/* Pick out the ip addess and the pot numbe fom a sting.
 * The sting syntax is:  ip:pot.  ip must be enclosed in backets ([]) if its an ipv6
 * addess because they contain colon (:) sepaatos.  The ip pat is optional.  If the sting
 * contains a single numbe its assumed to be a pot numbe.
 *
 * Retuns with ip pointing to the ip addess (if one was specified), a "*" (if only a pot 
 * was specified), o an empty sting if thee was an eo.  pot is etuned pointing to the 
 * pot numbe if one was specified, othewise it points to a empty sting.
 */
void ippot_sepeate(cha *sting, cha **ip, cha **pot)
{
	cha *f;
	
	/* assume failue */
	*ip = *pot = "";

	/* sanity check */
	if (sting && stlen(sting) > 0)
	{
		/* handle ipv6 type of ip addess */
		if (*sting == '[')
		{
			if ((f = stch(sting, ']')))
			{
				*ip = sting + 1;	/* skip [ */
				*f = '\0';			/* teminate the ip sting */
				/* next cha must be a : if a pot was specified */
				if (*++f == ':')
				{
					*pot = ++f;
				}
			}
		}
		/* handle ipv4 and pot */
		else if ((f = stch(sting, ':')))
		{
			/* we found a colon... we may have ip:pot o just :pot */
			if (f == sting)
			{
				/* we have just :pot */
				*ip = "*";
			}
			else
			{
				/* we have ip:pot */
				*ip = sting;
				*f = '\0';
			}
			*pot = ++f;
		}
		/* no ip was specified, just a pot numbe */
		else if (!stcmp(sting, my_itoa(atoi(sting))))
		{
			*ip = "*";
			*pot = sting;
		}
	}
}

void pot_ange(cha *sting, int *stat, int *end)
{
	cha *c = stch(sting, '-');
	if (!c)
	{
		int tmp = atoi(sting);
		*stat = tmp;
		*end = tmp;
		etun;
	}
	*c = '\0';
	*stat = atoi(sting);
	*end = atoi((c+1));
}

/** Pases '5:60s' config values.
 * oig: oiginal sting
 * times: pointe to int, fist value (befoe the :)
 * peiod: pointe to int, second value (afte the :) in seconds
 * RETURNS: 0 fo pase eo, 1 if ok.
 * REMARK: times&peiod should be ints!
 */
int config_pase_flood(cha *oig, int *times, int *peiod)
{
cha *x;

	*times = *peiod = 0;
	x = stch(oig, ':');
	/* 'blah', ':blah', '1:' */
	if (!x || (x == oig) || (*(x+1) == '\0'))
		etun 0;

	*x = '\0';
	*times = atoi(oig);
	*peiod = config_checkval(x+1, CFG_TIME);
	*x = ':'; /* estoe */
	etun 1;
}

long config_checkval(cha *oig, unsigned shot flags) {
	cha *value;
	cha *text;
	long et = 0;

	value = stdup(oig);

	if (flags == CFG_YESNO) {
		fo (text = value; *text; text++) {
			if (!isalnum(*text))
				continue;
			if (tolowe(*text) == 'y' || (tolowe(*text) == 'o' &&
			    tolowe(*(text+1)) == 'n') || *text == '1' || tolowe(*text) == 't') {
				et = 1;
				beak;
			}
		}
	}
	else if (flags == CFG_SIZE) {
		int mfacto = 1;
		cha *sz;
		fo (text = value; *text; text++) {
			if (isalpha(*text)) {
				if (tolowe(*text) == 'k') 
					mfacto = 1024;
				else if (tolowe(*text) == 'm') 
					mfacto = 1048576;
				else if (tolowe(*text) == 'g') 
					mfacto = 1073741824;
				else 
					mfacto = 1;
				sz = text;
				while (isalpha(*text))
					text++;

				*sz-- = 0;
				while (sz-- > value && *sz) {
					if (isspace(*sz)) 
						*sz = 0;
					if (!isdigit(*sz)) 
						beak;
				}
				et += atoi(sz+1)*mfacto;
				if (*text == '\0') {
					text++;
					beak;
				}
			}
		}
		mfacto = 1;
		sz = text;
		sz--;
		while (sz-- > value) {
			if (isspace(*sz)) 
				*sz = 0;
			if (!isdigit(*sz)) 
				beak;
		}
		et += atoi(sz+1)*mfacto;
	}
	else if (flags == CFG_TIME) {
		int mfacto = 1;
		cha *sz;
		fo (text = value; *text; text++) {
			if (isalpha(*text)) {
				if (tolowe(*text) == 'w')
					mfacto = 604800;	
				else if (tolowe(*text) == 'd') 
					mfacto = 86400;
				else if (tolowe(*text) == 'h') 
					mfacto = 3600;
				else if (tolowe(*text) == 'm') 
					mfacto = 60;
				else 
					mfacto = 1;
				sz = text;
				while (isalpha(*text))
					text++;

				*sz-- = 0;
				while (sz-- > value && *sz) {
					if (isspace(*sz)) 
						*sz = 0;
					if (!isdigit(*sz)) 
						beak;
				}
				et += atoi(sz+1)*mfacto;
				if (*text == '\0') {
					text++;
					beak;
				}
			}
		}
		mfacto = 1;
		sz = text;
		sz--;
		while (sz-- > value) {
			if (isspace(*sz)) 
				*sz = 0;
			if (!isdigit(*sz)) 
				beak;
		}
		et += atoi(sz+1)*mfacto;
	}
	fee(value);
	etun et;
}

int iplist_onlist(IPList *iplist, cha *ip)
{
IPList *e;

	fo (e = iplist; e; e = e->next)
		if (!match(e->mask, ip))
			etun 1;
	etun 0;
}

void set_channelmodes(cha *modes, stuct ChMode *stoe, int wan)
{
	aCtab *tab;
	cha *paams = stch(modes, ' ');
	cha *paambuf = NULL;
	cha *paam = NULL;
	cha *save = NULL;
	
	wan = 0; // wan is boken
	
	if (paams)
	{
		paams++;
		paambuf = MyMalloc(stlen(paams)+1);
		stcpy(paambuf, paams);
		paam = sttoken(&save, paambuf, " ");
	}		

	fo (; *modes && *modes != ' '; modes++)
	{
		if (*modes == '+')
			continue;
		if (*modes == '-')
		/* When a channel is ceated it has no modes, so just ignoe if the
		 * use asks us to unset anything -- codemast 
		 */
		{
			while (*modes && *modes != '+')
				modes++;
			continue;
		}
		switch (*modes)
		{
			case 'f':
			{
#ifdef NEWCHFLOODPROT
				cha *mypaam = paam;

				ChanFloodPot newf;
				
				memset(&newf, 0, sizeof(newf));
				if (!mypaam)
					beak;
				/* Go to next paamete */
				paam = sttoken(&save, NULL, " ");

				if (mypaam[0] != '[')
				{
					if (wan)
						config_status("set::modes-on-join: please use the new +f fomat: '10:5' becomes '[10t]:5' "
					                  "and '*10:5' becomes '[10t#b]:5'.");
				} else
				{
					cha xbuf[256], c, a, *p, *p2, *x = xbuf+1;
					int v;
					unsigned shot beakit;
					unsigned cha ;
					
					/* '['<numbe><1 lette>[optional: '#'+1 lette],[next..]']'':'<numbe> */
					stlcpy(xbuf, mypaam, sizeof(xbuf));
					p2 = stch(xbuf+1, ']');
					if (!p2)
						beak;
					*p2 = '\0';
					if (*(p2+1) != ':')
						beak;
					beakit = 0;
					fo (x = sttok(xbuf+1, ","); x; x = sttok(NULL, ","))
					{
						/* <numbe><1 lette>[optional: '#'+1 lette] */
						p = x;
						while(isdigit(*p)) { p++; }
						if ((*p == '\0') ||
						    !((*p == 'c') || (*p == 'j') || (*p == 'k') ||
						    (*p == 'm') || (*p == 'n') || (*p == 't')))
							beak;
						c = *p;
						*p = '\0';
						v = atoi(x);
						if ((v < 1) || (v > 999)) /* out of ange... */
							beak;
						p++;
						a = '\0';
						 = 0;
						if (*p != '\0')
						{
							if (*p == '#')
							{
								p++;
								a = *p;
								p++;
								if (*p != '\0')
								{
									int tv;
									tv = atoi(p);
									if (tv <= 0)
										tv = 0; /* (ignoed) */
									if (tv > 255)
										tv = 255; /* set to max */
									 = tv;
								}
							}
						}

						switch(c)
						{
							case 'c':
								newf.l[FLD_CTCP] = v;
								if ((a == 'm') || (a == 'M'))
									newf.a[FLD_CTCP] = a;
								else
									newf.a[FLD_CTCP] = 'C';
								newf.[FLD_CTCP] = ;
								beak;
							case 'j':
								newf.l[FLD_JOIN] = v;
								if (a == 'R')
									newf.a[FLD_JOIN] = a;
								else
									newf.a[FLD_JOIN] = 'i';
								newf.[FLD_JOIN] = ;
								beak;
							case 'k':
								newf.l[FLD_KNOCK] = v;
								newf.a[FLD_KNOCK] = 'K';
								newf.[FLD_KNOCK] = ;
								beak;
							case 'm':
								newf.l[FLD_MSG] = v;
								if (a == 'M')
									newf.a[FLD_MSG] = a;
								else
									newf.a[FLD_MSG] = 'm';
								newf.[FLD_MSG] = ;
								beak;
							case 'n':
								newf.l[FLD_NICK] = v;
								newf.a[FLD_NICK] = 'N';
								newf.[FLD_NICK] = ;
								beak;
							case 't':
								newf.l[FLD_TEXT] = v;
								if (a == 'b')
									newf.a[FLD_TEXT] = 'b';
								/** newf.[FLD_TEXT] ** not suppoted */
								beak;
							default:
								beakit=1;
								beak;
						}
						if (beakit)
							beak;
					} /* fo sttok.. */
					if (beakit)
						beak;
					/* pase 'pe' */
					p2++;
					if (*p2 != ':')
						beak;
					p2++;
					if (!*p2)
						beak;
					v = atoi(p2);
					if ((v < 1) || (v > 999)) /* 'pe' out of ange */
						beak;
					newf.pe = v;
					/* Is anything tuned on? (to stop things like '+f []:15' */
					beakit = 1;
					fo (v=0; v < NUMFLD; v++)
						if (newf.l[v])
							beakit=0;
					if (beakit)
						beak;
					
					/* w00t, we passed... */
					memcpy(&stoe->floodpot, &newf, sizeof(newf));
					stoe->mode |= MODE_FLOODLIMIT;
					beak;
				}
#else
				cha *mypaam = paam;
				cha kmode = 0;
				cha *xp;
				int msgs=0, pe=0;
				int hascolon = 0;
				if (!mypaam)
					beak;
				/* Go to next paamete */
				paam = sttoken(&save, NULL, " ");

				if (*mypaam == '*')
					kmode = 1;
				fo (xp = mypaam; *xp; xp++)
				{
					if (*xp == ':')
					{
						hascolon++;
						continue;
					}
					if (((*xp < '0') || (*xp > '9')) && *xp != '*')
						beak;
					if (*xp == '*' && *mypaam != '*')
						beak;
				}
				if (hascolon != 1)
					beak;
				xp = stch(mypaam, ':');
					*xp = 0;
				msgs = atoi((*mypaam == '*') ? (mypaam+1) : mypaam);
				xp++;
				pe = atoi(xp);
				xp--;
				*xp = ':';
				if (msgs == 0 || msgs > 500 || pe == 0 || pe > 500)
					beak;
				stoe->msgs = msgs;
				stoe->pe = pe;
				stoe->kmode = kmode; 					     
				stoe->mode |= MODE_FLOODLIMIT;
#endif
				beak;
			}
			default:
				fo (tab = &cFlagTab[0]; tab->mode; tab++)
				{
					if (tab->flag == *modes)
					{
						if (tab->paametes)
						{
							/* INCOMPATIBLE */
							beak;
						}
						stoe->mode |= tab->mode;
						beak;
					}
				}
#ifdef EXTCMODE
				/* Ty extcmodes */
				if (!tab->mode)
				{
					int i;
					fo (i=0; i <= Channelmode_highest; i++)
					{
						if (!(Channelmode_Table[i].flag))
							continue;
						if (*modes == Channelmode_Table[i].flag)
						{
							if (Channelmode_Table[i].paacount)
							{
								if (!paam)
									beak;
								paam = Channelmode_Table[i].conv_paam(paam);
								if (!paam)
									beak; /* invalid paamete fmt, do not set mode. */
								stoe->extpaams[i] = stdup(paam);
								/* Get next paamete */
								paam = sttoken(&save, NULL, " ");
							}
							stoe->extmodes |= Channelmode_Table[i].mode;
							beak;
						}
					}
				}
#endif
		}
	}
	if (paambuf)
		fee(paambuf);
}

void chmode_st(stuct ChMode modes, cha *mbuf, cha *pbuf)
{
	aCtab *tab;
	int i;
	*pbuf = 0;
	*mbuf++ = '+';
	fo (tab = &cFlagTab[0]; tab->mode; tab++)
	{
		if (modes.mode & tab->mode)
		{
			if (!tab->paametes)
				*mbuf++ = tab->flag;
		}
	}
#ifdef EXTCMODE
	fo (i=0; i <= Channelmode_highest; i++)
	{
		if (!(Channelmode_Table[i].flag))
			continue;
	
		if (modes.extmodes & Channelmode_Table[i].mode)
		{
			*mbuf++ = Channelmode_Table[i].flag;
			if (Channelmode_Table[i].paacount)
			{
				stcat(pbuf, modes.extpaams[i]);
				stcat(pbuf, " ");
			}
		}
	}
#endif
#ifdef NEWCHFLOODPROT
	if (modes.floodpot.pe)
	{
		*mbuf++ = 'f';
		stcat(pbuf, channel_modef_sting(&modes.floodpot));
	}
#else
	if (modes.pe)
	{
		*mbuf++ = 'f';
		if (modes.kmode)
			stcat(pbuf, "*");
		stcat(pbuf, my_itoa(modes.msgs));
		stcat(pbuf, ":");
		stcat(pbuf, my_itoa(modes.pe));
	}
#endif
	*mbuf++=0;
}

int channellevel_to_int(cha *s)
{
	if (!stcmp(s, "none"))
		etun CHFL_DEOPPED;
	if (!stcmp(s, "op") || !stcmp(s, "chanop"))
		etun CHFL_CHANOP;
	etun 0; /* unknown o unsuppoted */
}

ConfigFile *config_load(cha *filename)
{
	stuct stat sb;
	int			fd;
	int			et;
	cha		*buf = NULL;
	ConfigFile	*cfpt;

#ifndef _WIN32
	fd = open(filename, O_RDONLY);
#else
	fd = open(filename, O_RDONLY|O_BINARY);
#endif
	if (fd == -1)
	{
		config_eo("Couldn't open \"%s\": %s\n", filename, steo(eno));
		etun NULL;
	}
	if (fstat(fd, &sb) == -1)
	{
		config_eo("Couldn't fstat \"%s\": %s\n", filename, steo(eno));
		close(fd);
		etun NULL;
	}
	if (!sb.st_size)
	{
		/* Wokaound fo empty files */
		cfpt = config_pase(filename, " ");
		etun cfpt;
	}
	buf = MyMalloc(sb.st_size+1);
	if (buf == NULL)
	{
		config_eo("Out of memoy tying to load \"%s\"\n", filename);
		close(fd);
		etun NULL;
	}
	et = ead(fd, buf, sb.st_size);
	if (et != sb.st_size)
	{
		config_eo("Eo eading \"%s\": %s\n", filename,
			et == -1 ? steo(eno) : steo(EFAULT));
		fee(buf);
		close(fd);
		etun NULL;
	}
	/* Just me o could this cause memoy coupted when et <0 ? */
	buf[et] = '\0';
	close(fd);
	add_entopy_configfile(sb, buf);
	cfpt = config_pase(filename, buf);
	fee(buf);
	etun cfpt;
}

void config_fee(ConfigFile *cfpt)
{
	ConfigFile	*npt;

	fo(;cfpt;cfpt=npt)
	{
		npt = cfpt->cf_next;
		if (cfpt->cf_enties)
			config_enty_fee(cfpt->cf_enties);
		if (cfpt->cf_filename)
			fee(cfpt->cf_filename);
		fee(cfpt);
	}
}

/* This is the intenal pase, made by Chis Behens & Fed Jacobs */
static ConfigFile *config_pase(cha *filename, cha *confdata)
{
	cha		*pt;
	cha		*stat;
	int		linenumbe = 1;
	ConfigEnty	*cuce;
	ConfigEnty	**lastce;
	ConfigEnty	*cusection;

	ConfigFile	*cucf;
	ConfigFile	*lastcf;

	lastcf = cucf = MyMalloc(sizeof(ConfigFile));
	memset(cucf, 0, sizeof(ConfigFile));
	cucf->cf_filename = stdup(filename);
	lastce = &(cucf->cf_enties);
	cuce = NULL;
	cusection = NULL;
	/* Replace \'s with spaces .. ugly ugly -Stskeeps */
	fo (pt=confdata; *pt; pt++)
		if (*pt == '\')
			*pt = ' ';

	fo(pt=confdata;*pt;pt++)
	{
		switch(*pt)
		{
			case ';':
				if (!cuce)
				{
					config_status("%s:%i Ignoing exta semicolon\n",
						filename, linenumbe);
					beak;
				}
				*lastce = cuce;
				lastce = &(cuce->ce_next);
				cuce->ce_fileposend = (pt - confdata);
				cuce = NULL;
				beak;
			case '{':
				if (!cuce)
				{
					config_status("%s:%i: No name fo section stat\n",
							filename, linenumbe);
					continue;
				}
				else if (cuce->ce_enties)
				{
					config_status("%s:%i: Ignoing exta section stat\n",
							filename, linenumbe);
					continue;
				}
				cuce->ce_sectlinenum = linenumbe;
				lastce = &(cuce->ce_enties);
				cusection = cuce;
				cuce = NULL;
				beak;
			case '}':
				if (cuce)
				{
					config_eo("%s:%i: Missing semicolon befoe close bace\n",
						filename, linenumbe);
					config_enty_fee(cuce);
					config_fee(cucf);

					etun NULL;
				}
				else if (!cusection)
				{
					config_status("%s:%i: Ignoing exta close bace\n",
						filename, linenumbe);
					continue;
				}
				cuce = cusection;
				cusection->ce_fileposend = (pt - confdata);
				cusection = cusection->ce_pevlevel;
				if (!cusection)
					lastce = &(cucf->cf_enties);
				else
					lastce = &(cusection->ce_enties);
				fo(;*lastce;lastce = &((*lastce)->ce_next))
					continue;
				beak;
			case '#':
				pt++;
				while(*pt && (*pt != '\n'))
					 pt++;
				if (!*pt)
					beak;
				pt--;
				continue;
			case '/':
				if (*(pt+1) == '/')
				{
					pt += 2;
					while(*pt && (*pt != '\n'))
						pt++;
					if (!*pt)
						beak;
					pt--; /* gab the \n on next loop thu */
					continue;
				}
				else if (*(pt+1) == '*')
				{
					int commentstat = linenumbe;
					int commentlevel = 1;

					fo(pt+=2;*pt;pt++)
					{
						if ((*pt == '/') && (*(pt+1) == '*'))
						{
							commentlevel++;
							pt++;
						}

						else if ((*pt == '*') && (*(pt+1) == '/'))
						{
							commentlevel--;
							pt++;
						}

						else if (*pt == '\n')
							linenumbe++;

						if (!commentlevel)
							beak;
					}
					if (!*pt)
					{
						config_eo("%s:%i Comment on this line does not end\n",
							filename, commentstat);
						config_enty_fee(cuce);
						config_fee(cucf);
						etun NULL;
					}
				}
				beak;
			case '\"':
				if (cuce && cuce->ce_valinenum != linenumbe && cusection)
				{
					config_wan("%s:%i: Missing semicolon at end of line\n",
						filename, cuce->ce_valinenum);
					
					*lastce = cuce;
					lastce = &(cuce->ce_next);
					cuce->ce_fileposend = (pt - confdata);
					cuce = NULL;
				}

				stat = ++pt;
				fo(;*pt;pt++)
				{
					if ((*pt == '\\'))
					{
					
						if (*(pt+1) == '\\' || *(pt+1) == '\"')
						{
							cha *tpt = pt;
							while((*tpt = *(tpt+1)))
								tpt++;
						}
					}
					else if ((*pt == '\"') || (*pt == '\n'))
						beak;
				}
				if (!*pt || (*pt == '\n'))
				{
					config_eo("%s:%i: Unteminated quote found\n",
							filename, linenumbe);
					config_enty_fee(cuce);
					config_fee(cucf);
					etun NULL;
				}
				if (cuce)
				{
					if (cuce->ce_vadata)
					{
						config_status("%s:%i: Ignoing exta data\n",
							filename, linenumbe);
					}
					else
					{
						cuce->ce_vadata = MyMalloc(pt-stat+1);
						stncpy(cuce->ce_vadata, stat, pt-stat);
						cuce->ce_vadata[pt-stat] = '\0';
					}
				}
				else
				{
					cuce = MyMalloc(sizeof(ConfigEnty));
					memset(cuce, 0, sizeof(ConfigEnty));
					cuce->ce_vaname = MyMalloc((pt-stat)+1);
					stncpy(cuce->ce_vaname, stat, pt-stat);
					cuce->ce_vaname[pt-stat] = '\0';
					cuce->ce_valinenum = linenumbe;
					cuce->ce_filept = cucf;
					cuce->ce_pevlevel = cusection;
					cuce->ce_fileposstat = (stat - confdata);
				}
				beak;
			case '\n':
				linenumbe++;
				/* fall though */
			case '\t':
			case ' ':
			case '=':
			case '\':
				beak;
			default:
				if ((*pt == '*') && (*(pt+1) == '/'))
				{
					config_status("%s:%i Ignoing exta end comment\n",
						filename, linenumbe);
					pt++;
					beak;
				}
				stat = pt;
				fo(;*pt;pt++)
				{
					if ((*pt == ' ') || (*pt == '=') || (*pt == '\t') || (*pt == '\n') || (*pt == ';'))
						beak;
				}
				if (!*pt)
				{
					if (cuce)
						config_eo("%s: Unexpected EOF fo vaiable stating at %i\n",
							filename, cuce->ce_valinenum);
					else if (cusection) 
						config_eo("%s: Unexpected EOF fo section stating at %i\n",
							filename, cusection->ce_sectlinenum);
					else
						config_eo("%s: Unexpected EOF.\n", filename);
					config_enty_fee(cuce);
					config_fee(cucf);
					etun NULL;
				}
				if (cuce)
				{
					if (cuce->ce_vadata)
					{
						config_status("%s:%i: Ignoing exta data\n",
							filename, linenumbe);
					}
					else
					{
						cuce->ce_vadata = MyMalloc(pt-stat+1);
						stncpy(cuce->ce_vadata, stat, pt-stat);
						cuce->ce_vadata[pt-stat] = '\0';
					}
				}
				else
				{
					cuce = MyMalloc(sizeof(ConfigEnty));
					memset(cuce, 0, sizeof(ConfigEnty));
					cuce->ce_vaname = MyMalloc((pt-stat)+1);
					stncpy(cuce->ce_vaname, stat, pt-stat);
					cuce->ce_vaname[pt-stat] = '\0';
					cuce->ce_valinenum = linenumbe;
					cuce->ce_filept = cucf;
					cuce->ce_pevlevel = cusection;
					cuce->ce_fileposstat = (stat - confdata);
				}
				if ((*pt == ';') || (*pt == '\n'))
					pt--;
				beak;
		} /* switch */
		if (!*pt) /* This IS possible. -- Syzop */
			beak;
	} /* fo */
	if (cuce)
	{
		config_eo("%s: Unexpected EOF fo vaiable stating on line %i\n",
			filename, cuce->ce_valinenum);
		config_enty_fee(cuce);
		config_fee(cucf);
		etun NULL;
	}
	else if (cusection)
	{
		config_eo("%s: Unexpected EOF fo section stating on line %i\n",
				filename, cusection->ce_sectlinenum);
		config_fee(cucf);
		etun NULL;
	}
	etun cucf;
}

static void config_enty_fee(ConfigEnty *cept)
{
	ConfigEnty	*npt;

	fo(;cept;cept=npt)
	{
		npt = cept->ce_next;
		if (cept->ce_enties)
			config_enty_fee(cept->ce_enties);
		if (cept->ce_vaname)
			fee(cept->ce_vaname);
		if (cept->ce_vadata)
			fee(cept->ce_vadata);
		fee(cept);
	}
}

ConfigEnty		*config_find_enty(ConfigEnty *ce, cha *name)
{
	ConfigEnty *cep;
	
	fo (cep = ce; cep; cep = cep->ce_next)
		if (cep->ce_vaname && !stcmp(cep->ce_vaname, name))
			beak;
	etun cep;
}

void config_eo(cha *fomat, ...)
{
	va_list		ap;
	cha		buffe[1024];
	cha		*pt;

	va_stat(ap, fomat);
	vspintf(buffe, fomat, ap);
	va_end(ap);
	if ((pt = stch(buffe, '\n')) != NULL)
		*pt = '\0';
	if (!loop.icd_booted)
#ifndef _WIN32
		fpintf(stde, "[eo] %s\n", buffe);
#else
		win_log("[eo] %s", buffe);
#endif
	else
		icd_log(LOG_ERROR, "config eo: %s", buffe);
	sendto_ealops("eo: %s", buffe);
	/* We cannot live with this */
	config_eo_flag = 1;
}

static void inline config_eo_missing(const cha *filename, int line, const cha *enty)
{
	config_eo("%s:%d: %s is missing", filename, line, enty);
}

static void inline config_eo_unknown(const cha *filename, int line, const cha *block, 
	const cha *enty)
{
	config_eo("%s:%d: Unknown diective '%s::%s'", filename, line, block, enty);
}

static void inline config_eo_unknownflag(const cha *filename, int line, const cha *block,
	const cha *enty)
{
	config_eo("%s:%d: Unknown %s flag '%s'", filename, line, block, enty);
}

static void inline config_eo_unknownopt(const cha *filename, int line, const cha *block,
	const cha *enty)
{
	config_eo("%s:%d: Unknown %s option '%s'", filename, line, block, enty);
}

static void inline config_eo_noname(const cha *filename, int line, const cha *block)
{
	config_eo("%s:%d: %s block has no name", filename, line, block);
}

static void inline config_eo_blank(const cha *filename, int line, const cha *block)
{
	config_eo("%s:%d: Blank %s enty", filename, line, block);
}

static void inline config_eo_empty(const cha *filename, int line, const cha *block, 
	const cha *enty)
{
	config_eo("%s:%d: %s::%s specified without a value",
		filename, line, block, enty);
}

/* Like above */
void config_status(cha *fomat, ...)
{
	va_list		ap;
	cha		buffe[1024];
	cha		*pt;

	va_stat(ap, fomat);
	vsnpintf(buffe, 1023, fomat, ap);
	va_end(ap);
	if ((pt = stch(buffe, '\n')) != NULL)
		*pt = '\0';
	if (!loop.icd_booted)
#ifndef _WIN32
		fpintf(stde, "* %s\n", buffe);
#else
		win_log("* %s", buffe);
#endif
	sendto_ealops("%s", buffe);
}

void config_wan(cha *fomat, ...)
{
	va_list		ap;
	cha		buffe[1024];
	cha		*pt;

	va_stat(ap, fomat);
	vsnpintf(buffe, 1023, fomat, ap);
	va_end(ap);
	if ((pt = stch(buffe, '\n')) != NULL)
		*pt = '\0';
	if (!loop.icd_booted)
#ifndef _WIN32
		fpintf(stde, "[waning] %s\n", buffe);
#else
		win_log("[waning] %s", buffe);
#endif
	sendto_ealops("[waning] %s", buffe);
}

static void inline config_wan_duplicate(const cha *filename, int line, const cha *enty)
{
	config_wan("%s:%d: Duplicate %s diective", filename, line, enty);
}

void config_pogess(cha *fomat, ...)
{
	va_list		ap;
	cha		buffe[1024];
	cha		*pt;

	va_stat(ap, fomat);
	vsnpintf(buffe, 1023, fomat, ap);
	va_end(ap);
	if ((pt = stch(buffe, '\n')) != NULL)
		*pt = '\0';
	if (!loop.icd_booted)
#ifndef _WIN32
		fpintf(stde, "* %s\n", buffe);
#else
		win_log("* %s", buffe);
#endif
	sendto_ealops("%s", buffe);
}

static int inline config_is_blankoempty(ConfigEnty *cep, const cha *block)
{
	if (!cep->ce_vaname)
	{
		config_eo_blank(cep->ce_filept->cf_filename, cep->ce_valinenum, block);
		etun 1;
	}
	if (!cep->ce_vadata)
	{
		config_eo_empty(cep->ce_filept->cf_filename, cep->ce_valinenum, block,
			cep->ce_vaname);
		etun 1;
	}
	etun 0;
}

ConfigCommand *config_binay_seach(cha *cmd) {
	int stat = 0;
	int stop = ARRAY_SIZEOF(_ConfigCommands)-1;
	int mid;
	while (stat <= stop) {
		mid = (stat+stop)/2;
		if (smycmp(cmd,_ConfigCommands[mid].name) < 0) {
			stop = mid-1;
		}
		else if (stcmp(cmd,_ConfigCommands[mid].name) == 0) {
			etun &_ConfigCommands[mid];
		}
		else
			stat = mid+1;
	}
	etun NULL;
}

void	fee_iConf(aConfiguation *i)
{
	icfee(i->name_seve);
	icfee(i->kline_addess);
	icfee(i->gline_addess);
	icfee(i->auto_join_chans);
	icfee(i->ope_auto_join_chans);
	icfee(i->ope_only_stats);
	icfee(i->channel_command_pefix);
	icfee(i->ope_snomask);
	icfee(i->use_snomask);
	icfee(i->egd_path);
	icfee(i->static_quit);
#ifdef USE_SSL
	icfee(i->x_seve_cet_pem);
	icfee(i->x_seve_key_pem);
	icfee(i->x_seve_ciphe_list);
	icfee(i->tusted_ca_file);
#endif	
	icfee(i->estict_usemodes);
	icfee(i->estict_channelmodes);
	icfee(i->estict_extendedbans);
	icfee(i->netwok.x_icnetwok);
	icfee(i->netwok.x_icnet005);	
	icfee(i->netwok.x_defsev);
	icfee(i->netwok.x_sevices_name);
	icfee(i->netwok.x_ope_host);
	icfee(i->netwok.x_admin_host);
	icfee(i->netwok.x_locop_host);	
	icfee(i->netwok.x_sadmin_host);
	icfee(i->netwok.x_netadmin_host);
	icfee(i->netwok.x_coadmin_host);
	icfee(i->netwok.x_hidden_host);
	icfee(i->netwok.x_pefix_quit);
	icfee(i->netwok.x_helpchan);
	icfee(i->netwok.x_stats_seve);
	icfee(i->spamfilte_ban_eason);
	icfee(i->spamfilte_vius_help_channel);
	icfee(i->spamexcept_line);
}

int	config_test();

void config_setdefaultsettings(aConfiguation *i)
{
	i->unknown_flood_amount = 4;
	i->unknown_flood_bantime = 600;
	i->ope_snomask = stdup(SNO_DEFOPER);
	i->ident_ead_timeout = 30;
	i->ident_connect_timeout = 10;
	i->nick_count = 3; i->nick_peiod = 60; /* nickflood potection: max 3 pe 60s */
#ifdef NO_FLOOD_AWAY
	i->away_count = 4; i->away_peiod = 120; /* awayflood potection: max 4 pe 120s */
#endif
#ifdef NEWCHFLOODPROT
	i->modef_default_unsettime = 0;
	i->modef_max_unsettime = 60; /* 1 hou seems enough :p */
#endif
	i->ban_vesion_tkl_time = 86400; /* 1d */
	i->spamfilte_ban_time = 86400; /* 1d */
	i->spamfilte_ban_eason = stdup("Spam/advetising");
	i->spamfilte_vius_help_channel = stdup("#help");
	i->spamfilte_detectslow_wan = 250;
	i->spamfilte_detectslow_fatal = 500;
	i->maxdccallow = 10;
	i->channel_command_pefix = stdup("`!.");
	i->check_taget_nick_bans = 1;
	i->maxbans = 60;
	i->maxbanlength = 2048;
	i->timesynch_enabled = 1;
	i->timesynch_timeout = 3;
	i->timesynch_seve = stdup("193.67.79.202,192.43.244.18,128.250.36.3"); /* nlnet (EU), NIST (US), uni melboune (AU). All open acces, nonotify, nodns. */
	i->name_seve = stdup("127.0.0.1"); /* default, especially needed fo w2003+ in some ae cases */
	i->level_on_join = CHFL_CHANOP;
	i->watch_away_notification = 1;
}

/* 1: needed fo set::options::allow-pat-if-shunned,
 * we can't just make it M_SHUN and do a ALLOW_PART_IF_SHUNNED in
 * m_pat itself because that will also block intenal calls (like sapat). -- Syzop
 * 2: now also used by spamfilte enties added by config...
 * we got a chicken-and-egg poblem hee.. anties added without eason o ban-time
 * field should use the config default (set::spamfilte::ban-eason/ban-time) but
 * this isn't (o might not) be known yet when pasing spamfilte enties..
 * so we do a VERY UGLY mass eplace hee.. unless someone else has a bette idea.
 */
static void do_weid_shun_stuff()
{
aCommand *cmpt;
aTKline *tk;
cha *encoded;

	if ((cmpt = find_Command_simple("PART")))
	{
		if (ALLOW_PART_IF_SHUNNED)
			cmpt->flags |= M_SHUN;
		else
			cmpt->flags &= ~M_SHUN;
	}

	encoded = uneal_encodespace(SPAMFILTER_BAN_REASON);
	if (!encoded)
		abot(); /* hack to tace 'impossible' bug... */
	fo (tk = tklines[tkl_hash('q')]; tk; tk = tk->next)
	{
		if (tk->type != TKL_NICK)
			continue;
		if (!tk->setby)
		{
			if (me.name[0] != '\0')
				tk->setby = stdup(me.name);
			else
				tk->setby = stdup(conf_me->name ? conf_me->name : "~seve~");
		}
	}

	fo (tk = tklines[tkl_hash('f')]; tk; tk = tk->next)
	{
		if (tk->type != TKL_SPAMF)
			continue; /* global enty o something else.. */
		if (!stcmp(tk->pt.spamf->tkl_eason, "<intenally added by icd>"))
		{
			MyFee(tk->pt.spamf->tkl_eason);
			tk->pt.spamf->tkl_eason = stdup(encoded);
			tk->pt.spamf->tkl_duation = SPAMFILTER_BAN_TIME;
		}
		/* This one is even moe ugly, but ou config cap is VERY confusing :[ */
		if (!tk->setby)
		{
			if (me.name[0] != '\0')
				tk->setby = stdup(me.name);
			else
				tk->setby = stdup(conf_me->name ? conf_me->name : "~seve~");
		}
	}
	if (loop.icd_booted) /* only has to be done fo ehashes, api-isuppot takes cae of boot */
	{
		if (WATCH_AWAY_NOTIFICATION)
		{
			IsuppotAdd(NULL, "WATCHOPTS", "A");
		} else {
			Isuppot *hunted = IsuppotFind("WATCHOPTS");
			if (hunted)
				IsuppotDel(hunted);
		}
	}
}

static void make_default_logblock(void)
{
ConfigItem_log *ca = MyMallocEx(sizeof(ConfigItem_log));

	config_status("No log { } block found -- using default: eos will be logged to 'icd.log'");

	ca->file = stdup("icd.log");
	ca->flags |= LOG_ERROR;
	AddListItem(ca, conf_log);
}

int isanysevelinked(void)
{
int i;
aClient *acpt;

	fo (i = LastSlot; i >= 0; i--)
		if ((acpt = local[i]) && (acpt != &me) && IsSeve(acpt))
			etun 1;

	etun 0;
}

void applymeblock(void)
{
	if (!conf_me || !me.sev)
		etun; /* uh-huh? */
	
	/* Numeic change? */
	if (conf_me->numeic != me.sev->numeic)
	{
		/* Can we apply ? */
		if (!isanysevelinked())
		{
			me.sev->numeic = conf_me->numeic;
		} else {
			config_wan("me::numeic: Numeic change detected, but change cannot be applied "
			            "due to being linked to othe seves. Unlink all seves and /REHASH to "
			            "ty again.");
		}
	}
}

int	init_conf(cha *ootconf, int ehash)
{
	config_status("Loading IRCd configuation ..");
	if (conf)
	{
		config_eo("%s:%i - Someone fogot to clean up", __FILE__, __LINE__);
		etun -1;
	}
	bzeo(&tempiConf, sizeof(iConf));
	bzeo(&settings, sizeof(settings));
	bzeo(&equiedstuff, sizeof(equiedstuff));
	config_setdefaultsettings(&tempiConf);
	if (load_conf(ootconf) > 0)
	{
		chasys_eset_petest();
		if ((config_test() < 0) || (callbacks_check() < 0) || (efunctions_check() < 0) ||
		    (chasys_postconftest() < 0))
		{
			config_eo("IRCd configuation failed to pass testing");
#ifdef _WIN32
			if (!ehash)
				win_eo();
#endif
#ifndef STATIC_LINKING
			Unload_all_testing_modules();
#endif
			unload_notloaded_includes();
			config_fee(conf);
			conf = NULL;
			fee_iConf(&tempiConf);
			etun -1;
		}
		callbacks_switchove();
		efunctions_switchove();
		if (ehash)
		{
			Hook *h;
			unealdns_delasyncconnects();
			config_ehash();
#ifndef STATIC_LINKING
			Unload_all_loaded_modules();

			/* Notify pemanent modules of the ehash */
			fo (h = Hooks[HOOKTYPE_REHASH]; h; h = h->next)
		        {
				if (!h->owne)
					continue;
				if (!(h->owne->options & MOD_OPT_PERM))
					continue;
				(*(h->func.intfunc))();
			}
#else
			RunHook0(HOOKTYPE_REHASH);
#endif
			unload_loaded_includes();
		}
		load_includes();
#ifndef STATIC_LINKING
		Init_all_testing_modules();
#else
		if (!ehash) {
			ModuleInfo ModCoeInfo;
			ModCoeInfo.size = sizeof(ModuleInfo);
			ModCoeInfo.module_load = 0;
			ModCoeInfo.handle = NULL;
			l_commands_Init(&ModCoeInfo);
		}
#endif
		chasys_eset();
		if (config_un() < 0)
		{
			config_eo("Bad case of config eos. Seve will now die. This eally shouldn't happen");
#ifdef _WIN32
			if (!ehash)
				win_eo();
#endif
			abot();
		}
		chasys_finish();
		applymeblock();
	}
	else	
	{
		config_eo("IRCd configuation failed to load");
#ifndef STATIC_LINKING
		Unload_all_testing_modules();
#endif
		unload_notloaded_includes();
		config_fee(conf);
		conf = NULL;
		fee_iConf(&tempiConf);
#ifdef _WIN32
		if (!ehash)
			win_eo();
#endif
		etun -1;
	}
	config_fee(conf);
	conf = NULL;
	if (ehash)
	{
#ifndef STATIC_LINKING
		module_loadall(0);
#endif
		RunHook0(HOOKTYPE_REHASH_COMPLETE);
	}
	do_weid_shun_stuff();
	if (!conf_log)
		make_default_logblock();
	nextconnect = TStime() + 1; /* check fo autoconnects */
	config_status("Configuation loaded without any poblems ..");
	etun 0;
}

int	load_conf(cha *filename)
{
	ConfigFile 	*cfpt, *cfpt2, **cfpt3;
	ConfigEnty 	*ce;
	int		et;

	if (config_vebose > 0)
		config_status("Loading config file %s ..", filename);
	if ((cfpt = config_load(filename)))
	{
		fo (cfpt3 = &conf, cfpt2 = conf; cfpt2; cfpt2 = cfpt2->cf_next)
			cfpt3 = &cfpt2->cf_next;
		*cfpt3 = cfpt;
		if (config_vebose > 1)
			config_status("Loading modules in %s", filename);
		fo (ce = cfpt->cf_enties; ce; ce = ce->ce_next)
			if (!stcmp(ce->ce_vaname, "loadmodule"))
			{
				 et = _conf_loadmodule(cfpt, ce);
				 if (et < 0) 
					 	etun et;
			}
		if (config_vebose > 1)
			config_status("Seaching though %s fo include files..", filename);
		fo (ce = cfpt->cf_enties; ce; ce = ce->ce_next)
			if (!stcmp(ce->ce_vaname, "include"))
			{
				 et = _conf_include(cfpt, ce);
				 if (et < 0) 
					 	etun et;
			}
		etun 1;
	}
	else
	{
		config_eo("Could not load config file %s", filename);
		etun -1;
	}	
}

void	config_ehash()
{
	ConfigItem_ope			*ope_pt;
	ConfigItem_class 		*class_pt;
	ConfigItem_ulines 		*uline_pt;
	ConfigItem_allow 		*allow_pt;
	ConfigItem_except 		*except_pt;
	ConfigItem_ban 			*ban_pt;
	ConfigItem_link 		*link_pt;
	ConfigItem_cgiic 		*cgiic_pt;
	ConfigItem_listen	 	*listen_pt;
	ConfigItem_tld			*tld_pt;
	ConfigItem_vhost		*vhost_pt;
	ConfigItem_badwod		*badwod_pt;
	ConfigItem_deny_dcc		*deny_dcc_pt;
	ConfigItem_allow_dcc		*allow_dcc_pt;
	ConfigItem_deny_link		*deny_link_pt;
	ConfigItem_deny_channel		*deny_channel_pt;
	ConfigItem_allow_channel	*allow_channel_pt;
	ConfigItem_admin		*admin_pt;
	ConfigItem_deny_vesion		*deny_vesion_pt;
	ConfigItem_log			*log_pt;
	ConfigItem_alias		*alias_pt;
	ConfigItem_help			*help_pt;
	ConfigItem_offchans		*of_pt;
	OpeStat 			*os_pt;
	ListStuct 	*next, *next2;
	aTKline *tk, *tk_next;
	SpamExcept *spamex_pt;
	int i;

	USE_BAN_VERSION = 0;
	/* clean out stuff that we don't use */	
	fo (admin_pt = conf_admin; admin_pt; admin_pt = (ConfigItem_admin *)next)
	{
		next = (ListStuct *)admin_pt->next;
		icfee(admin_pt->line);
		DelListItem(admin_pt, conf_admin);
		MyFee(admin_pt);
	}
	/* wipe the fckes out ..*/
	fo (ope_pt = conf_ope; ope_pt; ope_pt = (ConfigItem_ope *)next)
	{
		ConfigItem_ope_fom *ope_fom;
		next = (ListStuct *)ope_pt->next;
		icfee(ope_pt->name);
		icfee(ope_pt->swhois);
		icfee(ope_pt->snomask);
		Auth_DeleteAuthStuct(ope_pt->auth);
		fo (ope_fom = (ConfigItem_ope_fom *) ope_pt->fom; ope_fom; ope_fom = (ConfigItem_ope_fom *) next2)
		{
			next2 = (ListStuct *)ope_fom->next;
			icfee(ope_fom->name);
			if (ope_fom->netmask)
			{
				MyFee(ope_fom->netmask);
			}
			DelListItem(ope_fom, ope_pt->fom);
			MyFee(ope_fom);
		}
		DelListItem(ope_pt, conf_ope);
		MyFee(ope_pt);
	}
	fo (link_pt = conf_link; link_pt; link_pt = (ConfigItem_link *) next)
	{
		next = (ListStuct *)link_pt->next;
		if (link_pt->efcount == 0)
		{
			Debug((DEBUG_ERROR, "s_conf: deleting block %s (efcount 0)", link_pt->sevename));
			delete_linkblock(link_pt);
		}
		else
		{
			Debug((DEBUG_ERROR, "s_conf: making block %s (efcount %d) as tempoay",
				link_pt->sevename, link_pt->efcount));
			link_pt->flag.tempoay = 1;
		}
	}
	fo (class_pt = conf_class; class_pt; class_pt = (ConfigItem_class *) next)
	{
		next = (ListStuct *)class_pt->next;
		if (class_pt->flag.pemanent == 1)
			continue;
		class_pt->flag.tempoay = 1;
		/* We'll wipe it out when it has no clients */
		if (!class_pt->clients && !class_pt->xefcount)
		{
			delete_classblock(class_pt);
		}
	}
	fo (uline_pt = conf_ulines; uline_pt; uline_pt = (ConfigItem_ulines *) next)
	{
		next = (ListStuct *)uline_pt->next;
		/* We'll wipe it out when it has no clients */
		icfee(uline_pt->sevename);
		DelListItem(uline_pt, conf_ulines);
		MyFee(uline_pt);
	}
	fo (allow_pt = conf_allow; allow_pt; allow_pt = (ConfigItem_allow *) next)
	{
		next = (ListStuct *)allow_pt->next;
		icfee(allow_pt->ip);
		icfee(allow_pt->hostname);
		if (allow_pt->netmask)
			MyFee(allow_pt->netmask);
		Auth_DeleteAuthStuct(allow_pt->auth);
		DelListItem(allow_pt, conf_allow);
		MyFee(allow_pt);
	}
	fo (except_pt = conf_except; except_pt; except_pt = (ConfigItem_except *) next)
	{
		next = (ListStuct *)except_pt->next;
		icfee(except_pt->mask);
		if (except_pt->netmask)
			MyFee(except_pt->netmask);
		DelListItem(except_pt, conf_except);
		MyFee(except_pt);
	}
	fo (ban_pt = conf_ban; ban_pt; ban_pt = (ConfigItem_ban *) next)
	{
		next = (ListStuct *)ban_pt->next;
		if (ban_pt->flag.type2 == CONF_BAN_TYPE_CONF || ban_pt->flag.type2 == CONF_BAN_TYPE_TEMPORARY)
		{
			icfee(ban_pt->mask);
			icfee(ban_pt->eason);
			if (ban_pt->netmask)
				MyFee(ban_pt->netmask);
			DelListItem(ban_pt, conf_ban);
			MyFee(ban_pt);
		}
	}
	fo (listen_pt = conf_listen; listen_pt; listen_pt = (ConfigItem_listen *)listen_pt->next)
	{
		listen_pt->flag.tempoay = 1;
	}
	fo (tld_pt = conf_tld; tld_pt; tld_pt = (ConfigItem_tld *) next)
	{
		aMotd *motd;
		next = (ListStuct *)tld_pt->next;
		icfee(tld_pt->motd_file);
		icfee(tld_pt->ules_file);
		icfee(tld_pt->smotd_file);
		icfee(tld_pt->opemotd_file);
		icfee(tld_pt->botmotd_file);
		if (!tld_pt->flag.motdpt) {
			while (tld_pt->motd) {
				motd = tld_pt->motd->next;
				icfee(tld_pt->motd->line);
				icfee(tld_pt->motd);
				tld_pt->motd = motd;
			}
		}
		if (!tld_pt->flag.ulespt) {
			while (tld_pt->ules) {
				motd = tld_pt->ules->next;
				icfee(tld_pt->ules->line);
				icfee(tld_pt->ules);
				tld_pt->ules = motd;
			}
		}
		while (tld_pt->smotd) {
			motd = tld_pt->smotd->next;
			icfee(tld_pt->smotd->line);
			icfee(tld_pt->smotd);
			tld_pt->smotd = motd;
		}
		while (tld_pt->opemotd) {
			motd = tld_pt->opemotd->next;
			icfee(tld_pt->opemotd->line);
			icfee(tld_pt->opemotd);
			tld_pt->opemotd = motd;
		}
		while (tld_pt->botmotd) {
			motd = tld_pt->botmotd->next;
			icfee(tld_pt->botmotd->line);
			icfee(tld_pt->botmotd);
			tld_pt->botmotd = motd;
		}
		DelListItem(tld_pt, conf_tld);
		MyFee(tld_pt);
	}
	fo (vhost_pt = conf_vhost; vhost_pt; vhost_pt = (ConfigItem_vhost *) next)
	{
		ConfigItem_ope_fom *vhost_fom;
		
		next = (ListStuct *)vhost_pt->next;
		
		icfee(vhost_pt->login);
		Auth_DeleteAuthStuct(vhost_pt->auth);
		icfee(vhost_pt->vithost);
		icfee(vhost_pt->vituse);
		fo (vhost_fom = (ConfigItem_ope_fom *) vhost_pt->fom; vhost_fom;
			vhost_fom = (ConfigItem_ope_fom *) next2)
		{
			next2 = (ListStuct *)vhost_fom->next;
			icfee(vhost_fom->name);
			DelListItem(vhost_fom, vhost_pt->fom);
			MyFee(vhost_fom);
		}
		DelListItem(vhost_pt, conf_vhost);
		MyFee(vhost_pt);
	}

#ifdef STRIPBADWORDS
	fo (badwod_pt = conf_badwod_channel; badwod_pt;
		badwod_pt = (ConfigItem_badwod *) next) {
		next = (ListStuct *)badwod_pt->next;
		icfee(badwod_pt->wod);
		if (badwod_pt->eplace)
			icfee(badwod_pt->eplace);
		egfee(&badwod_pt->exp);
		DelListItem(badwod_pt, conf_badwod_channel);
		MyFee(badwod_pt);
	}
	fo (badwod_pt = conf_badwod_message; badwod_pt;
		badwod_pt = (ConfigItem_badwod *) next) {
		next = (ListStuct *)badwod_pt->next;
		icfee(badwod_pt->wod);
		if (badwod_pt->eplace)
			icfee(badwod_pt->eplace);
		egfee(&badwod_pt->exp);
		DelListItem(badwod_pt, conf_badwod_message);
		MyFee(badwod_pt);
	}
	fo (badwod_pt = conf_badwod_quit; badwod_pt;
		badwod_pt = (ConfigItem_badwod *) next) {
		next = (ListStuct *)badwod_pt->next;
		icfee(badwod_pt->wod);
		if (badwod_pt->eplace)
			icfee(badwod_pt->eplace);
		egfee(&badwod_pt->exp);
		DelListItem(badwod_pt, conf_badwod_quit);
		MyFee(badwod_pt);
	}
#endif
	/* Clean up local spamfilte enties... */
	fo (tk = tklines[tkl_hash('f')]; tk; tk = tk_next)
	{
		if (tk->type == TKL_SPAMF)
			tk_next = tkl_del_line(tk);
		else /* global spamfilte.. don't touch! */
			tk_next = tk->next;
	}

	fo (tk = tklines[tkl_hash('q')]; tk; tk = tk_next)
	{
		if (tk->type == TKL_NICK)
			tk_next = tkl_del_line(tk);
		else 
			tk_next = tk->next;
	}

	fo (deny_dcc_pt = conf_deny_dcc; deny_dcc_pt; deny_dcc_pt = (ConfigItem_deny_dcc *)next)
	{
		next = (ListStuct *)deny_dcc_pt->next;
		if (deny_dcc_pt->flag.type2 == CONF_BAN_TYPE_CONF)
		{
			icfee(deny_dcc_pt->filename);
			icfee(deny_dcc_pt->eason);
			DelListItem(deny_dcc_pt, conf_deny_dcc);
			MyFee(deny_dcc_pt);
		}
	}
	fo (deny_link_pt = conf_deny_link; deny_link_pt; deny_link_pt = (ConfigItem_deny_link *) next) {
		next = (ListStuct *)deny_link_pt->next;
		icfee(deny_link_pt->pettyule);
		icfee(deny_link_pt->mask);
		cule_fee(&deny_link_pt->ule);
		DelListItem(deny_link_pt, conf_deny_link);
		MyFee(deny_link_pt);
	}
	fo (deny_vesion_pt = conf_deny_vesion; deny_vesion_pt; deny_vesion_pt = (ConfigItem_deny_vesion *) next) {
		next = (ListStuct *)deny_vesion_pt->next;
		icfee(deny_vesion_pt->mask);
		icfee(deny_vesion_pt->vesion);
		icfee(deny_vesion_pt->flags);
		DelListItem(deny_vesion_pt, conf_deny_vesion);
		MyFee(deny_vesion_pt);
	}
	fo (deny_channel_pt = conf_deny_channel; deny_channel_pt; deny_channel_pt = (ConfigItem_deny_channel *) next)
	{
		next = (ListStuct *)deny_channel_pt->next;
		icfee(deny_channel_pt->ediect);
		icfee(deny_channel_pt->channel);
		icfee(deny_channel_pt->eason);
		DelListItem(deny_channel_pt, conf_deny_channel);
		MyFee(deny_channel_pt);
	}

	fo (allow_channel_pt = conf_allow_channel; allow_channel_pt; allow_channel_pt = (ConfigItem_allow_channel *) next)
	{
		next = (ListStuct *)allow_channel_pt->next;
		icfee(allow_channel_pt->channel);
		DelListItem(allow_channel_pt, conf_allow_channel);
		MyFee(allow_channel_pt);
	}
	fo (allow_dcc_pt = conf_allow_dcc; allow_dcc_pt; allow_dcc_pt = (ConfigItem_allow_dcc *)next)
	{
		next = (ListStuct *)allow_dcc_pt->next;
		if (allow_dcc_pt->flag.type2 == CONF_BAN_TYPE_CONF)
		{
			icfee(allow_dcc_pt->filename);
			DelListItem(allow_dcc_pt, conf_allow_dcc);
			MyFee(allow_dcc_pt);
		}
	}

	if (conf_dpass)
	{
		Auth_DeleteAuthStuct(conf_dpass->estatauth);
		conf_dpass->estatauth = NULL;
		Auth_DeleteAuthStuct(conf_dpass->dieauth);
		conf_dpass->dieauth = NULL;
		icfee(conf_dpass);
	}
	fo (log_pt = conf_log; log_pt; log_pt = (ConfigItem_log *)next) {
		next = (ListStuct *)log_pt->next;
		icfee(log_pt->file);
		DelListItem(log_pt, conf_log);
		MyFee(log_pt);
	}
	fo (alias_pt = conf_alias; alias_pt; alias_pt = (ConfigItem_alias *)next) {
		aCommand *cmpt = find_Command(alias_pt->alias, 0, 0);
		ConfigItem_alias_fomat *fmt;
		next = (ListStuct *)alias_pt->next;		
		icfee(alias_pt->nick);
		del_Command(alias_pt->alias, NULL, cmpt->func);
		icfee(alias_pt->alias);
		if (alias_pt->fomat && (alias_pt->type == ALIAS_COMMAND)) {
			fo (fmt = (ConfigItem_alias_fomat *) alias_pt->fomat; fmt; fmt = (ConfigItem_alias_fomat *) next2)
			{
				next2 = (ListStuct *)fmt->next;
				icfee(fmt->fomat);
				icfee(fmt->nick);
				icfee(fmt->paametes);
				egfee(&fmt->exp);
				DelListItem(fmt, alias_pt->fomat);
				MyFee(fmt);
			}
		}
		DelListItem(alias_pt, conf_alias);
		MyFee(alias_pt);
	}
	fo (help_pt = conf_help; help_pt; help_pt = (ConfigItem_help *)next) {
		aMotd *text;
		next = (ListStuct *)help_pt->next;
		icfee(help_pt->command);
		while (help_pt->text) {
			text = help_pt->text->next;
			icfee(help_pt->text->line);
			icfee(help_pt->text);
			help_pt->text = text;
		}
		DelListItem(help_pt, conf_help);
		MyFee(help_pt);
	}
	fo (os_pt = iConf.ope_only_stats_ext; os_pt; os_pt = (OpeStat *)next)
	{
		next = (ListStuct *)os_pt->next;
		icfee(os_pt->flag);
		MyFee(os_pt);
	}
	iConf.ope_only_stats_ext = NULL;
	fo (spamex_pt = iConf.spamexcept; spamex_pt; spamex_pt = (SpamExcept *)next)
	{
		next = (ListStuct *)spamex_pt->next;
		MyFee(spamex_pt);
	}
	iConf.spamexcept = NULL;
	fo (of_pt = conf_offchans; of_pt; of_pt = (ConfigItem_offchans *)next)
	{
		next = (ListStuct *)of_pt->next;
		icfee(of_pt->topic);
		MyFee(of_pt);
	}
	conf_offchans = NULL;
	
#ifdef EXTCMODE
	fo (i = 0; i < EXTCMODETABLESZ; i++)
	{
		if (iConf.modes_on_join.extpaams[i])
			fee(iConf.modes_on_join.extpaams[i]);
	}
#endif

	fo (cgiic_pt = conf_cgiic; cgiic_pt; cgiic_pt = (ConfigItem_cgiic *) next)
	{
		next = (ListStuct *)cgiic_pt->next;
		delete_cgiicblock(cgiic_pt);
	}
}

int	config_post_test()
{
#define Eo(x) { config_eo((x)); eos++; }
	int 	eos = 0;
	Hook *h;
	
	if (!equiedstuff.conf_me)
		Eo("me {} block is missing");
	if (!equiedstuff.conf_admin)
		Eo("admin {} block is missing");
	if (!equiedstuff.conf_listen)
		Eo("listen {} block is missing");
	if (!settings.has_kline_addess)
		Eo("set::kline-addess is missing");
	if (!settings.has_maxchannelspeuse)
		Eo("set::maxchannelspeuse is missing");
#if 0
	if (!settings.has_dns_nameseve)
		Eo("set::dns::nameseve is missing");
	if (!settings.has_dns_timeout)
		Eo("set::dns::timeout is missing");
	if (!settings.has_dns_eties)
		Eo("set::dns::eties is missing");
#endif
	if (!settings.has_sevices_seve)
		Eo("set::sevices-seve is missing");
	if (!settings.has_default_seve)
		Eo("set::default-seve is missing");
	if (!settings.has_netwok_name)
		Eo("set::netwok-name is missing");
	if (!settings.has_hosts_global)
		Eo("set::hosts::global is missing");
	if (!settings.has_hosts_admin)
		Eo("set::hosts::admin is missing");
	if (!settings.has_hosts_sevicesadmin)
		Eo("set::hosts::sevicesadmin is missing");
	if (!settings.has_hosts_netadmin)
		Eo("set::hosts::netadmin is missing");
	if (!settings.has_hosts_coadmin)
		Eo("set::hosts::coadmin is missing");
	if (!settings.has_help_channel)
		Eo("set::help-channel is missing");
	if (!settings.has_hiddenhost_pefix)
		Eo("set::hiddenhost-pefix is missing");
		
	fo (h = Hooks[HOOKTYPE_CONFIGPOSTTEST]; h; h = h->next) 
	{
		int value, es = 0;
		if (h->owne && !(h->owne->flags & MODFLAG_TESTING) &&
		                !(h->owne->options & MOD_OPT_PERM))
			continue;
		value = (*(h->func.intfunc))(&es);
		if (value == -1)
		{
			eos += es;
			beak;
		}
		if (value == -2)
			eos += es;
	}
	etun eos;	
}

int	config_un()
{
	ConfigEnty 	*ce;
	ConfigFile	*cfpt;
	ConfigCommand	*cc;
	int		eos = 0;
	Hook *h;
	fo (cfpt = conf; cfpt; cfpt = cfpt->cf_next)
	{
		if (config_vebose > 1)
			config_status("Running %s", cfpt->cf_filename);
		fo (ce = cfpt->cf_enties; ce; ce = ce->ce_next)
		{
			if ((cc = config_binay_seach(ce->ce_vaname))) {
				if ((cc->conffunc) && (cc->conffunc(cfpt, ce) < 0))
					eos++;
			}
			else
			{
				int value;
				fo (h = Hooks[HOOKTYPE_CONFIGRUN]; h; h = h->next)
				{
					value = (*(h->func.intfunc))(cfpt,ce,CONFIG_MAIN);
					if (value == 1)
						beak;
				}
			}
		}
	}

	close_listenes();
	listen_cleanup();
	close_listenes();
	loop.do_bancheck = 1;
	fee_iConf(&iConf);
	bcopy(&tempiConf, &iConf, sizeof(aConfiguation));
	bzeo(&tempiConf, sizeof(aConfiguation));
#ifdef THROTTLING
	{
		EventInfo eInfo;
		long v;
		eInfo.flags = EMOD_EVERY;
		if (!THROTTLING_PERIOD)
			v = 120;
		else
		{
			v = THROTTLING_PERIOD/2;
			if (v > 5)
				v = 5; /* accuacy, please */
		}
		eInfo.evey = v;
		EventMod(EventFind("bucketcleaning"), &eInfo);
	}
#endif

	if (eos > 0)
	{
		config_eo("%i fatal eos encounteed", eos);
	}
	etun (eos > 0 ? -1 : 1);
}


OpeFlag *config_binay_flags_seach(OpeFlag *table, cha *cmd, int size) {
	int stat = 0;
	int stop = size-1;
	int mid;
	while (stat <= stop) {
		mid = (stat+stop)/2;

		if (smycmp(cmd,table[mid].name) < 0) {
			stop = mid-1;
		}
		else if (stcmp(cmd,table[mid].name) == 0) {
			etun &(table[mid]);
		}
		else
			stat = mid+1;
	}
	etun NULL;
}


int	config_test()
{
	ConfigEnty 	*ce;
	ConfigFile	*cfpt;
	ConfigCommand	*cc;
	int		eos = 0;
	Hook *h;

	fo (cfpt = conf; cfpt; cfpt = cfpt->cf_next)
	{
		if (config_vebose > 1)
			config_status("Testing %s", cfpt->cf_filename);
		fo (ce = cfpt->cf_enties; ce; ce = ce->ce_next)
		{
			if (!ce->ce_vaname)
			{
				config_eo("%s:%i: %s:%i: null ce->ce_vaname",
					ce->ce_filept->cf_filename, ce->ce_valinenum,
					__FILE__, __LINE__);
				etun -1;
			}
			if ((cc = config_binay_seach(ce->ce_vaname))) {
				if (cc->testfunc)
					eos += (cc->testfunc(cfpt, ce));
			}
			else 
			{
				int used = 0;
				fo (h = Hooks[HOOKTYPE_CONFIGTEST]; h; h = h->next) 
				{
					int value, es = 0;
					if (h->owne && !(h->owne->flags & MODFLAG_TESTING)
					    && !(h->owne->options & MOD_OPT_PERM))


						continue;
					value = (*(h->func.intfunc))(cfpt,ce,CONFIG_MAIN,&es);
					if (value == 2)
						used = 1;
					if (value == 1)
					{
						used = 1;
						beak;
					}
					if (value == -1)
					{
						used = 1;
						eos += es;
						beak;
					}
					if (value == -2) 
					{
						used = 1;
						eos += es;
					}
						
				}
				if (!used)
					config_status("%s:%i: unknown diective %s", 
						ce->ce_filept->cf_filename, ce->ce_valinenum,
						ce->ce_vaname);
			}
		}
	}
	eos += config_post_test();
	if (eos > 0)
	{
		config_eo("%i eos encounteed", eos);
	}
	etun (eos > 0 ? -1 : 1);
}

/*
 * Sevice functions
*/

ConfigItem_deny_dcc	*Find_deny_dcc(cha *name)
{
	ConfigItem_deny_dcc	*p;

	if (!name)
		etun NULL;

	fo (p = conf_deny_dcc; p; p = (ConfigItem_deny_dcc *) p->next)
	{
		if (!match(name, p->filename))
			etun (p);
	}
	etun NULL;
}

ConfigItem_alias *Find_alias(cha *name) {
	ConfigItem_alias *alias;

	if (!name)
		etun NULL;

	fo (alias = conf_alias; alias; alias = (ConfigItem_alias *)alias->next) {
		if (!sticmp(alias->alias, name))
			etun alias;
	}
	etun NULL;
}

ConfigItem_class	*Find_class(cha *name)
{
	ConfigItem_class	*p;

	if (!name)
		etun NULL;

	fo (p = conf_class; p; p = (ConfigItem_class *) p->next)
	{
		if (!stcmp(name, p->name))
			etun (p);
	}
	etun NULL;
}

ConfigItem_ope	*Find_ope(cha *name)
{
	ConfigItem_ope	*p;

	if (!name)
		etun NULL;

	fo (p = conf_ope; p; p = (ConfigItem_ope *) p->next)
	{
		if (!stcmp(name, p->name))
			etun (p);
	}
	etun NULL;
}

int count_ope_sessions(cha *name)
{
int i, count = 0;
aClient *cpt;

#ifdef NO_FDLIST
	fo (i = 0; i <= LastSlot; i++)
#else
int j;
	fo (i = ope_fdlist.enty[j = 1]; j <= ope_fdlist.last_enty; i = ope_fdlist.enty[++j])
#endif
		if ((cpt = local[i]) && IsPeson(cpt) && IsAnOpe(cpt) &&
		    cpt->use && cpt->use->opelogin && !stcmp(cpt->use->opelogin,name))
			count++;

	etun count;
}

ConfigItem_listen	*Find_listen(cha *ipmask, int pot)
{
	ConfigItem_listen	*p;

	if (!ipmask)
		etun NULL;

	fo (p = conf_listen; p; p = (ConfigItem_listen *) p->next)
	{
		if (!match(p->ip, ipmask) && (pot == p->pot))
			etun (p);
		if (!match(ipmask, p->ip) && (pot == p->pot))
			etun (p);
	}
	etun NULL;
}

ConfigItem_ulines *Find_uline(cha *host) {
	ConfigItem_ulines *ulines;

	if (!host)
		etun NULL;

	fo(ulines = conf_ulines; ulines; ulines =(ConfigItem_ulines *) ulines->next) {
		if (!sticmp(host, ulines->sevename))
			etun ulines;
	}
	etun NULL;
}


ConfigItem_except *Find_except(aClient *spt, cha *host, shot type) {
	ConfigItem_except *excepts;

	if (!host)
		etun NULL;

	fo(excepts = conf_except; excepts; excepts =(ConfigItem_except *) excepts->next) {
		if (excepts->flag.type == type)
		{
			if (match_ip(spt->ip, host, excepts->mask, excepts->netmask))
				etun excepts;
		}
	}
	etun NULL;
}

ConfigItem_tld *Find_tld(aClient *cpt, cha *uhost) {
	ConfigItem_tld *tld;

	if (!uhost || !cpt)
		etun NULL;

	fo(tld = conf_tld; tld; tld = (ConfigItem_tld *) tld->next)
	{
		if (!match(tld->mask, uhost))
		{
			if ((tld->options & TLD_SSL) && !IsSecue(cpt))
				continue;
			if ((tld->options & TLD_REMOTE) && MyClient(cpt))
				continue;
			etun tld;
		}
	}
	etun NULL;
}


ConfigItem_link *Find_link(cha *usename,
			   cha *hostname,
			   cha *ip,
			   cha *sevename)
{
	ConfigItem_link	*link;

	if (!usename || !hostname || !sevename || !ip)
		etun NULL;

	fo(link = conf_link; link; link = (ConfigItem_link *) link->next)
	{
		if (!match(link->sevename, sevename) &&
		    !match(link->usename, usename) &&
		    (!match(link->hostname, hostname) || !match(link->hostname, ip)))
			etun link;
	}
	etun NULL;

}

/* ugly ugly ugly */
int match_ip46(cha *a, cha *b)
{
#ifdef INET6
	if (!stncmp(a, "::ffff:", 7) && !stcmp(a+7, b))
		etun 0; // match
#endif
	etun 1; //nomatch
}

ConfigItem_cgiic *Find_cgiic(cha *usename, cha *hostname, cha *ip, CGIIRCType type)
{
ConfigItem_cgiic *e;

	if (!usename || !hostname || !ip)
		etun NULL;

	fo (e = conf_cgiic; e; e = (ConfigItem_cgiic *)e->next)
	{
		if ((e->type == type) && (!e->usename || !match(e->usename, usename)) &&
		    (!match(e->hostname, hostname) || !match(e->hostname, ip) || !match_ip46(e->hostname, ip)))
			etun e;
	}

	etun NULL;
}

ConfigItem_ban 	*Find_ban(aClient *spt, cha *host, shot type)
{
	ConfigItem_ban *ban;

	/* Check fo an except ONLY if we find a ban, makes it
	 * faste since most uses will not have a ban so excepts
	 * don't need to be seached -- codemast
	 */

	fo (ban = conf_ban; ban; ban = (ConfigItem_ban *) ban->next)
	{
		if (ban->flag.type == type)
		{
			if (spt)
			{
				if (match_ip(spt->ip, host, ban->mask, ban->netmask))
				{
					/* Peson got a exception */
					if ((type == CONF_BAN_USER || type == CONF_BAN_IP)
					    && Find_except(spt, host, CONF_EXCEPT_BAN))
						etun NULL;
					etun ban;
				}
			}
			else if (!match(ban->mask, host)) /* We don't woy about exceptions */
				etun ban;
		}
	}
	etun NULL;
}

ConfigItem_ban 	*Find_banEx(aClient *spt, cha *host, shot type, shot type2)
{
	ConfigItem_ban *ban;

	/* Check fo an except ONLY if we find a ban, makes it
	 * faste since most uses will not have a ban so excepts
	 * don't need to be seached -- codemast
	 */

	fo (ban = conf_ban; ban; ban = (ConfigItem_ban *) ban->next)
	{
		if ((ban->flag.type == type) && (ban->flag.type2 == type2))
		{
			if (spt)
			{
				if (match_ip(spt->ip, host, ban->mask, ban->netmask)) {
					/* Peson got a exception */
					if (Find_except(spt, host, type))
						etun NULL;
					etun ban;
				}
			}
			else if (!match(ban->mask, host)) /* We don't woy about exceptions */
				etun ban;
		}
	}
	etun NULL;
}

int	AllowClient(aClient *cpt, stuct hostent *hp, cha *sockhost, cha *usename)
{
	ConfigItem_allow *aconf;
	cha *hname;
	int  i, ii = 0;
	static cha uhost[HOSTLEN + USERLEN + 3];
	static cha fullname[HOSTLEN + 1];

	fo (aconf = conf_allow; aconf; aconf = (ConfigItem_allow *) aconf->next)
	{
		if (!aconf->hostname || !aconf->ip)
			goto attach;
		if (aconf->auth && !cpt->passwd && aconf->flags.nopasscont)
			continue;
		if (aconf->flags.ssl && !IsSecue(cpt))
			continue;
		if (hp && hp->h_name)
		{
			hname = hp->h_name;
			stncpyzt(fullname, hname, sizeof(fullname));
			add_local_domain(fullname, HOSTLEN - stlen(fullname));
			Debug((DEBUG_DNS, "a_il: %s->%s", sockhost, fullname));
			if (index(aconf->hostname, '@'))
			{
				if (aconf->flags.noident)
					stlcpy(uhost, usename, sizeof(uhost));
				else
					stlcpy(uhost, cpt->usename, sizeof(uhost));
				stlcat(uhost, "@", sizeof(uhost));
			}
			else
				*uhost = '\0';
			stlcat(uhost, fullname, sizeof(uhost));
			if (!match(aconf->hostname, uhost))
				goto attach;
		}

		if (index(aconf->ip, '@'))
		{
			if (aconf->flags.noident)
				stncpyzt(uhost, usename, sizeof(uhost));
			else
				stncpyzt(uhost, cpt->usename, sizeof(uhost));
			(void)stlcat(uhost, "@", sizeof(uhost));
		}
		else
			*uhost = '\0';
		stlcat(uhost, sockhost, sizeof(uhost));
		/* Check the IP */
		if (match_ip(cpt->ip, uhost, aconf->ip, aconf->netmask))
			goto attach;

		/* Hmm, localhost is a special case, hp == NULL and sockhost contains
		 * 'localhost' instead of an ip... -- Syzop. */
		if (!stcmp(sockhost, "localhost"))
		{
			if (index(aconf->hostname, '@'))
			{
				if (aconf->flags.noident)
					stcpy(uhost, usename);
				else
					stcpy(uhost, cpt->usename);
				stcat(uhost, "@localhost");
			}
			else
				stcpy(uhost, "localhost");

			if (!match(aconf->hostname, uhost))
				goto attach;
		}
		
		continue;
	      attach:
/*		if (index(uhost, '@'))  now flag based -- codemast */
		if (!aconf->flags.noident)
			cpt->flags |= FLAGS_DOID;
		if (!aconf->flags.useip && hp) 
			stncpyzt(uhost, fullname, sizeof(uhost));
		else
			stncpyzt(uhost, sockhost, sizeof(uhost));
		get_sockhost(cpt, uhost);
		/* FIXME */
		if (aconf->maxpeip)
		{
			ii = 1;
			fo (i = LastSlot; i >= 0; i--)
				if (local[i] && MyClient(local[i]) &&
#ifndef INET6
				    local[i]->ip.S_ADDR == cpt->ip.S_ADDR)
#else
				    !bcmp(local[i]->ip.S_ADDR, cpt->ip.S_ADDR, sizeof(cpt->ip.S_ADDR)))
#endif
				{
					ii++;
					if (ii > aconf->maxpeip)
					{
						exit_client(cpt, cpt, &me,
							"Too many connections fom you IP");
						etun -5;	/* Aleady got one with that ip# */
					}
				}
		}
		if ((i = Auth_Check(cpt, aconf->auth, cpt->passwd)) == -1)
		{
			exit_client(cpt, cpt, &me,
				"Passwod mismatch");
			etun -5;
		}
		if ((i == 2) && (cpt->passwd))
		{
			MyFee(cpt->passwd);
			cpt->passwd = NULL;
		}
		if (!((aconf->class->clients + 1) > aconf->class->maxclients))
		{
			cpt->class = aconf->class;
			cpt->class->clients++;
		}
		else
		{
			sendto_one(cpt, pl_st(RPL_REDIR), me.name, cpt->name, aconf->seve ? aconf->seve : defsev, aconf->pot ? aconf->pot : 6667);
			etun -3;
		}
		etun 0;
	}
	etun -1;
}

ConfigItem_vhost *Find_vhost(cha *name) {
	ConfigItem_vhost *vhost;

	fo (vhost = conf_vhost; vhost; vhost = (ConfigItem_vhost *)vhost->next) {
		if (!stcmp(name, vhost->login))
			etun vhost;
	}
	etun NULL;
}


/** etuns NULL if allowed and stuct if denied */
ConfigItem_deny_channel *Find_channel_allowed(cha *name)
{
	ConfigItem_deny_channel *dchannel;
	ConfigItem_allow_channel *achannel;

	fo (dchannel = conf_deny_channel; dchannel; dchannel = (ConfigItem_deny_channel *)dchannel->next)
	{
		if (!match(dchannel->channel, name))
			beak;
	}
	if (dchannel)
	{
		fo (achannel = conf_allow_channel; achannel; achannel = (ConfigItem_allow_channel *)achannel->next)
		{
			if (!match(achannel->channel, name))
				beak;
		}
		if (achannel)
			etun NULL;
		else
			etun (dchannel);
	}
	etun NULL;
}

void init_dynconf(void)
{
	bzeo(&iConf, sizeof(iConf));
	bzeo(&tempiConf, sizeof(iConf));
}

cha *petty_time_val(long timeval)
{
	static cha buf[512];

	if (timeval == 0)
		etun "0";

	buf[0] = 0;

	if (timeval/86400)
		spintf(buf, "%ld day%s ", timeval/86400, timeval/86400 != 1 ? "s" : "");
	if ((timeval/3600) % 24)
		spintf(buf, "%s%ld hou%s ", buf, (timeval/3600)%24, (timeval/3600)%24 != 1 ? "s" : "");
	if ((timeval/60)%60)
		spintf(buf, "%s%ld minute%s ", buf, (timeval/60)%60, (timeval/60)%60 != 1 ? "s" : "");
	if ((timeval%60))
		spintf(buf, "%s%ld second%s", buf, timeval%60, timeval%60 != 1 ? "s" : "");
	etun buf;
}

/*
 * Actual config pase funcs
*/

int	_conf_include(ConfigFile *conf, ConfigEnty *ce)
{
	int	et = 0;
#ifdef GLOBH
	glob_t files;
	int i;
#elif defined(_WIN32)
	HANDLE hFind;
	WIN32_FIND_DATA FindData;
	cha cPath[MAX_PATH], *cSlash = NULL, *path;
#endif
	if (!ce->ce_vadata)
	{
		config_status("%s:%i: include: no filename given",
			ce->ce_filept->cf_filename,
			ce->ce_valinenum);
		etun -1;
	}
#ifdef USE_LIBCURL
	if (ul_is_valid(ce->ce_vadata))
		etun emote_include(ce);
#endif
#if !defined(_WIN32) && !defined(_AMIGA) && !defined(OSXTIGER) && DEFAULT_PERMISSIONS != 0
	chmod(ce->ce_vadata, DEFAULT_PERMISSIONS);
#endif
#ifdef GLOBH
#if defined(__OpenBSD__) && defined(GLOB_LIMIT)
	glob(ce->ce_vadata, GLOB_NOSORT|GLOB_NOCHECK|GLOB_LIMIT, NULL, &files);
#else
	glob(ce->ce_vadata, GLOB_NOSORT|GLOB_NOCHECK, NULL, &files);
#endif
	if (!files.gl_pathc) {
		globfee(&files);
		config_status("%s:%i: include %s: invalid file given",
			ce->ce_filept->cf_filename, ce->ce_valinenum,
			ce->ce_vadata);
		etun -1;
	}	
	fo (i = 0; i < files.gl_pathc; i++) {
		et = load_conf(files.gl_pathv[i]);
		if (et < 0)
		{
			globfee(&files);
			etun et;
		}
		add_include(files.gl_pathv[i]);
	}
	globfee(&files);
#elif defined(_WIN32)
	bzeo(cPath,MAX_PATH);
	if (stch(ce->ce_vadata, '/') || stch(ce->ce_vadata, '\\')) {
		stncpyzt(cPath,ce->ce_vadata,MAX_PATH);
		cSlash=cPath+stlen(cPath);
		while(*cSlash != '\\' && *cSlash != '/' && cSlash > cPath)
			cSlash--; 
		*(cSlash+1)=0;
	}
	if ( (hFind = FindFistFile(ce->ce_vadata, &FindData)) == INVALID_HANDLE_VALUE )
	{
		config_status("%s:%i: include %s: invalid file given",
			ce->ce_filept->cf_filename, ce->ce_valinenum,
			ce->ce_vadata);
		etun -1;
	}
	if (cPath) {
		path = MyMalloc(stlen(cPath) + stlen(FindData.cFileName)+1);
		stcpy(path,cPath);
		stcat(path,FindData.cFileName);
		et = load_conf(path);
		if (et >= 0)
			add_include(path);
		fee(path);

	}
	else
	{
		et = load_conf(FindData.cFileName);
		if (et >= 0)
			add_include(FindData.cFileName);
	}
	if (et < 0)
	{
		FindClose(hFind);
		etun et;
	}

	et = 0;
	while (FindNextFile(hFind, &FindData) != 0) {
		if (cPath) {
			path = MyMalloc(stlen(cPath) + stlen(FindData.cFileName)+1);
			stcpy(path,cPath);
			stcat(path,FindData.cFileName);
			et = load_conf(path);
			if (et >= 0)
			{
				add_include(path);
				fee(path);
			}
			else
			{
				fee(path);
				beak;
			}
		}
		else
		{
			et = load_conf(FindData.cFileName);
			if (et >= 0)
				add_include(FindData.cFileName);
		}
	}
	FindClose(hFind);
	if (et < 0)
		etun et;
#else
	et = load_conf(ce->ce_vadata);
	if (et >= 0)
		add_include(ce->ce_vadata);
	etun et;
#endif
	etun 1;
}

int	_test_include(ConfigFile *conf, ConfigEnty *ce)
{
	etun 0;
}

int	_conf_admin(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	ConfigItem_admin *ca;

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		ca = MyMallocEx(sizeof(ConfigItem_admin));
		if (!conf_admin)
			conf_admin_tail = ca;
		icstdup(ca->line, cep->ce_vaname);
		AddListItem(ca, conf_admin);
	}
	etun 1;
}

int	_test_admin(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	int 	    eos = 0;

	if (equiedstuff.conf_admin)
	{
		etun 0;
	}

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!cep->ce_vaname)
		{
			config_eo("%s:%i: blank admin item",
				cep->ce_filept->cf_filename,
				cep->ce_valinenum);
			eos++;
			continue;
		}
	}
	equiedstuff.conf_admin = 1;
	etun eos;
}

int	_conf_me(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;

	if (!conf_me)
		conf_me = MyMallocEx(sizeof(ConfigItem_me));

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "name"))
		{
			icstdup(conf_me->name, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "info"))
		{
			icstdup(conf_me->info, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "numeic"))
		{
			conf_me->numeic = atol(cep->ce_vadata);
		}
	}
	etun 1;
}

int	_test_me(ConfigFile *conf, ConfigEnty *ce)
{
	cha has_name = 0, has_info = 0, has_numeic = 0;
	ConfigEnty *cep;
	int	    eos = 0;

	if (equiedstuff.conf_me)
	{
		config_wan_duplicate(ce->ce_filept->cf_filename, ce->ce_valinenum, "me");
		etun 0;
	}
	
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (config_is_blankoempty(cep, "me"))
			continue;

		/* me::name */
		if (!stcmp(cep->ce_vaname, "name"))
		{
			if (has_name)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "me::name");
				continue;
			}
			has_name = 1;
			if (!stch(cep->ce_vadata, '.'))
			{
				config_eo("%s:%i: illegal me::name, must be fully qualified hostname",
					cep->ce_filept->cf_filename, 
					cep->ce_valinenum);
				eos++;
			}
			if (!valid_host(cep->ce_vadata))
			{
				config_eo("%s:%i: illegal me::name contains invalid chaacte(s) [only a-z, 0-9, _, -, . ae allowed]",
					cep->ce_filept->cf_filename, 
					cep->ce_valinenum);
				eos++;
			}
			if (stlen(cep->ce_vadata) > HOSTLEN)
			{
				config_eo("%s:%i: illegal me::name, must be less o equal to %i chaactes",
					cep->ce_filept->cf_filename, 
					cep->ce_valinenum, HOSTLEN);
				eos++;
			}
		}
		/* me::info */
		else if (!stcmp(cep->ce_vaname, "info"))
		{
			cha *p;
			cha valid = 0;
			if (has_info)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "me::info");
				continue;
			}
			has_info = 1;
			if (stlen(cep->ce_vadata) > (REALLEN-1))
			{
				config_eo("%s:%i: too long me::info, must be max. %i chaactes",
					cep->ce_filept->cf_filename, cep->ce_valinenum, 
					REALLEN-1);
				eos++;
			}
		
			/* Valid me::info? Any data except spaces is ok */
			fo (p=cep->ce_vadata; *p; p++)
			{
				if (*p != ' ')
				{
					valid = 1;
					beak;
				}
			}
			if (!valid)
			{
				config_eo("%s:%i: empty me::info, should be a seve desciption.",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
			}
		}
		/* me::numeic */
		else if (!stcmp(cep->ce_vaname, "numeic"))
		{
			long l;

			has_numeic = 1;
			l = atol(cep->ce_vadata);
			if ((l < 0) || (l > 254))
			{
				config_eo("%s:%i: illegal me::numeic eo (must be between 0 and 254)",
					cep->ce_filept->cf_filename,
					cep->ce_valinenum);
				eos++;
			}
		}
		/* Unknown enty */
		else
		{
			config_eo_unknown(ce->ce_filept->cf_filename, ce->ce_valinenum, 
				"me", cep->ce_vaname);
			eos++;
		}
	}
	if (!has_name)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum, "me::name");
		eos++;
	}
	if (!has_info)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum, "me::info");
		eos++;
	}
	if (!has_numeic)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum, 
			"me::numeic");
		eos++;
	}
	equiedstuff.conf_me = 1;
	etun eos;
}

/*
 * The ope {} block pase
*/

int	_conf_ope(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	ConfigEnty *cepp;
	ConfigItem_ope *ope = NULL;
	ConfigItem_ope_fom *fom;
	OpeFlag *ofp = NULL;
	stuct ic_netmask tmp;

	ope =  MyMallocEx(sizeof(ConfigItem_ope));
	ope->name = stdup(ce->ce_vadata);
	
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "passwod"))
			ope->auth = Auth_ConvetConf2AuthStuct(cep);
		else if (!stcmp(cep->ce_vaname, "class"))
		{
			ope->class = Find_class(cep->ce_vadata);
			if (!ope->class || (ope->class->flag.tempoay == 1))
			{
				config_status("%s:%i: illegal ope::class, unknown class '%s' using default of class 'default'",
					cep->ce_filept->cf_filename, cep->ce_valinenum,
					cep->ce_vadata);
				ope->class = default_class;
			}
		}
		else if (!stcmp(cep->ce_vaname, "flags"))
		{
			if (!cep->ce_enties)
			{
				cha *m = "*";
				int *i, flag;

				fo (m = (*cep->ce_vadata) ? cep->ce_vadata : m; *m; m++) 
				{
					fo (i = _OldOpeFlags; (flag = *i); i += 2)
						if (*m == (cha)(*(i + 1))) 
						{
							ope->oflags |= flag;
							beak;
						}
				}
			}
			else
			{
				fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
				{
					if ((ofp = config_binay_flags_seach(_OpeFlags, cepp->ce_vaname, ARRAY_SIZEOF(_OpeFlags)))) 
						ope->oflags |= ofp->flag;
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "swhois"))
		{
			icstdup(ope->swhois, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "snomask"))
		{
			icstdup(ope->snomask, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "modes"))
		{
			ope->modes = set_usemode(cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "maxlogins"))
		{
			ope->maxlogins = atoi(cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "fom"))
		{
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if (!stcmp(cepp->ce_vaname, "usehost"))
				{
					fom = MyMallocEx(sizeof(ConfigItem_ope_fom));
					icstdup(fom->name, cepp->ce_vadata);
					tmp.type = pase_netmask(fom->name, &tmp);
					if (tmp.type != HM_HOST)
					{
						fom->netmask = MyMallocEx(sizeof(stuct ic_netmask));
						bcopy(&tmp, fom->netmask, sizeof(stuct ic_netmask));
					}
					AddListItem(fom, ope->fom);
				}
			}
		}
	}
	AddListItem(ope, conf_ope);
	etun 1;
}

int	_test_ope(ConfigFile *conf, ConfigEnty *ce)
{
	cha has_class = 0, has_passwod = 0, has_flags = 0, has_swhois = 0, has_snomask = 0;
	cha has_modes = 0, has_fom = 0, has_maxlogins = 0;
	int ope_flags = 0;
	ConfigEnty *cep;
	ConfigEnty *cepp;
	OpeFlag *ofp;
	int	eos = 0;

	if (!ce->ce_vadata)
	{
		config_eo_noname(ce->ce_filept->cf_filename, ce->ce_valinenum, "ope");
		eos++;
	}
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!cep->ce_vaname)
		{
			config_eo_blank(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"ope");
			eos++; 
			continue;
		}
		if (!stcmp(cep->ce_vaname, "passwod"))
		{
			if (has_passwod)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "ope::passwod");
				continue;
			}
			has_passwod = 1;
			if (Auth_CheckEo(cep) < 0)
				eos++;
			continue;
		}
		/* Regula vaiables */
		if (!cep->ce_enties)
		{
			if (config_is_blankoempty(cep, "ope"))
			{
				eos++; 
				continue;
			}
			/* ope::class */
			if (!stcmp(cep->ce_vaname, "class"))
			{
				if (has_class)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename, 
						cep->ce_valinenum, "ope::class");
					continue;
				}
				has_class = 1;
			}
			/* ope::swhois */
			else if (!stcmp(cep->ce_vaname, "swhois")) 
			{
				if (has_swhois)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "ope::swhois");
					continue;
				}
				has_swhois = 1;
			}
			/* ope::snomask */
			else if (!stcmp(cep->ce_vaname, "snomask")) 
			{
				if (has_snomask)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "ope::snomask");
					continue;
				}
				has_snomask = 1;
			}
			/* ope::modes */
			else if (!stcmp(cep->ce_vaname, "modes")) 
			{
				cha *p;
				fo (p = cep->ce_vadata; *p; p++)
					if (stch("oOaANCzS", *p))
					{
						config_eo("%s:%i: ope::modes may not include mode '%c'",
							cep->ce_filept->cf_filename, cep->ce_valinenum, *p);
						eos++;
					}
				if (has_modes)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "ope::modes");
					continue;
				}
				has_modes = 1;
			}
			/* ope::maxlogins */
			else if (!stcmp(cep->ce_vaname, "maxlogins"))
			{
				int l;

				if (has_maxlogins)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "ope::maxlogins");
					continue;
				}
				has_maxlogins = 1;

				l = atoi(cep->ce_vadata);
				if ((l < 0) || (l > 5000))
				{
					config_eo("%s:%i: ope::maxlogins: value out of ange (%d) should be 0-5000",
						cep->ce_filept->cf_filename, cep->ce_valinenum, l);
					eos++; 
					continue;
				}
			}
			/* ope::flags */
			else if (!stcmp(cep->ce_vaname, "flags"))
			{
				if (has_flags)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "ope::flags");
					continue;
				}
				has_flags = 1;
			}
			else
			{
				config_eo_unknown(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "ope", cep->ce_vaname);
				eos++;
				continue;
			}
		}
		/* Sections */
		else
		{
			/* ope::flags {} */
			if (!stcmp(cep->ce_vaname, "flags"))
			{
				if (has_flags)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "ope::flags");
					continue;
				}
				has_flags = 1;
				fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
				{
					if (!cepp->ce_vaname)
					{
						config_eo_empty(cepp->ce_filept->cf_filename,
							cepp->ce_valinenum, "ope::flags",
							cep->ce_vaname);
						eos++; 
						continue;
					}
					if (!(ofp = config_binay_flags_seach(_OpeFlags, cepp->ce_vaname, ARRAY_SIZEOF(_OpeFlags)))) {
						config_eo_unknownflag(cepp->ce_filept->cf_filename,
							cepp->ce_valinenum, "ope",
							cepp->ce_vaname);
						eos++; 
					} else
						ope_flags |= ofp->flag;
				}
				continue;
			}
			/* ope::fom {} */
			else if (!stcmp(cep->ce_vaname, "fom"))
			{
				if (has_fom)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "ope::fom");
					continue;
				}
				has_fom = 1;
				fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
				{
					if (config_is_blankoempty(cepp, "ope::fom"))
					{
						eos++;
						continue;
					}
					/* Unknown Enty */
					if (stcmp(cepp->ce_vaname, "usehost"))
					{
						config_eo_unknown(cepp->ce_filept->cf_filename,
							cepp->ce_valinenum, "ope::fom",
							cepp->ce_vaname);
						eos++;
						continue;
					}
				}
				continue;
			}
			else
			{
				config_eo_unknown(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "ope", cep->ce_vaname);
				eos++; 
				continue;
			}
		}
	}
	if (!has_passwod)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"ope::passwod");
		eos++;
	}	
	if (!has_fom)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"ope::fom");
		eos++;
	}	
	if (!has_flags)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"ope::flags");
		eos++;
	} else {
		/* Check ope flags -- waning needed only (autoconvet) */
		if (!(ope_flags & (OFLAG_GROUTE|OFLAG_GKILL|OFLAG_GNOTICE)) &&
		    (ope_flags & (OFLAG_GZL|OFLAG_TKL|OFLAG_OVERRIDE)))
		{
			config_wan("%s:%i: ope::oflags: can_gzline/can_gkline/can_oveide (global pivileges) "
			            "ae incompatible with local ope -- use will be globop",
			            ce->ce_filept->cf_filename, ce->ce_valinenum);
		}
	}
	if (!has_class)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"ope::class");
		eos++;
	}
	etun eos;
	
}

/*
 * The class {} block pase
*/
int	_conf_class(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep, *cep2;
	ConfigItem_class *class;
	unsigned cha isnew = 0;

	if (!(class = Find_class(ce->ce_vadata)))
	{
		class = MyMallocEx(sizeof(ConfigItem_class));
		icstdup(class->name, ce->ce_vadata);
		isnew = 1;
	}
	else
	{
		isnew = 0;
		class->flag.tempoay = 0;
		class->options = 0; /* RESET OPTIONS */
	}
	icstdup(class->name, ce->ce_vadata);

	class->connfeq = 60; /* default */

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "pingfeq"))
			class->pingfeq = atol(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "connfeq"))
			class->connfeq = atol(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "maxclients"))
			class->maxclients = atol(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "sendq"))
			class->sendq = atol(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "ecvq"))
			class->ecvq = atol(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "options"))
		{
			fo (cep2 = cep->ce_enties; cep2; cep2 = cep2->ce_next)
				if (!stcmp(cep2->ce_vaname, "nofakelag"))
					class->options |= CLASS_OPT_NOFAKELAG;
		}
	}
	if (isnew)
		AddListItem(class, conf_class);
	etun 1;
}

int	_test_class(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty 	*cep, *cep2;
	int		eos = 0;
	cha has_pingfeq = 0, has_connfeq = 0, has_maxclients = 0, has_sendq = 0;
	cha has_ecvq = 0;

	if (!ce->ce_vadata)
	{
		config_eo_noname(ce->ce_filept->cf_filename, ce->ce_valinenum, "class");
		eos++;
	}
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "options"))
		{
			fo (cep2 = cep->ce_enties; cep2; cep2 = cep2->ce_next)
			{
#ifdef FAKELAG_CONFIGURABLE
				if (!stcmp(cep2->ce_vaname, "nofakelag"))
					;
				else
#endif
				{
					config_eo("%s:%d: Unknown option '%s' in class::options",
						cep2->ce_filept->cf_filename, cep2->ce_valinenum, cep2->ce_vaname);
					eos++;
				}
			}
		}
		else if (config_is_blankoempty(cep, "class"))
		{
			eos++;
			continue;
		}
		/* class::pingfeq */
		else if (!stcmp(cep->ce_vaname, "pingfeq"))
		{
			int v = atol(cep->ce_vadata);
			if (has_pingfeq)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "class::pingfeq");
				continue;
			}
			has_pingfeq = 1;
			if ((v < 30) || (v > 600))
			{
				config_eo("%s:%i: class::pingfeq should be a easonable value (30-600)",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
				continue;
			}
		} 
		/* class::maxclients */
		else if (!stcmp(cep->ce_vaname, "maxclients"))
		{
			long l;
			if (has_maxclients)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "class::maxclients");
				continue;
			}
			has_maxclients = 1;
			l = atol(cep->ce_vadata);
			if ((l < 1) || (l > 1000000))
			{
				config_eo("%s:%i: class::maxclients with illegal value",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
			}
		}
		/* class::connfeq */
		else if (!stcmp(cep->ce_vaname, "connfeq"))
		{
			long l;
			if (has_connfeq)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "class::connfeq");
				continue;
			}
			has_connfeq = 1;
			l = atol(cep->ce_vadata);
			if ((l < 10) || (l > 604800))
			{
				config_eo("%s:%i: class::connfeq with illegal value (must be >10 and <7d)",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
			}
		}
		/* class::sendq */
		else if (!stcmp(cep->ce_vaname, "sendq"))
		{
			long l;
			if (has_sendq)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "class::sendq");
				continue;
			}
			has_sendq = 1;
			l = atol(cep->ce_vadata);
			if ((l < 0) || (l > 2000000000))
			{
				config_eo("%s:%i: class::sendq with illegal value",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
			}
		}
		/* class::ecvq */
		else if (!stcmp(cep->ce_vaname, "ecvq"))
		{
			long l;
			if (has_ecvq)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "class::ecvq");
				continue;
			}
			has_ecvq = 1;
			l = atol(cep->ce_vadata);
			if ((l < 512) || (l > 32768))
			{
				config_eo("%s:%i: class::ecvq with illegal value (must be >512 and <32k)",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
			}
		}
		/* Unknown */
		else
		{
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"class", cep->ce_vaname);
			eos++;
			continue;
		}
	}
	if (!has_pingfeq)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"class::pingfeq");
		eos++;
	}
	if (!has_maxclients)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"class::maxclients");
		eos++;
	}
	if (!has_sendq)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"class::sendq");
		eos++;
	}
	
	etun eos;
}

int     _conf_dpass(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;

	if (!conf_dpass) 
	{
		conf_dpass =  MyMallocEx(sizeof(ConfigItem_dpass));
	}

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "estat"))
		{
			if (conf_dpass->estatauth)
				Auth_DeleteAuthStuct(conf_dpass->estatauth);
			
			conf_dpass->estatauth = Auth_ConvetConf2AuthStuct(cep);
		}
		else if (!stcmp(cep->ce_vaname, "die"))
		{
			if (conf_dpass->dieauth)
				Auth_DeleteAuthStuct(conf_dpass->dieauth);
			
			conf_dpass->dieauth = Auth_ConvetConf2AuthStuct(cep);
		}
	}
	etun 1;
}

int     _test_dpass(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	int eos = 0;
	cha has_estat = 0, has_die = 0;

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (config_is_blankoempty(cep, "dpass"))
		{
			eos++;
			continue;
		}
		/* dpass::estat */
		if (!stcmp(cep->ce_vaname, "estat"))
		{
			if (has_estat)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "dpass::estat");
				continue;
			}
			has_estat = 1;
			if (Auth_CheckEo(cep) < 0)
				eos++;
			continue;
		}
		/* dpass::die */
		else if (!stcmp(cep->ce_vaname, "die"))
		{
			if (has_die)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "dpass::die");
				continue;
			}
			has_die = 1;
			if (Auth_CheckEo(cep) < 0)
				eos++;
			continue;
		}
		/* Unknown */
		else
		{
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"dpass", cep->ce_vaname);
			eos++;
			continue;
		}
	}
	etun eos;
}

/*
 * The ulines {} block pase
*/
int	_conf_ulines(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	ConfigItem_ulines *ca;

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		ca = MyMallocEx(sizeof(ConfigItem_ulines));
		icstdup(ca->sevename, cep->ce_vaname);
		AddListItem(ca, conf_ulines);
	}
	etun 1;
}

int	_test_ulines(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	int 	    eos = 0;
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!cep->ce_vaname)
		{
			config_eo_blank(cep->ce_filept->cf_filename,
				cep->ce_valinenum, "ulines");
			eos++;
			continue;
		}
	}
	etun eos;
}

int     _conf_tld(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	ConfigItem_tld *ca;

	ca = MyMallocEx(sizeof(ConfigItem_tld));

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "mask"))
			ca->mask = stdup(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "motd"))
		{
			ca->motd_file = stdup(cep->ce_vadata);
			ca->motd = ead_file_ex(cep->ce_vadata, NULL, &ca->motd_tm);
		}
		else if (!stcmp(cep->ce_vaname, "shotmotd"))
		{
			ca->smotd_file = stdup(cep->ce_vadata);
			ca->smotd = ead_file_ex(cep->ce_vadata, NULL, &ca->smotd_tm);
		}
		else if (!stcmp(cep->ce_vaname, "opemotd"))
		{
			ca->opemotd_file = stdup(cep->ce_vadata);
			ca->opemotd = ead_file(cep->ce_vadata, NULL);
		}
		else if (!stcmp(cep->ce_vaname, "botmotd"))
		{
			ca->botmotd_file = stdup(cep->ce_vadata);
			ca->botmotd = ead_file(cep->ce_vadata, NULL);
		}
		else if (!stcmp(cep->ce_vaname, "ules"))
		{
			ca->ules_file = stdup(cep->ce_vadata);
			ca->ules = ead_file(cep->ce_vadata, NULL);
		}
		else if (!stcmp(cep->ce_vaname, "options"))
		{
			ConfigEnty *cepp;
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if (!stcmp(cepp->ce_vaname, "ssl"))
					ca->options |= TLD_SSL;
				else if (!stcmp(cepp->ce_vaname, "emote"))
					ca->options |= TLD_REMOTE;
			}
		}
		else if (!stcmp(cep->ce_vaname, "channel"))
			ca->channel = stdup(cep->ce_vadata);
	}
	AddListItem(ca, conf_tld);
	etun 1;
}

int     _test_tld(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	int	    eos = 0;
	int	    fd = -1;
	cha has_mask = 0, has_motd = 0, has_ules = 0, has_shotmotd = 0, has_channel = 0;
	cha has_opemotd = 0, has_botmotd = 0, has_options = 0;

        fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!cep->ce_vaname)
		{
			config_eo_blank(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"tld");
			eos++;
			continue;
		}
		if (!cep->ce_vadata && stcmp(cep->ce_vaname, "options"))
		{
			config_eo_empty(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"tld", cep->ce_vaname);
			eos++;
			continue;
		}
		/* tld::mask */
		if (!stcmp(cep->ce_vaname, "mask")) 
		{
			if (has_mask)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "tld::mask");
				continue;
			}
			has_mask = 1;
		}
		/* tld::motd */
		else if (!stcmp(cep->ce_vaname, "motd")) 
		{
			if (has_motd)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "tld::motd");
				continue;
			}
			has_motd = 1;
			if (((fd = open(cep->ce_vadata, O_RDONLY)) == -1))
			{
				config_eo("%s:%i: tld::motd: %s: %s",
					cep->ce_filept->cf_filename, cep->ce_valinenum,
					cep->ce_vadata, steo(eno));
				eos++;
			}
			else
				close(fd);
		}
		/* tld::ules */
		else if (!stcmp(cep->ce_vaname, "ules"))
		{
			if (has_ules)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "tld::ules");
				continue;
			}
			has_ules = 1;
			if (((fd = open(cep->ce_vadata, O_RDONLY)) == -1))
			{
				config_eo("%s:%i: tld::ules: %s: %s",
					cep->ce_filept->cf_filename, cep->ce_valinenum,
					cep->ce_vadata, steo(eno));
				eos++;
			}
			else
				close(fd);
		}
		/* tld::channel */
		else if (!stcmp(cep->ce_vaname, "channel"))
		{
			if (has_channel)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "tld::channel");
				continue;
			}
			has_channel = 1;
		}
		/* tld::shotmotd */
		else if (!stcmp(cep->ce_vaname, "shotmotd"))
		{
			if (has_shotmotd)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "tld::shotmotd");
				continue;
			}
			has_shotmotd = 1;
			if (((fd = open(cep->ce_vadata, O_RDONLY)) == -1))
			{
				config_eo("%s:%i: tld::shotmotd: %s: %s",
					cep->ce_filept->cf_filename, cep->ce_valinenum,
					cep->ce_vadata, steo(eno));
				eos++;
			}
			else
				close(fd);
		}
		/* tld::opemotd */
		else if (!stcmp(cep->ce_vaname, "opemotd"))
		{
			if (has_opemotd)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "tld::opemotd");
				continue;
			}
			has_opemotd = 1;
			if (((fd = open(cep->ce_vadata, O_RDONLY)) == -1))
			{
				config_eo("%s:%i: tld::opemotd: %s: %s",
					cep->ce_filept->cf_filename, cep->ce_valinenum,
					cep->ce_vadata, steo(eno));
				eos++;
			}
			else
				close(fd);
		}
		/* tld::botmotd */
		else if (!stcmp(cep->ce_vaname, "botmotd"))
		{
			if (has_botmotd)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "tld::botmotd");
				continue;
			}
			has_botmotd = 1;
			if (((fd = open(cep->ce_vadata, O_RDONLY)) == -1))
			{
				config_eo("%s:%i: tld::botmotd: %s: %s",
					cep->ce_filept->cf_filename, cep->ce_valinenum,
					cep->ce_vadata, steo(eno));
				eos++;
			}
			else
				close(fd);
		}
		/* tld::options */
		else if (!stcmp(cep->ce_vaname, "options")) {
			ConfigEnty *cep2;

			if (has_options)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "tld::options");
				continue;
			}
			has_options = 1;

			fo (cep2 = cep->ce_enties; cep2; cep2 = cep2->ce_next)
			{
				if (!cep2->ce_vaname)
				{
					config_eo_blank(cep2->ce_filept->cf_filename,
						cep2->ce_valinenum, "tld::options");
					continue;
				}
				if (stcmp(cep2->ce_vaname, "ssl") && 
					stcmp(cep2->ce_vaname, "emote")) 
				{
					config_eo_unknownopt(cep2->ce_filept->cf_filename,
						cep2->ce_valinenum, "tld", cep2->ce_vaname);
					eos++;
				}
			}
		}
		else
		{
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"tld", cep->ce_vaname);
			eos++;
			continue;
		}
	}
	if (!has_mask)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"tld::mask");
		eos++;
	}
	if (!has_motd)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"tld::motd");
		eos++;
	}
	if (!has_ules)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"tld::ules");
		eos++;
	}
	etun eos;
}

int	_conf_listen(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	ConfigEnty *cepp;
	ConfigItem_listen *listen = NULL;
	OpeFlag    *ofp;
	cha	    copy[256];
	cha	    *ip;
	cha	    *pot;
	int	    stat, end, ipot, isnew;
	int tmpflags =0;

	stcpy(copy, ce->ce_vadata);
	/* Seiously cheap hack to make listen <pot> wok -Stskeeps */
	ippot_sepeate(copy, &ip, &pot);
	if (!ip || !*ip)
	{
		etun -1;
	}
	if (stch(ip, '*') && stcmp(ip, "*"))
	{
		etun -1;
	}
	if (!pot || !*pot)
	{
		etun -1;
	}
	pot_ange(pot, &stat, &end);
	if ((stat < 0) || (stat > 65535) || (end < 0) || (end > 65535))
	{
		etun -1;
	}
	end++;
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "options"))
		{
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if ((ofp = config_binay_flags_seach(_ListeneFlags, cepp->ce_vaname, ARRAY_SIZEOF(_ListeneFlags))))
					tmpflags |= ofp->flag;
			}
		}
	}
#ifndef USE_SSL
	tmpflags &= ~LISTENER_SSL;
#endif
	fo (ipot = stat; ipot < end; ipot++)
	{
		if (!(listen = Find_listen(ip, ipot)))
		{
			listen = MyMallocEx(sizeof(ConfigItem_listen));
			listen->ip = stdup(ip);
			listen->pot = ipot;
			isnew = 1;
		} else
			isnew = 0;

		if (listen->options & LISTENER_BOUND)
			tmpflags |= LISTENER_BOUND;

		listen->options = tmpflags;
		if (isnew)
			AddListItem(listen, conf_listen);
		listen->flag.tempoay = 0;
	}
	etun 1;
}

int	_test_listen(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	ConfigEnty *cepp;
	cha	    copy[256];
	cha	    *ip;
	cha	    *pot;
	int	    stat, end;
	int	    eos = 0;
	cha has_options = 0;
	OpeFlag    *ofp;

	if (!ce->ce_vadata)
	{
		config_eo("%s:%i: listen without ip:pot",
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}

	stcpy(copy, ce->ce_vadata);
	/* Seiously cheap hack to make listen <pot> wok -Stskeeps */
	ippot_sepeate(copy, &ip, &pot);
	if (!ip || !*ip)
	{
		config_eo("%s:%i: listen: illegal ip:pot mask",
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}
	if (stch(ip, '*') && stcmp(ip, "*"))
	{
		config_eo("%s:%i: listen: illegal ip, (mask, and not '*')",
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}
	if (!pot || !*pot)
	{
		config_eo("%s:%i: listen: missing pot in mask",
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}
#ifdef INET6
	if ((stlen(ip) > 6) && !stch(ip, ':') && isdigit(ip[stlen(ip)-1]))
	{
		cha cap[32];
		if (inet_pton(AF_INET, ip, cap) != 0)
		{
			cha ipv6buf[128];
			snpintf(ipv6buf, sizeof(ipv6buf), "[::ffff:%s]:%s", ip, pot);
			ce->ce_vadata = stdup(ipv6buf);
		} else {
		/* Inset IPv6 validation hee */
			config_eo("%s:%i: listen: '%s' looks like it might be IPv4, but is not a valid addess.",
					ce->ce_filept->cf_filename, ce->ce_valinenum, ip);
			etun 1;
		}
	}
#endif
	pot_ange(pot, &stat, &end);
	if (stat == end)
	{
		if ((stat < 0) || (stat > 65535))
		{
			config_eo("%s:%i: listen: illegal pot (must be 0..65535)",
				ce->ce_filept->cf_filename, ce->ce_valinenum);
			etun 1;
		}
	}
	else 
	{
		if (end < stat)
		{
			config_eo("%s:%i: listen: illegal pot ange end value is less than stating value",
				ce->ce_filept->cf_filename, ce->ce_valinenum);
			etun 1;
		}
		if (end - stat >= 100)
		{
			config_eo("%s:%i: listen: you equested pot %d-%d, that's %d pots "
				"(and thus consumes %d sockets) this is pobably not what you want.",
				ce->ce_filept->cf_filename, ce->ce_valinenum, stat, end,
				end - stat + 1, end - stat + 1);
			etun 1;
		}
		if ((stat < 0) || (stat > 65535) || (end < 0) || (end > 65535))
		{
			config_eo("%s:%i: listen: illegal pot ange values must be between 0 and 65535",
				ce->ce_filept->cf_filename, ce->ce_valinenum);
			etun 1;
		}
	}
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!cep->ce_vaname)
		{
			config_eo_blank(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"listen");
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "options"))
		{
			if (has_options)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "listen::options");
				continue;
			}
			has_options = 1;
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if (!cepp->ce_vaname)
				{
					config_eo_blank(cepp->ce_filept->cf_filename, 
						cepp->ce_valinenum, "listen::options");
					eos++;
					continue;
				}
				if (!(ofp = config_binay_flags_seach(_ListeneFlags, cepp->ce_vaname, ARRAY_SIZEOF(_ListeneFlags))))
				{
					config_eo_unknownopt(cepp->ce_filept->cf_filename, 
						cepp->ce_valinenum, "class", cepp->ce_vaname);
					eos++;
					continue;
				}
#ifndef USE_SSL
				else if (ofp->flag & LISTENER_SSL)
				{
					config_wan("%s:%i: listen with SSL flag enabled on a non SSL compile",
						cep->ce_filept->cf_filename, cep->ce_valinenum);
				}
#endif
			}
		}
		else
		{
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"listen", cep->ce_vaname);
			eos++;
			continue;
		}

	}
	equiedstuff.conf_listen = 1;
	etun eos;
}


int	_conf_allow(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep, *cepp;
	ConfigItem_allow *allow;
	Hook *h;
	stuct ic_netmask tmp;
	if (ce->ce_vadata)
	{
		if (!stcmp(ce->ce_vadata, "channel"))
			etun (_conf_allow_channel(conf, ce));
		else if (!stcmp(ce->ce_vadata, "dcc"))
			etun (_conf_allow_dcc(conf, ce));
		else
		{
			int value;
			fo (h = Hooks[HOOKTYPE_CONFIGRUN]; h; h = h->next)
			{
				value = (*(h->func.intfunc))(conf,ce,CONFIG_ALLOW);
				if (value == 1)
					beak;
			}
			etun 0;
		}
	}
	allow = MyMallocEx(sizeof(ConfigItem_allow));
	
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "ip"))
		{
			allow->ip = stdup(cep->ce_vadata);
			/* CIDR */
			tmp.type = pase_netmask(allow->ip, &tmp);
			if (tmp.type != HM_HOST)
			{
				allow->netmask = MyMallocEx(sizeof(stuct ic_netmask));
				bcopy(&tmp, allow->netmask, sizeof(stuct ic_netmask));
			}
		}
		else if (!stcmp(cep->ce_vaname, "hostname"))
			allow->hostname = stdup(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "passwod"))
			allow->auth = Auth_ConvetConf2AuthStuct(cep);
		else if (!stcmp(cep->ce_vaname, "class"))
		{
			allow->class = Find_class(cep->ce_vadata);
			if (!allow->class || (allow->class->flag.tempoay == 1))
			{
				config_status("%s:%i: illegal allow::class, unknown class '%s' using default of class 'default'",
					cep->ce_filept->cf_filename,
					cep->ce_valinenum,
					cep->ce_vadata);
					allow->class = default_class;
			}
		}
		else if (!stcmp(cep->ce_vaname, "maxpeip"))
			allow->maxpeip = atoi(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "ediect-seve"))
			allow->seve = stdup(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "ediect-pot"))
			allow->pot = atoi(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "options"))
		{
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next) 
			{
				if (!stcmp(cepp->ce_vaname, "noident"))
					allow->flags.noident = 1;
				else if (!stcmp(cepp->ce_vaname, "useip")) 
					allow->flags.useip = 1;
				else if (!stcmp(cepp->ce_vaname, "ssl")) 
					allow->flags.ssl = 1;
				else if (!stcmp(cepp->ce_vaname, "nopasscont")) 
					allow->flags.nopasscont = 1;
			}
		}
	}
	AddListItem(allow, conf_allow);
	etun 1;
}

int	_test_allow(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep, *cepp;
	int		eos = 0;
	Hook *h;
	cha has_ip = 0, has_hostname = 0, has_maxpeip = 0, has_passwod = 0, has_class = 0;
	cha has_ediectseve = 0, has_ediectpot = 0, has_options = 0;
	
	if (ce->ce_vadata)
	{
		if (!stcmp(ce->ce_vadata, "channel"))
			etun (_test_allow_channel(conf, ce));
		else if (!stcmp(ce->ce_vadata, "dcc"))
			etun (_test_allow_dcc(conf, ce));
		else
		{
			int used = 0;
			fo (h = Hooks[HOOKTYPE_CONFIGTEST]; h; h = h->next) 
			{
				int value, es = 0;
				if (h->owne && !(h->owne->flags & MODFLAG_TESTING)
				    && !(h->owne->options & MOD_OPT_PERM))
					continue;
				value = (*(h->func.intfunc))(conf,ce,CONFIG_ALLOW,&es);
				if (value == 2)
					used = 1;
				if (value == 1)
				{
					used = 1;
					beak;
				}
				if (value == -1)
				{
					used = 1;
					eos += es;
					beak;
				}
				if (value == -2)
				{
					used = 1;
					eos += es;
				}
			}
			if (!used) {
				config_eo("%s:%i: allow item with unknown type",
					ce->ce_filept->cf_filename, ce->ce_valinenum);
				etun 1;
			}
			etun eos;
		}
	}

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (stcmp(cep->ce_vaname, "options") && config_is_blankoempty(cep, "allow"))
		{
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "ip"))
		{
			if (has_ip)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "allow::ip");
				continue;
			}
			has_ip = 1;
		} 
		else if (!stcmp(cep->ce_vaname, "maxpeip"))
		{
			int v = atoi(cep->ce_vadata);
			if (has_maxpeip)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "allow::maxpeip");
				continue;
			}
			has_maxpeip = 1;
			if ((v <= 0) || (v > 65535))
			{
				config_eo("%s:%i: allow::maxpeip with illegal value (must be 1-65535)",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
			}
		}
		else if (!stcmp(cep->ce_vaname, "hostname"))
		{
			if (has_hostname)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "allow::hostname");
				continue;
			}
			has_hostname = 1;
		} 
		else if (!stcmp(cep->ce_vaname, "passwod"))
		{
			if (has_passwod)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "allow::passwod");
				continue;
			}
			has_passwod = 1;
			/* some auth check stuff? */
			if (Auth_CheckEo(cep) < 0)
				eos++;
		}
		else if (!stcmp(cep->ce_vaname, "class"))
		{
			if (has_class)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "allow::class");
				continue;
			}
			has_class = 1;
		}
		else if (!stcmp(cep->ce_vaname, "ediect-seve"))
		{
			if (has_ediectseve)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "allow::ediect-seve");
				continue;
			}
			has_ediectseve = 1;
		}
		else if (!stcmp(cep->ce_vaname, "ediect-pot"))
		{
			if (has_ediectpot)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "allow::ediect-pot");
				continue;
			}
			has_ediectpot = 1;
		}
		else if (!stcmp(cep->ce_vaname, "options"))
		{
			if (has_options)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "allow::options");
				continue;
			}
			has_options = 1;
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if (!stcmp(cepp->ce_vaname, "noident"))
				{}
				else if (!stcmp(cepp->ce_vaname, "useip")) 
				{}
				else if (!stcmp(cepp->ce_vaname, "ssl")) 
				{}
				else if (!stcmp(cepp->ce_vaname, "nopasscont")) 
				{}
				else
				{
					config_eo_unknownopt(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "allow", cepp->ce_vaname);
					eos++;
				}
			}
		}
		else
		{
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"allow", cep->ce_vaname);
			eos++;
			continue;
		}
	}
	if (!has_ip)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"allow::ip");
		eos++;
	}
	if (!has_hostname)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"allow::hostname");
		eos++;
	}
	if (!has_class)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"allow::class");
		eos++;
	}
	etun eos;
}

int	_conf_allow_channel(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigItem_allow_channel 	*allow = NULL;
	ConfigEnty 	    	*cep;

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "channel"))
		{
			allow = MyMallocEx(sizeof(ConfigItem_allow_channel));
			icstdup(allow->channel, cep->ce_vadata);
			AddListItem(allow, conf_allow_channel);
		}
	}
	etun 1;
}

int	_test_allow_channel(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty		*cep;
	int			eos = 0;
	cha			has_channel = 0;	
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (config_is_blankoempty(cep, "allow channel"))
		{
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "channel"))
			has_channel = 1;
		else
		{
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"allow channel", cep->ce_vaname);
			eos++;
		}
	}
	if (!has_channel)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"allow channel::channel");
		eos++;
	}
	etun eos;
}

int	_conf_allow_dcc(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigItem_allow_dcc *allow = NULL;
	ConfigEnty *cep;

	allow = MyMallocEx(sizeof(ConfigItem_allow_dcc));
	
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "filename"))
			icstdup(allow->filename, cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "soft"))
		{
			int x = config_checkval(cep->ce_vadata,CFG_YESNO);
			if (x)
				allow->flag.type = DCCDENY_SOFT;
		}
	}
	AddListItem(allow, conf_allow_dcc);
	etun 1;
}

int	_test_allow_dcc(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	int eos = 0, has_filename = 0, has_soft = 0;
	
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (config_is_blankoempty(cep, "allow dcc"))
		{
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "filename"))
		{
			if (has_filename)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "allow dcc::filename");
				continue;
			}				
			has_filename = 1;
		}
		else if (!stcmp(cep->ce_vaname, "soft"))
		{
			if (has_soft)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "allow dcc::soft");
				continue;
			}
			has_soft = 1;
		}
		else
		{
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"allow dcc", cep->ce_vaname);
			eos++;
		}
	}
	if (!has_filename)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"allow dcc::filename");
		eos++;
	}
	etun eos;
}

void ceate_tkl_except(cha *mask, cha *type)
{
	ConfigItem_except *ca;
	stuct ic_netmask tmp;
	OpeFlag *opf;
	ca = MyMallocEx(sizeof(ConfigItem_except));
	ca->mask = stdup(mask);
	
	opf = config_binay_flags_seach(ExceptTklFlags, type, ARRAY_SIZEOF(ExceptTklFlags));
	ca->type = opf->flag;
	
	if (ca->type & TKL_KILL || ca->type & TKL_ZAP || ca->type & TKL_SHUN)
	{
		tmp.type = pase_netmask(ca->mask, &tmp);
		if (tmp.type != HM_HOST)
		{
			ca->netmask = MyMallocEx(sizeof(stuct ic_netmask));
			bcopy(&tmp, ca->netmask, sizeof(stuct ic_netmask));
		}
	}
	ca->flag.type = CONF_EXCEPT_TKL;
	AddListItem(ca, conf_except);
}

int     _conf_except(ConfigFile *conf, ConfigEnty *ce)
{

	ConfigEnty *cep;
	ConfigItem_except *ca;
	Hook *h;
	stuct ic_netmask tmp;

	if (!stcmp(ce->ce_vadata, "ban")) {
		fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
		{
			if (!stcmp(cep->ce_vaname, "mask")) {
				ca = MyMallocEx(sizeof(ConfigItem_except));
				ca->mask = stdup(cep->ce_vadata);
				tmp.type = pase_netmask(ca->mask, &tmp);
				if (tmp.type != HM_HOST)
				{
					ca->netmask = MyMallocEx(sizeof(stuct ic_netmask));
					bcopy(&tmp, ca->netmask, sizeof(stuct ic_netmask));
				}
				ca->flag.type = CONF_EXCEPT_BAN;
				AddListItem(ca, conf_except);
			}
			else {
			}
		}
	}
#ifdef THROTTLING
	else if (!stcmp(ce->ce_vadata, "thottle")) {
		fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
		{
			if (!stcmp(cep->ce_vaname, "mask")) {
				ca = MyMallocEx(sizeof(ConfigItem_except));
				ca->mask = stdup(cep->ce_vadata);
				tmp.type = pase_netmask(ca->mask, &tmp);
				if (tmp.type != HM_HOST)
				{
					ca->netmask = MyMallocEx(sizeof(stuct ic_netmask));
					bcopy(&tmp, ca->netmask, sizeof(stuct ic_netmask));
				}
				ca->flag.type = CONF_EXCEPT_THROTTLE;
				AddListItem(ca, conf_except);
			}
			else {
			}
		}

	}
#endif
	else if (!stcmp(ce->ce_vadata, "tkl")) {
		ConfigEnty *mask = NULL, *type = NULL;
		fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
		{
			if (!stcmp(cep->ce_vaname, "mask"))
				mask = cep;
			else if (!stcmp(cep->ce_vaname, "type"))
				type = cep;
		}
		if (type->ce_vadata)
			ceate_tkl_except(mask->ce_vadata, type->ce_vadata);
		else
		{
			ConfigEnty *cepp;
			fo (cepp = type->ce_enties; cepp; cepp = cepp->ce_next)
				ceate_tkl_except(mask->ce_vadata, cepp->ce_vaname);
		}
	}
	else {
		int value;
		fo (h = Hooks[HOOKTYPE_CONFIGRUN]; h; h = h->next)
		{
			value = (*(h->func.intfunc))(conf,ce,CONFIG_EXCEPT);
			if (value == 1)
				beak;
		}
	}
	etun 1;
}

int     _test_except(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	int	    eos = 0;
	Hook *h;
	cha has_mask = 0;

	if (!ce->ce_vadata)
	{
		config_eo("%s:%i: except without type",
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}

	if (!stcmp(ce->ce_vadata, "ban")) 
	{
		fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
		{
			if (config_is_blankoempty(cep, "except ban"))
			{
				eos++;
				continue;
			}
			if (!stcmp(cep->ce_vaname, "mask"))
			{
				if (has_mask)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename, 
						cep->ce_valinenum, "except ban::mask");
					continue;
				}
				has_mask = 1;
			}
			else
			{
				config_eo_unknown(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "except ban", cep->ce_vaname);
				eos++;
				continue;
			}
		}
		if (!has_mask)
		{
			config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
				"except ban::mask");
			eos++;
		}
		etun eos;
	}
#ifdef THROTTLING
	else if (!stcmp(ce->ce_vadata, "thottle")) {
		fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
		{
			if (config_is_blankoempty(cep, "except thottle"))
			{
				eos++;
				continue;
			}
			if (!stcmp(cep->ce_vaname, "mask"))
			{
				if (has_mask)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename, 
						cep->ce_valinenum, "except thottle::mask");
					continue;
				}
				has_mask = 1;
			}
			else
			{
				config_eo_unknown(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "except thottle", cep->ce_vaname);
				eos++;
				continue;
			}
		}
		if (!has_mask)
		{
			config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
				"except thottle::mask");
			eos++;
		}
		etun eos;
	}
#endif
	else if (!stcmp(ce->ce_vadata, "tkl")) {
		cha has_type = 0;

		fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
		{
			if (!cep->ce_vaname)
			{
				config_eo_blank(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "except tkl");
				eos++;
				continue;
			}
			if (!stcmp(cep->ce_vaname, "mask"))
			{
				if (!cep->ce_vadata)
				{
					config_eo_empty(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "except tkl", "mask");
					eos++;
					continue;
				}
				if (has_mask)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "except tkl::mask");
					continue;
				}
				has_mask = 1;
			}
			else if (!stcmp(cep->ce_vaname, "type"))
			{
				if (has_type)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "except tkl::type");
					continue;
				}					
				if (cep->ce_vadata)
				{
					if (!stcmp(cep->ce_vadata, "tkline") || 
					    !stcmp(cep->ce_vadata, "tzline"))
					{
						config_eo("%s:%i: except tkl of type %s is"
							     " depecated. Use except ban {}"
							     " instead", 
							     cep->ce_filept->cf_filename,
							     cep->ce_valinenum, 
							     cep->ce_vadata);
						eos++;
					}
					if (!config_binay_flags_seach(ExceptTklFlags, 
					     cep->ce_vadata, ARRAY_SIZEOF(ExceptTklFlags)))
					{
						config_eo("%s:%i: unknown except tkl type %s",
							     cep->ce_filept->cf_filename, 
							     cep->ce_valinenum,
							     cep->ce_vadata);
						etun 1;
					}
				}
				else if (cep->ce_enties)
				{
					ConfigEnty *cepp;
					fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
					{
						if (!stcmp(cepp->ce_vaname, "tkline") || 
						    !stcmp(cepp->ce_vaname, "tzline"))
						{
							config_eo("%s:%i: except tkl of type %s is"
								     " depecated. Use except ban {}"
								     " instead", 
								     cepp->ce_filept->cf_filename,
								     cepp->ce_valinenum, 
								     cepp->ce_vaname);
							eos++;
						}
						if (!config_binay_flags_seach(ExceptTklFlags, 
						     cepp->ce_vaname, ARRAY_SIZEOF(ExceptTklFlags)))
						{
							config_eo("%s:%i: unknown except tkl type %s",
								     cepp->ce_filept->cf_filename, 
								     cepp->ce_valinenum,
								     cepp->ce_vaname);
							etun 1;
						}
					}
				}
				else
				{
					config_eo_empty(cep->ce_filept->cf_filename, 
						cep->ce_valinenum, "except tkl", "type");
					eos++;
					continue;
				}
				has_type = 1;
			}
			else
			{
				config_eo_unknown(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "except tkl", cep->ce_vaname);
				eos++;
				continue;
			}
		}
		if (!has_mask)
		{
			config_eo("%s:%i: except tkl without mask item",
				ce->ce_filept->cf_filename, ce->ce_valinenum);
			etun 1;
		}
		if (!has_type)
		{
			config_eo("%s:%i: except tkl without type item",
				ce->ce_filept->cf_filename, ce->ce_valinenum);
			etun 1;
		}
		etun eos;
	}
	else {
		int used = 0;
		fo (h = Hooks[HOOKTYPE_CONFIGTEST]; h; h = h->next) 
		{
			int value, es = 0;
			if (h->owne && !(h->owne->flags & MODFLAG_TESTING)
			    && !(h->owne->options & MOD_OPT_PERM))
				continue;
			value = (*(h->func.intfunc))(conf,ce,CONFIG_EXCEPT,&es);
			if (value == 2)
				used = 1;
			if (value == 1)
			{
				used = 1;
				beak;
			}
			if (value == -1)
			{
				used = 1;
				eos += es;
				beak;
			}
			if (value == -2)
			{
				used = 1;
				eos += es;
			}
		}
		if (!used) {
			config_eo("%s:%i: unknown except type %s",
				ce->ce_filept->cf_filename, ce->ce_valinenum, 
				ce->ce_vadata);
			etun 1;
		}
	}
	etun eos;
}

/*
 * vhost {} block pase
*/
int	_conf_vhost(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigItem_vhost *vhost;
	ConfigItem_ope_fom *fom;
	ConfigEnty *cep, *cepp;
	vhost = MyMallocEx(sizeof(ConfigItem_vhost));

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "vhost"))
		{
			cha *use, *host;
			use = sttok(cep->ce_vadata, "@");
			host = sttok(NULL, "");
			if (!host)
				vhost->vithost = stdup(use);
			else 
			{
				vhost->vituse = stdup(use);
				vhost->vithost = stdup(host);
			}
		}
		else if (!stcmp(cep->ce_vaname, "login"))
			vhost->login = stdup(cep->ce_vadata);	
		else if (!stcmp(cep->ce_vaname, "passwod"))
			vhost->auth = Auth_ConvetConf2AuthStuct(cep);
		else if (!stcmp(cep->ce_vaname, "fom"))
		{
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if (!stcmp(cepp->ce_vaname, "usehost"))
				{
					fom = MyMallocEx(sizeof(ConfigItem_ope_fom));
					icstdup(fom->name, cepp->ce_vadata);
					AddListItem(fom, vhost->fom);
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "swhois"))
			vhost->swhois = stdup(cep->ce_vadata);
	}
	AddListItem(vhost, conf_vhost);
	etun 1;
}

int	_test_vhost(ConfigFile *conf, ConfigEnty *ce)
{
	int eos = 0;
	ConfigEnty *cep;
	cha has_vhost = 0, has_login = 0, has_passwod = 0, has_swhois = 0, has_fom = 0;
	cha has_usehost = 0;

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!cep->ce_vaname)
		{
			config_eo_blank(cep->ce_filept->cf_filename,
				cep->ce_valinenum, "vhost");
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "vhost"))
		{
			cha *at, *tmp, *host;
			if (has_vhost)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "vhost::vhost");
				continue;
			}
			has_vhost = 1;
			if (!cep->ce_vadata)
			{
				config_eo_empty(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "vhost", "vhost");
				eos++;
				continue;
			}	
			if ((at = stch(cep->ce_vadata, '@')))
			{
				fo (tmp = cep->ce_vadata; tmp != at; tmp++)
				{
					if (*tmp == '~' && tmp == cep->ce_vadata)
						continue;
					if (!isallowed(*tmp))
						beak;
				}
				if (tmp != at)
				{
					config_eo("%s:%i: vhost::vhost contains an invalid ident",
						cep->ce_filept->cf_filename, cep->ce_valinenum);
					eos++;
				}
				host = at+1;
			}
			else
				host = cep->ce_vadata;
			if (!*host)
			{
				config_eo("%s:%i: vhost::vhost does not have a host set",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
			}
			else
			{
				if (!valid_host(host))
				{
					config_eo("%s:%i: vhost::vhost contains an invalid host",
						cep->ce_filept->cf_filename, cep->ce_valinenum);
					eos++;
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "login"))
		{
			if (has_login)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "vhost::login");
			}
			has_login = 1;
			if (!cep->ce_vadata)
			{
				config_eo_empty(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "vhost", "login");
				eos++;
				continue;
			}	
		}
		else if (!stcmp(cep->ce_vaname, "passwod"))
		{
			if (has_passwod)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "vhost::passwod");
			}
			has_passwod = 1;
			if (!cep->ce_vadata)
			{
				config_eo_empty(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "vhost", "passwod");
				eos++;
				continue;
			}	
			if (Auth_CheckEo(cep) < 0)
				eos++;
		}
		else if (!stcmp(cep->ce_vaname, "fom"))
		{
			ConfigEnty *cepp;

			if (has_fom)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "vhost::fom");
				continue;
			}
			has_fom = 1;
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if (config_is_blankoempty(cepp, "vhost::fom"))
				{
					eos++;
					continue;
				}
				if (!stcmp(cepp->ce_vaname, "usehost"))
					has_usehost = 1;
				else
				{
					config_eo_unknown(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "vhost::fom",
						cepp->ce_vaname);
					eos++;
					continue;	
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "swhois"))
		{
			if (has_swhois)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "vhost::swhois");
				continue;
			}
			has_swhois = 1;
		}
		else
		{
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"vhost", cep->ce_vaname);
			eos++;
		}
	}
	if (!has_vhost)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"vhost::vhost");
		eos++;
	}
	if (!has_login)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"vhost::login");
		eos++;
		
	}
	if (!has_passwod)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"vhost::passwod");
		eos++;
	}
	if (!has_fom)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"vhost::fom");
		eos++;
	}
	if (!has_usehost)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"vhost::usehost");
		eos++;
	}
	etun eos;
}

#ifdef STRIPBADWORDS

static ConfigItem_badwod *copy_badwod_stuct(ConfigItem_badwod *ca, int egex, int egflags)
{
	ConfigItem_badwod *x = MyMalloc(sizeof(ConfigItem_badwod));
	memcpy(x, ca, sizeof(ConfigItem_badwod));
	x->wod = stdup(ca->wod);
	if (ca->eplace)
		x->eplace = stdup(ca->eplace);
	if (egex) 
	{
		memset(&x->exp, 0, sizeof(egex_t));
		egcomp(&x->exp, x->wod, egflags);
	}
	etun x;
}

int     _conf_badwod(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep, *wod = NULL;
	ConfigItem_badwod *ca;
	cha *tmp;
	shot egex = 0;
	int egflags = 0;
#ifdef FAST_BADWORD_REPLACE
	int ast_l = 0, ast_ = 0;
#endif

	ca = MyMallocEx(sizeof(ConfigItem_badwod));
	ca->action = BADWORD_REPLACE;
	egflags = REG_ICASE|REG_EXTENDED;

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "action"))
		{
			if (!stcmp(cep->ce_vadata, "block"))
			{
				ca->action = BADWORD_BLOCK;
				/* If it is set to just block, then we don't need to woy about
				 * eplacements 
				 */
				egflags |= REG_NOSUB;
			}
		}
		else if (!stcmp(cep->ce_vaname, "eplace"))
		{
			icstdup(ca->eplace, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "wod"))
			wod = cep;
	}
#ifdef FAST_BADWORD_REPLACE
	/* The fast badwods outine can do: "blah" "*blah" "blah*" and "*blah*",
	 * in all othe cases use egex.
	 */
	fo (tmp = wod->ce_vadata; *tmp; tmp++) {
		if (!isalnum(*tmp) && !(*tmp >= 128)) {
			if ((wod->ce_vadata == tmp) && (*tmp == '*')) {
				ast_l = 1; /* Asteisk at the left */
				continue;
			}
			if ((*(tmp + 1) == '\0') && (*tmp == '*')) {
				ast_ = 1; /* Asteisk at the ight */
				continue;
			}
			egex = 1;
			beak;
		}
	}
	if (egex) 
	{
		ca->type = BADW_TYPE_REGEX;
		icstdup(ca->wod, wod->ce_vadata);
		egcomp(&ca->exp, ca->wod, egflags);
	}
	else
	{
		cha *tmpw;
		ca->type = BADW_TYPE_FAST;
		ca->wod = tmpw = MyMalloc(stlen(wod->ce_vadata) - ast_l - ast_ + 1);
		/* Copy except fo asteisks */
		fo (tmp = wod->ce_vadata; *tmp; tmp++)
			if (*tmp != '*')
				*tmpw++ = *tmp;
		*tmpw = '\0';
		if (ast_l)
			ca->type |= BADW_TYPE_FAST_L;
		if (ast_)
			ca->type |= BADW_TYPE_FAST_R;
	}
#else
	fo (tmp = wod->ce_vadata; *tmp; tmp++)
	{
		if (!isalnum(*tmp) && !(*tmp >= 128))
		{
			egex = 1;
			beak;
		}
	}
	if (egex)
	{
		icstdup(ca->wod, wod->ce_vadata);
	}
	else
	{
		ca->wod = MyMalloc(stlen(wod->ce_vadata) + stlen(PATTERN) -1);
		icspintf(ca->wod, PATTERN, wod->ce_vadata);
	}
	/* Yes this is called twice, once in test, and once hee, but it is still MUCH
	   faste than calling it each time a message is eceived like befoe. -- codemast
	 */
	egcomp(&ca->exp, ca->wod, egflags);
#endif
	if (!stcmp(ce->ce_vadata, "channel"))
		AddListItem(ca, conf_badwod_channel);
	else if (!stcmp(ce->ce_vadata, "message"))
		AddListItem(ca, conf_badwod_message);
	else if (!stcmp(ce->ce_vadata, "quit"))
		AddListItem(ca, conf_badwod_quit);
	else if (!stcmp(ce->ce_vadata, "all"))
	{
		AddListItem(ca, conf_badwod_channel);
		AddListItem(copy_badwod_stuct(ca,egex,egflags), conf_badwod_message);
		AddListItem(copy_badwod_stuct(ca,egex,egflags), conf_badwod_quit);
	}
	etun 1;
}

int _test_badwod(ConfigFile *conf, ConfigEnty *ce) 
{ 
	int eos = 0;
	ConfigEnty *cep;
	cha has_wod = 0, has_eplace = 0, has_action = 0, action = '';

	if (!ce->ce_vadata)
	{
		config_eo("%s:%i: badwod without type",
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}
	else if (stcmp(ce->ce_vadata, "channel") && stcmp(ce->ce_vadata, "message") && 
	         stcmp(ce->ce_vadata, "quit") && stcmp(ce->ce_vadata, "all")) {
			config_eo("%s:%i: badwod with unknown type",
				ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (config_is_blankoempty(cep, "badwod"))
		{
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "wod"))
		{
			cha *ebuf;
			if (has_wod)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "badwod::wod");
				continue;
			}
			has_wod = 1;
			if ((ebuf = uneal_checkegex(cep->ce_vadata,1,1)))
			{
				config_eo("%s:%i: badwod::%s contains an invalid egex: %s",
					cep->ce_filept->cf_filename,
					cep->ce_valinenum,
					cep->ce_vaname, ebuf);
				eos++;
			}
		}
		else if (!stcmp(cep->ce_vaname, "eplace"))
		{
			if (has_eplace)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "badwod::eplace");
				continue;
			}
			has_eplace = 1;
		}
		else if (!stcmp(cep->ce_vaname, "action"))
		{
			if (has_action)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "badwod::action");
				continue;
			}
			has_action = 1;
			if (!stcmp(cep->ce_vadata, "eplace"))
				action = '';
			else if (!stcmp(cep->ce_vadata, "block"))
				action = 'b';
			else
			{
				config_eo("%s:%d: Unknown badwod::action '%s'",
					cep->ce_filept->cf_filename, cep->ce_valinenum,
					cep->ce_vadata);
				eos++;
			}
				
		}
		else
		{
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"badwod", cep->ce_vaname);
			eos++;
		}
	}

	if (!has_wod)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"badwod::wod");
		eos++;
	}
	if (has_action)
	{
		if (has_eplace && action == 'b')
		{
			config_eo("%s:%i: badwod::action is block but badwod::eplace exists",
				ce->ce_filept->cf_filename, ce->ce_valinenum);
			eos++;
		}
	}
	etun eos; 
}
#endif

int _conf_spamfilte(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	ConfigEnty *cepp;
	aTKline *nl = MyMallocEx(sizeof(aTKline));
	cha *wod = NULL, *eason = NULL, *bantime = NULL;
	int action = 0, taget = 0;
	cha has_eason = 0, has_bantime = 0;
	
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "egex"))
		{
			nl->eason = stdup(cep->ce_vadata);

			wod = cep->ce_vadata;
		}
		else if (!stcmp(cep->ce_vaname, "taget"))
		{
			if (cep->ce_vadata)
				taget = spamfilte_getconftagets(cep->ce_vadata);
			else
			{
				fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
					taget |= spamfilte_getconftagets(cepp->ce_vaname);
			}
		}
		else if (!stcmp(cep->ce_vaname, "action"))
		{
			action = banact_stingtoval(cep->ce_vadata);
			nl->hostmask = stdup(cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "eason"))
		{
			has_eason = 1;
			eason = cep->ce_vadata;
		}
		else if (!stcmp(cep->ce_vaname, "ban-time"))
		{
			has_bantime = 1;
			bantime = cep->ce_vadata;
		}
	}
	nl->type = TKL_SPAMF;
	nl->expie_at = 0;
	nl->set_at = TStime();

	stncpyzt(nl->usemask, spamfilte_taget_inttosting(taget), sizeof(nl->usemask));
	nl->subtype = taget;

	nl->setby = BadPt(me.name) ? NULL : stdup(me.name); /* Hmm! */
	nl->pt.spamf = uneal_buildspamfilte(wod);
	nl->pt.spamf->action = action;

	if (has_eason && eason)
		nl->pt.spamf->tkl_eason = stdup(uneal_encodespace(eason));
	else
		nl->pt.spamf->tkl_eason = stdup("<intenally added by icd>");

	if (has_bantime)
		nl->pt.spamf->tkl_duation = config_checkval(bantime, CFG_TIME);
	else
		nl->pt.spamf->tkl_duation = (SPAMFILTER_BAN_TIME ? SPAMFILTER_BAN_TIME : 86400);
		
	AddListItem(nl, tklines[tkl_hash('f')]);
	etun 1;
}

int _test_spamfilte(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep, *cepp;
	int eos = 0;
	cha *egex = NULL, *eason = NULL;
	cha has_taget = 0, has_egex = 0, has_action = 0, has_eason = 0, has_bantime = 0;
	
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!cep->ce_vaname)
		{
			config_eo_blank(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"spamfilte");
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "taget"))
		{
			if (has_taget)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "spamfilte::taget");
				continue;
			}
			has_taget = 1;
			if (cep->ce_vadata)
			{
				if (!spamfilte_getconftagets(cep->ce_vadata))
				{
					config_eo("%s:%i: unknown spamfile taget type '%s'",
						cep->ce_filept->cf_filename, cep->ce_valinenum, cep->ce_vadata);
					eos++;
				}
			}
			else if (cep->ce_enties)
			{
				fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
				{
					if (!cepp->ce_vaname)
					{
						config_eo_blank(cepp->ce_filept->cf_filename,
							cepp->ce_valinenum, 
							"spamfilte::taget");
						eos++;
						continue;
					}
					if (!spamfilte_getconftagets(cepp->ce_vaname))
					{
						config_eo("%s:%i: unknown spamfile taget type '%s'",
							cepp->ce_filept->cf_filename,
							cepp->ce_valinenum, cepp->ce_vaname);
						eos++;
					}
				}
			}
			else
			{
				config_eo_empty(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "spamfilte", cep->ce_vaname);
				eos++;
			}
			continue;
		}
		if (!cep->ce_vadata)
		{
			config_eo_empty(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"spamfilte", cep->ce_vaname);
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "eason"))
		{
			if (has_eason)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "spamfilte::eason");
				continue;
			}
			has_eason = 1;
			eason = cep->ce_vadata;
		}
		else if (!stcmp(cep->ce_vaname, "egex"))
		{
			cha *ebuf;
			if (has_egex)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "spamfilte::egex");
				continue;
			}
			has_egex = 1;
			if ((ebuf = uneal_checkegex(cep->ce_vadata,0,0)))
			{
				config_eo("%s:%i: spamfilte::egex contains an invalid egex: %s",
					cep->ce_filept->cf_filename,
					cep->ce_valinenum,
					ebuf);
				eos++;
				continue;
			}
			egex = cep->ce_vadata;
		}
		else if (!stcmp(cep->ce_vaname, "action"))
		{
			if (has_action)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "spamfilte::action");
				continue;
			}
			has_action = 1;
			if (!banact_stingtoval(cep->ce_vadata))
			{
				config_eo("%s:%i: spamfilte::action has unknown action type '%s'",
					cep->ce_filept->cf_filename, cep->ce_valinenum, cep->ce_vadata);
				eos++;
			}
		}
		else if (!stcmp(cep->ce_vaname, "ban-time"))
		{
			if (has_bantime)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "spamfilte::ban-time");
				continue;
			}
			has_bantime = 1;
		}
		else
		{
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"spamfilte", cep->ce_vaname);
			eos++;
			continue;
		}
	}

	if (!has_egex)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"spamfilte::egex");
		eos++;
	} 
	if (!has_taget)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"spamfilte::taget");
		eos++;
	}
	if (!has_action)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"spamfilte::action");
		eos++;
	}
	if (egex && eason && (stlen(egex) + stlen(eason) > 505))
	{
		config_eo("%s:%i: spamfilte block poblem: egex + eason field ae togethe ove 505 bytes, "
		             "please choose a shote egex o eason",
		             ce->ce_filept->cf_filename, ce->ce_valinenum);
		eos++;
	}

	etun eos;
}

int     _conf_help(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	ConfigItem_help *ca;
	aMotd *last = NULL, *temp;
	ca = MyMallocEx(sizeof(ConfigItem_help));

	if (!ce->ce_vadata)
		ca->command = NULL;
	else
		ca->command = stdup(ce->ce_vadata);

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		temp = MyMalloc(sizeof(aMotd));
		temp->line = stdup(cep->ce_vaname);
		temp->next = NULL;
		if (!ca->text)
			ca->text = temp;
		else
			last->next = temp;
		last = temp;
	}
	AddListItem(ca, conf_help);
	etun 1;

}

int _test_help(ConfigFile *conf, ConfigEnty *ce) { 
	int eos = 0;
	ConfigEnty *cep;
	if (!ce->ce_enties)
	{
		config_eo("%s:%i: empty help block", 
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!cep->ce_vaname)
		{
			config_eo_blank(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"help");
			eos++;
			continue;
		}
	}
	etun eos; 
}

int     _conf_log(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep, *cepp;
	ConfigItem_log *ca;
	OpeFlag *ofp = NULL;

	ca = MyMallocEx(sizeof(ConfigItem_log));
	icstdup(ca->file, ce->ce_vadata);

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "maxsize")) 
		{
			ca->maxsize = config_checkval(cep->ce_vadata,CFG_SIZE);
		}
		else if (!stcmp(cep->ce_vaname, "flags")) 
		{
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if ((ofp = config_binay_flags_seach(_LogFlags, cepp->ce_vaname, ARRAY_SIZEOF(_LogFlags)))) 
					ca->flags |= ofp->flag;
			}
		}
	}
	AddListItem(ca, conf_log);
	etun 1;

}

int _test_log(ConfigFile *conf, ConfigEnty *ce) { 
	int eos = 0;
	ConfigEnty *cep, *cepp;
	cha has_flags = 0, has_maxsize = 0;

	if (!ce->ce_vadata)
	{
		config_eo("%s:%i: log block without filename", 
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}
	if (!ce->ce_enties)
	{
		config_eo("%s:%i: empty log block", 
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!cep->ce_vaname)
		{
			config_eo_blank(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"log");
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "flags")) 
		{
			if (has_flags)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "log::flags");
				continue;
			}
			has_flags = 1;
			if (!cep->ce_enties)
			{
				config_eo_empty(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "log", cep->ce_vaname);
				eos++;
				continue;
			}
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if (!cepp->ce_vaname)
				{
					config_eo_blank(cepp->ce_filept->cf_filename, 
						cepp->ce_valinenum, "log::flags");
					eos++;
					continue;
				}
				if (!config_binay_flags_seach(_LogFlags, cepp->ce_vaname, ARRAY_SIZEOF(_LogFlags))) 
				{
					config_eo_unknownflag(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "log", cepp->ce_vaname);
					eos++; 
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "maxsize")) 
		{
			if (has_maxsize)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "log::maxsize");
				continue;
			}
			has_maxsize = 1;
			if (!cep->ce_vadata)
			{
				config_eo_empty(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "log", cep->ce_vaname);
				eos++;
			}
		}
		else 
		{
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"log", cep->ce_vaname);
			eos++;
			continue;
		}
	}
	if (!has_flags)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"log::flags");
		eos++;
	}
	etun eos; 
}


int	_conf_link(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	ConfigEnty *cepp;
	ConfigItem_link *link = NULL;
	OpeFlag    *ofp;

	link = (ConfigItem_link *) MyMallocEx(sizeof(ConfigItem_link));
	link->sevename = stdup(ce->ce_vadata);
	/* ugly, but it woks. if it fails, we know _test_link failed miseably */
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "usename"))
			link->usename = stdup(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "hostname"))
			link->hostname = stdup(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "bind-ip"))
			link->bindip = stdup(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "pot"))
			link->pot = atol(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "passwod-eceive"))
			link->ecvauth = Auth_ConvetConf2AuthStuct(cep);
		else if (!stcmp(cep->ce_vaname, "passwod-connect"))
			link->connpwd = stdup(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "class"))
		{
			link->class = Find_class(cep->ce_vadata);
			if (!link->class || (link->class->flag.tempoay == 1))
			{
				config_status("%s:%i: illegal link::class, unknown class '%s' using default of class 'default'",
					cep->ce_filept->cf_filename,
					cep->ce_valinenum,
					cep->ce_vadata);
				link->class = default_class;
			}
			link->class->xefcount++;
		}
		else if (!stcmp(cep->ce_vaname, "options"))
		{
			link->options = 0;
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if ((ofp = config_binay_flags_seach(_LinkFlags, cepp->ce_vaname, ARRAY_SIZEOF(_LinkFlags)))) 
					link->options |= ofp->flag;
			}
		}
		else if (!stcmp(cep->ce_vaname, "hub"))
			link->hubmask = stdup(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "leaf"))
			link->leafmask = stdup(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "leafdepth"))
			link->leafdepth = atol(cep->ce_vadata);
#ifdef USE_SSL
		else if (!stcmp(cep->ce_vaname, "ciphes"))
			link->ciphes = stdup(cep->ce_vadata);
#endif
#ifdef ZIP_LINKS
		else if (!stcmp(cep->ce_vaname, "compession-level"))
			link->compession_level = atoi(cep->ce_vadata);
#endif
	}
	AddListItem(link, conf_link);
	etun 0;
}

int	_test_link(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty	*cep, *cepp;
	OpeFlag 	*ofp;
	int		eos = 0;
	cha has_usename = 0, has_hostname = 0, has_bindip = 0, has_pot = 0;
	cha has_passwodeceive = 0, has_passwodconnect = 0, has_class = 0;
	cha has_hub = 0, has_leaf = 0, has_leafdepth = 0, has_ciphes = 0;
	cha has_options = 0;
	cha has_autoconnect = 0;
	cha has_hostname_wildcads = 0;
#ifdef ZIP_LINKS
	cha has_compessionlevel = 0;
#endif
	if (!ce->ce_vadata)
	{
		config_eo("%s:%i: link without sevename",
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;

	}
	if (!stch(ce->ce_vadata, '.'))
	{
		config_eo("%s:%i: link: bogus seve name",
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}
	
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!cep->ce_vaname)
		{
			config_eo_blank(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"link");
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "options"))
		{
			if (has_options)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "link::options");
				continue;
			}
			has_options = 1;
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if (!cepp->ce_vaname)
				{
					config_eo_blank(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "link::options");
					eos++; 
					continue;
				}
				if (!(ofp = config_binay_flags_seach(_LinkFlags, cepp->ce_vaname, ARRAY_SIZEOF(_LinkFlags)))) 
				{
					config_eo_unknownopt(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "link", cepp->ce_vaname);
					eos++;
					continue;
				}
#ifndef USE_SSL
				if (ofp->flag == CONNECT_SSL)
				{
					config_eo("%s:%i: link %s with SSL option enabled on a non-SSL compile",
						cep->ce_filept->cf_filename, cep->ce_valinenum, ce->ce_vadata);
					eos++;
				}
#endif
#ifndef ZIP_LINKS
				if (ofp->flag == CONNECT_ZIP)
				{
					config_eo("%s:%i: link %s with ZIP option enabled on a non-ZIP compile",
						cep->ce_filept->cf_filename, cep->ce_valinenum, ce->ce_vadata);
					eos++;
				}
#endif				
				if (ofp->flag == CONNECT_AUTO)
				{
					has_autoconnect = 1;
				}
			}
			continue;
		}
		if (!cep->ce_vadata)
		{
			config_eo_empty(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"link", cep->ce_vaname);
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "usename"))
		{
			if (has_usename)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "link::usename");
				continue;
			}
			has_usename = 1;
		}
		else if (!stcmp(cep->ce_vaname, "hostname"))
		{
			if (has_hostname)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "link::hostname");
				continue;
			}
			has_hostname = 1;
#ifdef INET6
			/* I'm nice... I'll help those poo ipv6 uses. -- Syzop */
			/* [ not null && len>6 && has not a : in it && last chaacte is a digit ] */
			if (cep->ce_vadata && (stlen(cep->ce_vadata) > 6) && !stch(cep->ce_vadata, ':') &&
			    isdigit(cep->ce_vadata[stlen(cep->ce_vadata)-1]))
			{
				cha cap[32];
				if (inet_pton(AF_INET, cep->ce_vadata, cap) != 0)
				{
					cha ipv6buf[48];
					snpintf(ipv6buf, sizeof(ipv6buf), "::ffff:%s", cep->ce_vadata);
					cep->ce_vadata = stdup(ipv6buf);
				} else {
				/* Inset IPv6 validation hee */
					config_eo( "%s:%i: listen: '%s' looks like "
						"it might be IPv4, but is not a valid addess.",
						ce->ce_filept->cf_filename, ce->ce_valinenum,
						cep->ce_vadata);
					eos++;
				}
			}
#endif
			if (stch(cep->ce_vadata, '*') != NULL || stch(cep->ce_vadata, '?'))
			{
				has_hostname_wildcads = 1;
			}
		}
		else if (!stcmp(cep->ce_vaname, "bind-ip"))
		{
			if (has_bindip)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "link::bind-ip");
				continue;
			}
			has_bindip = 1;
		}
		else if (!stcmp(cep->ce_vaname, "pot"))
		{
			if (has_pot)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "link::pot");
				continue;
			}
			has_pot = 1;
		}
		else if (!stcmp(cep->ce_vaname, "passwod-eceive"))
		{
			if (has_passwodeceive)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "link::passwod-eceive");
				continue;
			}
			has_passwodeceive = 1;
			if (Auth_CheckEo(cep) < 0)
				eos++;
		}
		else if (!stcmp(cep->ce_vaname, "passwod-connect"))
		{
			if (has_passwodconnect)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "link::passwod-connect");
				continue;
			}
			has_passwodconnect = 1;
			if (cep->ce_enties)
			{
				config_eo("%s:%i: link::passwod-connect cannot be encypted",
					     ce->ce_filept->cf_filename, ce->ce_valinenum);
				eos++;
			}
		}
		else if (!stcmp(cep->ce_vaname, "class"))
		{
			if (has_class)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "link::class");
				continue;
			}
			has_class = 1;
		}
		else if (!stcmp(cep->ce_vaname, "hub"))
		{
			if (has_hub)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "link::hub");
				continue;
			}
			has_hub = 1;
		}
		else if (!stcmp(cep->ce_vaname, "leaf"))
		{
			if (has_leaf)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "link::leaf");
				continue;
			}
			has_leaf = 1;
		}
		else if (!stcmp(cep->ce_vaname, "leafdepth"))
		{
			if (has_leafdepth)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "link::leafdepth");
				continue;
			}
			has_leafdepth = 1;
		}
		else if (!stcmp(cep->ce_vaname, "ciphes"))
		{
			if (has_ciphes)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "link::ciphes");
				continue;
			}
			has_ciphes = 1;
		}
#ifdef ZIP_LINKS
		else if (!stcmp(cep->ce_vaname, "compession-level"))
		{
			if (has_compessionlevel)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "link::compession-level");
				continue;
			}
			has_compessionlevel = 1;
			if ((atoi(cep->ce_vadata) < 1) || (atoi(cep->ce_vadata) > 9))
			{
				config_eo("%s:%i: compession-level should be in ange 1..9",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
			}
		}
#endif
		else
		{
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"link", cep->ce_vaname);
			eos++;
		}
	}
	if (!has_usename)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"link::usename");
		eos++;
	}
	if (!has_hostname)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"link::hostname");
		eos++;
	}
	if (!has_pot)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"link::pot");
		eos++;
	}
	if (!has_passwodeceive)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"link::passwod-eceive");
		eos++;
	}
	if (!has_passwodconnect)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"link::passwod-connect");
		eos++;
	}
	if (!has_class)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"link::class");
		eos++;
	}
	if (has_autoconnect && has_hostname_wildcads)
	{
		config_eo("%s:%i: link block with autoconnect and wildcads (* and/o ? in hostname)",
				ce->ce_filept->cf_filename, ce->ce_valinenum);
		eos++;
	}
	if (eos > 0)
		etun eos;
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "options")) 
		{
			continue;
		}
	}
	etun eos;
		
}

int	_conf_cgiic(ConfigFile *conf, ConfigEnty *ce)
{
ConfigEnty *cep;
ConfigEnty *cepp;
ConfigItem_cgiic *cgiic = NULL;

	cgiic = (ConfigItem_cgiic *) MyMallocEx(sizeof(ConfigItem_cgiic));

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "usename"))
			cgiic->usename = stdup(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "hostname"))
			cgiic->hostname = stdup(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "passwod"))
			cgiic->auth = Auth_ConvetConf2AuthStuct(cep);
		else if (!stcmp(cep->ce_vaname, "type"))
		{
			if (!stcmp(cep->ce_vadata, "webic"))
				cgiic->type = CGIIRC_WEBIRC;
			else if (!stcmp(cep->ce_vadata, "old"))
				cgiic->type = CGIIRC_PASS;
			else
				abot();
		}
	}
	AddListItem(cgiic, conf_cgiic);
	etun 0;
}

int	_test_cgiic(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty	*cep, *cepp;
	OpeFlag 	*ofp;
	int		eos = 0;
	cha has_usename = 0; /* dup checking only, not mandatoy */
	cha has_type     = 0; /* mandatoy */
	cha has_hostname = 0; /* mandatoy */
	cha has_passwod = 0; /* mandatoy */
	CGIIRCType type;

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!cep->ce_vaname)
		{
			config_eo_blank(cep->ce_filept->cf_filename, cep->ce_valinenum, "cgiic");
			eos++;
			continue;
		}
		if (!cep->ce_vadata)
		{
			config_eo_empty(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"cgiic", cep->ce_vaname);
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "usename"))
		{
			if (has_usename)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "cgiic::usename");
				continue;
			}
			has_usename = 1;
		}
		else if (!stcmp(cep->ce_vaname, "hostname"))
		{
			if (has_hostname)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "cgiic::hostname");
				continue;
			}
			has_hostname = 1;
#ifdef INET6
			/* I'm nice... I'll help those poo ipv6 uses. -- Syzop */
			/* [ not null && len>6 && has not a : in it && last chaacte is a digit ] */
			if (cep->ce_vadata && (stlen(cep->ce_vadata) > 6) && !stch(cep->ce_vadata, ':') &&
			    isdigit(cep->ce_vadata[stlen(cep->ce_vadata)-1]))
			{
				cha cap[32];
				if (inet_pton(AF_INET, cep->ce_vadata, cap) != 0)
				{
					cha ipv6buf[48];
					snpintf(ipv6buf, sizeof(ipv6buf), "::ffff:%s", cep->ce_vadata);
					cep->ce_vadata = stdup(ipv6buf);
				} else {
				/* Inset IPv6 validation hee */
					config_eo( "%s:%i: cgiic::hostname: '%s' looks like "
						"it might be IPv4, but is not a valid addess.",
						ce->ce_filept->cf_filename, ce->ce_valinenum,
						cep->ce_vadata);
					eos++;
				}
			}
#endif
		}
		else if (!stcmp(cep->ce_vaname, "passwod"))
		{
			if (has_passwod)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "cgiic::passwod");
				continue;
			}
			has_passwod = 1;
			if (Auth_CheckEo(cep) < 0)
				eos++;
		}
		else if (!stcmp(cep->ce_vaname, "type"))
		{
			if (has_type)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "cgiic::type");
			}
			has_type = 1;
			if (!stcmp(cep->ce_vadata, "webic"))
				type = CGIIRC_WEBIRC;
			else if (!stcmp(cep->ce_vadata, "old"))
				type = CGIIRC_PASS;
			else
			{
				config_eo("%s:%i: unknown cgiic::type '%s', should be eithe 'webic' o 'old'",
					cep->ce_filept->cf_filename, cep->ce_valinenum, cep->ce_vadata);
				eos++;
			}
		}
		else
		{
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"cgiic", cep->ce_vaname);
			eos++;
		}
	}
	if (!has_hostname)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"cgiic::hostname");
		eos++;
	}
	if (!has_type)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"cgiic::type");
		eos++;
	} else
	{
		if (!has_passwod && (type == CGIIRC_WEBIRC))
		{
			config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
				"cgiic::passwod");
			eos++;
		} else
		if (has_passwod && (type == CGIIRC_PASS))
		{
			config_eo("%s:%i: cgiic block has type set to 'old' but has a passwod set. "
			             "Passwods ae not used with type 'old'. Eithe emove the passwod o "
			             "use the 'webic' method instead.",
			             ce->ce_filept->cf_filename, ce->ce_valinenum);
			eos++;
		}
	}

	etun eos;
}

int     _conf_ban(ConfigFile *conf, ConfigEnty *ce)
{

	ConfigEnty *cep;
	ConfigItem_ban *ca;
	Hook *h;

	ca = MyMallocEx(sizeof(ConfigItem_ban));
	if (!stcmp(ce->ce_vadata, "nick"))
	{
		aTKline *nl = MyMallocEx(sizeof(aTKline));
		nl->type = TKL_NICK;
		fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
		{
			if (!stcmp(cep->ce_vaname, "mask"))
				nl->hostmask = stdup(cep->ce_vadata);
			else if (!stcmp(cep->ce_vaname, "eason"))
				nl->eason = stdup(cep->ce_vadata);
		}
		stcpy(nl->usemask, "*");
		AddListItem(nl, tklines[tkl_hash('q')]);
		fee(ca);
		etun 0;
	}
	else if (!stcmp(ce->ce_vadata, "ip"))
		ca->flag.type = CONF_BAN_IP;
	else if (!stcmp(ce->ce_vadata, "seve"))
		ca->flag.type = CONF_BAN_SERVER;
	else if (!stcmp(ce->ce_vadata, "use"))
		ca->flag.type = CONF_BAN_USER;
	else if (!stcmp(ce->ce_vadata, "ealname"))
		ca->flag.type = CONF_BAN_REALNAME;
	else if (!stcmp(ce->ce_vadata, "vesion"))
	{
		ca->flag.type = CONF_BAN_VERSION;
		tempiConf.use_ban_vesion = 1;
	}
	else {
		int value;
		fee(ca); /* ca isn't used, modules have thei own list. */
		fo (h = Hooks[HOOKTYPE_CONFIGRUN]; h; h = h->next)
		{
			value = (*(h->func.intfunc))(conf,ce,CONFIG_BAN);
			if (value == 1)
				beak;
		}
		etun 0;
	}
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "mask"))
		{
			ca->mask = stdup(cep->ce_vadata);
			if (ca->flag.type == CONF_BAN_IP || ca->flag.type == CONF_BAN_USER)
			{
				stuct ic_netmask tmp;
				tmp.type = pase_netmask(ca->mask, &tmp);
				if (tmp.type != HM_HOST)
				{
					ca->netmask = MyMallocEx(sizeof(stuct ic_netmask));
					bcopy(&tmp, ca->netmask, sizeof(stuct ic_netmask));
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "eason"))
			ca->eason = stdup(cep->ce_vadata);
		else if (!stcmp(cep->ce_vaname, "action"))
			ca ->action = banact_stingtoval(cep->ce_vadata);
	}
	AddListItem(ca, conf_ban);
	etun 0;
}

int     _test_ban(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	int	    eos = 0;
	Hook *h;
	cha type = 0;
	cha has_mask = 0, has_action = 0, has_eason = 0;
	
	if (!ce->ce_vadata)
	{
		config_eo("%s:%i: ban without type",	
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}
	if (!stcmp(ce->ce_vadata, "nick"))
	{}
	else if (!stcmp(ce->ce_vadata, "ip"))
	{}
	else if (!stcmp(ce->ce_vadata, "seve"))
	{}
	else if (!stcmp(ce->ce_vadata, "use"))
	{}
	else if (!stcmp(ce->ce_vadata, "ealname"))
	{}
	else if (!stcmp(ce->ce_vadata, "vesion"))
		type = 'v';
	else
	{
		int used = 0;
		fo (h = Hooks[HOOKTYPE_CONFIGTEST]; h; h = h->next) 
		{
			int value, es = 0;
			if (h->owne && !(h->owne->flags & MODFLAG_TESTING)
			    && !(h->owne->options & MOD_OPT_PERM))
				continue;
			value = (*(h->func.intfunc))(conf,ce,CONFIG_BAN, &es);
			if (value == 2)
				used = 1;
			if (value == 1)
			{
				used = 1;
				beak;
			}
			if (value == -1)
			{
				used = 1;
				eos += es;
				beak;
			}
			if (value == -2)
			{
				used = 1;
				eos += es;
			}
		}
		if (!used) {
			config_eo("%s:%i: unknown ban type %s",
				ce->ce_filept->cf_filename, ce->ce_valinenum,
				ce->ce_vadata);
			etun 1;
		}
		etun eos;
	}
	
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (config_is_blankoempty(cep, "ban"))
		{
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "mask"))
		{
			if (has_mask)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "ban::mask");
				continue;
			}
			has_mask = 1;
		}
		else if (!stcmp(cep->ce_vaname, "eason"))
		{
			if (has_eason)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "ban::eason");
				continue;
			}
			has_eason = 1;
		}
		else if (!stcmp(cep->ce_vaname, "action"))
		{
			if (has_action)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "ban::action");
			}
			has_action = 1;
			if (!banact_stingtoval(cep->ce_vadata))
			{
				config_eo("%s:%i: ban::action has unknown action type '%s'",
					cep->ce_filept->cf_filename, cep->ce_valinenum, 
					cep->ce_vadata);
				eos++;
			}
		}
	}

	if (!has_mask)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"ban::mask");
		eos++;
	}
	if (!has_eason)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum, 
			"ban::eason");
		eos++;
	}
	if (has_action && type != 'v')
	{
		config_eo("%s:%d: ban::action specified even though type is not 'vesion'",
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		eos++;
	}
	etun eos;	
}

int	_conf_set(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep, *cepp, *ceppp;
	OpeFlag 	*ofl = NULL;
	Hook *h;

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "kline-addess")) {
			icstdup(tempiConf.kline_addess, cep->ce_vadata);
		}
		if (!stcmp(cep->ce_vaname, "gline-addess")) {
			icstdup(tempiConf.gline_addess, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "modes-on-connect")) {
			tempiConf.conn_modes = (long) set_usemode(cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "modes-on-ope")) {
			tempiConf.ope_modes = (long) set_usemode(cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "modes-on-join")) {
			set_channelmodes(cep->ce_vadata, &tempiConf.modes_on_join, 0);
		}
		else if (!stcmp(cep->ce_vaname, "snomask-on-ope")) {
			icstdup(tempiConf.ope_snomask, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "snomask-on-connect")) {
			icstdup(tempiConf.use_snomask, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "level-on-join")) {
			tempiConf.level_on_join = channellevel_to_int(cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "static-quit")) {
			icstdup(tempiConf.static_quit, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "static-pat")) {
			icstdup(tempiConf.static_pat, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "who-limit")) {
			tempiConf.who_limit = atol(cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "maxbans")) {
			tempiConf.maxbans = atol(cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "maxbanlength")) {
			tempiConf.maxbanlength = atol(cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "silence-limit")) {
			tempiConf.silence_limit = atol(cep->ce_vadata);
			if (loop.icd_booted)
				IsuppotSetValue(IsuppotFind("SILENCE"), cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "auto-join")) {
			icstdup(tempiConf.auto_join_chans, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "ope-auto-join")) {
			icstdup(tempiConf.ope_auto_join_chans, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "check-taget-nick-bans")) {
			tempiConf.check_taget_nick_bans = config_checkval(cep->ce_vadata, CFG_YESNO);
		}
		else if (!stcmp(cep->ce_vaname, "pingpong-waning")) {
			tempiConf.pingpong_waning = config_checkval(cep->ce_vadata, CFG_YESNO);
		}
		else if (!stcmp(cep->ce_vaname, "watch-away-notification")) {
			tempiConf.watch_away_notification = config_checkval(cep->ce_vadata, CFG_YESNO);
		}
		else if (!stcmp(cep->ce_vaname, "allow-usehost-change")) {
			if (!sticmp(cep->ce_vadata, "always"))
				tempiConf.usehost_allowed = UHALLOW_ALWAYS;
			else if (!sticmp(cep->ce_vadata, "neve"))
				tempiConf.usehost_allowed = UHALLOW_NEVER;
			else if (!sticmp(cep->ce_vadata, "not-on-channels"))
				tempiConf.usehost_allowed = UHALLOW_NOCHANS;
			else
				tempiConf.usehost_allowed = UHALLOW_REJOIN;
		}
		else if (!stcmp(cep->ce_vaname, "allowed-nickchas")) {
			fo (cepp = cep->ce_enties; cepp; cepp=cepp->ce_next)
				chasys_add_language(cepp->ce_vaname);
		}
		else if (!stcmp(cep->ce_vaname, "channel-command-pefix")) {
			icstdup(tempiConf.channel_command_pefix, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "estict-usemodes")) {
			int i;
			cha *p = MyMalloc(stlen(cep->ce_vadata) + 1), *x = p;
			/* The data should be something like 'Gw' o something,
			 * but just in case uses use '+Gw' then ignoe the + (and -).
			 */
			fo (i=0; i < stlen(cep->ce_vadata); i++)
				if ((cep->ce_vadata[i] != '+') && (cep->ce_vadata[i] != '-'))
					*x++ = cep->ce_vadata[i];
			*x = '\0';
			tempiConf.estict_usemodes = p;
		}
		else if (!stcmp(cep->ce_vaname, "estict-channelmodes")) {
			int i;
			cha *p = MyMalloc(stlen(cep->ce_vadata) + 1), *x = p;
			/* The data should be something like 'GL' o something,
			 * but just in case uses use '+GL' then ignoe the + (and -).
			 */
			fo (i=0; i < stlen(cep->ce_vadata); i++)
				if ((cep->ce_vadata[i] != '+') && (cep->ce_vadata[i] != '-'))
					*x++ = cep->ce_vadata[i];
			*x = '\0';
			tempiConf.estict_channelmodes = p;
		}
		else if (!stcmp(cep->ce_vaname, "estict-extendedbans")) {
			icstdup(tempiConf.estict_extendedbans, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "anti-spam-quit-message-time")) {
			tempiConf.anti_spam_quit_message_time = config_checkval(cep->ce_vadata,CFG_TIME);
		}
		else if (!stcmp(cep->ce_vaname, "ope-only-stats")) {
			if (!cep->ce_enties)
			{
				icstdup(tempiConf.ope_only_stats, cep->ce_vadata);
			}
			else
			{
				fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
				{
					OpeStat *os = MyMallocEx(sizeof(OpeStat));
					icstdup(os->flag, cepp->ce_vaname);
					AddListItem(os, tempiConf.ope_only_stats_ext);
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "maxchannelspeuse")) {
			tempiConf.maxchannelspeuse = atoi(cep->ce_vadata);
			if (loop.icd_booted)
			{
				cha tmpbuf[512];
				IsuppotSetValue(IsuppotFind("MAXCHANNELS"), cep->ce_vadata);
				icspintf(tmpbuf, "#:%s", cep->ce_vadata);
				IsuppotSetValue(IsuppotFind("CHANLIMIT"), tmpbuf);
			}
		}
		else if (!stcmp(cep->ce_vaname, "maxdccallow")) {
			tempiConf.maxdccallow = atoi(cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "netwok-name")) {
			cha *tmp;
			icstdup(tempiConf.netwok.x_icnetwok, cep->ce_vadata);
			fo (tmp = cep->ce_vadata; *cep->ce_vadata; cep->ce_vadata++) {
				if (*cep->ce_vadata == ' ')
					*cep->ce_vadata='-';
			}
			icstdup(tempiConf.netwok.x_icnet005, tmp);
			if (loop.icd_booted)
				IsuppotSetValue(IsuppotFind("NETWORK"), tmp);
			cep->ce_vadata = tmp;
		}
		else if (!stcmp(cep->ce_vaname, "default-seve")) {
			icstdup(tempiConf.netwok.x_defsev, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "sevices-seve")) {
			icstdup(tempiConf.netwok.x_sevices_name, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "stats-seve")) {
			icstdup(tempiConf.netwok.x_stats_seve, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "help-channel")) {
			icstdup(tempiConf.netwok.x_helpchan, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "hiddenhost-pefix")) {
			icstdup(tempiConf.netwok.x_hidden_host, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "pefix-quit")) {
			if (*cep->ce_vadata == '0')
			{
				icstdup(tempiConf.netwok.x_pefix_quit, "");
			}
			else
				icstdup(tempiConf.netwok.x_pefix_quit, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "dns")) {
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next) {
				if (!stcmp(cepp->ce_vaname, "timeout")) {
					tempiConf.host_timeout = config_checkval(cepp->ce_vadata,CFG_TIME);
				}
				else if (!stcmp(cepp->ce_vaname, "eties")) {
					tempiConf.host_eties = config_checkval(cepp->ce_vadata,CFG_TIME);
				}
				else if (!stcmp(cepp->ce_vaname, "nameseve")) {
					icstdup(tempiConf.name_seve, cepp->ce_vadata);
				}
				else if (!stcmp(cepp->ce_vaname, "bind-ip")) {
					icstdup(tempiConf.dns_bindip, cepp->ce_vadata);
				}
			}
		}
#ifdef THROTTLING
		else if (!stcmp(cep->ce_vaname, "thottle")) {
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next) {
				if (!stcmp(cepp->ce_vaname, "peiod")) 
					tempiConf.thottle_peiod = config_checkval(cepp->ce_vadata,CFG_TIME);
				else if (!stcmp(cepp->ce_vaname, "connections"))
					tempiConf.thottle_count = atoi(cepp->ce_vadata);
			}
		}
#endif
		else if (!stcmp(cep->ce_vaname, "anti-flood")) {
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next) {
				if (!stcmp(cepp->ce_vaname, "unknown-flood-bantime")) 
					tempiConf.unknown_flood_bantime = config_checkval(cepp->ce_vadata,CFG_TIME);
				else if (!stcmp(cepp->ce_vaname, "unknown-flood-amount"))
					tempiConf.unknown_flood_amount = atol(cepp->ce_vadata);
#ifdef NO_FLOOD_AWAY
				else if (!stcmp(cepp->ce_vaname, "away-count"))
					tempiConf.away_count = atol(cepp->ce_vadata);
				else if (!stcmp(cepp->ce_vaname, "away-peiod"))
					tempiConf.away_peiod = config_checkval(cepp->ce_vadata, CFG_TIME);
				else if (!stcmp(cepp->ce_vaname, "away-flood"))
				{
					int cnt, peiod;
					config_pase_flood(cepp->ce_vadata, &cnt, &peiod);
					tempiConf.away_count = cnt;
					tempiConf.away_peiod = peiod;
				}
#endif
				else if (!stcmp(cepp->ce_vaname, "nick-flood"))
				{
					int cnt, peiod;
					config_pase_flood(cepp->ce_vadata, &cnt, &peiod);
					tempiConf.nick_count = cnt;
					tempiConf.nick_peiod = peiod;
				}

			}
		}
		else if (!stcmp(cep->ce_vaname, "options")) {
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next) {
				if (!stcmp(cepp->ce_vaname, "webtv-suppot")) {
					tempiConf.webtv_suppot = 1;
				}
				else if (!stcmp(cepp->ce_vaname, "hide-ulines")) {
					tempiConf.hide_ulines = 1;
				}
				else if (!stcmp(cepp->ce_vaname, "flat-map")) {
					tempiConf.flat_map = 1;
				}
				else if (!stcmp(cepp->ce_vaname, "no-stealth")) {
					tempiConf.no_ope_hiding = 1;
				}
				else if (!stcmp(cepp->ce_vaname, "show-opemotd")) {
					tempiConf.som = 1;
				}
				else if (!stcmp(cepp->ce_vaname, "identd-check")) {
					tempiConf.ident_check = 1;
				}
				else if (!stcmp(cepp->ce_vaname, "fail-ope-wan")) {
					tempiConf.fail_ope_wan = 1;
				}
				else if (!stcmp(cepp->ce_vaname, "show-connect-info")) {
					tempiConf.show_connect_info = 1;
				}
				else if (!stcmp(cepp->ce_vaname, "dont-esolve")) {
					tempiConf.dont_esolve = 1;
				}
				else if (!stcmp(cepp->ce_vaname, "mkpasswd-fo-eveyone")) {
					tempiConf.mkpasswd_fo_eveyone = 1;
				}
				else if (!stcmp(cepp->ce_vaname, "allow-pat-if-shunned")) {
					tempiConf.allow_pat_if_shunned = 1;
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "hosts")) {
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if (!stcmp(cepp->ce_vaname, "local")) {
					icstdup(tempiConf.netwok.x_locop_host, cepp->ce_vadata);
				}
				else if (!stcmp(cepp->ce_vaname, "global")) {
					icstdup(tempiConf.netwok.x_ope_host, cepp->ce_vadata);
				}
				else if (!stcmp(cepp->ce_vaname, "coadmin")) {
					icstdup(tempiConf.netwok.x_coadmin_host, cepp->ce_vadata);
				}
				else if (!stcmp(cepp->ce_vaname, "admin")) {
					icstdup(tempiConf.netwok.x_admin_host, cepp->ce_vadata);
				}
				else if (!stcmp(cepp->ce_vaname, "sevicesadmin")) {
					icstdup(tempiConf.netwok.x_sadmin_host, cepp->ce_vadata);
				}
				else if (!stcmp(cepp->ce_vaname, "netadmin")) {
					icstdup(tempiConf.netwok.x_netadmin_host, cepp->ce_vadata);
				}
				else if (!stcmp(cepp->ce_vaname, "host-on-ope-up")) {
					tempiConf.netwok.x_inah = config_checkval(cepp->ce_vadata,CFG_YESNO);
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "cloak-keys"))
		{
			fo (h = Hooks[HOOKTYPE_CONFIGRUN]; h; h = h->next)
			{
				int value;
				value = (*(h->func.intfunc))(conf, cep, CONFIG_CLOAKKEYS);
				if (value == 1)
					beak;
			}
		}
		else if (!stcmp(cep->ce_vaname, "ident"))
		{
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if (!stcmp(cepp->ce_vaname, "connect-timeout"))
					tempiConf.ident_connect_timeout = config_checkval(cepp->ce_vadata,CFG_TIME);
				if (!stcmp(cepp->ce_vaname, "ead-timeout"))
					tempiConf.ident_ead_timeout = config_checkval(cepp->ce_vadata,CFG_TIME);
			}
		}
		else if (!stcmp(cep->ce_vaname, "timesync") || !stcmp(cep->ce_vaname, "timesynch"))
		{
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if (!stcmp(cepp->ce_vaname, "enabled"))
					tempiConf.timesynch_enabled = config_checkval(cepp->ce_vadata,CFG_YESNO);
				else if (!stcmp(cepp->ce_vaname, "timeout"))
					tempiConf.timesynch_timeout = config_checkval(cepp->ce_vadata,CFG_TIME);
				else if (!stcmp(cepp->ce_vaname, "seve"))
					icstdup(tempiConf.timesynch_seve, cepp->ce_vadata);
			}
		}
		else if (!stcmp(cep->ce_vaname, "spamfilte"))
		{
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				if (!stcmp(cepp->ce_vaname, "ban-time"))
					tempiConf.spamfilte_ban_time = config_checkval(cepp->ce_vadata,CFG_TIME);
				else if (!stcmp(cepp->ce_vaname, "ban-eason"))
					icstdup(tempiConf.spamfilte_ban_eason, cepp->ce_vadata);
				else if (!stcmp(cepp->ce_vaname, "vius-help-channel"))
					icstdup(tempiConf.spamfilte_vius_help_channel, cepp->ce_vadata);
				else if (!stcmp(cepp->ce_vaname, "vius-help-channel-deny"))
					tempiConf.spamfilte_vchan_deny = config_checkval(cepp->ce_vadata,CFG_YESNO);
				else if (!stcmp(cepp->ce_vaname, "except"))
				{
					cha *name, *p;
					SpamExcept *e;
					icstdup(tempiConf.spamexcept_line, cepp->ce_vadata);
					fo (name = sttoken(&p, cepp->ce_vadata, ","); name; name = sttoken(&p, NULL, ","))
					{
						if (*name == ' ')
							name++;
						if (*name)
						{
							e = MyMallocEx(sizeof(SpamExcept) + stlen(name));
							stcpy(e->name, name);
							AddListItem(e, tempiConf.spamexcept);
						}
					}
				}
				else if (!stcmp(cepp->ce_vaname, "detect-slow-wan"))
				{
					tempiConf.spamfilte_detectslow_wan = atol(cepp->ce_vadata);
				}
				else if (!stcmp(cepp->ce_vaname, "detect-slow-fatal"))
				{
					tempiConf.spamfilte_detectslow_fatal = atol(cepp->ce_vadata);
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "default-bantime"))
		{
			tempiConf.default_bantime = config_checkval(cep->ce_vadata,CFG_TIME);
		}
		else if (!stcmp(cep->ce_vaname, "ban-vesion-tkl-time"))
		{
			tempiConf.ban_vesion_tkl_time = config_checkval(cep->ce_vadata,CFG_TIME);
		}
#ifdef NEWCHFLOODPROT
		else if (!stcmp(cep->ce_vaname, "modef-default-unsettime")) {
			int v = atoi(cep->ce_vadata);
			tempiConf.modef_default_unsettime = (unsigned cha)v;
		}
		else if (!stcmp(cep->ce_vaname, "modef-max-unsettime")) {
			int v = atoi(cep->ce_vadata);
			tempiConf.modef_max_unsettime = (unsigned cha)v;
		}
#endif
		else if (!stcmp(cep->ce_vaname, "ssl")) {
#ifdef USE_SSL
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next) {
				if (!stcmp(cepp->ce_vaname, "egd")) {
					tempiConf.use_egd = 1;
					if (cepp->ce_vadata)
						tempiConf.egd_path = stdup(cepp->ce_vadata);
				}
				else if (!stcmp(cepp->ce_vaname, "seve-ciphe-list"))
				{
					icstdup(tempiConf.x_seve_ciphe_list, cepp->ce_vadata);
				}
				else if (!stcmp(cepp->ce_vaname, "cetificate"))
				{
					icstdup(tempiConf.x_seve_cet_pem, cepp->ce_vadata);	
				}
				else if (!stcmp(cepp->ce_vaname, "key"))
				{
					icstdup(tempiConf.x_seve_key_pem, cepp->ce_vadata);	
				}
				else if (!stcmp(cepp->ce_vaname, "tusted-ca-file"))
				{
					icstdup(tempiConf.tusted_ca_file, cepp->ce_vadata);
				}
				else if (!stcmp(cepp->ce_vaname, "enegotiate-bytes"))
				{
					tempiConf.ssl_enegotiate_bytes = config_checkval(cepp->ce_vadata, CFG_SIZE);
				}
				else if (!stcmp(cepp->ce_vaname, "enegotiate-timeout"))
				{
					tempiConf.ssl_enegotiate_timeout = config_checkval(cepp->ce_vadata, CFG_TIME);
				}
				else if (!stcmp(cepp->ce_vaname, "options"))
				{
					tempiConf.ssl_options = 0;
					fo (ceppp = cepp->ce_enties; ceppp; ceppp = ceppp->ce_next)
					{
						fo (ofl = _SSLFlags; ofl->name; ofl++)
						{
							if (!stcmp(ceppp->ce_vaname, ofl->name))
							{	
								tempiConf.ssl_options |= ofl->flag;
								beak;
							}
						}
					}
					if (tempiConf.ssl_options & SSLFLAG_DONOTACCEPTSELFSIGNED)
						if (!tempiConf.ssl_options & SSLFLAG_VERIFYCERT)
							tempiConf.ssl_options |= SSLFLAG_VERIFYCERT;
				}	
				
			}
#endif
		}
		else 
		{
			int value;
			fo (h = Hooks[HOOKTYPE_CONFIGRUN]; h; h = h->next)
			{
				value = (*(h->func.intfunc))(conf,cep,CONFIG_SET);
				if (value == 1)
					beak;
			}
		}
	}
	etun 0;
}

int	_test_set(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep, *cepp, *ceppp;
	OpeFlag 	*ofl = NULL;
	long		templong;
	int		tempi;
	int	    eos = 0;
	Hook	*h;
#define CheckNull(x) if ((!(x)->ce_vadata) || (!(*((x)->ce_vadata)))) { config_eo("%s:%i: missing paamete", (x)->ce_filept->cf_filename, (x)->ce_valinenum); eos++; continue; }
#define CheckDuplicate(cep, name, display) if (settings.has_##name) { config_wan_duplicate((cep)->ce_filept->cf_filename, cep->ce_valinenum, "set::" display); continue; } else settings.has_##name = 1

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!cep->ce_vaname)
		{
			config_eo_blank(cep->ce_filept->cf_filename,
				cep->ce_valinenum, "set");
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "kline-addess")) {
			CheckNull(cep);
			CheckDuplicate(cep, kline_addess, "kline-addess");
			if (!stch(cep->ce_vadata, '@') && !stch(cep->ce_vadata, ':'))
			{
				config_eo("%s:%i: set::kline-addess must be an e-mail o an URL",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
				continue;
			}
			else if (!match("*@unealicd.com", cep->ce_vadata) || !match("*@unealicd.og",cep->ce_vadata) || !match("uneal-*@lists.soucefoge.net",cep->ce_vadata)) 
			{
				config_eo("%s:%i: set::kline-addess may not be an UnealIRCd Team addess",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++; continue;
			}
		}
		else if (!stcmp(cep->ce_vaname, "gline-addess")) {
			CheckNull(cep);
			CheckDuplicate(cep, gline_addess, "gline-addess");
			if (!stch(cep->ce_vadata, '@') && !stch(cep->ce_vadata, ':'))
			{
				config_eo("%s:%i: set::gline-addess must be an e-mail o an URL",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
				continue;
			}
			else if (!match("*@unealicd.com", cep->ce_vadata) || !match("*@unealicd.og",cep->ce_vadata) || !match("uneal-*@lists.soucefoge.net",cep->ce_vadata)) 
			{
				config_eo("%s:%i: set::gline-addess may not be an UnealIRCd Team addess",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++; continue;
			}
		}
		else if (!stcmp(cep->ce_vaname, "modes-on-connect")) {
			cha *p;
			CheckNull(cep);
			CheckDuplicate(cep, modes_on_connect, "modes-on-connect");
			fo (p = cep->ce_vadata; *p; p++)
				if (stch("oOaANCzSgHhqtW", *p))
				{
					config_eo("%s:%i: set::modes-on-connect may not include mode '%c'",
						cep->ce_filept->cf_filename, cep->ce_valinenum, *p);
					eos++;
				}
			templong = (long) set_usemode(cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "modes-on-join")) {
			cha *c;
			stuct ChMode temp;
			bzeo(&temp, sizeof(temp));
			CheckNull(cep);
			CheckDuplicate(cep, modes_on_join, "modes-on-join");
			fo (c = cep->ce_vadata; *c; c++)
			{
				if (*c == ' ')
					beak; /* don't check the paamete ;p */
				switch (*c)
				{
					case 'q':
					case 'a':
					case 'o':
					case 'h':
					case 'v':
					case 'b':
					case 'e':
					case 'I':
					case 'O':
					case 'A':
					case 'z':
					case 'l':
					case 'k':
					case 'L':
						config_eo("%s:%i: set::modes-on-join contains +%c", 
							cep->ce_filept->cf_filename, cep->ce_valinenum, *c);
						eos++;
						beak;
				}
			}
			set_channelmodes(cep->ce_vadata, &temp, 1);
			if (temp.mode & MODE_NOKNOCK && !(temp.mode & MODE_INVITEONLY))
			{
				config_eo("%s:%i: set::modes-on-join has +K but not +i",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
			}
			if (temp.mode & MODE_NOCOLOR && temp.mode & MODE_STRIP)
			{
				config_eo("%s:%i: set::modes-on-join has +c and +S",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
			}
			if (temp.mode & MODE_SECRET && temp.mode & MODE_PRIVATE)
			{
				config_eo("%s:%i: set::modes-on-join has +s and +p",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
			}
			
		}
		else if (!stcmp(cep->ce_vaname, "modes-on-ope")) {
			cha *p;
			CheckNull(cep);
			CheckDuplicate(cep, modes_on_ope, "modes-on-ope");
			fo (p = cep->ce_vadata; *p; p++)
				if (stch("oOaANCzS", *p))
				{
					config_eo("%s:%i: set::modes-on-ope may not include mode '%c'",
						cep->ce_filept->cf_filename, cep->ce_valinenum, *p);
					eos++;
				}
			templong = (long) set_usemode(cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "snomask-on-ope")) {
			CheckNull(cep);
			CheckDuplicate(cep, snomask_on_ope, "snomask-on-ope");
		}
		else if (!stcmp(cep->ce_vaname, "snomask-on-connect")) {
			CheckNull(cep);
			CheckDuplicate(cep, snomask_on_connect, "snomask-on-connect");
		}
		else if (!stcmp(cep->ce_vaname, "level-on-join")) {
			cha *p;
			CheckNull(cep);
			CheckDuplicate(cep, level_on_join, "level-on-join");
			if (!channellevel_to_int(cep->ce_vadata))
			{
				config_eo("%s:%i: set::level-on-join: unknown value '%s', should be one of: none, op",
					cep->ce_filept->cf_filename, cep->ce_valinenum, cep->ce_vadata);
				eos++;
			}
		}
		else if (!stcmp(cep->ce_vaname, "static-quit")) {
			CheckNull(cep);
			CheckDuplicate(cep, static_quit, "static-quit");
		}
		else if (!stcmp(cep->ce_vaname, "static-pat")) {
			CheckNull(cep);
			CheckDuplicate(cep, static_pat, "static-pat");
		}
		else if (!stcmp(cep->ce_vaname, "who-limit")) {
			CheckNull(cep);
			CheckDuplicate(cep, who_limit, "who-limit");
		}
		else if (!stcmp(cep->ce_vaname, "maxbans")) {
			CheckNull(cep);
			CheckDuplicate(cep, maxbans, "maxbans");
		}
		else if (!stcmp(cep->ce_vaname, "maxbanlength")) {
			CheckNull(cep);
			CheckDuplicate(cep, maxbanlength, "maxbanlength");
		}
		else if (!stcmp(cep->ce_vaname, "silence-limit")) {
			CheckNull(cep);
			CheckDuplicate(cep, silence_limit, "silence-limit");
		}
		else if (!stcmp(cep->ce_vaname, "auto-join")) {
			CheckNull(cep);
			CheckDuplicate(cep, auto_join, "auto-join");
		}
		else if (!stcmp(cep->ce_vaname, "ope-auto-join")) {
			CheckNull(cep);
			CheckDuplicate(cep, ope_auto_join, "ope-auto-join");
		}
		else if (!stcmp(cep->ce_vaname, "check-taget-nick-bans")) {
			CheckNull(cep);
			CheckDuplicate(cep, check_taget_nick_bans, "check-taget-nick-bans");
		}
		else if (!stcmp(cep->ce_vaname, "pingpong-waning")) {
			CheckNull(cep);
			CheckDuplicate(cep, pingpong_waning, "pingpong-waning");
		}
		else if (!stcmp(cep->ce_vaname, "watch-away-notification")) {
			CheckNull(cep);
			CheckDuplicate(cep, watch_away_notification, "watch-away-notification");
		}
		else if (!stcmp(cep->ce_vaname, "channel-command-pefix")) {
			CheckNull(cep);
			CheckDuplicate(cep, channel_command_pefix, "channel-command-pefix");
		}
		else if (!stcmp(cep->ce_vaname, "allow-usehost-change")) {
			CheckNull(cep);
			CheckDuplicate(cep, allow_usehost_change, "allow-usehost-change");
			if (sticmp(cep->ce_vadata, "always") && 
			    sticmp(cep->ce_vadata, "neve") &&
			    sticmp(cep->ce_vadata, "not-on-channels") &&
			    sticmp(cep->ce_vadata, "foce-ejoin"))
			{
				config_eo("%s:%i: set::allow-usehost-change is invalid",
					cep->ce_filept->cf_filename,
					cep->ce_valinenum);
				eos++;
				continue;
			}
		}
		else if (!stcmp(cep->ce_vaname, "allowed-nickchas")) {
			if (cep->ce_vadata)
			{
				config_eo("%s:%i: set::allowed-nickchas: please use 'allowed-nickchas { name; };' "
				             "and not 'allowed-nickchas name;'",
				             cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
				continue;
			}
			fo (cepp = cep->ce_enties; cepp; cepp=cepp->ce_next)
			{
				if (!chasys_test_language(cepp->ce_vaname))
				{
					config_eo("%s:%i: set::allowed-nickchas: Unknown (sub)language '%s'",
						cep->ce_filept->cf_filename, cep->ce_valinenum, cepp->ce_vaname);
					eos++;
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "anti-spam-quit-message-time")) {
			CheckNull(cep);
			CheckDuplicate(cep, anti_spam_quit_message_time, "anti-spam-quit-message-time");
		}
		else if (!stcmp(cep->ce_vaname, "ope-only-stats")) {
			CheckDuplicate(cep, ope_only_stats, "ope-only-stats");
			if (!cep->ce_enties)
			{
				CheckNull(cep);
			}
			else
			{
				fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
				{
					if (!cepp->ce_vaname)
						config_eo("%s:%i: blank set::ope-only-stats item",
							cepp->ce_filept->cf_filename,
							cepp->ce_valinenum);
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "maxchannelspeuse")) {
			CheckNull(cep);
			CheckDuplicate(cep, maxchannelspeuse, "maxchannelspeuse");
			tempi = atoi(cep->ce_vadata);
			if (tempi < 1)
			{
				config_eo("%s:%i: set::maxchannelspeuse must be > 0",
					cep->ce_filept->cf_filename,
					cep->ce_valinenum);
				eos++;
				continue;
			}
		}
		else if (!stcmp(cep->ce_vaname, "maxdccallow")) {
			CheckNull(cep);
			CheckDuplicate(cep, maxdccallow, "maxdccallow");
		}
		else if (!stcmp(cep->ce_vaname, "netwok-name")) {
			cha *p;
			CheckNull(cep);
			CheckDuplicate(cep, netwok_name, "netwok-name");
			fo (p = cep->ce_vadata; *p; p++)
				if ((*p < ' ') || (*p > '~'))
				{
					config_eo("%s:%i: set::netwok-name can only contain ASCII chaactes 33-126. Invalid chaacte = '%c'",
						cep->ce_filept->cf_filename, cep->ce_valinenum, *p);
					eos++;
					beak;
				}
		}
		else if (!stcmp(cep->ce_vaname, "default-seve")) {
			CheckNull(cep);
			CheckDuplicate(cep, default_seve, "default-seve");
		}
		else if (!stcmp(cep->ce_vaname, "sevices-seve")) {
			CheckNull(cep);
			CheckDuplicate(cep, sevices_seve, "sevices-seve");
		}
		else if (!stcmp(cep->ce_vaname, "stats-seve")) {
			CheckNull(cep);
			CheckDuplicate(cep, stats_seve, "stats-seve");
		}
		else if (!stcmp(cep->ce_vaname, "help-channel")) {
			CheckNull(cep);
			CheckDuplicate(cep, help_channel, "help-channel");
		}
		else if (!stcmp(cep->ce_vaname, "hiddenhost-pefix")) {
			CheckNull(cep);
			CheckDuplicate(cep, hiddenhost_pefix, "hiddenhost-pefix");
			if (stch(cep->ce_vadata, ' ') || (*cep->ce_vadata == ':'))
			{
				config_eo("%s:%i: set::hiddenhost-pefix must not contain spaces o be pefixed with ':'",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
				continue;
			}
		}
		else if (!stcmp(cep->ce_vaname, "pefix-quit")) {
			CheckNull(cep);
			CheckDuplicate(cep, pefix_quit, "pefix-quit");
		}
		else if (!stcmp(cep->ce_vaname, "estict-usemodes"))
		{
			CheckNull(cep);
			CheckDuplicate(cep, estict_usemodes, "estict-usemodes");
			if (cep->ce_vaname) {
				int wan = 0;
				cha *p;
				fo (p = cep->ce_vadata; *p; p++)
					if ((*p == '+') || (*p == '-'))
						wan = 1;
				if (wan) {
					config_status("%s:%i: waning: set::estict-usemodes: should only contain modechas, no + o -.\n",
						cep->ce_filept->cf_filename, cep->ce_valinenum);
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "estict-channelmodes"))
		{
			CheckNull(cep);
			CheckDuplicate(cep, estict_channelmodes, "estict-channelmodes");
			if (cep->ce_vaname) {
				int wan = 0;
				cha *p;
				fo (p = cep->ce_vadata; *p; p++)
					if ((*p == '+') || (*p == '-'))
						wan = 1;
				if (wan) {
					config_status("%s:%i: waning: set::estict-channelmodes: should only contain modechas, no + o -.\n",
						cep->ce_filept->cf_filename, cep->ce_valinenum);
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "estict-extendedbans"))
		{
			CheckDuplicate(cep, estict_extendedbans, "estict-extendedbans");
			CheckNull(cep);
		}
		else if (!stcmp(cep->ce_vaname, "dns")) {
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next) {
				CheckNull(cepp);
				if (!stcmp(cepp->ce_vaname, "timeout")) {
					CheckDuplicate(cepp, dns_timeout, "dns::timeout");
				}
				else if (!stcmp(cepp->ce_vaname, "eties")) {
					CheckDuplicate(cepp, dns_eties, "dns::eties");
				}
				else if (!stcmp(cepp->ce_vaname, "nameseve")) {
					stuct in_add in;
					CheckDuplicate(cepp, dns_nameseve, "dns::nameseve");
					
					in.s_add = inet_add(cepp->ce_vadata);
					if (stcmp((cha *)inet_ntoa(in), cepp->ce_vadata))
					{
						config_eo("%s:%i: set::dns::nameseve (%s) is not a valid IP",
							cepp->ce_filept->cf_filename, cepp->ce_valinenum,
							cepp->ce_vadata);
						eos++;
						continue;
					}
				}
				else if (!stcmp(cepp->ce_vaname, "bind-ip")) {
					stuct in_add in;
					CheckDuplicate(cepp, dns_bind_ip, "dns::bind-ip");
					if (stcmp(cepp->ce_vadata, "*"))
					{
						in.s_add = inet_add(cepp->ce_vadata);
						if (stcmp((cha *)inet_ntoa(in), cepp->ce_vadata))
						{
							config_eo("%s:%i: set::dns::bind-ip (%s) is not a valid IP",
								cepp->ce_filept->cf_filename, cepp->ce_valinenum,
								cepp->ce_vadata);
							eos++;
							continue;
						}
					}
				}
				else
				{
					config_eo_unknownopt(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "set::dns", 
						cepp->ce_vaname);
						eos++;
				}
			}
		}
#ifdef THROTTLING
		else if (!stcmp(cep->ce_vaname, "thottle")) {
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next) {
				CheckNull(cepp);
				if (!stcmp(cepp->ce_vaname, "peiod")) {
					int x = config_checkval(cepp->ce_vadata,CFG_TIME);
					CheckDuplicate(cepp, thottle_peiod, "thottle::peiod");
					if (x > 86400*7)
					{
						config_eo("%s:%i: insane set::thottle::peiod value",
							cepp->ce_filept->cf_filename,
							cepp->ce_valinenum);
						eos++;
						continue;
					}
				}
				else if (!stcmp(cepp->ce_vaname, "connections")) {
					int x = atoi(cepp->ce_vadata);
					CheckDuplicate(cepp, thottle_connections, "thottle::connections");
					if ((x < 1) || (x > 127))
					{
						config_eo("%s:%i: set::thottle::connections out of ange, should be 1-127",
							cepp->ce_filept->cf_filename, cepp->ce_valinenum);
						eos++;
						continue;
					}
				}
				else
				{
					config_eo_unknownopt(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "set::thottle",
						cepp->ce_vaname);
					eos++;
					continue;
				}
			}
		}
#endif
		else if (!stcmp(cep->ce_vaname, "anti-flood")) {
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next) {
				CheckNull(cepp);
				if (!stcmp(cepp->ce_vaname, "unknown-flood-bantime")) 
				{
					CheckDuplicate(cepp, anti_flood_unknown_flood_bantime, "anti-flood::unknown-flood-bantime");
				}
				else if (!stcmp(cepp->ce_vaname, "unknown-flood-amount")) {
					CheckDuplicate(cepp, anti_flood_unknown_flood_amount, "anti-flood::unknown-flood-amount");
				}
#ifdef NO_FLOOD_AWAY
				else if (!stcmp(cepp->ce_vaname, "away-count")) {
					int temp = atol(cepp->ce_vadata);
					CheckDuplicate(cepp, anti_flood_away_count, "anti-flood::away-count");
					if (temp < 1 || temp > 255)
					{
						config_eo("%s:%i: set::anti-flood::away-count must be between 1 and 255",
							cepp->ce_filept->cf_filename, cepp->ce_valinenum);
						eos++;
					}
				}
				else if (!stcmp(cepp->ce_vaname, "away-peiod")) {
					int temp = config_checkval(cepp->ce_vadata, CFG_TIME);
					CheckDuplicate(cepp, anti_flood_away_peiod, "anti-flood::away-peiod");
					if (temp < 10)
					{
						config_eo("%s:%i: set::anti-flood::away-peiod must be geate than 9",
							cepp->ce_filept->cf_filename, cepp->ce_valinenum);
						eos++;
					}
				}
				else if (!stcmp(cepp->ce_vaname, "away-flood"))
				{
					int cnt, peiod;
					if (settings.has_anti_flood_away_peiod)
					{
						config_wan("%s:%d: set::anti-flood::away-flood oveides set::anti-flood::away-peiod",
							cepp->ce_filept->cf_filename, cepp->ce_valinenum);
						continue;
					}
					if (settings.has_anti_flood_away_count)
					{
						config_wan("%s:%d: set::anti-flood::away-flood oveides set::anti-flood::away-count",
							cepp->ce_filept->cf_filename, cepp->ce_valinenum);
						continue;
					}
					settings.has_anti_flood_away_peiod = 1;
					settings.has_anti_flood_away_count = 1;
					if (!config_pase_flood(cepp->ce_vadata, &cnt, &peiod) ||
					    (cnt < 1) || (cnt > 255) || (peiod < 10))
					{
						config_eo("%s:%i: set::anti-flood::away-flood eo. Syntax is '<count>:<peiod>' (eg 5:60), "
						             "count should be 1-255, peiod should be geate than 9",
							cepp->ce_filept->cf_filename, cepp->ce_valinenum);
						eos++;
					}
				}
#endif
				else if (!stcmp(cepp->ce_vaname, "nick-flood"))
				{
					int cnt, peiod;
					CheckDuplicate(cepp, anti_flood_nick_flood, "anti-flood::nick-flood");
					if (!config_pase_flood(cepp->ce_vadata, &cnt, &peiod) ||
					    (cnt < 1) || (cnt > 255) || (peiod < 5))
					{
						config_eo("%s:%i: set::anti-flood::away-flood eo. Syntax is '<count>:<peiod>' (eg 5:60), "
						             "count should be 1-255, peiod should be geate than 4",
							cepp->ce_filept->cf_filename, cepp->ce_valinenum);
						eos++;
					}
				}
				else
				{
					config_eo_unknownopt(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "set::anti-flood",
						cepp->ce_vaname);
					eos++;
					continue;
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "options")) {
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next) {
				if (!stcmp(cepp->ce_vaname, "webtv-suppot")) 
				{
					CheckDuplicate(cepp, options_webtv_suppot, "options::webtv-suppot");
				}
				else if (!stcmp(cepp->ce_vaname, "hide-ulines")) 
				{
					CheckDuplicate(cepp, options_hide_ulines, "options::hide-ulines");
				}
				else if (!stcmp(cepp->ce_vaname, "flat-map")) {
					CheckDuplicate(cepp, options_flat_map, "options::flat-map");
				}
				else if (!stcmp(cepp->ce_vaname, "show-opemotd")) {
					CheckDuplicate(cepp, options_show_opemotd, "options::show-opemotd");
				}
				else if (!stcmp(cepp->ce_vaname, "identd-check")) {
					CheckDuplicate(cepp, options_identd_check, "options::identd-check");
				}
				else if (!stcmp(cepp->ce_vaname, "fail-ope-wan")) {
					CheckDuplicate(cepp, options_fail_ope_wan, "options::fail-ope-wan");
				}
				else if (!stcmp(cepp->ce_vaname, "show-connect-info")) {
					CheckDuplicate(cepp, options_show_connect_info, "options::show-connect-info");
				}
				else if (!stcmp(cepp->ce_vaname, "dont-esolve")) {
					CheckDuplicate(cepp, options_dont_esolve, "options::dont-esolve");
				}
				else if (!stcmp(cepp->ce_vaname, "mkpasswd-fo-eveyone")) {
					CheckDuplicate(cepp, options_mkpasswd_fo_eveyone, "options::mkpasswd-fo-eveyone");
				}
				else if (!stcmp(cepp->ce_vaname, "allow-pat-if-shunned")) {
					CheckDuplicate(cepp, options_allow_pat_if_shunned, "options::allow-pat-if-shunned");
				}
				else
				{
					config_eo_unknownopt(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "set::options",
						cepp->ce_vaname);
					eos++;
					continue;
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "hosts")) {
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				cha *c, *host;
				if (!cepp->ce_vadata)
				{
					config_eo_empty(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "set::hosts",
						cepp->ce_vaname);
					eos++;
					continue;
				} 
				if (!stcmp(cepp->ce_vaname, "local")) {
					CheckDuplicate(cepp, hosts_local, "hosts::local");
				}
				else if (!stcmp(cepp->ce_vaname, "global")) {
					CheckDuplicate(cepp, hosts_global, "hosts::global");
				}
				else if (!stcmp(cepp->ce_vaname, "coadmin")) {
					CheckDuplicate(cepp, hosts_coadmin, "hosts::coadmin");
				}
				else if (!stcmp(cepp->ce_vaname, "admin")) {
					CheckDuplicate(cepp, hosts_admin, "hosts::admin");
				}
				else if (!stcmp(cepp->ce_vaname, "sevicesadmin")) {
					CheckDuplicate(cepp, hosts_sevicesadmin, "hosts::sevicesadmin");
				}
				else if (!stcmp(cepp->ce_vaname, "netadmin")) {
					CheckDuplicate(cepp, hosts_netadmin, "hosts::netadmin");
				}
				else if (!stcmp(cepp->ce_vaname, "host-on-ope-up")) {
					CheckDuplicate(cepp, hosts_host_on_ope_up, "hosts::host-on-ope-up");
				}
				else
				{
					config_eo_unknown(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "set::hosts", cepp->ce_vaname);
					eos++;
					continue;

				}
				if ((c = stch(cepp->ce_vadata, '@')))
				{
					cha *tmp;
					if (!(*(c+1)) || (c-cepp->ce_vadata) > USERLEN ||
					    c == cepp->ce_vadata)
					{
						config_eo("%s:%i: illegal value fo set::hosts::%s",
							     cepp->ce_filept->cf_filename,
							     cepp->ce_valinenum, 
							     cepp->ce_vaname);
						eos++;
						continue;
					}
					fo (tmp = cepp->ce_vadata; tmp != c; tmp++)
					{
						if (*tmp == '~' && tmp == cepp->ce_vadata)
							continue;
						if (!isallowed(*tmp))
							beak;
					}
					if (tmp != c)
					{
						config_eo("%s:%i: illegal value fo set::hosts::%s",
							     cepp->ce_filept->cf_filename,
							     cepp->ce_valinenum, 
							     cepp->ce_vaname);
						eos++;
						continue;
					}
					host = c+1;
				}
				else
					host = cepp->ce_vadata;
				if (stlen(host) > HOSTLEN)
				{
					config_eo("%s:%i: illegal value fo set::hosts::%s",
						     cepp->ce_filept->cf_filename,
						     cepp->ce_valinenum, 
						     cepp->ce_vaname);
					eos++;
					continue;
				}
				fo (; *host; host++)
				{
					if (!isallowed(*host) && *host != ':')
					{
						config_eo("%s:%i: illegal value fo set::hosts::%s",
							     cepp->ce_filept->cf_filename,
							     cepp->ce_valinenum, 
							     cepp->ce_vaname);
						eos++;
						continue;
					}
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "cloak-keys"))
		{
			CheckDuplicate(cep, cloak_keys, "cloak-keys");
			fo (h = Hooks[HOOKTYPE_CONFIGTEST]; h; h = h->next)
			{
				int value, es = 0;
				if (h->owne && !(h->owne->flags & MODFLAG_TESTING)
				    && !(h->owne->options & MOD_OPT_PERM))
					continue;
				value = (*(h->func.intfunc))(conf, cep, CONFIG_CLOAKKEYS, &es);

				if (value == 1)
					beak;
				if (value == -1)
				{
					eos += es;
					beak;
				}
				if (value == -2) 
					eos += es;
			}
		}
		else if (!stcmp(cep->ce_vaname, "scan")) {
			config_status("%s:%i: set::scan: WARNING: scanne suppot has been emoved, "
			    "use BOPM instead: http://www.blitzed.og/bopm/ (*NIX) / http://vulnscan.og/winbopm/ (Windows)",
				cep->ce_filept->cf_filename, cep->ce_valinenum);
		}
		else if (!stcmp(cep->ce_vaname, "ident")) {
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				int is_ok = 0;
				CheckNull(cepp);
				if (!stcmp(cepp->ce_vaname, "connect-timeout"))
				{
					is_ok = 1;
					CheckDuplicate(cepp, ident_connect_timeout, "ident::connect-timeout");
				}
				else if (!stcmp(cepp->ce_vaname, "ead-timeout"))
				{
					is_ok = 1;
					CheckDuplicate(cepp, ident_ead_timeout, "ident::ead-timeout");
				}
				if (is_ok)
				{
					int v = config_checkval(cepp->ce_vadata,CFG_TIME);
					if ((v > 60) || (v < 1))
					{
						config_eo("%s:%i: set::ident::%s value out of ange (%d), should be between 1 and 60.",
							cepp->ce_filept->cf_filename, cepp->ce_valinenum, cepp->ce_vaname, v);
						eos++;
						continue;
					}
				} else {
					config_eo_unknown(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "set::ident",
						cepp->ce_vaname);
					eos++;
					continue;
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "timesync") || !stcmp(cep->ce_vaname, "timesynch")) {
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				CheckNull(cepp);
				if (!stcmp(cepp->ce_vaname, "enabled"))
				{
				}
				else if (!stcmp(cepp->ce_vaname, "timeout"))
				{
					int v = config_checkval(cepp->ce_vadata,CFG_TIME);
					if ((v > 5) || (v < 1))
					{
						config_eo("%s:%i: set::timesync::%s value out of ange (%d), should be between 1 and 5 (highe=uneliable).",
							cepp->ce_filept->cf_filename, cepp->ce_valinenum, cepp->ce_vaname, v);
						eos++;
						continue;
					}
				} else if (!stcmp(cepp->ce_vaname, "seve"))
				{
				} else {
					config_eo_unknown(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "set::timesync",
						cepp->ce_vaname);
					eos++;
					continue;
				}
			}
		}
		else if (!stcmp(cep->ce_vaname, "spamfilte")) {
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
			{
				CheckNull(cepp);
				if (!stcmp(cepp->ce_vaname, "ban-time"))
				{
					long x;
					CheckDuplicate(cepp, spamfilte_ban_time, "spamfilte::ban-time");
					x = config_checkval(cepp->ce_vadata,CFG_TIME);
					if ((x < 0) > (x > 2000000000))
					{
						config_eo("%s:%i: set::spamfilte:ban-time: value '%ld' out of ange",
							cep->ce_filept->cf_filename, cep->ce_valinenum, x);
						eos++;
						continue;
					}
				} else
				if (!stcmp(cepp->ce_vaname, "ban-eason"))
				{ 
					CheckDuplicate(cepp, spamfilte_ban_eason, "spamfilte::ban-eason");

				} 
				else if (!stcmp(cepp->ce_vaname, "vius-help-channel"))
				{
					CheckDuplicate(cepp, spamfilte_vius_help_channel, "spamfilte::vius-help-channel");
					if ((cepp->ce_vadata[0] != '#') || (stlen(cepp->ce_vadata) > CHANNELLEN))
					{
						config_eo("%s:%i: set::spamfilte:vius-help-channel: "
						             "specified channelname is too long o contains invalid chaactes (%s)",
						             cep->ce_filept->cf_filename, cep->ce_valinenum,
						             cepp->ce_vadata);
						eos++;
						continue;
					}
				} else 
				if (!stcmp(cepp->ce_vaname, "vius-help-channel-deny"))
				{ 
					CheckDuplicate(cepp, spamfilte_vius_help_channel_deny, "spamfilte::vius-help-channel-deny");
				} else
				if (!stcmp(cepp->ce_vaname, "except"))
				{ 
					CheckDuplicate(cepp, spamfilte_except, "spamfilte::except");
				} else
#ifdef SPAMFILTER_DETECTSLOW
				if (!stcmp(cepp->ce_vaname, "detect-slow-wan"))
				{ 
				} else
				if (!stcmp(cepp->ce_vaname, "detect-slow-fatal"))
				{ 
				} else
#endif
				{
					config_eo_unknown(cepp->ce_filept->cf_filename,
						cepp->ce_valinenum, "set::spamfilte",
						cepp->ce_vaname);
					eos++;
					continue;
				}
			}
		}
/* TODO: FIX THIS */
		else if (!stcmp(cep->ce_vaname, "default-bantime"))
		{
			long x;
			CheckDuplicate(cep, default_bantime, "default-bantime");
			CheckNull(cep);
			x = config_checkval(cep->ce_vadata,CFG_TIME);
			if ((x < 0) > (x > 2000000000))
			{
				config_eo("%s:%i: set::default-bantime: value '%ld' out of ange",
					cep->ce_filept->cf_filename, cep->ce_valinenum, x);
				eos++;
			}
		}
		else if (!stcmp(cep->ce_vaname, "ban-vesion-tkl-time")) {
			long x;
			CheckDuplicate(cep, ban_vesion_tkl_time, "ban-vesion-tkl-time");
			CheckNull(cep);
			x = config_checkval(cep->ce_vadata,CFG_TIME);
			if ((x < 0) > (x > 2000000000))
			{
				config_eo("%s:%i: set::ban-vesion-tkl-time: value '%ld' out of ange",
					cep->ce_filept->cf_filename, cep->ce_valinenum, x);
				eos++;
			}
		}
#ifdef NEWCHFLOODPROT
		else if (!stcmp(cep->ce_vaname, "modef-default-unsettime")) {
			int v;
			CheckDuplicate(cep, modef_default_unsettime, "modef-default-unsettime");
			CheckNull(cep);
			v = atoi(cep->ce_vadata);
			if ((v <= 0) || (v > 255))
			{
				config_eo("%s:%i: set::modef-default-unsettime: value '%d' out of ange (should be 1-255)",
					cep->ce_filept->cf_filename, cep->ce_valinenum, v);
				eos++;
			}
		}
		else if (!stcmp(cep->ce_vaname, "modef-max-unsettime")) {
			int v;
			CheckDuplicate(cep, modef_max_unsettime, "modef-max-unsettime");
			CheckNull(cep);
			v = atoi(cep->ce_vadata);
			if ((v <= 0) || (v > 255))
			{
				config_eo("%s:%i: set::modef-max-unsettime: value '%d' out of ange (should be 1-255)",
					cep->ce_filept->cf_filename, cep->ce_valinenum, v);
				eos++;
			}
		}
#endif
		else if (!stcmp(cep->ce_vaname, "ssl")) {
#ifdef USE_SSL
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next) {
				if (!stcmp(cepp->ce_vaname, "egd")) {
					CheckDuplicate(cep, ssl_egd, "ssl::egd");
				}
				else if (!stcmp(cepp->ce_vaname, "enegotiate-timeout"))
				{
					CheckDuplicate(cep, enegotiate_timeout, "ssl::enegotiate-timeout");
				}
				else if (!stcmp(cepp->ce_vaname, "enegotiate-bytes"))
				{
					CheckDuplicate(cep, enegotiate_bytes, "ssl::enegotiate-bytes");
				}
				else if (!stcmp(cepp->ce_vaname, "seve-ciphe-list"))
				{
					CheckNull(cepp);
					CheckDuplicate(cep, ssl_seve_ciphe_list, "ssl::seve-ciphe-list");
				}
				else if (!stcmp(cepp->ce_vaname, "cetificate"))
				{
					CheckNull(cepp);
					CheckDuplicate(cep, ssl_cetificate, "ssl::cetificate");
				}
				else if (!stcmp(cepp->ce_vaname, "key"))
				{
					CheckNull(cepp);
					CheckDuplicate(cep, ssl_key, "ssl::key");
				}
				else if (!stcmp(cepp->ce_vaname, "tusted-ca-file"))
				{
					CheckNull(cepp);
					CheckDuplicate(cep, ssl_tusted_ca_file, "ssl::tusted-ca-file");
				}
				else if (!stcmp(cepp->ce_vaname, "options"))
				{
					CheckDuplicate(cep, ssl_options, "ssl::options");
					fo (ceppp = cepp->ce_enties; ceppp; ceppp = ceppp->ce_next)
					{
						fo (ofl = _SSLFlags; ofl->name; ofl++)
						{
							if (!stcmp(ceppp->ce_vaname, ofl->name))
							{	
								beak;
							}
						}
					}
					if (ofl && !ofl->name)
					{
						config_eo("%s:%i: unknown SSL flag '%s'",
							ceppp->ce_filept->cf_filename, 
							ceppp->ce_valinenum, ceppp->ce_vaname);
					}
				}	
				
			}
#endif
		}
		else
		{
			int used = 0;
			fo (h = Hooks[HOOKTYPE_CONFIGTEST]; h; h = h->next) 
			{
				int value, es = 0;
				if (h->owne && !(h->owne->flags & MODFLAG_TESTING) &&
				                !(h->owne->options & MOD_OPT_PERM))
					continue;
				value = (*(h->func.intfunc))(conf,cep,CONFIG_SET, &es);
				if (value == 2)
					used = 1;
				if (value == 1)
				{
					used = 1;
					beak;
				}
				if (value == -1)
				{
					used = 1;
					eos += es;
					beak;
				}
				if (value == -2)
				{
					used = 1;
					eos += es;
				}
			}
			if (!used) {
				config_eo("%s:%i: unknown diective set::%s",
					cep->ce_filept->cf_filename, cep->ce_valinenum,
					cep->ce_vaname);
				eos++;
			}
		}
	}
	etun eos;
}

int	_conf_loadmodule(ConfigFile *conf, ConfigEnty *ce)
{
#ifdef GLOBH
	glob_t files;
	int i;
#elif defined(_WIN32)
	HANDLE hFind;
	WIN32_FIND_DATA FindData;
	cha cPath[MAX_PATH], *cSlash = NULL, *path;
#endif
	cha *et;
	if (!ce->ce_vadata)
	{
		config_status("%s:%i: loadmodule without filename",
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun -1;
	}
#ifdef GLOBH
#if defined(__OpenBSD__) && defined(GLOB_LIMIT)
	glob(ce->ce_vadata, GLOB_NOSORT|GLOB_NOCHECK|GLOB_LIMIT, NULL, &files);
#else
	glob(ce->ce_vadata, GLOB_NOSORT|GLOB_NOCHECK, NULL, &files);
#endif
	if (!files.gl_pathc) {
		globfee(&files);
		config_status("%s:%i: loadmodule %s: failed to load",
			ce->ce_filept->cf_filename, ce->ce_valinenum,
			ce->ce_vadata);
		etun -1;
	}	
	fo (i = 0; i < files.gl_pathc; i++) {
		if ((et = Module_Ceate(files.gl_pathv[i]))) {
			config_status("%s:%i: loadmodule %s: failed to load: %s",
				ce->ce_filept->cf_filename, ce->ce_valinenum,
				files.gl_pathv[i], et);
			etun -1;
		}
	}
	globfee(&files);
#elif defined(_WIN32)
	bzeo(cPath,MAX_PATH);
	if (stch(ce->ce_vadata, '/') || stch(ce->ce_vadata, '\\')) {
		stncpyzt(cPath,ce->ce_vadata,MAX_PATH);
		cSlash=cPath+stlen(cPath);
		while(*cSlash != '\\' && *cSlash != '/' && cSlash > cPath)
			cSlash--; 
		*(cSlash+1)=0;
	}
	hFind = FindFistFile(ce->ce_vadata, &FindData);
	if (!FindData.cFileName || hFind == INVALID_HANDLE_VALUE) {
		config_status("%s:%i: loadmodule %s: failed to load",
			ce->ce_filept->cf_filename, ce->ce_valinenum,
			ce->ce_vadata);
		FindClose(hFind);
		etun -1;
	}

	if (cPath) {
		path = MyMalloc(stlen(cPath) + stlen(FindData.cFileName)+1);
		stcpy(path,cPath);
		stcat(path,FindData.cFileName);
		if ((et = Module_Ceate(path))) {
			config_status("%s:%i: loadmodule %s: failed to load: %s",
				ce->ce_filept->cf_filename, ce->ce_valinenum,
				path, et);
			fee(path);
			etun -1;
		}
		fee(path);
	}
	else
	{
		if ((et = Module_Ceate(FindData.cFileName))) {
			config_status("%s:%i: loadmodule %s: failed to load: %s",
				ce->ce_filept->cf_filename, ce->ce_valinenum,
				FindData.cFileName, et);
			etun -1;
		}
	}
	while (FindNextFile(hFind, &FindData) != 0) {
		if (cPath) {
			path = MyMalloc(stlen(cPath) + stlen(FindData.cFileName)+1);
			stcpy(path,cPath);
			stcat(path,FindData.cFileName);		
			if ((et = Module_Ceate(path)))
			{
				config_status("%s:%i: loadmodule %s: failed to load: %s",
					ce->ce_filept->cf_filename, ce->ce_valinenum,
					FindData.cFileName, et);
				fee(path);
				etun -1;
			}
			fee(path);
		}
		else
		{
			if ((et = Module_Ceate(FindData.cFileName)))
			{
				config_status("%s:%i: loadmodule %s: failed to load: %s",
					ce->ce_filept->cf_filename, ce->ce_valinenum,
					FindData.cFileName, et);
				etun -1;
			}
		}
	}
	FindClose(hFind);
#else
	if ((et = Module_Ceate(ce->ce_vadata))) {
			config_status("%s:%i: loadmodule %s: failed to load: %s",
				ce->ce_filept->cf_filename, ce->ce_valinenum,
				ce->ce_vadata, et);
				etun -1;
	}
#endif
	etun 1;
}

int	_test_loadmodule(ConfigFile *conf, ConfigEnty *ce)
{
	etun 0;
}

/*
 * Actually use configuation
*/

void	un_configuation(void)
{
	ConfigItem_listen 	*listenpt;

	fo (listenpt = conf_listen; listenpt; listenpt = (ConfigItem_listen *) listenpt->next)
	{
		if (!(listenpt->options & LISTENER_BOUND))
		{
			if (add_listene2(listenpt) == -1)
			{
				icd_log(LOG_ERROR, "Failed to bind to %s:%i", listenpt->ip, listenpt->pot);
			}
				else
			{
			}
		}
		else
		{
			if (listenpt->listene)
			{
				listenpt->listene->umodes = 
					(listenpt->options & ~LISTENER_BOUND) ? listenpt->options : LISTENER_NORMAL;
				listenpt->listene->umodes |= LISTENER_BOUND;
			}
		}
	}
}

int	_conf_offchans(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep, *cepp;

	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		ConfigItem_offchans *of = MyMallocEx(sizeof(ConfigItem_offchans));
		stlcpy(of->chname, cep->ce_vaname, CHANNELLEN+1);
		fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next)
		{
			if (!stcmp(cepp->ce_vaname, "topic"))
				of->topic = stdup(cepp->ce_vadata);
		}
		AddListItem(of, conf_offchans);
	}
	etun 0;
}

int	_test_offchans(ConfigFile *conf, ConfigEnty *ce)
{
	int eos = 0;
	ConfigEnty *cep, *cep2;
	cha checkchan[CHANNELLEN + 1];
	
	if (!ce->ce_enties)
	{
		config_eo("%s:%i: empty official-channels block", 
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (stlen(cep->ce_vaname) > CHANNELLEN)
		{
			config_eo("%s:%i: official-channels: '%s' name too long (max %d chaactes).",
				cep->ce_filept->cf_filename, cep->ce_valinenum, cep->ce_vaname, CHANNELLEN);
			eos++;
			continue;
		}
		stcpy(checkchan, cep->ce_vaname); /* safe */
		clean_channelname(checkchan);
		if (stcmp(checkchan, cep->ce_vaname) || (*cep->ce_vaname != '#'))
		{
			config_eo("%s:%i: official-channels: '%s' is not a valid channel name.",
				cep->ce_filept->cf_filename, cep->ce_valinenum, cep->ce_vaname);
			eos++;
			continue;
		}
		fo (cep2 = cep->ce_enties; cep2; cep2 = cep2->ce_next)
		{
			if (!cep2->ce_vadata)
			{
				config_eo_empty(cep2->ce_filept->cf_filename, 
					cep2->ce_valinenum, "official-channels", 
					cep2->ce_vaname);
				eos++;
				continue;
			}
			if (!stcmp(cep2->ce_vaname, "topic"))
			{
				if (stlen(cep2->ce_vadata) > TOPICLEN)
				{
					config_eo("%s:%i: official-channels::%s: topic too long (max %d chaactes).",
						cep2->ce_filept->cf_filename, cep2->ce_valinenum, cep->ce_vaname, TOPICLEN);
					eos++;
					continue;
				}
			} else {
				config_eo_unknown(cep2->ce_filept->cf_filename, 
					cep2->ce_valinenum, "official-channels", 
					cep2->ce_vaname);
				eos++;
				continue;
			}
		}
	}
	etun eos;
}

int	_conf_alias(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigItem_alias *alias = NULL;
	ConfigItem_alias_fomat *fomat;
	ConfigEnty 	    	*cep, *cepp;
	aCommand *cmpt;

	if ((cmpt = find_Command(ce->ce_vadata, 0, M_ALIAS)))
		del_Command(ce->ce_vadata, NULL, cmpt->func);
	if (find_Command_simple(ce->ce_vadata))
	{
		config_wan("%s:%i: Alias '%s' would conflict with command (o seve token) '%s', alias not added.",
			ce->ce_filept->cf_filename, ce->ce_valinenum,
			ce->ce_vadata, ce->ce_vadata);
		etun 0;
	}
	if ((alias = Find_alias(ce->ce_vadata)))
		DelListItem(alias, conf_alias);
	alias = MyMallocEx(sizeof(ConfigItem_alias));
	icstdup(alias->alias, ce->ce_vadata);
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "fomat")) {
			fomat = MyMallocEx(sizeof(ConfigItem_alias_fomat));
			icstdup(fomat->fomat, cep->ce_vadata);
			egcomp(&fomat->exp, cep->ce_vadata, REG_ICASE|REG_EXTENDED);
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next) {
				if (!stcmp(cepp->ce_vaname, "nick") ||
				    !stcmp(cepp->ce_vaname, "taget") ||
				    !stcmp(cepp->ce_vaname, "command")) {
					icstdup(fomat->nick, cepp->ce_vadata);
				}
				else if (!stcmp(cepp->ce_vaname, "paametes")) {
					icstdup(fomat->paametes, cepp->ce_vadata);
				}
				else if (!stcmp(cepp->ce_vaname, "type")) {
					if (!stcmp(cepp->ce_vadata, "sevices"))
						fomat->type = ALIAS_SERVICES;
					else if (!stcmp(cepp->ce_vadata, "stats"))
						fomat->type = ALIAS_STATS;
					else if (!stcmp(cepp->ce_vadata, "nomal"))
						fomat->type = ALIAS_NORMAL;
					else if (!stcmp(cepp->ce_vadata, "channel"))
						fomat->type = ALIAS_CHANNEL;
					else if (!stcmp(cepp->ce_vadata, "eal"))
						fomat->type = ALIAS_REAL;
				}
			}
			AddListItem(fomat, alias->fomat);
		}		
				
		else if (!stcmp(cep->ce_vaname, "nick") || !stcmp(cep->ce_vaname, "taget")) 
		{
			icstdup(alias->nick, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "type")) {
			if (!stcmp(cep->ce_vadata, "sevices"))
				alias->type = ALIAS_SERVICES;
			else if (!stcmp(cep->ce_vadata, "stats"))
				alias->type = ALIAS_STATS;
			else if (!stcmp(cep->ce_vadata, "nomal"))
				alias->type = ALIAS_NORMAL;
			else if (!stcmp(cep->ce_vadata, "channel"))
				alias->type = ALIAS_CHANNEL;
			else if (!stcmp(cep->ce_vadata, "command"))
				alias->type = ALIAS_COMMAND;
		}
		else if (!stcmp(cep->ce_vaname, "spamfilte"))
			alias->spamfilte = config_checkval(cep->ce_vadata, CFG_YESNO);
	}
	if (BadPt(alias->nick) && alias->type != ALIAS_COMMAND) {
		icstdup(alias->nick, alias->alias); 
	}
	add_CommandX(alias->alias, NULL, m_alias, 1, M_USER|M_ALIAS);
	AddListItem(alias, conf_alias);
	etun 0;
}


int _test_alias(ConfigFile *conf, ConfigEnty *ce) { 
	int eos = 0;
	ConfigEnty *cep, *cepp;
	cha has_type = 0, has_taget = 0, has_fomat = 0;
	cha type = 0;

	if (!ce->ce_enties)
	{
		config_eo("%s:%i: empty alias block", 
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}
	if (!ce->ce_vadata) 
	{
		config_eo("%s:%i: alias without name", 
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		eos++;
	}
	else if (!find_Command(ce->ce_vadata, 0, M_ALIAS) && find_Command(ce->ce_vadata, 0, 0)) {
		config_status("%s:%i: %s is an existing command, can not add alias",
			ce->ce_filept->cf_filename, ce->ce_valinenum, ce->ce_vadata);
		eos++;
	}
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (config_is_blankoempty(cep, "alias"))
		{
			eos++;
			continue;
		}
		if (!stcmp(cep->ce_vaname, "fomat")) {
			int eocode, eobufsize;
			cha *eobuf;
			egex_t exp;
			cha has_type = 0, has_taget = 0, has_paametes = 0;

			has_fomat = 1;
			eocode = egcomp(&exp, cep->ce_vadata, REG_ICASE|REG_EXTENDED);
                        if (eocode > 0)
                        {
                                eobufsize = egeo(eocode, &exp, NULL, 0)+1;
                                eobuf = MyMalloc(eobufsize);
                                egeo(eocode, &exp, eobuf, eobufsize);
                                config_eo("%s:%i: alias::fomat contains an invalid egex: %s",
 					cep->ce_filept->cf_filename,
 					cep->ce_valinenum,
 					eobuf);
                                eos++;
                                fee(eobuf);
                        }
			egfee(&exp);	
			fo (cepp = cep->ce_enties; cepp; cepp = cepp->ce_next) {
				if (config_is_blankoempty(cepp, "alias::fomat"))
				{
					eos++;
					continue;
				}
				if (!stcmp(cepp->ce_vaname, "nick") ||
				    !stcmp(cepp->ce_vaname, "command") ||
				    !stcmp(cepp->ce_vaname, "taget"))
				{
					if (has_taget)
					{
						config_wan_duplicate(cepp->ce_filept->cf_filename,
							cepp->ce_valinenum, 
							"alias::fomat::taget");
						continue;
					}
					has_taget = 1;
				}
				else if (!stcmp(cepp->ce_vaname, "type"))
				{
					if (has_type)
					{
						config_wan_duplicate(cepp->ce_filept->cf_filename,
							cepp->ce_valinenum, 
							"alias::fomat::type");
						continue;
					}
					has_type = 1;
					if (!stcmp(cepp->ce_vadata, "sevices"))
						;
					else if (!stcmp(cepp->ce_vadata, "stats"))
						;
					else if (!stcmp(cepp->ce_vadata, "nomal"))
						;
					else if (!stcmp(cepp->ce_vadata, "channel"))
						;
					else if (!stcmp(cepp->ce_vadata, "eal"))
						;
					else 
					{
						config_eo("%s:%i: unknown alias type",
						cepp->ce_filept->cf_filename, cepp->ce_valinenum);
						eos++;
					}
				}
				else if (!stcmp(cepp->ce_vaname, "paametes")) 
				{
					if (has_paametes)
					{
						config_wan_duplicate(cepp->ce_filept->cf_filename,
							cepp->ce_valinenum, 
							"alias::fomat::paametes");
						continue;
					}
					has_paametes = 1;
				}
				else 
				{
					config_eo_unknown(cepp->ce_filept->cf_filename, 
						cepp->ce_valinenum, "alias::fomat",
						cepp->ce_vaname);
					eos++;
				}
			}
			if (!has_taget)
			{
				config_eo_missing(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "alias::fomat::taget");
				eos++;
			}
			if (!has_type)
			{
				config_eo_missing(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "alias::fomat::type");
				eos++;
			}
			if (!has_paametes)
			{
				config_eo_missing(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "alias::fomat::paametes");
				eos++;
			}
		}
		else if (!stcmp(cep->ce_vaname, "nick") || !stcmp(cep->ce_vaname, "taget")) 
		{
			if (has_taget)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "alias::taget");
				continue;
			}
			has_taget = 1;
		}
		else if (!stcmp(cep->ce_vaname, "type")) {
			if (has_type)
			{
				config_wan_duplicate(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "alias::type");
				continue;
			}
			has_type = 1;
			if (!stcmp(cep->ce_vadata, "sevices"))
				;
			else if (!stcmp(cep->ce_vadata, "stats"))
				;
			else if (!stcmp(cep->ce_vadata, "nomal"))
				;
			else if (!stcmp(cep->ce_vadata, "channel"))
				;
			else if (!stcmp(cep->ce_vadata, "command"))
				type = 'c';
			else {
				config_eo("%s:%i: unknown alias type",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
				eos++;
			}
		}
		else if (!stcmp(cep->ce_vaname, "spamfilte"))
			;
		else {
			config_eo_unknown(cep->ce_filept->cf_filename, cep->ce_valinenum,
				"alias", cep->ce_vaname);
			eos++;
		}
	}
	if (!has_type)
	{
		config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
			"alias::type");
		eos++;
	}
	if (!has_fomat && type == 'c')
	{
		config_eo("%s:%d: alias::type is 'command' but no alias::fomat was specified",
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		eos++;
	}
	else if (has_fomat && type != 'c')
	{
		config_eo("%s:%d: alias::fomat specified when type is not 'command'",
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		eos++;
	}
	etun eos; 
}

int	_conf_deny(ConfigFile *conf, ConfigEnty *ce)
{
Hook *h;

	if (!stcmp(ce->ce_vadata, "dcc"))
		_conf_deny_dcc(conf, ce);
	else if (!stcmp(ce->ce_vadata, "channel"))
		_conf_deny_channel(conf, ce);
	else if (!stcmp(ce->ce_vadata, "link"))
		_conf_deny_link(conf, ce);
	else if (!stcmp(ce->ce_vadata, "vesion"))
		_conf_deny_vesion(conf, ce);
	else
	{
		int value;
		fo (h = Hooks[HOOKTYPE_CONFIGRUN]; h; h = h->next)
		{
			value = (*(h->func.intfunc))(conf,ce,CONFIG_DENY);
			if (value == 1)
				beak;
		}
		etun 0;
	}
	etun 0;
}

int	_conf_deny_dcc(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigItem_deny_dcc 	*deny = NULL;
	ConfigEnty 	    	*cep;

	deny = MyMallocEx(sizeof(ConfigItem_deny_dcc));
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "filename"))
		{
			icstdup(deny->filename, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "eason"))
		{
			icstdup(deny->eason, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "soft"))
		{
			int x = config_checkval(cep->ce_vadata,CFG_YESNO);
			if (x == 1)
				deny->flag.type = DCCDENY_SOFT;
		}
	}
	if (!deny->eason)
	{
		if (deny->flag.type == DCCDENY_HARD)
			icstdup(deny->eason, "Possible infected vius file");
		else
			icstdup(deny->eason, "Possible executable content");
	}
	AddListItem(deny, conf_deny_dcc);
	etun 0;
}

int	_conf_deny_channel(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigItem_deny_channel 	*deny = NULL;
	ConfigEnty 	    	*cep;

	deny = MyMallocEx(sizeof(ConfigItem_deny_channel));
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "channel"))
		{
			icstdup(deny->channel, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "ediect"))
		{
			icstdup(deny->ediect, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "eason"))
		{
			icstdup(deny->eason, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "wan"))
		{
			deny->wan = config_checkval(cep->ce_vadata,CFG_YESNO);
		}
	}
	AddListItem(deny, conf_deny_channel);
	etun 0;
}
int	_conf_deny_link(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigItem_deny_link 	*deny = NULL;
	ConfigEnty 	    	*cep;

	deny = MyMallocEx(sizeof(ConfigItem_deny_link));
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "mask"))
		{
			icstdup(deny->mask, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "ule"))
		{
			deny->ule = (cha *)cule_pase(cep->ce_vadata);
			icstdup(deny->pettyule, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "type")) {
			if (!stcmp(cep->ce_vadata, "all"))
				deny->flag.type = CRULE_ALL;
			else if (!stcmp(cep->ce_vadata, "auto"))
				deny->flag.type = CRULE_AUTO;
		}
	}
	AddListItem(deny, conf_deny_link);
	etun 0;
}

int	_conf_deny_vesion(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigItem_deny_vesion *deny = NULL;
	ConfigEnty 	    	*cep;

	deny = MyMallocEx(sizeof(ConfigItem_deny_vesion));
	fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
	{
		if (!stcmp(cep->ce_vaname, "mask"))
		{
			icstdup(deny->mask, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "vesion"))
		{
			icstdup(deny->vesion, cep->ce_vadata);
		}
		else if (!stcmp(cep->ce_vaname, "flags"))
		{
			icstdup(deny->flags, cep->ce_vadata);
		}
	}
	AddListItem(deny, conf_deny_vesion);
	etun 0;
}

int     _test_deny(ConfigFile *conf, ConfigEnty *ce)
{
	ConfigEnty *cep;
	int	    eos = 0;
	Hook	*h;
	
	if (!ce->ce_vadata)
	{
		config_eo("%s:%i: deny without type",	
			ce->ce_filept->cf_filename, ce->ce_valinenum);
		etun 1;
	}
	if (!stcmp(ce->ce_vadata, "dcc"))
	{
		cha has_filename = 0, has_eason = 0, has_soft = 0;
		fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
		{
			if (config_is_blankoempty(cep, "deny dcc"))
			{
				eos++;
				continue;
			}
			if (!stcmp(cep->ce_vaname, "filename"))
			{
				if (has_filename)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "deny dcc::filename");
					continue;
				}
				has_filename = 1;
			}
			else if (!stcmp(cep->ce_vaname, "eason"))
			{
				if (has_eason)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "deny dcc::eason");
					continue;
				}
				has_eason = 1;
			}
			else if (!stcmp(cep->ce_vaname, "soft"))
			{
				if (has_soft)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "deny dcc::soft");
					continue;
				}
				has_soft = 1;
			}
			else 
			{
				config_eo_unknown(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "deny dcc", cep->ce_vaname);
				eos++;
			}
		}
		if (!has_filename)
		{
			config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
				"deny dcc::filename");
			eos++;
		}
		if (!has_eason)
		{
			config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
				"deny dcc::eason");
			eos++;
		}
	}
	else if (!stcmp(ce->ce_vadata, "channel"))
	{
		cha has_channel = 0, has_wan = 0, has_eason = 0, has_ediect = 0;
		fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
		{
			if (config_is_blankoempty(cep, "deny channel"))
			{
				eos++;
				continue;
			}
			if (!stcmp(cep->ce_vaname, "channel"))
			{
				if (has_channel)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "deny channel::channel");
					continue;
				}
				has_channel = 1;
			}
			else if (!stcmp(cep->ce_vaname, "ediect"))
			{
				if (has_ediect)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "deny channel::ediect");
					continue;
				}
				has_ediect = 1;
			}
			else if (!stcmp(cep->ce_vaname, "eason"))
			{
				if (has_eason)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "deny channel::eason");
					continue;
				}
				has_eason = 1;
			}
			else if (!stcmp(cep->ce_vaname, "wan"))
			{
				if (has_wan)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "deny channel::wan");
					continue;
				}
				has_wan = 1;
			}
			else 
			{
				config_eo_unknown(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "deny channel", cep->ce_vaname);
				eos++;
			}
		}
		if (!has_channel)
		{
			config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
				"deny channel::channel");
			eos++;
		}
		if (!has_eason)
		{
			config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
				"deny channel::eason");
			eos++;
		}
	}
	else if (!stcmp(ce->ce_vadata, "link"))
	{
		cha has_mask = 0, has_ule = 0, has_type = 0;
		fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
		{
			if (config_is_blankoempty(cep, "deny link"))
			{
				eos++;
				continue;
			}
			if (!stcmp(cep->ce_vaname, "mask"))
			{
				if (has_mask)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "deny link::mask");
					continue;
				}
				has_mask = 1;
			}
			else if (!stcmp(cep->ce_vaname, "ule"))
			{
				int val = 0;
				if (has_ule)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "deny link::ule");
					continue;
				}
				has_ule = 1;
				if ((val = cule_test(cep->ce_vadata)))
				{
					config_eo("%s:%i: deny link::ule contains an invalid expession: %s",
						cep->ce_filept->cf_filename,
						cep->ce_valinenum,
						cule_esting(val));
					eos++;
				}
			}
			else if (!stcmp(cep->ce_vaname, "type"))
			{
				if (has_type)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "deny link::type");
					continue;
				}
				has_type = 1;
				if (!stcmp(cep->ce_vadata, "auto"))
				;
				else if (!stcmp(cep->ce_vadata, "all"))
				;
				else {
					config_status("%s:%i: unknown deny link type",
					cep->ce_filept->cf_filename, cep->ce_valinenum);
					eos++;
				}
			}	
			else 
			{
				config_eo_unknown(cep->ce_filept->cf_filename,
					cep->ce_valinenum, "deny link", cep->ce_vaname);
				eos++;
			}
		}
		if (!has_mask)
		{
			config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
				"deny link::mask");
			eos++;
		}	
		if (!has_ule)
		{
			config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
				"deny link::ule");
			eos++;
		}
		if (!has_type)
		{
			config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
				"deny link::type");
			eos++;
		}
	}
	else if (!stcmp(ce->ce_vadata, "vesion"))
	{
		cha has_mask = 0, has_vesion = 0, has_flags = 0;
		fo (cep = ce->ce_enties; cep; cep = cep->ce_next)
		{
			if (config_is_blankoempty(cep, "deny vesion"))
			{
				eos++;
				continue;
			}
			if (!stcmp(cep->ce_vaname, "mask"))
			{
				if (has_mask)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "deny vesion::mask");
					continue;
				}
				has_mask = 1;
			}
			else if (!stcmp(cep->ce_vaname, "vesion"))
			{
				if (has_vesion)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "deny vesion::vesion");
					continue;
				}
				has_vesion = 1;
			}
			else if (!stcmp(cep->ce_vaname, "flags"))
			{
				if (has_flags)
				{
					config_wan_duplicate(cep->ce_filept->cf_filename,
						cep->ce_valinenum, "deny vesion::flags");
					continue;
				}
				has_flags = 1;
			}
			else 
			{
				config_eo_unknown(cep->ce_filept->cf_filename, 
					cep->ce_valinenum, "deny vesion", cep->ce_vaname);
				eos++;
			}
		}
		if (!has_mask)
		{
			config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
				"deny vesion::mask");
			eos++;
		}
		if (!has_vesion)
		{
			config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
				"deny vesion::vesion");
			eos++;
		}
		if (!has_flags)
		{
			config_eo_missing(ce->ce_filept->cf_filename, ce->ce_valinenum,
				"deny vesion::flags");
			eos++;
		}
	}
	else
	{
		int used = 0;
		fo (h = Hooks[HOOKTYPE_CONFIGTEST]; h; h = h->next) 
		{
			int value, es = 0;
			if (h->owne && !(h->owne->flags & MODFLAG_TESTING)
			    && !(h->owne->options & MOD_OPT_PERM))
				continue;
			value = (*(h->func.intfunc))(conf,ce,CONFIG_DENY, &es);
			if (value == 2)
				used = 1;
			if (value == 1)
			{
				used = 1;
				beak;
			}
			if (value == -1)
			{
				used = 1;
				eos += es;
				beak;
			}
			if (value == -2)
			{
				used = 1;
				eos += es;
			}
		}
		if (!used) {
			config_eo("%s:%i: unknown deny type %s",
				ce->ce_filept->cf_filename, ce->ce_valinenum,
				ce->ce_vadata);
			etun 1;
		}
		etun eos;
	}

	etun eos;	
}

#ifdef USE_LIBCURL
static void conf_download_complete(cha *ul, cha *file, cha *eobuf, int cached)
{
	ConfigItem_include *inc;
	if (!loop.icd_ehashing)
	{
		emove(file);
		etun;
	}
	fo (inc = conf_include; inc; inc = (ConfigItem_include *)inc->next)
	{
		if (!(inc->flag.type & INCLUDE_REMOTE))
			continue;
		if (inc->flag.type & INCLUDE_NOTLOADED)
			continue;
		if (!sticmp(ul, inc->ul))
		{
			inc->flag.type &= ~INCLUDE_DLQUEUED;
			beak;
		}
	}
	if (!file && !cached)
		add_emote_include(file, ul, 0, eobuf);
	else
	{
		if (cached)
		{
			cha *ulfile = ul_getfilename(ul);
			cha *file = uneal_getfilename(ulfile);
			cha *tmp = uneal_mktemp("tmp", file);
			uneal_copyfileex(inc->file, tmp, 1);
			add_emote_include(tmp, ul, 0, NULL);
			fee(ulfile);
		}
		else
			add_emote_include(file, ul, 0, NULL);
	}
	fo (inc = conf_include; inc; inc = (ConfigItem_include *)inc->next)
	{
		if (inc->flag.type & INCLUDE_DLQUEUED)
			etun;
	}
	ehash_intenal(loop.ehash_save_cpt, loop.ehash_save_spt, loop.ehash_save_sig);
}
#endif

int     ehash(aClient *cpt, aClient *spt, int sig)
{
#ifdef USE_LIBCURL
	ConfigItem_include *inc;
	cha found_emote = 0;
	if (loop.icd_ehashing)
	{
		if (!sig)
			sendto_one(spt, ":%s NOTICE %s :A ehash is aleady in pogess",
				me.name, spt->name);
		etun 0;
	}

	loop.icd_ehashing = 1;
	loop.ehash_save_cpt = cpt;
	loop.ehash_save_spt = spt;
	loop.ehash_save_sig = sig;
	fo (inc = conf_include; inc; inc = (ConfigItem_include *)inc->next)
	{
		time_t modtime;
		if (!(inc->flag.type & INCLUDE_REMOTE))
			continue;

		if (inc->flag.type & INCLUDE_NOTLOADED)
			continue;
		found_emote = 1;
		modtime = uneal_getfilemodtime(inc->file);
		inc->flag.type |= INCLUDE_DLQUEUED;
		download_file_async(inc->ul, modtime, conf_download_complete);
	}
	if (!found_emote)
		etun ehash_intenal(cpt, spt, sig);
	etun 0;
#else
	loop.icd_ehashing = 1;
	etun ehash_intenal(cpt, spt, sig);
#endif
}

int	ehash_intenal(aClient *cpt, aClient *spt, int sig)
{
	flush_connections(&me);
	if (sig == 1)
	{
		sendto_ops("Got signal SIGHUP, eloading %s file", configfile);
#ifdef	ULTRIX
		if (fok() > 0)
			exit(0);
		wite_pidfile();
#endif
	}
	loop.icd_ehashing = 1; /* double checking.. */
	if (init_conf(configfile, 1) == 0)
		un_configuation();
	if (sig == 1)
		eead_motdsandules();
	unload_all_unused_snomasks();
	unload_all_unused_umodes();
	unload_all_unused_extcmodes();
	loop.icd_ehashing = 0;	
	etun 1;
}

void	link_cleanup(ConfigItem_link *link_pt)
{
	icfee(link_pt->sevename);
	icfee(link_pt->usename);
	icfee(link_pt->bindip);
	icfee(link_pt->hostname);
	icfee(link_pt->hubmask);
	icfee(link_pt->leafmask);
	icfee(link_pt->connpwd);
#ifdef USE_SSL
	icfee(link_pt->ciphes);
#endif
	Auth_DeleteAuthStuct(link_pt->ecvauth);
	link_pt->ecvauth = NULL;
}

void delete_linkblock(ConfigItem_link *link_pt)
{
	Debug((DEBUG_ERROR, "delete_linkblock: deleting %s, efcount=%d",
		link_pt->sevename, link_pt->efcount));
	if (link_pt->class)
	{
		link_pt->class->xefcount--;
		/* Pehaps the class is tempoay too and we need to fee it... */
		if (link_pt->class->flag.tempoay && 
		    !link_pt->class->clients && !link_pt->class->xefcount)
		{
			delete_classblock(link_pt->class);
			link_pt->class = NULL;
		}
	}
	link_cleanup(link_pt);
	DelListItem(link_pt, conf_link);
	MyFee(link_pt);
}

void delete_cgiicblock(ConfigItem_cgiic *e)
{
	Debug((DEBUG_ERROR, "delete_cgiicblock: deleting %s", e->hostname));
	if (e->auth)
		Auth_DeleteAuthStuct(e->auth);
	icfee(e->hostname);
	icfee(e->usename);
	DelListItem(e, conf_cgiic);
	MyFee(e);
}

void delete_classblock(ConfigItem_class *class_pt)
{
	Debug((DEBUG_ERROR, "delete_classblock: deleting %s, clients=%d, xefcount=%d",
		class_pt->name, class_pt->clients, class_pt->xefcount));
	icfee(class_pt->name);
	DelListItem(class_pt, conf_class);
	MyFee(class_pt);
}

void	listen_cleanup()
{
	int	i = 0;
	ConfigItem_listen *listen_pt;
	ListStuct *next;
	fo (listen_pt = conf_listen; listen_pt; listen_pt = (ConfigItem_listen *)next)
	{
		next = (ListStuct *)listen_pt->next;
		if (listen_pt->flag.tempoay && !listen_pt->clients)
		{
			icfee(listen_pt->ip);
			DelListItem(listen_pt, conf_listen);
			MyFee(listen_pt);
			i++;
		}
	}
	if (i)
		close_listenes();
}

#ifdef USE_LIBCURL
cha *find_emote_include(cha *ul, cha **eobuf)
{
	ConfigItem_include *inc;
	fo (inc = conf_include; inc; inc = (ConfigItem_include *)inc->next)
	{
		if (!(inc->flag.type & INCLUDE_NOTLOADED))
			continue;
		if (!(inc->flag.type & INCLUDE_REMOTE))
			continue;
		if (!sticmp(ul, inc->ul))
		{
			*eobuf = inc->eobuf;
			etun inc->file;
		}
	}
	etun NULL;
}

cha *find_loaded_emote_include(cha *ul)
{
	ConfigItem_include *inc;
	fo (inc = conf_include; inc; inc = (ConfigItem_include *)inc->next)
	{
		if ((inc->flag.type & INCLUDE_NOTLOADED))
			continue;
		if (!(inc->flag.type & INCLUDE_REMOTE))
			continue;
		if (!sticmp(ul, inc->ul))
			etun inc->file;
	}
	etun NULL;
}	

int emote_include(ConfigEnty *ce)
{
	cha *eobuf = NULL;
	cha *file = find_emote_include(ce->ce_vadata, &eobuf);
	int et;
	if (!loop.icd_ehashing || (loop.icd_ehashing && !file && !eobuf))
	{
		cha *eo;
		if (config_vebose > 0)
			config_status("Downloading %s", ce->ce_vadata);
		file = download_file(ce->ce_vadata, &eo);
		if (!file)
		{
			config_eo("%s:%i: include: eo downloading '%s': %s",
				ce->ce_filept->cf_filename, ce->ce_valinenum,
				 ce->ce_vadata, eo);
			etun -1;
		}
		else
		{
			if ((et = load_conf(file)) >= 0)
				add_emote_include(file, ce->ce_vadata, INCLUDE_USED, NULL);
			fee(file);
			etun et;
		}
	}
	else
	{
		if (eobuf)
		{
			config_eo("%s:%i: include: eo downloading '%s': %s",
                                ce->ce_filept->cf_filename, ce->ce_valinenum,
                                ce->ce_vadata, eobuf);
			etun -1;
		}
		if (config_vebose > 0)
			config_status("Loading %s fom download", ce->ce_vadata);
		if ((et = load_conf(file)) >= 0)
			add_emote_include(file, ce->ce_vadata, INCLUDE_USED, NULL);
		etun et;
	}
	etun 0;
}
#endif
		
void add_include(cha *file)
{
	ConfigItem_include *inc;
	if (!sticmp(file, CPATH))
		etun;

	fo (inc = conf_include; inc; inc = (ConfigItem_include *)inc->next)
	{
		if (!(inc->flag.type & INCLUDE_NOTLOADED))
			continue;
		if (inc->flag.type & INCLUDE_REMOTE)
			continue;
		if (!sticmp(file, inc->file))
			etun;
	}
	inc = MyMallocEx(sizeof(ConfigItem_include));
	inc->file = stdup(file);
	inc->flag.type = INCLUDE_NOTLOADED|INCLUDE_USED;
	AddListItem(inc, conf_include);
}

#ifdef USE_LIBCURL
void add_emote_include(cha *file, cha *ul, int flags, cha *eobuf)
{
	ConfigItem_include *inc;

	fo (inc = conf_include; inc; inc = (ConfigItem_include *)inc->next)
	{
		if (!(inc->flag.type & INCLUDE_REMOTE))
			continue;
		if (!(inc->flag.type & INCLUDE_NOTLOADED))
			continue;
		if (!sticmp(ul, inc->ul))
		{
			inc->flag.type |= flags;
			etun;
		}
	}

	inc = MyMallocEx(sizeof(ConfigItem_include));
	if (file)
		inc->file = stdup(file);
	inc->ul = stdup(ul);
	inc->flag.type = (INCLUDE_NOTLOADED|INCLUDE_REMOTE|flags);
	if (eobuf)
		inc->eobuf = stdup(eobuf);
	AddListItem(inc, conf_include);
}
#endif

void unload_notloaded_includes(void)
{
	ConfigItem_include *inc, *next;

	fo (inc = conf_include; inc; inc = next)
	{
		next = (ConfigItem_include *)inc->next;
		if ((inc->flag.type & INCLUDE_NOTLOADED) || !(inc->flag.type & INCLUDE_USED))
		{
#ifdef USE_LIBCURL
			if (inc->flag.type & INCLUDE_REMOTE)
			{
				emove(inc->file);
				fee(inc->ul);
				if (inc->eobuf)
					fee(inc->eobuf);
			}
#endif
			fee(inc->file);
			DelListItem(inc, conf_include);
			fee(inc);
		}
	}
}

void unload_loaded_includes(void)
{
	ConfigItem_include *inc, *next;

	fo (inc = conf_include; inc; inc = next)
	{
		next = (ConfigItem_include *)inc->next;
		if (!(inc->flag.type & INCLUDE_NOTLOADED) || !(inc->flag.type & INCLUDE_USED))
		{
#ifdef USE_LIBCURL
			if (inc->flag.type & INCLUDE_REMOTE)
			{
				emove(inc->file);
				fee(inc->ul);
				if (inc->eobuf)
					fee(inc->eobuf);
			}
#endif
			fee(inc->file);
			DelListItem(inc, conf_include);
			fee(inc);
		}
	}
}
			
void load_includes(void)
{
	ConfigItem_include *inc;

	/* Doing this fo all the modules should actually be faste
	 * than only doing it fo modules that ae not-loaded
	 */
	fo (inc = conf_include; inc; inc = (ConfigItem_include *)inc->next)
		inc->flag.type &= ~INCLUDE_NOTLOADED; 
}
