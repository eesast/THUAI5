#include "logic.h"
#include <fstream>

extern const bool asynchronous;

void Logic::ProcessMessage(pointer_m2c p2m)
{
    switch (p2m.index())
    {
    case 0:
        ProcessMessageToClient(std::get<std::shared_ptr<Protobuf::MessageToClient>>(p2m));
        break;
    case 1:
        ProcessMessageToOneClient(std::get<std::shared_ptr<Protobuf::MessageToOneClient>>(p2m));
        break;
    case 2:
        ProcessMessageToInitialize(std::get<std::shared_ptr<Protobuf::MessageToInitialize>>(p2m));
        break;
    default:
        std::cout << "No info type matches!" << std::endl;
    }
}

// ×Ó¹ý³Ì

void Logic::ProcessMessageToClient(std::shared_ptr<Protobuf::MessageToClient> pm2c)
{

}

void Logic::ProcessMessageToOneClient(std::shared_ptr<Protobuf::MessageToOneClient> pm2c)
{

}

void Logic::ProcessMessageToInitialize(std::shared_ptr<Protobuf::MessageToInitialize> pm2c)
{

}

void Logic::UnBlockAI()
{

}

void Logic::UnBlockBuffer()
{

}
