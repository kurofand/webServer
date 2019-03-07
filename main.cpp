#include "server.cpp"

int main()
{
	Server *server=new Server();
	server->start();
	server->run();
	delete server;
	return 0;
}
