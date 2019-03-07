#include "server.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
//#include <string.h>
#include <sstream>
#include <fstream>

#include <regex>

#define port 80

std::string returnVal(std::string line)
{
	return line.substr(line.rfind("=")+1, std::string::npos);
}

Server::Server()
{
	iSocket=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	std::fstream settingsFile;
	settingsFile.open("settings");
	if(settingsFile.is_open())
	{
		std::string line;
		while(getline(settingsFile, line))
			if(line[0]!='#')
			{
				if(line.find("server root")!=std::string::npos)
					this->serverRoot=returnVal(line);
				if(line.find("index file")!=std::string::npos)
					this->indexFile=returnVal(line);
			}
		settingsFile.close();
	}
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
		std::cout<<"bind error"<<std::endl;
		return false;
	}
	if(listen(iSocket, 5)==-1)
	{
		std::cout<<"listen error"<<std::endl;
		return false;
	}
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
		std::cout<<buf<<std::endl;
		request.append(buf);
		std::string fileName=request.substr(request.find("/"), request.find("HTTP")-request.find("/")-1);
		if((fileName=="/")&&(this->indexFile!=""))
			fileName+=this->indexFile;
		if(this->serverRoot!="")
			fileName=serverRoot+fileName;
		std::stringstream response;
		//response<<"HTTP/1.1 200 OK\r\n\r\n";
		response<<this->prepareAnswer(&fileName);
		write(con, response.str().c_str(), response.str().length());
		close(con);
	}
}

std::string Server::prepareAnswer(std::string *fileName)
{
	std::string response;
	std::fstream file;
	file.open(*fileName);
	if(file.is_open())
	{
		response="HTTP/1.1 200 OK\r\n\r\n";
		std::string line;
		while(getline(file, line))
			response.append(line+"\n");
	}
	else
		response="HTTP/1.1 404 Not Found\r\n\r\n";
	return response;
}

bool Server::stop()
{
	close(iSocket);
}

Server::~Server()
{
	stop();
}
