#include "Socket.h"
#include "PostOffice.h"

#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
using namespace std;

int main(void)
{

	Socket twitchConnection;
	Address commandParser("commandParser");
	PostOffice::instance()->registerAddress(commandParser);
	Address transceiver("transceiver");

	auto transceiverLambda = [&twitchConnection, commandParser, transceiver](void) -> void
	{
		const int bufferLength = 1024;
		char* buffer = new char[bufferLength];

		PostOffice* postOffice = PostOffice::instance();
		Address myAddress = transceiver;
		if (!postOffice->registerAddress(myAddress))
		{
			cout << "Unable to register address " << myAddress << endl;
			return;
		}

		std::string incompleteMessage = "";

		while (twitchConnection.connected())
		{

			if (PostOffice::isValidInstance(postOffice) && postOffice->checkMail(myAddress))
			{
				Message receivedMessage = postOffice->getMail(myAddress);

				std::size_t size = receivedMessage.size();
				std::size_t sentBytes = 0;
				const char* ptr = receivedMessage.raw();

				do
				{
					int sent = twitchConnection.send(ptr + sentBytes, size - sentBytes, 100);

					if (sent < 0)
					{
						cout << "Something bad happened while sending message" << endl;
						break;
					}

					sentBytes += sent;
				} while (sentBytes < size);
			}

			int receivedBytes = twitchConnection.receive(buffer, bufferLength - 1);
			if (receivedBytes == 0)
			{
				usleep(30);
				continue;
			}
			buffer[receivedBytes] = '\0';

			std::string currentMessage = "";
			std::stringstream s(buffer);
			do
			{
				std::getline(s, currentMessage, '\n');
				incompleteMessage += currentMessage;

				if (!incompleteMessage.empty() && incompleteMessage.back() == '\r')
				{
					incompleteMessage = incompleteMessage.substr(0, incompleteMessage.size() - 1);
					if (PostOffice::isValidInstance(postOffice))
					{
						cout << "Sending incoming command '" << incompleteMessage << "' to " << commandParser << endl;
						Message msg(incompleteMessage.c_str(), incompleteMessage.size() + 1);
						postOffice->sendMessage(commandParser, msg);
					}
					else
					{
						cout << "Invalid PostOffice instance." << endl;
						return;
					}

					incompleteMessage = "";
				}
			} while (!s.eof());

			cout << "Received " << receivedBytes << " bytes: " << endl << buffer << endl;
		}

		delete [] buffer;
	};

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


	std::list<std::thread> threads;
	threads.push_back(std::thread(transceiverLambda));

	for (std::thread& t : threads)
	{
		t.join();
	}

	twitchConnection.close();

	return 0;
}
