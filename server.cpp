#include "server.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <cstring>

#include <ctime>

#define port 80

struct readingMultiple
{
	std::string name;
	bool reading=false;
};

std::string returnVal(std::string line)
{
	return line.substr(line.rfind("=")+1, std::string::npos);
}

void turnFalse(bool a)
{
	if(a)
		a=!a;
}

Server::Server()
{
	iSocket=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	std::fstream settingsFile;
	settingsFile.open("settings");
	if(settingsFile.is_open())
	{
		std::string line;
		readingMultiple rMultiple;
		while(getline(settingsFile, line))
			if(line[0]!='#')
			{
				if(line.find("server root")!=std::string::npos)
				{
					this->serverRoot=returnVal(line);
					turnFalse(rMultiple.reading);
				}
				if(line.find("index file")!=std::string::npos)
				{
					this->indexFile=returnVal(line);
					turnFalse(rMultiple.reading);
				}
				if(line.find("logging")!=std::string::npos)
				{
					this->isLog=returnVal(line)=="1";
					turnFalse(rMultiple.reading);
				}
				if(line.find("log directory")!=std::string::npos)
				{
					this->logFile=returnVal(line)+"/log";
					turnFalse(rMultiple.reading);
				}
				if(line.find("black list")!=std::string::npos)
				{
					rMultiple.name="blackList";
					rMultiple.reading=true;
				}
				if(rMultiple.name=="blackList"&&rMultiple.reading)
					this->blackList.push_back(line);
			}
		settingsFile.close();
	}
	else
		std::cout<<"Error with reading settings file"<<std::endl;
}

bool Server::start()
{
	struct sockaddr_in stSockAddr;
	memset(&stSockAddr, 0, sizeof(stSockAddr));
	stSockAddr.sin_family=AF_INET;
	stSockAddr.sin_port=htons(port);
	stSockAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	if(bind(iSocket, (struct sockaddr*) &stSockAddr, sizeof(stSockAddr))==-1)
	{
		if(this->isLog)
			this->writeToLog("Bind error. Retrying to start in 5 seconds...");
		else
			std::cout<<"bind error"<<std::endl;
		this->status=State::bindError;
		return false;
	}
	if(listen(iSocket, 5)==-1)
	{
		if(this->isLog)
			this->writeToLog("Listen error");
		else
			std::cout<<"listen error"<<std::endl;
		this->status=State::listenError;
		return false;
	}
	if(this->isLog)
		writeToLog("Successfully started");
	else
		std::cout<<"Successfully started"<<std::endl;
	this->status=State::runned;
	return true;
}

void Server::run()
{
	struct sockaddr_in cliAddr;
	socklen_t sLen=sizeof(cliAddr);
	int con=accept(iSocket, (struct sockaddr*) &cliAddr, &sLen);
	if(con>=0)
	{
		const uint16_t bufSize=1024;
		char buf[bufSize];
		std::string request;
		ssize_t result=recv(con, &buf, bufSize, NULL);
		request.append(buf);
		std::string fileName;
		std::string log="Received \"";
		std::stringstream response;
		if(request.find("HTTP")!=std::string::npos)
		{
			fileName=request.substr(request.find("/"), request.find("HTTP")-request.find("/")-1);
			if((fileName=="/")&&(this->indexFile!=""))
				fileName+=this->indexFile;
			if(this->serverRoot!="")
				fileName=serverRoot+fileName;
			log+=request.substr(0, request.find(" HTTP"));
			bool isBanned=false;
			for(uint32_t i=0;i<blackList.size();i++)
			{
				std::string subAddr=(blackList.at(i).find(".*")!=std::string::npos)?blackList.at(i).erase(blackList.at(i).find(".*"), std::string::npos):blackList.at(i);
				std::string currAddr=inet_ntoa(cliAddr.sin_addr);
				if(currAddr.find(subAddr)!=std::string::npos)
				{
					response<<this->prepareAnswer(nullptr, 403);
					isBanned=true;
					break;
				}
			}
			if(!isBanned)
				response<<this->prepareAnswer(&fileName, 200);
		}
		else
		{
			log+=request;
			response<<this->prepareAnswer(nullptr, 400);
		}
		log+="\" request from ";
		log+=inet_ntoa(cliAddr.sin_addr);
		this->writeToLog(log.c_str());
		write(con, response.str().c_str(), response.str().length());
		close(con);
	}
}

std::string Server::prepareAnswer(std::string *fileName, uint16_t statusCode)
{
	std::string response;
	switch(statusCode)
	{
		case 200:
		{
			std::fstream file;
			file.open(*fileName);
			if(file.is_open())
			{
				response="HTTP/1.1 200 OK\r\n\r\n";
				std::string line;
				while(getline(file, line))
					response.append(line+"\n");
				file.close();
			}
			else
				response="HTTP/1.1 404 Not Found\r\n\r\n";
			break;
		}
		case 400:
		{
			response="HTTP/1.1 400 Bad Request\r\n\r\n";
			break;
		}
		case 403:
		{
			response="HTTP/1.1 403 Forbidden\r\n\r\n";
			break;
		}
	}
	return response;
}

void Server::writeToLog(const char* line)
{
	std::ofstream file;
	file.open(this->logFile, std::ofstream::out|std::ofstream::app);
	if(file.is_open())
	{
		std::time_t currTime=std::time(nullptr);
		char bstr[25];
		std::strftime(bstr, 25, "%a %b %d %H:%M:%S %Y", std::localtime(&currTime));
		file<<bstr<<": "<<line<<std::endl;
		file.close();
	}
}

bool Server::stop()
{
	close(iSocket);
}

Server::~Server()
{
	stop();
}
