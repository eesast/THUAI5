#include "../include/API.h"

const static double PI = 3.14159265358979323846;

bool API::MovePlayer(uint32_t timeInMilliseconds, double angleInRadian)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Move);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);

    return 0; //!此处还没有定好使用哪种接口发送信息，return 0只是占位符
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
    return 0;
}

bool API::UseCommonSkill()
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseCommonSkill);
    return 0;
}

bool API::Send(int toPlayerID, std::string to_message)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Send);
    message.set_toplayerid(toPlayerID);
    message.set_message(to_message);
    return 0;
}

bool API::Pick(THUAI5::PropType proptype)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::Pick);
    message.set_proptype(Protobuf::PropType(proptype));
    return 0;
}

bool API::ThrowProp(uint32_t timeInMilliseconds, double angleInRadian)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::ThrowProp);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);
    return 0;
}

bool API::UseProp()
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseProp);
    return 0;
}

bool API::ThrowGem(uint32_t timeInMilliseconds, double angleInRadian, uint32_t gemNum)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::ThrowGem);
    message.set_timeinmilliseconds(timeInMilliseconds);
    message.set_angle(angleInRadian);
    message.set_gemsize(gemNum);
    return 0;
}

bool API::UseGem(uint32_t gemNum)
{
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::UseGem);
    message.set_gemsize(gemNum);
    return 0;
}

std::vector<std::shared_ptr<const THUAI5::Character>> API::GetCharacters() const
{
    
}

APIBuilder::APIBuilder(bool type)
{
    if (type)
    {
        api = std::make_shared<API>();
    }
    else
    {
        api = std::make_shared<DebugAPI>();
    }
}

std::shared_ptr<IAPI> APIBuilder::get_api()
{
    return this->api;
}

APIBuilder_1::APIBuilder_1(bool type) :APIBuilder(type)
{

}

void APIBuilder_1::set_SendInfo()
{

}

void APIBuilder_1::set_Empty()
{

}

void APIBuilder_1::set_GetInfo()
{

}

void APIBuilder_1::set_getCounter()
{

}

void APIBuilder_1::set_WaitThread()
{

}