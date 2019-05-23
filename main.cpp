#include "server.cpp"
#include <signal.h>

Server *server;

void sigHandler(int s)
{
	if(s==2)
	{
		if(server->isLog)
			server->writeToLog("Server stopped");
		delete server;
		exit(0);
	}
}

int main()
{
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler=sigHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags=0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	server=new Server();
	while((!server->start())&&(server->status==State::bindError))
		sleep(5);
	while(server->status==State::runned)
		server->run();
	return 0;
}
