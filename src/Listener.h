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

/**
 * CListenerBase<InheritedClass>
 *
 * Implements a generic socket listener.
 */
template<typename InheritedClass>
class CListenerBase : public CSocketEvents {
private:
	SOCKET m_Listener; /**< the listening socket */

	virtual bool Read(bool DontProcess) {
		char PeerAddress[MAX_SOCKADDR_LEN];
		socklen_t PeerSize = sizeof(PeerAddress);
		SOCKET Client;

		Client = accept(m_Listener, (sockaddr *)PeerAddress, &PeerSize);

		if (Client != INVALID_SOCKET) {
			Accept(Client, (sockaddr *)PeerAddress);
		}

		return true;
	}

	virtual void Write(void) { }
	virtual void Error(void) { }
	virtual bool HasQueuedData(void) { return false; }
	virtual bool DoTimeout(void) { return false; }
	virtual bool ShouldDestroy(void) { return false; }

	virtual const char *GetClassName(void) { return "CListenerBase"; }

	/**
	 * Initialize
	 *
	 * Initialized the listener object.
	 *
	 * @param Listener a listening socket
	 */
	void Initialize(SOCKET Listener) {
		m_Listener = Listener;

		if (m_Listener == INVALID_SOCKET) {
			return;
		}

		g_Bouncer->RegisterSocket(m_Listener, static_cast<CSocketEvents *>(this));
	}

protected:
	virtual void Accept(SOCKET Client, const sockaddr *PeerAddress) = 0;

public:
	/**
	 * CListenerBase
	 *
	 * Constructs a new CListenerBase object.
	 *
	 * @param Port the port of the socket listener
	 * @param BindIp the ip address used for binding the listener
	 * @param Family the socket family of the listener (AF_INET or AF_INET6)
	 */
	CListenerBase(unsigned int Port, const char *BindIp = NULL, int Family = AF_INET) {
		Initialize(g_Bouncer->CreateListener(Port, BindIp, Family));
	}

	CListenerBase(void) {
		// used for unfreezing listeners
	}

	/**
	 * Freeze
	 *
	 * Persists the socket listener in a box.
	 *
	 * @param Box the box
	 */
	RESULT(bool) Freeze(CAssocArray *Box) {
		Box->AddInteger("~listener.fd", m_Listener);
		g_Bouncer->UnregisterSocket(m_Listener);
		m_Listener = INVALID_SOCKET;

		delete this;

		RETURN(bool, true);
	}

	/**
	 * Thaw
	 *
	 * Depersists a listener object.
	 *
	 * @param Box the box which is being used for storing the listener
	 */
	static RESULT(InheritedClass *) Thaw(CAssocArray *Box) {
		InheritedClass *Listener = new InheritedClass();

		if (Listener == NULL) {
			THROW(InheritedClass *, Generic_OutOfMemory, "new operator failed.");
		}

		Listener->Initialize(Box->ReadInteger("~listener.fd"));

		RETURN(InheritedClass *, Listener);
	}

	/**
	 * ~CListenerBase
	 *
	 * Destructs a listener object.
	 */
	virtual ~CListenerBase(void) {
		if (g_Bouncer != NULL && m_Listener != INVALID_SOCKET) {
			g_Bouncer->UnregisterSocket(m_Listener);
		}

		closesocket(m_Listener);
	}

	/**
	 * Destroy
	 *
	 * Destroys a listener object and calls its destructor.
	 */
	virtual void Destroy(void) {
		delete this;
	}

	/**
	 * IsValid
	 *
	 * Verifies that the underlying socket object is valid.
	 */
	virtual bool IsValid(void) { 
		if (m_Listener != INVALID_SOCKET) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * GetSocket
	 *
	 * Returns the socket which is used by the listener object.
	 */
	virtual SOCKET GetSocket(void) {
		return m_Listener;
	}
};

/**
 * IMPL_SOCKETLISTENER
 *
 * Can be used to implement a custom listener.
 */
#define IMPL_SOCKETLISTENER(ClassName) class ClassName : public CListenerBase<ClassName>
