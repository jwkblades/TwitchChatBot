#include "Socket.h"

#include <iostream>
#include <string>
#include <fstream>
using namespace std;

int main(void)
{

	Socket twitchConnection;

	int connectionRet = twitchConnection.connect("irc.chat.twitch.tv", "6667");
	cout << "Connection return value: " << connectionRet << endl;

	ifstream fileIn("/home/james/.twitch_oauth.key");
	std::string oauthToken = "";
	if (!fileIn.is_open())
	{
		cout << "ERROR: Unable to open file..." << endl;
	}
	fileIn >> oauthToken;
	fileIn.close();


	std::string authString = "PASS " + oauthToken + "\r\n";
	int sentBytes = twitchConnection.send(authString.c_str(), authString.size());
	cout << "Sent " << sentBytes << " bytes" << endl;
	std::string nickString = "NICK betawar1305\r\n";
	sentBytes = twitchConnection.send(nickString.c_str(), nickString.size());
	cout << "Sent " << sentBytes << " bytes" << endl;

	const int bufferLength = 1024;
	char* buffer = new char[bufferLength];

	int receivedBytes = twitchConnection.receive(buffer, bufferLength - 1);
	buffer[receivedBytes] = '\0';

	cout << "Received " << receivedBytes << " bytes: " << endl << buffer << endl;

	twitchConnection.close();
	delete [] buffer;

	return 0;
}
