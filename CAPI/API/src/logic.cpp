#include "../include/logic.h"

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

// 子过程 
 
void Logic::ProcessMessageToClient(std::shared_ptr<Protobuf::MessageToClient> pm2c)
{
    switch (pm2c->messagetype())
    {
    case Protobuf::MessageType::StartGame:
        LoadBuffer(pm2c); // 加载信息到buffer

        AI_loop = true;
        UnBlockAI();
        std::cout << "Start Game!" << std::endl;
        break;
    case Protobuf::MessageType::Gaming:
        LoadBuffer(pm2c);
        break;
    case Protobuf::MessageType::EndGame:
        std::cout << "End Game!" << std::endl;
        break;
    default:
        std::cout << "Invalid MessageType of MessageToClient!" << std::endl;
    }
}

void Logic::ProcessMessageToOneClient(std::shared_ptr<Protobuf::MessageToOneClient> pm2oc)
{
    switch (pm2oc->messagetype())
    {
    case Protobuf::MessageType::ValidPlayer:
        std::cout << "Valid player!" << std::endl;
        break;
    case Protobuf::MessageType::InvalidPlayer:
        /*sw_AI = false;
        UnBlockAI();*/
        std::cout << "Invalid player!" << std::endl;
        break;
    case Protobuf::MessageType::Send:
        MessageStorage.push(pm2oc->message());
        break;
    default:
        std::cout << "Invalid MessageType of MessageToOneClient!" << std::endl;
    }
}

void Logic::ProcessMessageToInitialize(std::shared_ptr<Protobuf::MessageToInitialize> pm2i)
{
    switch (pm2i->messagetype())
    {
    case Protobuf::MessageType::StartGame:
        std::cout << "loading map.." << std::endl;
        // pm2i->mapserial
    default:
        break;
    }
}

void Logic::LoadBuffer(std::shared_ptr<Protobuf::MessageToClient>)
{
    // 更新buffer内容
    {
        std::lock_guard<std::mutex> lck(mtx_buffer);

        // 具体操作，等state完善了以后再写
        // ...

        buffer_updated = true;
        counter_buffer += 1;

        // 判断state是否被player访问
        // 如果已经被访问，则控制state的mutex已经被上锁
        // 如果还没有被访问，则没有被上锁。注意在更新时也需要对state上锁！
        if (mtx_state.try_lock())
        {
            Update();
            mtx_state.unlock();
        }
    }
    // 唤醒由于buffer未更新而被阻塞的线程
    cv_buffer.notify_one();
}

void Logic::PlayerWrapper(std::function<void()> player)
{
    {
        std::unique_lock<std::mutex> lock(mtx_ai);
        cv_ai.wait(lock, [this]() {return AI_start; }); // 突然发现此处不能返回atomic_bool类型，所以THUAI4才会搞出另一个控制AI是否启动的标志值
    }
    while (AI_loop)
    {
        player();
    }
}

void Logic::UnBlockAI()
{
    {
        std::lock_guard<std::mutex> lock(mtx_ai);
        AI_start = true;
    }
    cv_ai.notify_one();
}

void Logic::UnBlockBuffer()
{
    {
        std::lock_guard<std::mutex> lock(mtx_buffer);
        buffer_updated = true;
    }
    cv_buffer.notify_one();
}

void Logic::Update()
{
    // 交换两个指针的位置
    State* temp = pState;
    pState = pBuffer;
    pBuffer = temp;

    // pBuffer已经指向访问过的，无用的pState
    buffer_updated = false;
    counter_state = counter_buffer;
}
