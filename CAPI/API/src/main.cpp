#include <tclap/CmdLine.h>
#include "../API/include/AI.h"
#include "../API/include/logic.h"
#include "../API/include/structures.h"

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

int thuai5_main(int argc, char** argv, CreateAIFunc AIBuilder)
{ 
    std::string sIP;
    uint16_t sPort;
    int pID;
    int tID;
    THUAI5::SoftwareType software;
    THUAI5::HardwareType hardware;
    int level = 0;
    std::string filename;

    try
    {
        TCLAP::CmdLine cmd("THUAI5 C++ interface commandline parameter introduction");

        TCLAP::ValueArg<std::string> serverIP("I", "serverIP", "Server`s IP 127.0.0.1 in default", false, "127.0.0.1", "string");
        cmd.add(serverIP);

        TCLAP::ValueArg<uint16_t> serverPort("P", "serverPort", "Port the server listens to 7777 in default", false, 7777, "USORT");
        cmd.add(serverPort);

        std::vector<int> validPlayerIDs{ 0, 1, 2, 3 };
        TCLAP::ValuesConstraint<int> playerIdConstraint(validPlayerIDs);
        TCLAP::ValueArg<int> playerID("p", "playerID", "Player ID 0,1,2,3 valid only", true, -1, &playerIdConstraint);
        cmd.add(playerID);

        std::vector<int> validTeamIDs{ 0, 1 };
        TCLAP::ValuesConstraint<int> temIdConstraint(validTeamIDs);
        TCLAP::ValueArg<int> teamID("t", "teamID", "Team ID, which can only be 0 or 1", true, -1, &temIdConstraint);
        cmd.add(teamID);

        std::string DebugDesc = "Set this flag to use API for debugging.\n"
            "If \"-f\" is not set, the log will be printed on the screen.\n"
            "Or you could specify a file to store it.";
        TCLAP::SwitchArg debug("d", "debug", DebugDesc);
        cmd.add(debug);

        TCLAP::ValueArg<std::string> FileName("f", "filename", "Specify a file to store the log.", false, "", "string");
        cmd.add(FileName);

        TCLAP::SwitchArg warning("w", "warning", "Warn of some obviously invalid operations (only when \"-d\" is set).");
        cmd.add(warning);

        cmd.parse(argc, argv);
        extern const THUAI5::SoftwareType playerSoftware; // Extern variable, actually defined in AI.cpp
        extern const THUAI5::HardwareType playerHardware;
        pID = playerID.getValue();
        tID = teamID.getValue();
        software = playerSoftware;
        hardware = playerHardware;
        sIP = serverIP.getValue();
        sPort = serverPort.getValue();

        bool d = debug.getValue();
        bool w = warning.getValue();
        if (d)
        {
            level =  w;
        }
        filename = FileName.getValue();
    }
    catch (TCLAP::ArgException& e) // catch exceptions
    {
        std::cerr << "Parsing error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
    Logic logic(tID, pID, software, hardware);
    extern const bool asynchronous;
    std::cout << "*******************basic info*******************" << std::endl;
    std::cout << "asynchronous: " << asynchronous << std::endl;
    std::cout << "server IP: " << sIP.c_str() << std::endl;
    std::cout << "server port: " << sPort << std::endl;
    std::cout << "team ID: " << tID << std::endl;
    std::cout << "port ID: " << pID << std::endl;
    std::cout << "software type: " << THUAI5::software_dict[software] << std::endl;
    std::cout << "passive skill type: " << THUAI5::software_dict[software] << std::endl;
    std::cout << "debug level: " << level << std::endl;
    std::cout << "file name: " << filename << std::endl;
    std::cout << "************************************************" << std::endl;
    logic.Main(sIP.c_str(), sPort, AIBuilder, level, filename);
    return 0;
}

std::unique_ptr<IAI> CreateAI()
{
    return std::make_unique<AI>();
}

int main(int argc, char* argv[])
{
    return thuai5_main(argc, argv, CreateAI);
}
