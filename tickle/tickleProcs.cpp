/*******************************************************************************
 * shroudBNC - an object-oriented framework for IRC                            *
 * Copyright (C) 2005 Gunnar Beutner                                           *
 *                                                                             *
 * This program is free software; you can redistribute it and/or               *
 * modify it under the terms of the GNU General Public License                 *
 * as published by the Free Software Foundation; either version 2              *
 * of the License, or (at your option) any later version.                      *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with this program; if not, write to the Free Software                 *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 *******************************************************************************/

#include "StdAfx.h"
#include "tickle.h"
#include "tickleProcs.h"
#include "../ModuleFar.h"
#include "../BouncerCore.h"
#include "../SocketEvents.h"
#include "../Connection.h"
#include "../IRCConnection.h"
#include "../ClientConnection.h"
#include "../Channel.h"
#include "../BouncerUser.h"
#include "../BouncerConfig.h"
#include "../Nick.h"
#include "../Queue.h"
#include "../FloodControl.h"
#include "../ModuleFar.h"
#include "../Module.h"

static char* g_Context = NULL;

binding_t* g_Binds = NULL;
int g_BindCount = 0;

extern "C" int Bnc_Init(Tcl_Interp*);

int Tcl_ProcInit(Tcl_Interp* interp) {
	return Bnc_Init(interp);
}

const char* getuser(const char* Option) {
	CBouncerUser* User = g_Bouncer->GetUser(g_Context);

	if (!User)
		return NULL;
	else
		return User->GetConfig()->ReadString(Option);
}

int setuser(const char* Option, const char* Value) {
	CBouncerUser* User = g_Bouncer->GetUser(g_Context);

	if (!User)
		return 0;
	else {		
		User->GetConfig()->WriteString(Option, Value);

		return 1;
	}
}

void die(void) {
	g_Bouncer->Shutdown();
}

void setctx(const char* ctx) {
	free(g_Context);
	g_Context = ctx ? strdup(ctx) : NULL;
}

const char* getctx(void) {
	return g_Context;
}

const char* bncuserlist(void) {
	static char* UserList = NULL;

	UserList = (char*)realloc(UserList, 1);
	UserList[0] = '\0';

	int Count = g_Bouncer->GetUserCount();
	CBouncerUser** Users = g_Bouncer->GetUsers();

	for (int i = 0; i < Count; i++) {
		if (Users[i]) {
			const char* User = Users[i]->GetUsername();
			UserList = (char*)realloc(UserList, strlen(UserList) + strlen(User) + 4);
			if (*UserList)
				strcat(UserList, " ");

			strcat(UserList, "{");
			strcat(UserList, User);
			strcat(UserList, "}");
		}
	}

	return UserList;
}

const char* internalchannels(void) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return NULL;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return NULL;

	static char* ChanList = NULL;

	ChanList = (char*)realloc(ChanList, 1);
	ChanList[0] = '\0';

	int Count = IRC->GetChannelCount();
	CChannel** Channels = IRC->GetChannels();

	for (int i = 0; i < Count; i++) {
		if (Channels[i]) {
			const char* Channel = Channels[i]->GetName();
			ChanList = (char*)realloc(ChanList, strlen(ChanList) + strlen(Channel) + 4);
			if (*ChanList)
				strcat(ChanList, " ");

			strcat(ChanList, "{");
			strcat(ChanList, Channel);
			strcat(ChanList, "}");
		}
	}

	return ChanList;
}

const char* gettopic(const char* Channel) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return NULL;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return NULL;


	CChannel* Chan = IRC->GetChannel(Channel);

	if (!Chan)
		return NULL;
	else
		return Chan->GetTopic();
}

const char* getchanmodes(const char* Channel) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return NULL;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return NULL;


	CChannel* Chan = IRC->GetChannel(Channel);

	if (!Chan)
		return NULL;
	else
		return Chan->GetChanModes();
}

void rehash(void) {
	RehashInterpreter();

	g_Bouncer->GlobalNotice("Rehashing TCL module", true);
}


int internalbind(const char* type, const char* proc) {
	for (int i = 0; i < g_BindCount; i++) {
		if (g_Binds[i].valid && strcmp(g_Binds[i].proc, proc) == 0)
			return 0;
	}

	g_Binds = (binding_s*)realloc(g_Binds, sizeof(binding_t) * ++g_BindCount);

	int n = g_BindCount - 1;

	g_Binds[n].valid = false;

	if (strcmpi(type, "client") == 0)
		g_Binds[n].type = Type_Client;
	else if (strcmpi(type, "server") == 0)
		g_Binds[n].type = Type_Server;
	else if (strcmpi(type, "pulse") == 0)
		g_Binds[n].type = Type_Pulse;
	else if (strcmpi(type, "pre") == 0)
		g_Binds[n].type = Type_PreScript;
	else if (strcmpi(type, "post") == 0)
		g_Binds[n].type = Type_PostScript;
	else if (strcmpi(type, "attach") == 0)
		g_Binds[n].type = Type_Attach;
	else if (strcmpi(type, "detach") == 0)
		g_Binds[n].type = Type_Detach;
	else if (strcmpi(type, "modec") == 0)
		g_Binds[n].type = Type_SingleMode;
	else {
		g_Binds[n].type = Type_Invalid;
		throw "Invalid bind type.";
	}

	g_Binds[n].proc = strdup(proc);
	g_Binds[n].valid = true;

	return 1;
}

int internalunbind(const char* type, const char* proc) {
	binding_type_e bindtype;

	if (strcmpi(type, "client") == 0)
		bindtype = Type_Client;
	else if (strcmpi(type, "server") == 0)
		bindtype = Type_Server;
	else if (strcmpi(type, "pulse") == 0)
		bindtype = Type_Pulse;
	else if (strcmpi(type, "pre") == 0)
		bindtype = Type_PreScript;
	else if (strcmpi(type, "post") == 0)
		bindtype = Type_PostScript;
	else if (strcmpi(type, "attach") == 0)
		bindtype = Type_Attach;
	else if (strcmpi(type, "detach") == 0)
		bindtype = Type_Detach;
	else if (strcmpi(type, "modec") == 0)
		bindtype = Type_SingleMode;
	else
		return 0;

	for (int i = 0; i < g_BindCount; i++) {
		if (g_Binds[i].valid && g_Binds[i].type == bindtype && strcmp(g_Binds[i].proc, proc) == 0) {
			free(g_Binds[i].proc);
			g_Binds[i].valid = false;
		}
	}

	return 1;
}

int putserv(const char* text) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (Context == NULL)
		return 0;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return 0;

	IRC->InternalWriteLine(text);

	return 1;
}

int putclient(const char* text) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (Context == NULL)
		return 0;

	CClientConnection* Client = Context->GetClientConnection();

	if (!Client)
		return 0;

	Client->InternalWriteLine(text);

	return 1;
}

void jump(void) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return;

	Context->Reconnect();
	SetLatchedReturnValue(false);
}

bool onchan(const char* Nick, const char* Channel) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return false;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return false;

	if (Channel) {
		CChannel* Chan = IRC->GetChannel(Channel);

		if (Chan && Chan->GetNames()->Get(Nick))
			return true;
		else
			return false;
	} else {
		for (int i = 0; i < IRC->GetChannelCount(); i++) {
			CChannel* Chan = IRC->GetChannels()[i];

			if (Chan && Chan->GetNames()->Get(Nick))
				return true;
		}

		return false;
	}
}

const char* topic(const char* Channel) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return NULL;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return NULL;

	CChannel* Chan = IRC->GetChannel(Channel);

	if (!Chan)
		return NULL;

	return Chan->GetTopic();
}

const char* topicnick(const char* Channel) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return NULL;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return NULL;

	CChannel* Chan = IRC->GetChannel(Channel);

	if (!Chan)
		return NULL;

	return Chan->GetTopicNick();
}

int topicstamp(const char* Channel) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return 0;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return 0;

	CChannel* Chan = IRC->GetChannel(Channel);

	if (!Chan)
		return 0;

	return Chan->GetTopicStamp();
}

const char* getchanmode(const char* Channel) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return NULL;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return NULL;

	CChannel* Chan = IRC->GetChannel(Channel);

	if (!Chan)
		return NULL;

	return Chan->GetChanModes();
}

const char* internalchanlist(const char* Channel) {
	static char* NickList = NULL;
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return NULL;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return NULL;

	CChannel* Chan = IRC->GetChannel(Channel);

	if (!Chan)
		return NULL;

	NickList = (char*)realloc(NickList, 1);
	NickList[0] = '\0';

	CHashtable<CNick*, false>* Names = Chan->GetNames();

	int a = 0;

	while (hash_t<CNick*>* NickHash = Names->Iterate(a++)) {
		const char* Nick = NickHash->Value->GetNick();
		NickList = (char*)realloc(NickList, strlen(NickList) + strlen(Nick) + 4);
		if (*NickList)
			strcat(NickList, " ");

		strcat(NickList, "{");
		strcat(NickList, Nick);
		strcat(NickList, "}");
	}

	return NickList;
}

bool isop(const char* Nick, const char* Channel) {
	static char* NickList = NULL;
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return false;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return false;

	CChannel* Chan = IRC->GetChannel(Channel);

	if (Chan) {
		CNick* User = Chan->GetNames()->Get(Nick);

		return User->IsOp();
	} else {
		for (int i = 0; i < IRC->GetChannelCount(); i++) {
			CChannel* Chan = IRC->GetChannels()[i];

			if (Chan && Chan->GetNames()->Get(Nick) && Chan->GetNames()->Get(Nick)->IsOp())
				return true;
		}

		return false;
	}
}

bool isvoice(const char* Nick, const char* Channel) {
	static char* NickList = NULL;
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return false;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return false;

	CChannel* Chan = IRC->GetChannel(Channel);

	if (Chan) {
		CNick* User = Chan->GetNames()->Get(Nick);

		return User->IsVoice();
	} else {
		for (int i = 0; i < IRC->GetChannelCount(); i++) {
			CChannel* Chan = IRC->GetChannels()[i];

			if (Chan && Chan->GetNames()->Get(Nick) && Chan->GetNames()->Get(Nick)->IsVoice())
				return true;
		}

		return false;
	}
}

bool ishalfop(const char* Nick, const char* Channel) {
	static char* NickList = NULL;
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return false;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return false;

	CChannel* Chan = IRC->GetChannel(Channel);

	if (Chan) {
		CNick* User = Chan->GetNames()->Get(Nick);

		return User->IsHalfop();
	} else {
		for (int i = 0; i < IRC->GetChannelCount(); i++) {
			CChannel* Chan = IRC->GetChannels()[i];

			if (Chan && Chan->GetNames()->Get(Nick) && Chan->GetNames()->Get(Nick)->IsHalfop())
				return true;
		}

		return false;
	}
}

const char* channel(const char* Function, const char* Channel, const char* Parameter) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return NULL;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return "-1";

	CChannel* Chan = IRC->GetChannel(Channel);

	if (strcmpi(Function, "ison") == 0) {
		if (!Parameter) {
			if (Chan)
				return "1";
			else
				return "0";
		} else {
			if (!Chan)
				return "-1";
			else {
				CNick* Nick = Chan->GetNames()->Get(Parameter);

				if (Nick)
					return "1";
				else
					return "0";
			}
		}
	}

	if (strcmpi(Function, "join") == 0) {
		IRC->WriteLine("JOIN :%s", Channel);
		return "1";
	} else if (strcmpi(Function, "part") == 0) {
		IRC->WriteLine("PART :%s", Channel);
		return "1";
	} else if (strcmpi(Function, "mode") == 0) {
		IRC->WriteLine("MODE %s %s", Channel, Parameter);
		return "1";
	}

	if (!Chan)
		return "-1";

	if (strcmpi(Function, "topic") == 0)
		return Chan->GetTopic();

	if (strcmpi(Function, "topicnick") == 0)
		return Chan->GetTopicNick();

	static char* Buffer = NULL;

	if (strcmpi(Function, "topicstamp") == 0) {
		Buffer = (char*)realloc(Buffer, 20);

		sprintf(Buffer, "%d", Chan->GetTopicStamp());

		return Buffer;
	}

	if (strcmpi(Function, "hastopic") == 0)
		if (Chan->HasTopic())
			return "1";
		else
			return "0";

	if (strcmpi(Function, "hasnames") == 0)
		if (Chan->HasNames())
			return "1";
		else
			return "0";

	if (strcmpi(Function, "modes") == 0)
		return Chan->GetChanModes();

	if (strcmpi(Function, "nicks") == 0) {
		static char* NickList = NULL;

		NickList = (char*)realloc(NickList, 1);
		NickList[0] = '\0';

		CHashtable<CNick*, false>* Names = Chan->GetNames();

		int a = 0;

		while (hash_t<CNick*>* NickHash = Names->Iterate(a++)) {
			const char* Nick = NickHash->Value->GetNick();
			NickList = (char*)realloc(NickList, strlen(NickList) + strlen(Nick) + 4);
			if (*NickList)
				strcat(NickList, " ");

			strcat(NickList, "{");
			strcat(NickList, Nick);
			strcat(NickList, "}");
		}

		return NickList;
	}

	if (strcmpi(Function, "prefix") == 0) {
		CNick* Nick = Chan->GetNames()->Get(Parameter);

		static char outPref[2];

		if (Nick) {
			outPref[0] = Chan->GetHighestUserFlag(Nick->GetPrefixes());
			outPref[1] = '\0';

			return outPref;
		} else
			return "-1";
	}

	return "Function should be one of: ison join part mode topic topicnick topicstamp hastopic hasnames modes nicks prefix";
}

const char* user(const char* Function, const char* User, const char* Parameter, const char* Parameter2) {
	static char Buffer[1024];

	CBouncerUser* Context;

	if (User)
		Context = g_Bouncer->GetUser(User);
	else
		Context = g_Bouncer->GetUser(g_Context);

	if (strcmpi(Function, "isvalid") == 0)
		return Context ? "1" : "0";

	if (strcmpi(Function, "add") == 0) {
		if (Context)
			return "0";
		else {
			g_Bouncer->CreateUser(User, Parameter);

			return "1";
		}
	}

	if (strcmpi(Function, "remove") == 0) {
		if (Context) {
			g_Bouncer->RemoveUser(User);
			return "0";
		} else
			return "-1";
	}

	if (!Context)
		return "-1";

	if (strcmpi(Function, "server") == 0) {
		if (!Parameter)
			return Context->GetServer();
		else {
			Context->SetServer(Parameter);

			return "1";
		}
	}

	if (strcmpi(Function, "port") == 0) {
		if (!Parameter) {
			sprintf(Buffer, "%d", Context->GetPort());

			return Buffer;
		} else {
			Context->SetPort(atoi(Parameter));

			return "1";
		}
	}

	if (strcmpi(Function, "realname") == 0)
		if (!Parameter)
			return Context->GetRealname();
		else {
			Context->SetRealname(Parameter);

			return "1";
		}
	if (strcmpi(Function, "uptime") == 0) {
		sprintf(Buffer, "%d", Context->IRCUptime());

		return Buffer;
	}

	if (strcmpi(Function, "lock") == 0) {
		if (!Parameter) {
			sprintf(Buffer, "%d", Context->IsLocked());

			return Buffer;
		} else {
			if (atoi(Parameter))
				Context->Lock();
			else
				Context->Unlock();

			return "1";
		}
	}

	if (strcmpi(Function, "admin") == 0) {
		if (!Parameter) {
			sprintf(Buffer, "%d", Context->IsAdmin());

			return Buffer;
		} else {
			if (atoi(Parameter))
				Context->SetAdmin(true);
			else
				Context->SetAdmin(false);

			return "1";
		}
	}

	if (strcmpi(Function, "hasserver") == 0)
		return Context->GetIRCConnection() ? "1" : "0";

	if (strcmpi(Function, "hasclient") == 0)
		return Context->GetClientConnection() ? "1" : "0";

	if (strcmpi(Function, "nick") == 0) {
		if (!Parameter)
			return Context->GetNick();
		else {
			CIRCConnection* IRC = Context->GetIRCConnection();

			IRC->WriteLine("NICK :%s", Parameter);

			return "1";
		}
	}

	if (strcmpi(Function, "set") == 0) {
		if (!Parameter)
			return "-1";
		else {
			Context->GetConfig()->WriteString(Parameter, Parameter2);
			return "1";
		}
	}

	if (strcmpi(Function, "get") == 0) {
		if (!Parameter)
			return "-1";
		else
			return Context->GetConfig()->ReadString(Parameter);
	}

	if (strcmpi(Function, "log") == 0) {
		Context->Log("%s", Parameter);

		return "1";
	}

	if (strcmpi(Function, "client") == 0) {
		CClientConnection* Client = Context->GetClientConnection();

		if (!Client)
			return NULL;
		else {
			SOCKET Sock = Client->GetSocket();

			sockaddr_in saddr;
			socklen_t saddr_size = sizeof(saddr);

			getpeername(Sock, (sockaddr*)&saddr, &saddr_size);

			hostent* hent = gethostbyaddr((const char*)&saddr.sin_addr, sizeof(in_addr), AF_INET);

			if (hent)
				return hent->h_name;
			else
				return "<unknown>";
		}
	}

	return "Function should be one of: add remove server port realname uptime lock admin hasserver hasclient nick set get log";
}

const char* getbncuser(const char* User, const char* Type, const char* Parameter2) {
	static char Buffer[1024];
	CBouncerUser* Context = g_Bouncer->GetUser(User);

	if (!Context)
		throw "Invalid user.";

	if (strcmpi(Type, "server") == 0)
		return Context->GetServer();
	else if (strcmpi(Type, "realserver") == 0) {
		CIRCConnection* IRC = Context->GetIRCConnection();;

		if (IRC)
			return IRC->GetServer();
		else
			return NULL;
	} else if (strcmpi(Type, "port") == 0) {
		sprintf(Buffer, "%d", Context->GetPort());

		return Buffer;
	} else if (strcmpi(Type, "realname") == 0)
		return Context->GetRealname();
	else if (strcmpi(Type, "nick") == 0)
		return Context->GetNick();
	else if (strcmpi(Type, "uptime") == 0) {
		sprintf(Buffer, "%d", Context->IRCUptime());

		return Buffer;
	} else if (strcmpi(Type, "lock") == 0)
		return Context->IsLocked() ? "1" : "0";
	else if (strcmpi(Type, "admin") == 0)
		return Context->IsAdmin() ? "1" : "0";
	else if (strcmpi(Type, "hasserver") == 0)
		return Context->GetIRCConnection() ? "1" : "0";
	else if (strcmpi(Type, "hasclient") == 0)
		return Context->GetClientConnection() ? "1" : "0";
	else if (strcmpi(Type, "tag") == 0) {
		if (!Parameter2)
			return NULL;

		char* Buf = (char*)malloc(strlen(Parameter2) + 5);
		sprintf(Buf, "tag.%s", Parameter2);

		const char* ReturnValue = Context->GetConfig()->ReadString(Buf);

		free(Buf);

		return ReturnValue;
	} else
		throw "Type should be one of: server port realname nick uptime lock admin hasserver hasclient tag";
}

int setbncuser(const char* User, const char* Type, const char* Value, const char* Parameter2) {
	static char Buffer[1024];
	CBouncerUser* Context = g_Bouncer->GetUser(User);

	if (!Context)
		throw "Invalid user.";

	if (strcmpi(Type, "server") == 0)
		Context->SetServer(Value);
	else if (strcmpi(Type, "port") == 0)
		Context->SetPort(atoi(Value));
	else if (strcmpi(Type, "realname") == 0)
		Context->SetRealname(Value);
	else if (strcmpi(Type, "nick") == 0)
		Context->SetNick(Value);
	else if (strcmpi(Type, "lock") == 0) {
		if (atoi(Value))
			Context->Lock();
		else
			Context->Unlock();
	} else if (strcmpi(Type, "admin") == 0)
		Context->SetAdmin(atoi(Value) ? true : false);
	else if (strcmpi(Type, "tag") == 0 && Value) {
		char* Buf = (char*)malloc(strlen(Value) + 5);
		sprintf(Buf, "tag.%s", Value);

		Context->GetConfig()->WriteString(Buf, Parameter2);

		free(Buf);
	} else
		throw "Type should be one of: server port realname nick lock admin tag";


	return 1;
}

void addbncuser(const char* User, const char* Password) {
	CBouncerUser* U = g_Bouncer->CreateUser(User, Password);

	if (!U)
		throw "Could not create user.";
}

void delbncuser(const char* User) {
	if (!g_Bouncer->RemoveUser(User))
		throw "Could not remove user.";
}

int simul(const char* User, const char* Command) {
	CBouncerUser* Context;

	Context = g_Bouncer->GetUser(User);

	if (!Context)
		return -1;
	else {
		Context->Simulate(Command);

		return 1;
	}
}

const char* getchanhost(const char* Nick, const char* Channel) {
	CBouncerUser* Context;

	Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return NULL;
	else {
		CIRCConnection* IRC = Context->GetIRCConnection();

		if (IRC) {
			CChannel** Chans = IRC->GetChannels();
			int ChanCount = IRC->GetChannelCount();

			for (int i = 0; i < ChanCount; i++) {
				if (Chans[i]) {
					CNick* U = Chans[i]->GetNames()->Get(Nick);

					if (U && U->GetSite() != NULL)
						return U->GetSite();
				}
					
			}
		}

		return NULL;
	}

}

int getchanjoin(const char* Nick, const char* Channel) {
	static char* NickList = NULL;
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return 0;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return 0;

	CChannel* Chan = IRC->GetChannel(Channel);

	if (!Chan)
		return 0;

	CNick* User = Chan->GetNames()->Get(Nick);

	return User->GetChanJoin();
}

int getchanidle(const char* Nick, const char* Channel) {
	static char* NickList = NULL;
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (!Context)
		return 0;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return 0;

	CChannel* Chan = IRC->GetChannel(Channel);

	if (!Chan)
		return 0;

	CNick* User = Chan->GetNames()->Get(Nick);

	return time(NULL) - User->GetIdleSince();
}

const char* duration(int Interval) {
	// TODO: implement

	return "not yet implemented.";
}

int ticklerand(int limit) {
	int val = int(rand() / float(RAND_MAX) * limit);

	if (val > limit - 1)
		return limit - 1;
	else
		return val;
}

const char* bncversion(void) {
	return "0.3 0030000";
}

const char* bncnumversion(void) {
	return "0030000";
}

int bncuptime(void) {
	return g_Bouncer->GetStartup();
}

int floodcontrol(const char* Function) {
	CBouncerUser* User = g_Bouncer->GetUser(g_Context);

	if (!User)
		return 0;

	CIRCConnection* IRC = User->GetIRCConnection();

	if (!IRC)
		return 0;

	CFloodControl* FloodControl = IRC->GetFloodControl();

	if (strcmpi(Function, "bytes") == 0)
		return FloodControl->GetBytes();
	else if (strcmpi(Function, "items") == 0)
		return FloodControl->GetRealQueueSize();
	else if (strcmpi(Function, "on") == 0) {
		FloodControl->Enable();
		return 1;
	} else if (strcmpi(Function, "off") == 0) {
		FloodControl->Disable();
		return 1;
	}

	throw "Function should be one of: bytes items on off";
}

int clearqueue(const char* Queue) {
	CBouncerUser* User = g_Bouncer->GetUser(g_Context);

	if (!User)
		return 0;

	CIRCConnection* IRC = User->GetIRCConnection();

	if (!IRC)
		return 0;

	CQueue* TheQueue = NULL;

	if (strcmpi(Queue, "mode") == 0)
		TheQueue = IRC->GetQueueHigh();
	else if (strcmpi(Queue, "server") == 0)
		TheQueue = IRC->GetQueueMiddle();
	else if (strcmpi(Queue, "help") == 0)
		TheQueue = IRC->GetQueueLow();
	else if (strcmpi(Queue, "all") == 0)
		TheQueue = (CQueue*)IRC->GetFloodControl();
	else
		throw "Queue should be one of: mode server help all";

	int Size;

	if (TheQueue == IRC->GetFloodControl())
		Size = IRC->GetFloodControl()->GetRealQueueSize();
	else
		Size = TheQueue->GetQueueSize();

	TheQueue->FlushQueue();

	return Size;
}

int queuesize(const char* Queue) {
	CBouncerUser* User = g_Bouncer->GetUser(g_Context);

	if (!User)
		return 0;

	CIRCConnection* IRC = User->GetIRCConnection();

	if (!IRC)
		return 0;

	CQueue* TheQueue = NULL;

	if (strcmpi(Queue, "mode") == 0)
		TheQueue = IRC->GetQueueHigh();
	else if (strcmpi(Queue, "server") == 0)
		TheQueue = IRC->GetQueueMiddle();
	else if (strcmpi(Queue, "help") == 0)
		TheQueue = IRC->GetQueueLow();
	else if (strcmpi(Queue, "all") == 0)
		TheQueue = (CQueue*)IRC->GetFloodControl();
	else
		throw "Queue should be one of: mode server help all";

	int Size;

	if (TheQueue == IRC->GetFloodControl())
		Size = IRC->GetFloodControl()->GetRealQueueSize();
	else
		Size = TheQueue->GetQueueSize();

	return Size;
}

int puthelp(const char* text) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (Context == NULL)
		return 0;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return 0;

	IRC->GetQueueLow()->QueueItem(text);

	return 1;
}

int putquick(const char* text) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (Context == NULL)
		return 0;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return 0;

	IRC->GetQueueHigh()->QueueItem(text);

	return 1;
}

const char* getisupport(const char* Feature) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (Context == NULL)
		return NULL;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return NULL;

	return IRC->GetISupport(Feature);
}

int requiresparam(char Mode) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (Context == NULL)
		return 0;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return 0;

	return IRC->RequiresParameter(Mode);
}

bool isprefixmode(char Mode) {
	CBouncerUser* Context = g_Bouncer->GetUser(g_Context);

	if (Context == NULL)
		return 0;

	CIRCConnection* IRC = Context->GetIRCConnection();

	if (!IRC)
		return 0;

	return IRC->IsNickMode(Mode);	
}

const char* bncmodules(void) {
	static char* Buffer = NULL;

	CModule** Modules = g_Bouncer->GetModules();
	int ModuleCount = g_Bouncer->GetModuleCount();

	if (Buffer)
		*Buffer = '\0';

	for (int i = 0; i < ModuleCount; i++) {
		if (Modules[i]) {
			const char* File = Modules[i]->GetFilename();
			const void* Ptr = Modules[i]->GetHandle();
			const void* ModPtr = Modules[i]->GetModule();

			char* P = (char*)realloc(Buffer, (Buffer ? strlen(Buffer) : 0) + strlen(File) + 50);

			if (!Buffer)
				P[0] = '\0';

			Buffer = P;

			sprintf(Buffer + strlen(Buffer), "%s{{%d} {%s} {%p} {%p}}", *Buffer ? " " : "", i, File, Ptr, ModPtr);
		}
	}

	return Buffer;
}