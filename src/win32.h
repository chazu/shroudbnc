/*******************************************************************************
 * shroudBNC - an object-oriented framework for IRC                            *
 * Copyright (C) 2005-2007,2010 Gunnar Beutner                                 *
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

#ifndef WIN32_H
#define WIN32_H

// win32 specific header

#define WIN32_LEAN_AND_MEAN 1
#define _WIN32_WINNT 0x0500
#define NTDDI_VERSION 0x05000100

#ifndef SWIG
#	include <windows.h>
#	include <winsock2.h>
#	include <ws2tcpip.h>
#	include <ws2ipdef.h>
#	include <shlwapi.h>
#	include <direct.h>
#	include <io.h>
#else /* SWIG */
typedef struct { char __addr[16]; } sockaddr_in6;
#endif /* SWIG */

#define mkdir _mkdir

#ifndef S_IRUSR
#define S_IRUSR 0
#endif /* S_IRUSR */

#ifndef S_IWUSR
#define S_IWUSR 0
#endif /* S_IWUSR */

#ifndef S_IXUSR
#define S_IXUSR 0
#endif /* S_IXUSR */

#ifndef S_IRGRP
#define S_IRGRP 0
#endif /* S_IRGRP */

#ifndef S_IROTH
#define S_IROTH 0
#endif /* S_IROTH */

#ifdef _DEBUG
#	include <dbghelp.h>
#	include <intrin.h>
#endif /* _DEBUG */

#if !defined(socklen_t)
typedef int socklen_t;
#endif /* !defined(socklen_t) */

#undef GetClassName

#undef strcasecmp
#define strcasecmp strcmpi

#define EXPORT __declspec(dllexport)

#ifndef _MSC_VER
#undef LoadLibrary
#define LoadLibrary(lpLibFileName) (HMODULE)lt_dlopen(lpLibFileName)
#undef FreeLibrary
#define FreeLibrary(hLibModule) hLibModule ? !lt_dlclose((lt_dlhandle)hLibModule) : 0
#undef GetProcAddress
#define GetProcAddress(hModule, lpProcName) lt_dlsym((lt_dlhandle)hModule, lpProcName)
#endif /* _MSC_VER */

#undef HAVE_AF_INET6
#define HAVE_AF_INET6
#undef HAVE_STRUCT_IN6_ADDR
#define HAVE_STRUCT_IN6_ADDR
#undef HAVE_STRUCT_SOCKADDR_IN6
#define HAVE_STRUCT_SOCKADDR_IN6

#define MAXPATHLEN MAX_PATH

#ifdef SBNC
#	define SBNCAPI __declspec(dllexport)
#else /* SBNC */
#	define SBNCAPI __declspec(dllimport)
#endif /* SBNC */

#endif /* WIN32_H */
