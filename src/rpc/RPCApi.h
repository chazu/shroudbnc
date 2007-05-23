/*******************************************************************************
 * shroudBNC - an object-oriented framework for IRC                            *
 * Copyright (C) 2005-2007 Gunnar Beutner                                      *
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

#ifndef _RPCAPI_H
#define _RPCAPI_H

#if !defined(RPCCLIENT) && !defined(RPCSERVER)
#	define RPCCLIENT
#endif

#ifdef _WIN32
#	if defined(SBNC) && defined(RPCCLIENT)
#		define RPCAPI __declspec(dllexport)
#	else
#		define RPCAPI __declspec(dllimport)
#	endif
#else
#	define RPCAPI
#endif

#ifdef _WIN32
typedef HANDLE PIPE;
#else
typedef int PIPE;

#define WSAGetLastError() errno

int GetStdHandle(int Handle);
int ReadFile(int File, void *Buffer, int Size, int *Read, void *Dummy);
int WriteFile(int File, const void *Buffer, int Size, int *Written, void *Dummy);
#endif

typedef struct PipePair_s {
	PIPE In;
	PIPE Out;
} PipePair_t;

typedef enum Function_e {
	Function_safe_socket,
	Function_safe_getpeername,
	Function_safe_getsockname,
	Function_safe_bind,
	Function_safe_connect,
	Function_safe_listen,
	Function_safe_accept,
	Function_safe_poll,
	Function_safe_recv,
	Function_safe_send,
	Function_safe_shutdown,
	Function_safe_closesocket,
	Function_safe_getsockopt,
	Function_safe_setsockopt,
	Function_safe_ioctlsocket,
	Function_safe_errno,
	Function_safe_print,
	Function_safe_scan,
	Function_safe_scan_passwd,
	Function_safe_sendto,
	Function_safe_recvfrom,

	Function_safe_put_string,
	Function_safe_put_integer,
	Function_safe_put_box,
	Function_safe_remove,
	Function_safe_get_string,
	Function_safe_get_integer,
	Function_safe_get_box,
	Function_safe_enumerate,
	Function_safe_rename,
	Function_safe_get_parent,
	Function_safe_get_name,

	Function_safe_exit,

	last_function
} Function_t;

typedef enum Type_e {
	Integer,
	Pointer,
	Block
} Type_t;

#define Flag_None 0
#define Flag_Out 1
#define Flag_Alloc 2

typedef struct Value_s {
	Type_t Type;
	char Flags;
	int NeedFree;

	union {
		int Integer;

		struct {
			unsigned int Size;
			void *Pointer;
			void *Block;
		};
	};
} Value_t;

#define RPC_INT(Int) RpcBuildInteger(Int)
#define RPC_BLOCK(Ptr, Size, Flag) RpcBuildBlock(Ptr, Size, Flag)
#define RPC_POINTER(Ptr) RpcBuildPointer(Ptr)

void RpcFreeValue(Value_t Value);
void RpcFatal(void);

#ifdef RPCSERVER
int RpcInvokeClient(char *Program, PipePair_t *Pipes);
int RpcRunServer(PipePair_t Pipes);
int RpcProcessCall(char *Data, size_t Length, PIPE Out);
#endif

#ifdef RPCCLIENT
int RpcInvokeFunction(PIPE PipeIn, PIPE PipeOut, Function_t Function, Value_t *Arguments, unsigned int ArgumentCount, Value_t *ReturnValue);
Value_t RpcBuildInteger(int Value);
Value_t RpcBuildBlock(const void *Pointer, int Size, char Flag);
Value_t RpcBuildPointer(const void *Pointer);
#endif
#endif
