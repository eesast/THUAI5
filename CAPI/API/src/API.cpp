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

bool API::ThrowGem(uint32_t timeInMilliseconds, double angleInRadian, uint32_t gemNum)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::ThrowGem);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);
    message.set_gemsize(gemNum);
    return logic.SendInfo(message);
}

bool API::UseGem(uint32_t gemNum)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseGem);
    message.set_gemsize(gemNum);
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

std::vector<std::shared_ptr<const THUAI5::Character>> API::GetCharacters() const
{
    return logic.GetCharacters();
}

std::vector<std::shared_ptr<const THUAI5::Bullet>> API::GetBullets() const
{
    return logic.GetBullets();
}

std::vector<std::shared_ptr<const THUAI5::Prop>> API::GetProps() const
{
    return logic.GetProps();
}

std::shared_ptr<const THUAI5::Character> API::GetSelfInfo() const
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

void API::PrintBullets() const
{
    std::cout << "******************Bullets******************" << std::endl;
    auto bullets = logic.GetBullets();
    for (int i = 0;i<bullets.size();i++)
    {
        std::cout << "Bullet " << i << ":" << std::endl;
        std::cout << "facingDirection: " <<bullets[i]->facingDirection << std::endl
                  << "guid: " << bullets[i]->guid << std::endl
                  << "parentTeamID: " << bullets[i]->parentTeamID << std::endl
                  << "place: " << THUAI5::place_dict[bullets[i]->place] << std::endl
                  << "type: " << THUAI5::bullet_dict[bullets[i]->type] << std::endl
                  << "x: " << bullets[i]->x << std::endl
                  << "y: " << bullets[i]->y << std::endl;
    }
    std::cout << "*******************************************" << std::endl;
}

void API::PrintCharacters() const
{
    std::cout << "******************Characters******************" << std::endl;
    auto characters = logic.GetCharacters();
    for(int i = 0;i<characters.size();i++)
    {
        std::cout << "Character " << i << ":" << std::endl;
        std::cout << "activeSkillType: " << THUAI5::active_dict[characters[i]->activeSkillType] << std::endl
                  << "attackRange: " << characters[i]->attackRange << std::endl
                  << "buff: " << THUAI5::buff_dict[characters[i]->buff] << std::endl
                  << "bulletNum: " << characters[i]->bulletNum << std::endl
                  << "bulletType: " << THUAI5::bullet_dict[characters[i]->bulletType] << std::endl
                  << "canMove: " << characters[i]->canMove << std::endl
                  << "CD: " << characters[i]->CD << std::endl
                  << "gemNum: " << characters[i]->gemNum << std::endl
                  << "guid: " << characters[i]->guid << std::endl
                  << "isResetting: " << characters[i]->isResetting << std::endl
                  << "life: " << characters[i]->life << std::endl
                  << "lifeNum: " << characters[i]->lifeNum << std::endl
                  << "passiveSkillType: " << THUAI5::passive_dict[characters[i]->passiveSkillType] << std::endl
                  << "place: " << THUAI5::place_dict[characters[i]->place] << std::endl
                  << "playerID: " << characters[i]->playerID << std::endl
                  << "prop: " << THUAI5::prop_dict[characters[i]->prop] << std::endl
                  << "radius: " << characters[i]->radius << std::endl
                  << "score: " << characters[i]->score << std::endl
                  << "speed: " << characters[i]->speed << std::endl
                  << "teamID: " << characters[i]->teamID << std::endl
                  << "timeUntilCommonSkillAvailable: " << characters[i]->timeUntilCommonSkillAvailable << std::endl
                  << "timeUntilUltimateSkillAvailable: " << characters[i]->timeUntilUltimateSkillAvailable << std::endl
                  << "vampire: " << characters[i]->vampire << std::endl
                  << "x: " << characters[i]->x << std::endl
                  << "y: " << characters[i]->y << std::endl;
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
        std::cout << "activeSkillType: " << THUAI5::active_dict[selfinfo->activeSkillType] << std::endl
                  << "attackRange: " << selfinfo->attackRange << std::endl
                  << "buff: " << THUAI5::buff_dict[selfinfo->buff] << std::endl
                  << "bulletNum: " << selfinfo->bulletNum << std::endl
                  << "bulletType: " << THUAI5::bullet_dict[selfinfo->bulletType] << std::endl
                  << "canMove: " << selfinfo->canMove << std::endl
                  << "CD: " << selfinfo->CD << std::endl
                  << "gemNum: " << selfinfo->gemNum << std::endl
                  << "guid: " << selfinfo->guid << std::endl
                  << "isResetting: " << selfinfo->isResetting << std::endl
                  << "life: " << selfinfo->life << std::endl
                  << "lifeNum: " << selfinfo->lifeNum << std::endl
                  << "passiveSkillType: " << THUAI5::passive_dict[selfinfo->passiveSkillType] << std::endl
                  << "place: " << THUAI5::place_dict[selfinfo->place] << std::endl
                  << "playerID: " << selfinfo->playerID << std::endl
                  << "prop: " << THUAI5::prop_dict[selfinfo->prop] << std::endl
                  << "radius: " << selfinfo->radius << std::endl
                  << "score: " << selfinfo->score << std::endl
                  << "speed: " << selfinfo->speed << std::endl
                  << "teamID: " << selfinfo->teamID << std::endl
                  << "timeUntilCommonSkillAvailable: " << selfinfo->timeUntilCommonSkillAvailable << std::endl
                  << "timeUntilUltimateSkillAvailable: " << selfinfo->timeUntilUltimateSkillAvailable << std::endl
                  << "vampire: " << selfinfo->vampire << std::endl
                  << "x: " << selfinfo->x << std::endl
                  << "y: " << selfinfo->y << std::endl;
    }
    std::cout << "********************************************" << std::endl;
}
