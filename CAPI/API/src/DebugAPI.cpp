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
        if (selfInfo->bulletNum == 0)
        {
            Out << "[Warning: You are out of bullets.]" << std::endl;
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
        if(!CanUseActiveSkill(selfInfo))
        {
            return false;
        }
        else
        {
            Out << "[Info: Using " << THUAI5::active_dict[selfInfo->activeSkillType] << ".]" << std::endl;
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

bool DebugAPI::ThrowGem(uint32_t timeInMilliseconds, double angleInRadian, uint32_t gemNum)
{
    Out << "Call ThrowGem(" << timeInMilliseconds << angleInRadian << gemNum << ") at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    if (ExamineValidity)
    {
        
        auto selfInfo = logic.GetSelfInfo();
        if (selfInfo->isResetting) // 正在复活中
        {
            Out << "[Warning: You have been slained.]" << std::endl;
            return false;
        }
        if (selfInfo->gemNum == 0)
        {
            Out << "[Warning: You don't have any gems.]" << std::endl;
            return false;
        }
    }

    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::ThrowGem);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);
    message.set_gemsize(gemNum);
    return logic.SendInfo(message);
}

bool DebugAPI::UseGem(uint32_t gemNum)
{
    Out << "Call UseGem(" << gemNum << ") at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    if (ExamineValidity)
    {
        
        auto selfInfo = logic.GetSelfInfo();
        if (selfInfo->isResetting) // 正在复活中
        {
            Out << "[Warning: You have been slained.]" << std::endl;
            return false;
        }
        if (selfInfo->gemNum == 0)
        {
            Out << "[Warning: You don't have any gems.]" << std::endl;
            return false;
        }
    }

    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseGem);
    message.set_gemsize(gemNum);
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

std::vector<std::shared_ptr<const THUAI5::Character>> DebugAPI::GetCharacters() const
{
    Out << "Call GetCharacters() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    return logic.GetCharacters();
}

std::vector<std::shared_ptr<const THUAI5::Bullet>> DebugAPI::GetBullets() const
{
    Out << "Call GetBullets() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    return logic.GetBullets();
}

std::vector<std::shared_ptr<const THUAI5::Prop>> DebugAPI::GetProps() const
{
    Out << "Call GetProps() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    return logic.GetProps();
}

std::shared_ptr<const THUAI5::Character> DebugAPI::GetSelfInfo() const
{
    Out << "Call GetSelfInfo() at " << Time::TimeSinceStart(StartPoint) << "ms" << std::endl;
    return logic.GetSelfInfo();
}

THUAI5::PlaceType DebugAPI::GetPlaceType(int CellX, int CellY) const
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

bool DebugAPI::CanPick(THUAI5::PropType propType, std::shared_ptr<const THUAI5::Character>& selfInfo)
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

bool DebugAPI::CanUseActiveSkill(std::shared_ptr<const THUAI5::Character>& selfInfo)
{
    double timeUntilCommonSkillAvailable = selfInfo->timeUntilCommonSkillAvailable;
    if(timeUntilCommonSkillAvailable==0.0)
    {
        return true;
    }
    Out << "[Warning: common skill is not available, please wait for " << timeUntilCommonSkillAvailable << " s.]" << std::endl;  
    return false;
}