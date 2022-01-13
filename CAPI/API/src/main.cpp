#include <tclap/CmdLine.h>
#include"../API/include/AI.h"
#include"../API/include/logic.h"

Logic& logic;

int thuai5_main(int argc, char** argv, CreateAIFunc AIBuilder)
{
	int pID;
	int tID;
	THUAI5::ActiveSkillType aSkill;
	THUAI5::PassiveSkillType pSkill;
	int level = 0;
	std::string filename;

	try
	{
		TCLAP::CmdLine cmd("THUAI5 C++ interface commandline parameter introduction");

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
		extern const THUAI5::ActiveSkillType activeSkill; // Entern variable, actually defined in AI.cpp
		extern const THUAI5::PassiveSkillType passiveSkill;
		pID = playerID.getValue();
		tID = teamID.getValue();
		aSkill = activeSkill;
		pSkill = passiveSkill;

		bool d = debug.getValue();
		bool w = warning.getValue();
		if (d)
		{
			level = 1 + w;
		}
		filename = FileName.getValue();
	}
	catch (TCLAP::ArgException& e) // catch exceptions
	{
		std::cerr << "Parsing error: " << e.error() << " for arg " << e.argId() << std::endl;
		return 1;
	}
	logic.Main(pID, tID, aSkill, pSkill, AIBuilder, level, filename);
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
