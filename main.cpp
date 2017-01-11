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
#include <set>
using namespace std;

int main(void)
{

	Socket twitchConnection;
	Address commandParser("commandParser");
	Address transceiver("transceiver");
	Address pointAdder("pointAdder");


	auto pointAdderLambda = [pointAdder, commandParser](void) -> void
	{
		Throttler pointIncrementer(std::chrono::seconds(20));
		PostOffice* postOffice = PostOffice::instance();
		Address myAddress = pointAdder;

		std::set<std::string> members = {};
		nlohmann::json points = nlohmann::json::object();

		std::ifstream jsonIn("members.json");
		if (jsonIn.good())
		{
			jsonIn >> points;
		}
		jsonIn.close();

		std::string command = "";
		while (command != "SHUTDOWN")
		{
			if (pointIncrementer.check(1))
			{
				{
					RAIIMutex memberLock(&members);
					for (const std::string& member : members)
					{
						int value = 0;
						if (points[member].is_null())
						{
							points[member] = nlohmann::json::object();
						}
						if (!points[member]["points"].is_null())
						{
							value = points[member]["points"].get<int>();
						}
						points[member]["points"] = ++value;
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
			std::vector<std::string> parts = split(command, " ");
			if (parts.empty())
			{
				usleep(300);
				continue;
			}
			command = parts.at(0);
			parts.erase(parts.begin());

			if (command == "JOIN" && !parts.empty())
			{
				members.insert(parts.at(0));
				if (points[parts.at(0)].is_null())
				{
					points[parts.at(0)] = nlohmann::json::object();
				}
				points[parts.at(0)]["role"] = "member";
			}
			else if (command == "PART" && !parts.empty())
			{
				members.erase(members.find(parts.at(0)));
			}
			else if (command == "MODE" && parts.size() >= 2)
			{
				if (points[parts.at(1)].is_null())
				{
					points[parts.at(1)] = nlohmann::json::object();
				}
				points[parts.at(1)]["role"] = parts.at(0);
			}
			else if (command == "GETROLE" && !parts.empty())
			{
				if (points[parts.at(0)].is_null())
				{
					points[parts.at(0)] = nlohmann::json::object();
				}

				std::string role = "member";
				if (points[parts.at(0)]["role"].is_string())
				{
					role = points[parts.at(0)]["role"].get<std::string>();
				}
				parts.insert(parts.begin(), role);
				sendMessage(commandParser, "RETURNROLE " + join(parts, " "));
			}
		};
	};
	auto commandParserLambda = [commandParser, pointAdder, transceiver](void) -> void
	{
		PostOffice* postOffice = PostOffice::instance();
		Address myAddress = commandParser;
		SCOPE_EXIT [&](){ cout << "Exiting " << myAddress << endl; };

		Throttler namesTimeout(std::chrono::seconds(60));

		std::string command = "";
		std::string prefix = "";
		std::vector<std::string> params;


		std::list<std::string> membersInRaffle;
		bool raffleOpen = false;

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

			std::string role = "";
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

			/*
			 * FIXME Need to come up with a better asynchronous messaging protocol
			 *       (though it will probably still run over the PostOffice).
			 */
			if (command == "RETURNROLE")
			{
				role = params.front();
				params.erase(params.begin());

				std::string user = params.front();
				params.erase(params.begin());
				
				prefix = ":" + user + "!" + user + "@" + user + ".tmi.twitch.tv";

				command = params.front();
				params.erase(params.begin());
			}

			cout << "Prefix='" << prefix << "' Command='" << command << "' Params=['" << join(params, "', '") << "']" << endl;
			if (command == "PING")
			{
				sendMessage(transceiver, "PONG " + join(params, " ") + "\r\n");
			}
			else if (command == "PRIVMSG")
			{
				std::string channel = params.front();
				if (params.back() == "!points")
				{
					nlohmann::json points = nlohmann::json::object();
					std::ifstream jsonIn("members.json");
					if (jsonIn.good())
					{
						jsonIn >> points;
					}
					jsonIn.close();

					std::string user = split(prefix.substr(1), "!").at(0);

					cout << "Retrieving points for '" << user << "' on " << channel << endl;
					if (points[user]["points"].is_null())
					{
						sendMessage(transceiver, "PRIVMSG " + channel + " :@" + user + ", your currently have no points.\r\n");
					}
					else
					{
						sendMessage(transceiver, "PRIVMSG " + channel + " :@" + user + ", your point total is " + points[user]["points"].dump() + ".\r\n");
					}
				}
				else if (params.back() == "!raffleStart")
				{
					std::string user = split(prefix.substr(1), "!").at(0);
					if (role == "+o")
					{
						raffleOpen = true;
						sendMessage(transceiver, "PRIVMSG " + channel + " :The raffle is now open! Send '!joinRaffle' (no quotes) to join.\r\n");
					}
					else if (role != "")
					{
						sendMessage(transceiver, "PRIVMSG " + channel + " :@" + user + ", you don't have permissions to execute that command.\r\n");
					}
					else
					{
						sendMessage(pointAdder, "GETROLE " + user + " PRIVMSG " + join(params, " "));
					}
				}
				else if (params.back() == "!raffleJoin")
				{
					/*
					 * NOTE This isn't done per-say. It has a drastic flaw in that it doesn't
					 *      actually cost points to join the raffle. We will want to change
					 *      that. But given that I don't really like the way the asynchronous
					 *      messaging is working out with the GETROLE side of things, I am
					 *      going to take some time to architect this and come back with a
					 *      better option.
					 */
					std::string user = split(prefix.substr(1), "!").at(0);
					membersInRaffle.push_back(user);
					sendMessage(transceiver, "PRIVMSG " + channel + " :@" + user + ", you are now in the raffle.\r\n");
				}

			}
			else if (command == "JOIN" || command == "PART")
			{
				std::string user = split(prefix.substr(1), "!").at(0);
				sendMessage(pointAdder, command + " " + user);
			}
			else if (command == "MODE" && params.size() >= 3)
			{
				sendMessage(pointAdder, "MODE " + params.at(1) + " " + params.at(2));
			}
			else if (command == "376") // the end of the MOTD message.
			{
				sendMessage(transceiver, "CAP REQ twitch.tv/membership\r\n");
				sendMessage(transceiver, "JOIN #betawar1305\r\n");
			}
			else if (command == "353") // members list
			{
				std::vector<std::string> names = split(params.back(), " ");
				for (const std::string& name : names)
				{
					sendMessage(pointAdder, "JOIN " + name);
				}
			}
			else if (command == "366") // member list finished
			{
				// empty
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
