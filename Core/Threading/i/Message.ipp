#include "Message.h"

template<typename T>
T Message::get(void) const
{
	return (T)mBuffer;
}
