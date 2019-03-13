#include <string>

const uint8_t stopped=0;
const uint8_t bindError=1;
const uint8_t listenError=2;
const uint8_t runned=3;

class Server
{
	public:
		Server();
		bool start();
		void run();
		bool stop();
		void writeToLog(const char* line);
		bool isLog=false;
		uint8_t status=stopped;
		~Server();
	private:
		int iSocket;
		std::string serverRoot, indexFile, logFile;
		std::string prepareAnswer(std::string *fileName);
};
