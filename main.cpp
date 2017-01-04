#include "Socket.h"
#include "PostOffice.h"

#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
using namespace std;

#include <vector>

std::vector<std::string> split(std::string str, const std::string& delimiter)
{
	std::vector<std::string> parts;

	while (!str.empty())
	{
		std::size_t pos = str.find(delimiter);
		std::string part = str.substr(0, pos);
		if (pos == std::string::npos)
		{
			str = "";
		}
		else
		{
			str = str.substr(pos + delimiter.size());
		}

		if (!part.empty())
		{
			parts.push_back(part);
		}
	}

	return parts;
}


int main(void)
{

	Socket twitchConnection;
	Address commandParser("commandParser");
	PostOffice::instance()->registerAddress(commandParser);
	Address transceiver("transceiver");

	auto commandParserLambda = [commandParser, transceiver](void) -> void
	{
		PostOffice* postOffice = PostOffice::instance();
		Address myAddress = commandParser;

		std::string command = "";
		while (command != "SHUTDOWN")
		{
			Message incomingMsg;
			if (PostOffice::isValidInstance(postOffice) && postOffice->checkMail(myAddress))
			{
				incomingMsg = postOffice->getMail(myAddress);
			}

			if (incomingMsg.size() == 0)
			{
				usleep(300);
				continue;
			}

			cout << myAddress << " received '" << incomingMsg.raw() << "' " << incomingMsg.size() << endl << endl;

			command = std::string(incomingMsg.raw(), incomingMsg.size() - 1);

			std::vector<std::string> parts = split(command, " ");
			for (const std::string& part : parts)
			{
				cout << "Part: '" << part << "'" << endl;
			}

			if (parts.empty())
			{
				continue;
			}

			if (parts.at(0).empty())
			{
				// we have an empty prefix/ command...
				continue;
			}

			if(parts.at(0).at(0) == ':')
			{
				// We have a prefix, meaning that the second item (if it exists) is the command.
				parts.erase(parts.begin());
			}
			if (parts.empty())
			{
				continue;
			}

			cout << "Command: " << command << endl;
			command = parts.at(0);
			if (command == "PING")
			{
				std::string pong = "PONG " + parts.at(1) + "\r\n";

				if (PostOffice::isValidInstance(postOffice))
				{
					Message msg(pong.c_str(), pong.size() + 1);
					postOffice->sendMessage(transceiver, msg);
				}

			}
			else if (command == "376")
			{
				// the end of the MOTD message.
				std::string cmd = "JOIN #betawar1305\r\n";

				if (PostOffice::isValidInstance(postOffice))
				{
					Message msg(cmd.c_str(), cmd.size() + 1);
					postOffice->sendMessage(transceiver, msg);
				}
			}


		}
		cout << "Exiting " << myAddress << endl;
	};

	auto transceiverLambda = [&twitchConnection, commandParser, transceiver](void) -> void
	{
		const int bufferLength = 1024;
		char* buffer = new char[bufferLength];

		PostOffice* postOffice = PostOffice::instance();
		Address myAddress = transceiver;
		if (!postOffice->registerAddress(myAddress))
		{
			cout << "Unable to register address " << myAddress << endl;
			if (PostOffice::isValidInstance(postOffice))
			{
				std::string shutdownCommandParser = "SHUTDOWN";
				Message msg(shutdownCommandParser.c_str(), shutdownCommandParser.size() + 1);
				postOffice->sendMessage(commandParser, msg);
			}
			cout << "Exiting " << myAddress << endl;
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
						cout << "Sending incoming command '" << incompleteMessage << "' " << (incompleteMessage.size() + 1) << " to " << commandParser << endl;
						Message msg(incompleteMessage.c_str(), incompleteMessage.size() + 1);
						postOffice->sendMessage(commandParser, msg);
					}
					else
					{
						cout << "Invalid PostOffice instance." << endl;
						break;
					}

					incompleteMessage = "";
				}
			} while (!s.eof());

			cout << "Received " << receivedBytes << " bytes: " << endl << buffer << endl;
		}

		delete [] buffer;
		if (PostOffice::isValidInstance(postOffice))
		{
			std::string shutdownCommandParser = "SHUTDOWN";
			Message msg(shutdownCommandParser.c_str(), shutdownCommandParser.size() + 1);
			postOffice->sendMessage(commandParser, msg);
		}
		cout << "Exiting " << myAddress << endl;
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
	threads.push_back(std::thread(commandParserLambda));

	for (std::thread& t : threads)
	{
		t.join();
	}

	twitchConnection.close();

	return 0;
}
