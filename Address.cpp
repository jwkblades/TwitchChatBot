#include "Address.h"

Address::Address(const std::string& address):
	mAddress(address)
{
	// empty
}
Address::Address(const Address& src)
{
	*this = src;
}

Address::~Address(void)
{
	// empty
}

bool Address::operator<(const Address& other) const
{
	return mAddress < other.mAddress;
}

Address& Address::operator=(const std::string& src)
{
	mAddress = src;
	return *this;
}
Address& Address::operator=(const Address& src)
{
	if (this == &src)
	{
		return *this;
	}

	mAddress = src.mAddress;
	return *this;
}

std::ostream& operator<<(std::ostream& out, const Address& addr)
{
	out << addr.mAddress;
	return out;
}
