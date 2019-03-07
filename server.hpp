#include <string>

class Server
{
	public:
		Server();
		bool start();
		void run();
		bool stop();
		~Server();
	private:
		int iSocket;
		std::string serverRoot, indexFile;
		std::string prepareAnswer(std::string *fileName);
};
