#ifndef __ADDRESS_H
#define __ADDRESS_H

#include <ostream>
#include <string>

class Address
{
public:
	Address(const std::string& address);
	Address(const Address& src);
	~Address(void);

	bool operator<(const Address& other) const;

	Address& operator=(const Address& src);

	friend std::ostream& operator<<(std::ostream& out, const Address& addr);
private:
	std::string mAddress;
};

#endif
