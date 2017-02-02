#include "Socket.h"

Socket::Socket(void):
	mConnected(false),
	mInvalid(true),
	mReadReady(false),
	mReadyReadyOOB(false),
	mWriteReady(false),
	mTimedout(false),
	mSocket(-1),
	mReceivedBytes(0)
{
	// empty
}

Socket::Socket(int sock):
	mConnected(true),
	mInvalid(false),
	mReadReady(false),
	mReadyReadyOOB(false),
	mWriteReady(false),
	mTimedout(false),
	mSocket(sock),
	mReceivedBytes(0)
{
	// empty
}

Socket::~Socket(void)
{
	if (mConnected)
	{
		close();
	}
}

int Socket::bind(const char* port, const char* ip)
{
	addrinfo hints;
	addrinfo* res;
	int ret;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; //AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(ip, port, &hints, &res) != 0)
	{
		cerr << "Getaddrinfo error " << errno << endl;
		freeaddrinfo(res);
		return -1;
	}

	int socketReuseValue = 1;
	for (addrinfo* p = res; p != NULL; p = p->ai_next)
	{
		mSocket = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (mSocket == -1)
		{
			continue;
		}

		if (::setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &socketReuseValue, sizeof(socketReuseValue)) == -1)
		{
			cerr << "Unable to reuse socket " << errno << " " << strerror(errno) << endl;
			::close(mSocket);
			mSocket = -1;
			continue;
		}
		if (::bind(mSocket, res->ai_addr, res->ai_addrlen) == -1)
		{
			cerr << "Unable to bind " << errno << " " << strerror(errno) << endl;
			::close(mSocket);
			mSocket = -1;
			continue;
		}
		break;
	}
	freeaddrinfo(res);

	if (mSocket == -1)
	{
		return EAGAIN;
	}

	ret = ::listen(mSocket, SOCKET_CONNECTION_LIMIT);
	mConnected = (ret == 0);
	return ret;
}

Socket* Socket::accept(bool block)
{
	fd_set rset;
	sockaddr_storage connInfo;
	socklen_t addrlen = sizeof(connInfo);
	int nsock = -1; 

	FD_ZERO(&rset);
	FD_SET(mSocket, &rset);

	timeval timeout = {0, 30};
	int selectVal = ::select(mSocket + 1, &rset, NULL, NULL, &timeout);

	if (selectVal < 0)
	{
		std::cerr << "Error encountered in ::select " << errno << ", " << strerror(errno) << std::endl;
		return NULL;
	}

	if (block || FD_ISSET(mSocket, &rset))
	{
		nsock = ::accept(mSocket, (sockaddr*)&connInfo, &addrlen);
	}

	if (nsock == -1)
	{
		return NULL;
	}
	return new Socket(nsock);
}

bool Socket::connected(void) const
{
	return mConnected;
}

int Socket::recvLength(void) const
{
	return mReceivedBytes;
}

int Socket::close(void)
{
	int ret = -1;
	if (mConnected)
	{
		::shutdown(mSocket, SHUT_RDWR);
		ret = ::close(mSocket);
		mConnected = false;

		if (ret != 0)
		{
			throw "Socket::close failed";
		}
		mSocket = -1;
	}
	return ret;
}

int Socket::connect(const char* ip, const char* port)
{
	addrinfo hints;
	addrinfo* results = NULL;
	int ret = -1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;

	do
	{
		usleep(30);
		ret = getaddrinfo(ip, port, &hints, &results);
	} while (ret == EAI_AGAIN);

	if (ret != 0)
	{
		cout << "getaddrifo error: " << gai_strerror(ret) << endl;
		return ret;
	}

	int sock = -1;
	for (addrinfo* addr = results; addr != NULL; addr = addr->ai_next)
	{
		sock = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (sock == -1)
		{
			cout << "Unable to connect socket! " << errno << strerror(errno) << endl;
			continue;
		}
		ret = ::connect(sock, addr->ai_addr, addr->ai_addrlen);

		if (ret == 0)
		{
			cout << "Connection good for fd=" << sock << endl;
			mConnected = true;
			mInvalid = false;
			mSocket = sock;
			break;
		}
	}

	freeaddrinfo(results);
	return ret;
}

int Socket::receive(char* buffer, int bufferLength, int timeout)
{
	int received = 0; // Other end hung up.
	int flags = 0;

	do
	{
		checkForReady(POLLIN | POLLPRI, timeout);
		if (!mConnected)
		{
			cerr << "Socket not connected." << endl;
			received = 0;
			break;
		}
		if (mTimedout)
		{
			break;
		}
		
		if (mReadReady || mReadyReadyOOB)
		{
			if (mReadyReadyOOB)
			{
				flags = MSG_OOB;
			}

			int ret = ::recv(mSocket, buffer + received, bufferLength - received, flags);

			if (ret == 0 /* Other side shut down */)
			{
				cerr << "Socket shut down by other side." << endl;
				received = 0;
				mConnected = false;
				break;
			}
			else if (ret < 0)
			{
				cerr << "Fewer than 0 bytes received." << endl;
				received = 0;
				return -1;
			}
			else
			{
				received += ret;
			}
		}
	} while (received < bufferLength);

	mReceivedBytes = received;

	return received;
}

int Socket::send(const char* buffer, int bufferLength, bool critical)
{
	int flags = MSG_DONTWAIT | MSG_NOSIGNAL;
	int sent = 0;

	if (critical)
	{
		flags |= MSG_OOB;
	}

	do
	{
		checkForReady(POLLOUT, 30);
		if (!mConnected)
		{
			return -42;
		}

		if (!mConnected || !mWriteReady)
		{
			sent = -1;
			break;
		}

		int ret = ::send(mSocket, buffer + sent, bufferLength - sent, flags);

		if (ret == EPIPE)
		{
			cout << "EPIPE encountered!" << endl;
			mConnected = false;
			mInvalid = true;
			break;
		}
		else if (ret == -1)
		{
			break;
		}

		sent += ret;
	} while (sent < bufferLength); 

	return sent;
}

void Socket::checkForReady(short events, int timeout)
{
	if (!mConnected)
	{
		cout << "HERER" << endl;
		return;
	}

	int ret = -1;
	pollfd pollInfo;
	pollInfo.fd = mSocket;
	pollInfo.events = events;
	pollInfo.revents = 0;

	ret = poll(&pollInfo, 1, timeout);
	mReceivedBytes = 0;

	if (ret == -1)
	{
		cerr << "Error encountered: " << ret << ", errno: " << errno << strerror(errno) << endl;
		mConnected = false;
	}
	else if (ret == 0 /* Socket timed out */)
	{
		mTimedout = true;
	}
	else if (mConnected)
	{
		mTimedout = false;
		mInvalid = pollInfo.revents & POLLNVAL;
		mReadReady = pollInfo.revents & POLLIN;
		mReadyReadyOOB = pollInfo.revents & POLLPRI;
		mWriteReady = pollInfo.revents & POLLOUT;
		mConnected = !(pollInfo.revents & POLLHUP);
	}

}
