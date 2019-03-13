#include <string>

const uint8_t STOPPED=0;
const uint8_t BIND_ERROR=1;
const uint8_t LISTEN_ERROR=2;
const uint8_t RUNNED=3;

class Server
{
	public:
		Server();
		bool start();
		void run();
		bool stop();
		void writeToLog(const char* line);
		bool isLog=false;
		uint8_t status=STOPPED;
		~Server();
	private:
		int iSocket;
		std::string serverRoot, indexFile, logFile;
		std::string prepareAnswer(std::string *fileName, bool valid);
};
