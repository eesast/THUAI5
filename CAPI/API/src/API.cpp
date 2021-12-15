#include "../include/API.h"

const static double PI = 3.14159265358979323846;

API::API(const LogicInterface& logicInterface):LogicInterface(logicInterface)
{

}

bool API::MovePlayer(uint32_t timeInMilliseconds, double angleInRadian)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Move);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);

    return SendInfo(message); //!此处还没有定好使用哪种接口发送信息，return 0只是占位符
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

bool API::Attack(uint32_t timeInMilliseconds, double angleInRadian)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Attack);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);
    return SendInfo(message);
}

bool API::UseCommonSkill()
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseCommonSkill);
    return SendInfo(message);
}

bool API::Send(int toPlayerID, std::string to_message)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Send);
    message.set_toplayerid(toPlayerID);
    message.set_message(to_message);
    return SendInfo(message);
}

bool API::Pick(THUAI5::PropType proptype)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Pick);
    message.set_proptype(Protobuf::PropType(proptype));
    return SendInfo(message);
}

bool API::ThrowProp(uint32_t timeInMilliseconds, double angleInRadian)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::ThrowProp);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);
    return SendInfo(message);
}

bool API::UseProp()
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseProp);
    return SendInfo(message);
}

bool API::ThrowGem(uint32_t timeInMilliseconds, double angleInRadian, uint32_t gemNum)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::ThrowGem);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);
    message.set_gemsize(gemNum);
    return SendInfo(message);
}

bool API::UseGem(uint32_t gemNum)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseGem);
    message.set_gemsize(gemNum);
    return SendInfo(message);
}

std::vector<std::shared_ptr<const THUAI5::Character>> API::GetCharacters() const
{
    std::vector<std::shared_ptr<const THUAI5::Character>> temp;
    temp.assign(pState->characters.begin(), pState->characters.end());
    return temp;
}

std::vector<std::shared_ptr<const THUAI5::Wall>> API::GetWalls() const
{
    std::vector<std::shared_ptr<const THUAI5::Wall>> temp;
    temp.assign(pState->walls.begin(), pState->walls.end());
    return temp;
}

std::vector<std::shared_ptr<const THUAI5::Bullet>> API::GetBullets() const
{
    std::vector<std::shared_ptr<const THUAI5::Bullet>> temp;
    temp.assign(pState->bullets.begin(), pState->bullets.end());
    return temp;
}

std::vector<std::shared_ptr<const THUAI5::Prop>> API::GetProps() const
{
    std::vector<std::shared_ptr<const THUAI5::Prop>> temp;
    temp.assign(pState->props.begin(), pState->props.end());
    return temp;
}

std::shared_ptr<const THUAI5::Character> API::GetSelfInfo() const
{
    return pState->self;
}

