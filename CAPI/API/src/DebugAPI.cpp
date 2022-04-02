#include "../include/API.h"
#include "../include/utils.hpp"
#include "../include/structures.h"

const static double PI = 3.14159265358979323846;

void DebugAPI::StartTimer()
{
    StartPoint = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(StartPoint);
    Out << "=== AI.play() ===" << std::endl;
    Out << "Current time: " << ctime(&t);
}

void DebugAPI::EndTimer()
{
    Out << "Time elapsed: " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    Out << std::endl;
}

bool DebugAPI::MovePlayer(uint32_t timeInMilliseconds, double angleInRadian)
{
    Out << "Call MovePlayer(" << timeInMilliseconds << "," << angleInRadian << ") at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Move);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);

    return logic.SendInfo(message); 
}

bool DebugAPI::MoveUp(uint32_t timeInMilliseconds)
{
    return MovePlayer(timeInMilliseconds, PI);
}

bool DebugAPI::MoveDown(uint32_t timeInMilliseconds)
{
    return MovePlayer(timeInMilliseconds, 0);
}

bool DebugAPI::MoveLeft(uint32_t timeInMilliseconds)
{
    return MovePlayer(timeInMilliseconds, PI * 1.5);
}

bool DebugAPI::MoveRight(uint32_t timeInMilliseconds)
{
    return MovePlayer(timeInMilliseconds, PI * 0.5);
}

bool DebugAPI::Attack(double angleInRadian)
{
    Out << "Call Attack(" << angleInRadian << ") at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    if (ExamineValidity)
    {
        
        auto selfInfo = logic.GetSelfInfo();
        if (selfInfo->isResetting)
        {
            Out << "[Warning: You have been slained.]" << std::endl;
            return false;
        }
        if (selfInfo->signalJammerNum == 0)
        {
            Out << "[Warning: You are out of signal jammers.]" << std::endl;
            return false;
        }
    }

    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Attack);
    message.set_angle(angleInRadian);
    return logic.SendInfo(message);
}

bool DebugAPI::UseCommonSkill()
{
    Out << "Call UseCommonSkill() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    if (ExamineValidity)
    {
        
        auto selfInfo = logic.GetSelfInfo();
        if (selfInfo->isResetting)
        {
            Out << "[Warning: You have been slained.]" << std::endl;
            return false;
        }
        if(!CanUseSoftware(selfInfo))
        {
            return false;
        }
        else
        {
            Out << "[Info: Using " << THUAI5::software_dict[selfInfo->softwareType] << ".]" << std::endl;
        }
    }

    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseCommonSkill);
    return logic.SendInfo(message);
}

bool DebugAPI::Send(int toPlayerID, std::string to_message)
{
    Out << "Call Send(" << toPlayerID << "," << to_message << ") at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    if (ExamineValidity)
    {
        if (toPlayerID < 0 || toPlayerID >= 4)
        {
            Out << "[Error: Illegal player ID.]" << std::endl;
            return false;
        }
        else
        {
            Out << "[Info: The message is " << to_message << ".]" << std::endl;
        }
    }

    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Send);
    message.set_toplayerid(toPlayerID);
    message.set_message(to_message);
    return logic.SendInfo(message);
}

bool DebugAPI::Pick(THUAI5::PropType proptype)
{
    Out << "Call Pick(" << THUAI5::prop_dict[proptype] << ") at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    if (ExamineValidity)
    {
        
        auto selfInfo = logic.GetSelfInfo();
        if (selfInfo->isResetting)
        {
            Out << "[Warning: You have been slained.]" << std::endl;
            return false;
        }
        if (!CanPick(proptype, selfInfo))
        {
            Out << "[Warning: No such property to pick within the cell.]" << std::endl;
            return false;
        }
    }

    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Pick);
    message.set_proptype(Protobuf::PropType(proptype));
    return logic.SendInfo(message);
}

bool DebugAPI::ThrowProp(uint32_t timeInMilliseconds, double angleInRadian)
{
    Out << "Call ThrowProp(" << timeInMilliseconds << "," << angleInRadian << ") at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    if (ExamineValidity)
    {
        
        auto selfInfo = logic.GetSelfInfo();
        if (selfInfo->isResetting) // 正在复活中
        {
            Out << "[Warning: You have been slained.]" << std::endl;
            return false;
        }
        if (selfInfo->prop == THUAI5::PropType::NullPropType)
        {
            Out << "[Warning: You don't have any props.]" << std::endl;
            return false;
        }
        else
        {
            Out << "[Info: Throw prop: " << THUAI5::prop_dict[selfInfo->prop] << ".]" << std::endl;
        }
    }

    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::ThrowProp);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);
    return logic.SendInfo(message);
}

bool DebugAPI::UseProp()
{
    Out << "Call UseProp() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    if (ExamineValidity)
    {
        
        auto selfInfo = logic.GetSelfInfo();
        if (selfInfo->isResetting) // 正在复活中
        {
            Out << "[Warning: You have been slained.]" << std::endl;
            return false;
        }
        if (selfInfo->prop == THUAI5::PropType::NullPropType)
        {
            Out << "[Warning: You don't have any props.]" << std::endl;
            return false;
        }
        else
        {
            Out << "[Info: Use prop: " << THUAI5::prop_dict[selfInfo->prop] << ".]" << std::endl;
        }
    }

    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseProp);
    return logic.SendInfo(message);
}

bool DebugAPI::ThrowCPU(uint32_t timeInMilliseconds, double angleInRadian, uint32_t cpuNum)
{
    Out << "Call ThrowCPU(" << timeInMilliseconds << angleInRadian << cpuNum << ") at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    if (ExamineValidity)
    {
        
        auto selfInfo = logic.GetSelfInfo();
        if (selfInfo->isResetting) // 正在复活中
        {
            Out << "[Warning: You have been slained.]" << std::endl;
            return false;
        }
        if (selfInfo->cpuNum == 0)
        {
            Out << "[Warning: You don't have any CPUs.]" << std::endl;
            return false;
        }
    }

    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::ThrowGem);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);
    message.set_gemsize(cpuNum);
    return logic.SendInfo(message);
}

bool DebugAPI::UseCPU(uint32_t cpuNum)
{
    Out << "Call UseCPU(" << cpuNum << ") at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    if (ExamineValidity)
    {
        
        auto selfInfo = logic.GetSelfInfo();
        if (selfInfo->isResetting) // 正在复活中
        {
            Out << "[Warning: You have been slained.]" << std::endl;
            return false;
        }
        if (selfInfo->cpuNum == 0)
        {
            Out << "[Warning: You don't have any CPUs.]" << std::endl;
            return false;
        }
    }

    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseGem);
    message.set_gemsize(cpuNum);
    return logic.SendInfo(message);
}

bool DebugAPI::Wait()
{
    Out << "Call Wait() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    if (logic.GetCounter() == -1)
    {
        return false;
    }
    logic.WaitThread();
    return true;
}

bool DebugAPI::MessageAvailable()
{
    Out << "Call MessageAvailable() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    return !logic.Empty();
}

std::optional<std::string> DebugAPI::TryGetMessage()
{
    Out << "Call TryGetMessage() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    auto info = logic.GetInfo();
    if (ExamineValidity)
    {
        if (!info)
        {
            Out << "[Warning: Failed to get a message.]" << std::endl;
        }
        else
        {
            Out << "[Info: The message is: " << info.value() << ".]" << std::endl;
        }
    }
    return info;
}

std::vector<std::shared_ptr<const THUAI5::Robot>> DebugAPI::GetRobots() const
{
    Out << "Call GetRobots() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    return logic.GetRobots();
}

std::vector<std::shared_ptr<const THUAI5::SignalJammer>> DebugAPI::GetSignalJammers() const
{
    Out << "Call GetSignalJammers() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    return logic.GetSignalJammers();
}

std::vector<std::shared_ptr<const THUAI5::Prop>> DebugAPI::GetProps() const
{
    Out << "Call GetProps() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    return logic.GetProps();
}

std::shared_ptr<const THUAI5::Robot> DebugAPI::GetSelfInfo() const
{
    Out << "Call GetSelfInfo() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    return logic.GetSelfInfo();
}

THUAI5::PlaceType DebugAPI::GetPlaceType(int32_t CellX, int32_t CellY) const
{
    Out << "Call GetPlaceType(" << CellX << "," << CellY << ") at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    return logic.GetPlaceType(CellX, CellY);
}

uint32_t DebugAPI::GetTeamScore() const
{
    Out << "Call GetTeamScore() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    return logic.GetTeamScore();
}

const std::vector<int64_t> DebugAPI::GetPlayerGUIDs() const
{
    Out << "Call GetTeamGUIDs() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    return logic.GetPlayerGUIDs();
}

int DebugAPI::GetFrameCount() const
{
    Out << "Call GetCounterOfFrames() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    return logic.GetCounter();
}

bool DebugAPI::CanPick(THUAI5::PropType propType, std::shared_ptr<const THUAI5::Robot>& selfInfo)
{
    Out << "(Auto revoking)";
    std::vector<std::shared_ptr<const THUAI5::Prop>> props = GetProps();
    for (auto it : props)
	{
		if (Space::InSameCell(selfInfo->x, selfInfo->y, it->x, it->y) && propType == it->type)
		{
			return true;
		}
	}
	return false;
}

bool DebugAPI::CanUseSoftware(std::shared_ptr<const THUAI5::Robot>& selfInfo)
{
    double timeUntilCommonSkillAvailable = selfInfo->timeUntilCommonSkillAvailable;
    if(timeUntilCommonSkillAvailable==0.0)
    {
        return true;
    }
    Out << "[Warning: common skill is not available, please wait for " << timeUntilCommonSkillAvailable << " s.]" << std::endl;  
    return false;
}

void DebugAPI::PrintSignalJammers() const
{
    Out << "Call PrintSignalJammers() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    Out << "******************Bullets******************" << std::endl;
    auto jammers = logic.GetSignalJammers();
    for (int i = 0;i< jammers.size();i++)
    {
        Out << "SignalJammer " << i << ":" << std::endl;
        Out << "facingDirection: " << jammers[i]->facingDirection << std::endl
                  << "guid: " << jammers[i]->guid << std::endl
                  << "parentTeamID: " << jammers[i]->parentTeamID << std::endl
                  << "place: " << THUAI5::place_dict[jammers[i]->place] << std::endl
                  << "type: " << THUAI5::jammer_dict[jammers[i]->type] << std::endl
                  << "x: " << jammers[i]->x << std::endl
                  << "y: " << jammers[i]->y << std::endl;
    }
    Out << "*******************************************" << std::endl;
}

void DebugAPI::PrintRobots() const
{
    Out << "Call PrintRobots() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    Out << "******************Characters******************" << std::endl;
    auto robots = logic.GetRobots();
    for(int i = 0;i<robots.size();i++)
    {
        Out << "Robot " << i << ":" << std::endl;
        Out << "softwareType: " << THUAI5::software_dict[robots[i]->softwareType] << std::endl
                  << "attackRange: " << robots[i]->attackRange << std::endl
                  << "signalJammerNum: " << robots[i]->signalJammerNum << std::endl
                  << "signalJammerType: " << THUAI5::jammer_dict[robots[i]->signalJammerType] << std::endl
                  << "canMove: " << robots[i]->canMove << std::endl
                  << "CD: " << robots[i]->CD << std::endl
                  << "cpuNum: " << robots[i]->cpuNum << std::endl
                  << "guid: " << robots[i]->guid << std::endl
                  << "isResetting: " << robots[i]->isResetting << std::endl
                  << "life: " << robots[i]->life << std::endl
                  << "lifeNum: " << robots[i]->lifeNum << std::endl
                  << "hardwareType: " << THUAI5::hardware_dict[robots[i]->hardwareType] << std::endl
                  << "place: " << THUAI5::place_dict[robots[i]->place] << std::endl
                  << "playerID: " << robots[i]->playerID << std::endl
                  << "prop: " << THUAI5::prop_dict[robots[i]->prop] << std::endl
                  << "radius: " << robots[i]->radius << std::endl
                  << "score: " << robots[i]->score << std::endl
                  << "speed: " << robots[i]->speed << std::endl
                  << "teamID: " << robots[i]->teamID << std::endl
                  << "timeUntilCommonSkillAvailable: " << robots[i]->timeUntilCommonSkillAvailable << std::endl
                  << "timeUntilUltimateSkillAvailable: " << robots[i]->timeUntilUltimateSkillAvailable << std::endl
                  << "emissionAccessory: " << robots[i]->emissionAccessory << std::endl
                  << "x: " << robots[i]->x << std::endl
                  << "y: " << robots[i]->y << std::endl;
        if (robots[i]->buff.size() != 0)
        {
            std::cout << "buff: ";
            for (int j = 0; j < robots[i]->buff.size(); j++)
            {
                std::cout << THUAI5::buff_dict[robots[i]->buff[j]] << ' ';
            }
            std::cout << std::endl;
        }
    }
    Out << "**********************************************" << std::endl;
}

void DebugAPI::PrintProps() const
{
    Out << "Call PrintProps() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    Out << "******************Props******************" << std::endl;
    auto props = logic.GetProps();
    for(int i = 0;i<props.size();i++)
    {
        Out << "Prop " << i << ":" << std::endl;
        Out << "facingDirection: " << props[i]->facingDirection << std::endl
                  << "guid: " << props[i]->guid << std::endl
                  << "place: " << THUAI5::place_dict[props[i]->place] << std::endl
                  << "size: " << props[i]->size << std::endl
                  << "type: " << THUAI5::prop_dict[props[i]->type] << std::endl
                  << "x: " << props[i]->x << std::endl
                  << "y: " << props[i]->y << std::endl;
    }
    Out << "*****************************************" << std::endl;
}

void DebugAPI::PrintSelfInfo() const
{
    Out << "Call PrintSelfInfo() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    Out << "******************Selfinfo******************" << std::endl;
    auto selfinfo = logic.GetSelfInfo();
    if(selfinfo!=nullptr)
    {
        Out << "softwareType: " << THUAI5::software_dict[selfinfo->softwareType] << std::endl
                  << "attackRange: " << selfinfo->attackRange << std::endl
                  << "signalJammerNum: " << selfinfo->signalJammerNum << std::endl
                  << "signalJammerType: " << THUAI5::jammer_dict[selfinfo->signalJammerType] << std::endl
                  << "canMove: " << selfinfo->canMove << std::endl
                  << "CD: " << selfinfo->CD << std::endl
                  << "cpuNum: " << selfinfo->cpuNum << std::endl
                  << "guid: " << selfinfo->guid << std::endl
                  << "isResetting: " << selfinfo->isResetting << std::endl
                  << "life: " << selfinfo->life << std::endl
                  << "lifeNum: " << selfinfo->lifeNum << std::endl
                  << "hardwareType: " << THUAI5::hardware_dict[selfinfo->hardwareType] << std::endl
                  << "place: " << THUAI5::place_dict[selfinfo->place] << std::endl
                  << "playerID: " << selfinfo->playerID << std::endl
                  << "prop: " << THUAI5::prop_dict[selfinfo->prop] << std::endl
                  << "radius: " << selfinfo->radius << std::endl
                  << "score: " << selfinfo->score << std::endl
                  << "speed: " << selfinfo->speed << std::endl
                  << "teamID: " << selfinfo->teamID << std::endl
                  << "timeUntilCommonSkillAvailable: " << selfinfo->timeUntilCommonSkillAvailable << std::endl
                  << "timeUntilUltimateSkillAvailable: " << selfinfo->timeUntilUltimateSkillAvailable << std::endl
                  << "emissionAccessory: " << selfinfo->emissionAccessory << std::endl
                  << "x: " << selfinfo->x << std::endl
                  << "y: " << selfinfo->y << std::endl;
        if (selfinfo->buff.size() != 0)
        {
            std::cout << "buff: ";
            for (int j = 0; j < selfinfo->buff.size(); j++)
            {
                std::cout << THUAI5::buff_dict[selfinfo->buff[j]] << ' ';
            }
            std::cout << std::endl;
        }
    }
    Out << "********************************************" << std::endl;
}
