#include "Socket.h"
#include "PostOffice.h"
#include "Throttler.h"
#include "ScopeExit.h"
#include "utils.h"

#include "json/src/json.hpp"

#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <vector>
using namespace std;

int main(void)
{

	Socket twitchConnection;
	Address commandParser("commandParser");
	Address transceiver("transceiver");
	Address pointAdder("pointAdder");

	std::list<std::string> members = {};

	auto pointAdderLambda = [pointAdder, &members](void) -> void
	{
		Throttler pointIncrementer(std::chrono::seconds(60));

		nlohmann::json points = nlohmann::json::object();

		std::ifstream jsonIn("members.json");
		if (jsonIn.good())
		{
			jsonIn >> points;
		}
		jsonIn.close();

		while (!PostOffice::instance()->checkMail(pointAdder))
		{
			if (pointIncrementer.check(1))
			{
				{
					RAIIMutex memberLock(&members);
					for (const std::string& member : members)
					{
						int value = 0;
						if (!points[member].is_null())
						{
							value = points[member].get<int>();
						}
						points[member] = ++value;
					}
				}
				pointIncrementer.addUnit();

				std::ofstream jsonIn("members.json");
				if (jsonIn.good())
				{
					jsonIn << points;
				}
				jsonIn.close();

			}
			else
			{
				usleep(100000);
			}
		};
	};
	auto commandParserLambda = [commandParser, transceiver, &members](void) -> void
	{
		PostOffice* postOffice = PostOffice::instance();
		Address myAddress = commandParser;
		SCOPE_EXIT [&](){ cout << "Exiting " << myAddress << endl; };

		Throttler namesTimeout(std::chrono::seconds(60));
		bool previousMemberUpdateFinished = false;

		std::string command = "";
		std::string prefix = "";
		std::vector<std::string> params;
		while (command != "SHUTDOWN")
		{
			if (namesTimeout.check(1))
			{
				//sendMessage(transceiver, "NAMES #betawar1305\r\n");
				sendMessage(transceiver, "PART #betawar1305\r\n");
				sendMessage(transceiver, "JOIN #betawar1305\r\n");
				namesTimeout.addUnit();
			}

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

			command = std::string(incomingMsg.raw(), incomingMsg.size() - 1);
			prefix = "";
			params.clear();

			std::vector<std::string> parts = split(command, " ");
			if (parts.empty() || parts.at(0).empty()) // we have an empty prefix/ command...
			{
				continue;
			}

			if(parts.at(0).at(0) == ':')
			{
				// We have a prefix, meaning that the second item (if it exists) is the command.
				prefix = parts.at(0);
				parts.erase(parts.begin());
			}

			if (parts.empty())
			{
				continue;
			}

			command = parts.at(0);
			parts.erase(parts.begin());

			while (!parts.empty())
			{
				if (parts.at(0).size() > 0 && parts.at(0).at(0) == ':')
				{
					parts.at(0) = parts.at(0).substr(1);
					params.push_back(join(parts, " "));
					parts.clear();
				}
				else
				{
					params.push_back(parts.at(0));
					parts.erase(parts.begin());
				}
			}

			cout << "Prefix='" << prefix << "' Command='" << command << "' Params=['" << join(params, "', '") << "']" << endl;
			if (command == "PING")
			{
				sendMessage(transceiver, "PONG " + join(params, " ") + "\r\n");
			}
			else if (command == "PRIVMSG")
			{
				if (params.back() == "!points")
				{
					nlohmann::json points = nlohmann::json::object();
					std::ifstream jsonIn("members.json");
					if (jsonIn.good())
					{
						jsonIn >> points;
					}
					jsonIn.close();

					std::string channel = params.front();
					std::string user = split(prefix.substr(1), "!").at(0);

					cout << "Retrieving points for '" << user << "' on " << channel << endl;
					if (points[user].is_null())
					{
						sendMessage(transceiver, "PRIVMSG " + channel + " :@" + user + ", your currently have no points.\r\n");
					}
					else
					{
						sendMessage(transceiver, "PRIVMSG " + channel + " :@" + user + ", your point total is " + points[user].dump() + ".\r\n");
					}
				}
			}
			else if (command == "376") // the end of the MOTD message.
			{
				sendMessage(transceiver, "JOIN #betawar1305\r\n");
			}
			else if (command == "353") // members list
			{
				RAIIMutex memberLock(&members);
				if (previousMemberUpdateFinished)
				{
					members.clear();
					previousMemberUpdateFinished = false;
				}
				std::vector<std::string> names = split(params.back(), " ");
				members.insert(members.begin(), names.begin(), names.end());
			}
			else if (command == "366") // member list finished
			{
				RAIIMutex memberLock(&members);
				previousMemberUpdateFinished = true;
				for (const std::string& member : members)
				{
					cout << "=== Member: " << member << endl;
				}

			}
		}
	};

	auto transceiverLambda = [&twitchConnection, commandParser, transceiver, pointAdder](void) -> void
	{
		const int bufferLength = 1024;
		char* buffer = new char[bufferLength];
		Throttler throttle;
		PostOffice* postOffice = PostOffice::instance();
		Address myAddress = transceiver;

		SCOPE_EXIT [&](void) -> void
		{
			delete [] buffer;
			sendMessage(commandParser, "SHUTDOWN");
			sendMessage(pointAdder, "SHUTDOWN");
			cout << "Exiting " << myAddress << endl;
		};

		std::string incompleteMessage = "";
		while (twitchConnection.connected())
		{

			if (PostOffice::isValidInstance(postOffice) && postOffice->checkMail(myAddress) && throttle.check(20))
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
				throttle.addUnit();
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
					sendMessage(commandParser, incompleteMessage);
					incompleteMessage = "";
				}
			} while (!s.eof());
		}

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

	sendMessage(transceiver, "PASS " + oauthToken + "\r\n");
	sendMessage(transceiver, "NICK betawar1305\r\n");
	//sendMessage(transceiver, "CAP REQ :twitch.tv/membership\r\n");

	std::list<std::thread> threads;
	threads.push_back(std::thread(transceiverLambda));
	threads.push_back(std::thread(commandParserLambda));
	threads.push_back(std::thread(pointAdderLambda));

	for (std::thread& t : threads)
	{
		t.join();
	}

	twitchConnection.close();
	PostOffice::finalize();

	return 0;
}
