#include "../include/API.h"
#include "../include/utils.hpp"

const static double PI = 3.14159265358979323846;

bool API::MovePlayer(uint32_t timeInMilliseconds, double angleInRadian)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Move);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);

    return logic.SendInfo(message);
}

bool API::MoveUp(uint32_t timeInMilliseconds)
{
    return MovePlayer(timeInMilliseconds, PI);
}

bool API::MoveDown(uint32_t timeInMilliseconds)
{
    return MovePlayer(timeInMilliseconds, 0);
}

bool API::MoveLeft(uint32_t timeInMilliseconds)
{
    return MovePlayer(timeInMilliseconds, PI * 1.5);
}

bool API::MoveRight(uint32_t timeInMilliseconds)
{
    return MovePlayer(timeInMilliseconds, PI * 0.5);
}

bool API::Attack(double angleInRadian)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Attack);
    message.set_angle(angleInRadian);
    return logic.SendInfo(message);
}

bool API::UseCommonSkill()
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseCommonSkill);
    return logic.SendInfo(message);
}

bool API::Send(int toPlayerID, std::string to_message)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Send);
    message.set_toplayerid(toPlayerID);
    message.set_message(to_message);
    return logic.SendInfo(message);
}

bool API::Pick(THUAI5::PropType proptype)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Pick);
    message.set_proptype(Protobuf::PropType(proptype));
    return logic.SendInfo(message);
}

bool API::ThrowProp(uint32_t timeInMilliseconds, double angleInRadian)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::ThrowProp);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);
    return logic.SendInfo(message);
}

bool API::UseProp()
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseProp);
    return logic.SendInfo(message);
}

bool API::ThrowCPU(uint32_t timeInMilliseconds, double angleInRadian, uint32_t cpuNum)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::ThrowGem);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);
    message.set_gemsize(cpuNum);
    return logic.SendInfo(message);
}

bool API::UseCPU(uint32_t cpuNum)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseGem);
    message.set_gemsize(cpuNum);
    return logic.SendInfo(message);
}

bool API::Wait()
{
    if (logic.GetCounter() == -1)
    {
        return false;
    }
    logic.WaitThread();
    return true;
}

bool API::MessageAvailable()
{
    return !logic.Empty();
}

std::optional<std::string> API::TryGetMessage()
{
    return logic.GetInfo();
}

std::vector<std::shared_ptr<const THUAI5::Robot>> API::GetRobots() const
{
    return logic.GetRobots();
}

std::vector<std::shared_ptr<const THUAI5::SignalJammer>> API::GetSignalJammers() const
{
    return logic.GetSignalJammers();
}

std::vector<std::shared_ptr<const THUAI5::Prop>> API::GetProps() const
{
    return logic.GetProps();
}

std::shared_ptr<const THUAI5::Robot> API::GetSelfInfo() const
{
    return logic.GetSelfInfo();
}

uint32_t API::GetTeamScore() const
{
    return logic.GetTeamScore();
}

const std::vector<int64_t> API::GetPlayerGUIDs() const
{
    return logic.GetPlayerGUIDs();
}

int API::GetFrameCount() const
{
    return logic.GetCounter();
}

THUAI5::PlaceType API::GetPlaceType(int32_t CellX, int32_t CellY) const
{
    return logic.GetPlaceType(CellX, CellY);
}

void API::PrintSignalJammers() const
{
    std::cout << "******************Bullets******************" << std::endl;
    auto jammers = logic.GetSignalJammers();
    for (int i = 0;i<jammers.size();i++)
    {
        std::cout << "SignalJammer " << i << ":" << std::endl;
        std::cout << "facingDirection: " <<jammers[i]->facingDirection << std::endl
                  << "guid: " << jammers[i]->guid << std::endl
                  << "parentTeamID: " << jammers[i]->parentTeamID << std::endl
                  << "place: " << THUAI5::place_dict[jammers[i]->place] << std::endl
                  << "type: " << THUAI5::jammer_dict[jammers[i]->type] << std::endl
                  << "x: " << jammers[i]->x << std::endl
                  << "y: " << jammers[i]->y << std::endl;
    }
    std::cout << "*******************************************" << std::endl;
}

void API::PrintRobots() const
{
    std::cout << "******************Characters******************" << std::endl;
    auto robots = logic.GetRobots();
    for(int i = 0;i< robots.size();i++)
    {
        std::cout << "Robot " << i << ":" << std::endl;
        std::cout << "softwareType: " << THUAI5::software_dict[robots[i]->softwareType] << std::endl
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
                  << "emisionAccessory: " << robots[i]->emissionAccessory << std::endl
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
    std::cout << "**********************************************" << std::endl;
}

void API::PrintProps() const
{
    std::cout << "******************Props******************" << std::endl;
    auto props = logic.GetProps();
    for(int i = 0;i<props.size();i++)
    {
        std::cout << "Prop " << i << ":" << std::endl;
        std::cout << "facingDirection: " << props[i]->facingDirection << std::endl
                  << "guid: " << props[i]->guid << std::endl
                  << "place: " << THUAI5::place_dict[props[i]->place] << std::endl
                  << "size: " << props[i]->size << std::endl
                  << "type: " << THUAI5::prop_dict[props[i]->type] << std::endl
                  << "x: " << props[i]->x << std::endl
                  << "y: " << props[i]->y << std::endl;
    }
    std::cout << "*****************************************" << std::endl;
}

void API::PrintSelfInfo() const
{
    std::cout << "******************Selfinfo******************" << std::endl;
    auto selfinfo = logic.GetSelfInfo();
    if (selfinfo != nullptr)
    {
        std::cout << "softwareType: " << THUAI5::software_dict[selfinfo->softwareType] << std::endl
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
                  << "emisionAccessory: " << selfinfo->emissionAccessory << std::endl
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
    std::cout << "********************************************" << std::endl;
}
