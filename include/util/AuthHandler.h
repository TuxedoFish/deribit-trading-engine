#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include <string>

class AuthHandler
{
public:
	static std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);

private:
};

#endif