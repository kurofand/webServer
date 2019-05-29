#include <string>
#include <vector>

enum class State: uint8_t
{
	stopped=0,
	bindError,
	listenError,
	runned
};

class Server
{
	public:
		Server();
		bool start();
		void run();
		bool stop();
		void writeToLog(const char* line);
		bool isLog=false;
		State status=State::stopped;
		~Server();
	private:
		int iSocket;
		std::string serverRoot, indexFile, logFile;
		std::string prepareAnswer(std::string *fileName, uint16_t statusCode);
		std::vector<std::string> blackList;
};
