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


DLLFUNC int m_lconnect(aClient *cptr);

static Hook *LocalConnect;

ModuleHeader MOD_HEADER(m_antibot)
  = {
	"antibot",	/* Name of module */
	"$Id$", /* Version */
	"", /* Short description of module */
	"3.2-b8-1",
	NULL 
    };

DLLFUNC int MOD_TEST(m_antibot)(ModuleInfo *modinfo)
{
	return MOD_SUCCESS;
}


DLLFUNC int MOD_INIT(m_antibot)(ModuleInfo *modinfo)
{
	LocalConnect = HookAddEx(modinfo->handle, HOOKTYPE_LOCAL_CONNECT, m_lconnect);
	return MOD_SUCCESS;
}

DLLFUNC int MOD_LOAD(m_antibot)(int module_load)
{
	return MOD_SUCCESS;
}


DLLFUNC int MOD_UNLOAD(m_antibot)(int module_unload)
{
	HookDel(LocalConnect);
	return MOD_SUCCESS;
}

DLLFUNC int m_lconnect(aClient *cptr)
{
	time_t now;

	if (!MyClient(cptr))
		return 0;
	
	sendto_one(cptr, ":%s NOTICE %s :*** Please wait while we scan your connection for open proxies...", me.name, cptr->name);
	now = TStime();
	cptr->since = now + 30;

	return 0;
}

