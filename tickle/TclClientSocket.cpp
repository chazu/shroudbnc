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
#include "../BouncerCore.h"
#include "../SocketEvents.h"
#include "../Connection.h"
#include "../Hashtable.h"
#include "TclClientSocket.h"

extern Tcl_Interp* g_Interp;

CHashtable<CTclClientSocket*, false>* g_TclClientSockets;
extern int g_SocketIdx;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTclClientSocket::CTclClientSocket(SOCKET Socket, sockaddr_in Peer) {
	m_Socket = Socket;
	m_Wrap = g_Bouncer->WrapSocket(Socket, Peer);
	g_Bouncer->RegisterSocket(Socket, this);

	char Buf[20];

	itoa(g_SocketIdx, Buf, 10);
	m_Idx = g_SocketIdx;
	g_SocketIdx++;

	g_TclClientSockets->Add(Buf, this);

	m_Control = NULL;
}

CTclClientSocket::~CTclClientSocket() {
	g_Bouncer->UnregisterSocket(m_Socket);
	g_Bouncer->DeleteWrapper(m_Wrap);
	closesocket(m_Socket);

	char Buf[20];
	itoa(m_Idx, Buf, 10);

	g_TclClientSockets->Remove(Buf);
}

void CTclClientSocket::Destroy(void) {
	if (m_Control) {
		Tcl_Obj* objv[3];

		char ptr[20];
		itoa(m_Idx, ptr, 10);

		objv[0] = Tcl_NewStringObj(m_Control, strlen(m_Control));
		Tcl_IncrRefCount(objv[0]);

		objv[1] = Tcl_NewStringObj(ptr, strlen(ptr));
		Tcl_IncrRefCount(objv[1]);

		objv[2] = Tcl_NewStringObj("", 0);
		Tcl_IncrRefCount(objv[2]);

		Tcl_EvalObjv(g_Interp, 3, objv, TCL_EVAL_GLOBAL);

		Tcl_DecrRefCount(objv[2]);
		Tcl_DecrRefCount(objv[1]);
		Tcl_DecrRefCount(objv[0]);
	}

	delete this;
}

bool CTclClientSocket::Read(void) {
	bool Ret = m_Wrap->Read();

	char* Line;

	while (m_Wrap->ReadLine(&Line) && m_Control) {
		Tcl_Obj* objv[3];
		
		char ptr[20];
		itoa(m_Idx, ptr, 10);

		objv[0] = Tcl_NewStringObj(m_Control, strlen(m_Control));
		Tcl_IncrRefCount(objv[0]);

		objv[1] = Tcl_NewStringObj(ptr, strlen(ptr));
		Tcl_IncrRefCount(objv[1]);

		Tcl_DString dsText;

		Tcl_DStringInit(&dsText);
		Tcl_ExternalToUtfDString(NULL, Line, -1, &dsText);

		objv[2] = Tcl_NewStringObj(Tcl_DStringValue(&dsText), strlen(Tcl_DStringValue(&dsText)));
		Tcl_IncrRefCount(objv[2]);

		int Code = Tcl_EvalObjv(g_Interp, 3, objv, TCL_EVAL_GLOBAL);

		Tcl_DecrRefCount(objv[1]);
		Tcl_DecrRefCount(objv[0]);
		Tcl_DStringFree(&dsText);

		g_Bouncer->Free(Line);
	}

	return Ret;
}

void CTclClientSocket::Write(void) {
	m_Wrap->Write();
}

void CTclClientSocket::Error(void) {
	m_Wrap->Error();
}

bool CTclClientSocket::HasQueuedData(void) {
	return m_Wrap->HasQueuedData();
}

bool CTclClientSocket::DoTimeout(void) {
	return m_Wrap->DoTimeout();
}

void CTclClientSocket::SetControlProc(const char* Proc) {
	m_Control = strdup(Proc);
}

const char* CTclClientSocket::GetControlProc(void) {
	return m_Control;
}

void CTclClientSocket::WriteLine(const char* Line) {
	m_Wrap->WriteLine(Line);
}

const char* CTclClientSocket::ClassName(void) {
	return "CTclClientSocket";
}

int CTclClientSocket::GetIdx(void) {
	return m_Idx;
}