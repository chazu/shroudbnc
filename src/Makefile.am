bin_PROGRAMS=sbnc

sbnc_SOURCES=Banlist.cpp \
	Cache.cpp \
	Config.cpp \
	Core.cpp \
	Log.cpp \
	User.cpp \
	Channel.cpp \
	ClientConnection.cpp \
	ClientConnectionMultiplexer.cpp \
	Connection.cpp \
	DnsEvents.cpp \
	DnsSocket.cpp \
	FIFOBuffer.cpp \
	FloodControl.cpp \
	IdentSupport.cpp \
	IRCConnection.cpp \
	Keyring.cpp \
	Module.cpp \
	Nick.cpp \
	Queue.cpp \
	sbnc.cpp \
	Timer.cpp \
	TrafficStats.cpp \
	utility.cpp \
	Banlist.h \
	Config.h \
	Core.h \
	Log.h \
	User.h \
	Cache.h \
	Channel.h \
	ClientConnection.h \
	ClientConnectionMultiplexer.h \
	Connection.h \
	DnsEvents.h \
	DnsSocket.h \
	FIFOBuffer.h \
	FloodControl.h \
	Hashtable.h \
	IdentSupport.h \
	IRCConnection.h \
	Keyring.h \
	List.h \
	Listener.h \
	ModuleFar.h \
	Module.h \
	Nick.h \
	Object.h \
	Result.h \
	Queue.h \
	sbnc.h \
	SocketEvents.h \
	StdAfx.h \
	Timer.h \
	TrafficStats.h \
	unix.h \
	utility.h \
	Vector.h \
	win32.h

sbnc_LDADD=${LIBCARES} ../third-party/md5/libmd5.la ../third-party/mmatch/libmmatch.la ${LIBSNPRINTF} ${LIBLTDL}
sbnc_LDFLAGS=-export-dynamic

AM_CFLAGS=-DSBNC ${LTDLINCL} ${CARESINCL} ${SNPRINTFINCL} -I../third-party/mmatch
AM_CXXFLAGS=-DSBNC ${LTDLINCL} ${CARESINCL} ${SNPRINTFINCL} -I../third-party/mmatch
