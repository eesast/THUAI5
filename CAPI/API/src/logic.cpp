#include "../include/logic.h"

// 这样写可以吗
extern const THUAI5::ActiveSkillType playerActiveSkill;
extern const THUAI5::PassiveSkillType playerPassiveSkill;

MultiThreadClientCommunicationBuilder::MultiThreadClientCommunicationBuilder(Logic*& pLogic):pLogic(pLogic)
{}

MultiThreadClientCommunicationBuilder_A::MultiThreadClientCommunicationBuilder_A(Logic*& pLogic) : MultiThreadClientCommunicationBuilder(pLogic)
{}

std::shared_ptr<MultiThreadClientCommunication>  MultiThreadClientCommunicationBuilder_A::get_comm()
{
    auto MultiThreadOnReceive = [&](pointer_m2c p2M)
    {
        pLogic->ProcessMessage(p2M);
    };
    auto MultiThreadOnClose = [&]()
    {
        pLogic->AI_loop = false;
        pLogic->UnBlockBuffer();
    };
    auto MultiThreadOnConnect = [&]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        Protobuf::MessageToServer message;
        message.set_messagetype(Protobuf::MessageType::AddPlayer);
        message.set_playerid(pLogic->PlayerID());
        message.set_teamid(pLogic->TeamID());
        message.set_askill1((Protobuf::ActiveSkillType)playerActiveSkill);
        return message;
    };
    std::shared_ptr<MultiThreadClientCommunication> comm = std::make_shared<MultiThreadClientCommunication>(MultiThreadOnConnect,MultiThreadOnReceive,MultiThreadOnClose);
    return comm;
}

APIBuilder::APIBuilder(Logic*& pLogic):pLogic(pLogic)
{}

APIBuilder_A::APIBuilder_A(Logic*& pLogic) : APIBuilder(pLogic)
{
   
    
}

std::shared_ptr<IAPI> APIBuilder_A::get_api()
{
    auto SendInfo = [this](Protobuf::MessageToServer& m2s)
    {
        m2s.set_playerid(pLogic->PlayerID());
        m2s.set_teamid(pLogic->TeamID());
        return (pLogic->pComm->Send(m2s));
    };

    auto Empty = [this]()
    {
        return pLogic->MessageStorage.empty();
    };

    auto GetInfo = [this](std::string& s)
    {
        return pLogic->MessageStorage.try_pop();
    };

    auto GetCounter = [this]()
    {
        return pLogic->counter_state;
    };

    auto WaitThread = [this]()
    {
        std::unique_lock<std::mutex> lock(pLogic->mtx_buffer);
        pLogic->cv_buffer.wait(lock, [this]() {return pLogic->buffer_updated; });
        pLogic->Update();
    };

    State* pState = pLogic->pState;

    pLogicInterface = std::make_shared<LogicInterface>(SendInfo, Empty, GetInfo, GetCounter, WaitThread, pState);

    std::shared_ptr<IAPI> api = std::make_shared<API>(*pLogicInterface);
    return api;
}

BuilderDirector::BuilderDirector(Logic*& plogic,int type)
{
    // 后续会加
    switch (type)
    {
    case 1:
        api_builder = std::make_shared<APIBuilder_A>(0);
        comm_builder = std::make_shared<MultiThreadClientCommunicationBuilder_A>();
        break;
    default:
        api_builder = std::make_shared<APIBuilder_A>(1);
        comm_builder = std::make_shared<MultiThreadClientCommunicationBuilder_A>();
        break;
    }
}

std::shared_ptr<IAPI> BuilderDirector::get_api()
{
    return api_builder->get_api();
}

std::shared_ptr<MultiThreadClientCommunication> BuilderDirector::get_comm()
{
    return comm_builder->get_comm();
}

Logic::Logic(int teamID, int playerID) :teamID(teamID), playerID(playerID)
{
    pDirector = std::make_unique<BuilderDirector>(1);
    pState = &state[0];
    pBuffer = &state[1];
}

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
        std::cerr << "No info type matches std::varient!" << std::endl;
    }
}

// 子过程 
 
void Logic::ProcessMessageToClient(std::shared_ptr<Protobuf::MessageToClient> pm2c)
{
    switch (pm2c->messagetype())
    {
    case Protobuf::MessageType::StartGame:
        LoadBuffer(pm2c); // 加载信息到buffer

        // 记录guid信息
        State::playerGUIDS.clear();
        // 此处需要对guid进行重新载入，但proto里好像没有？
        // TODO

        AI_loop = true;
        UnBlockAI();
        std::cout << "Start Game!" << std::endl;
        break;

    case Protobuf::MessageType::Gaming:
        LoadBuffer(pm2c);
        break;

    case Protobuf::MessageType::EndGame:
        AI_loop = false;
        {
            std::lock_guard<std::mutex> lck(mtx_buffer);
            buffer_updated = true;
        }
        cv_buffer.notify_one();
        std::cout << "End Game!" << std::endl;
        break;

    default:
        std::cerr << "Invalid MessageType of MessageToClient!" << std::endl;
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
        AI_loop = false;
        UnBlockAI();
        std::cout << "Invalid player!" << std::endl;
        break;
    case Protobuf::MessageType::Send:
        MessageStorage.emplace(pm2oc->message());
        break;
    default:
        std::cerr << "Invalid MessageType of MessageToOneClient!" << std::endl;
    }
}

// 这块还没想好怎么写

//void Logic::ProcessMessageToInitialize(std::shared_ptr<Protobuf::MessageToInitialize> pm2i)
//{
//    switch (pm2i->messagetype())
//    {
//    case Protobuf::MessageType::StartGame:
//        std::cout << "loading map.." << std::endl;
//        pm2i->mapserial();
//    default:
//        break;
//    }
//}

void Logic::LoadBuffer(std::shared_ptr<Protobuf::MessageToClient> pm2c)
{
    // 更新buffer内容
    {
        std::lock_guard<std::mutex> lck(mtx_buffer);

        // 具体操作，等state完善了以后再写
        
        // 1.清除原有信息
        pBuffer->characters.clear();
        pBuffer->walls.clear();
        pBuffer->props.clear();
        pBuffer->bullets.clear();

        // 2.信息不能全盘接受，要根据现有的视野范围接受（话说是这么用吗...）
        int selfX = pState->self->x;
        int selfY = pState->self->y;
        for (auto it = pm2c->gameobjmessage().begin(); it != pm2c->gameobjmessage().end(); it++)
        {
            if (it->has_messageofcharacter())
            {
                // 此信息是属于自身的
                if (it->messageofcharacter().teamid() == ID::GetTeamID() && it->messageofcharacter().playerid() == ID::GetPlayerID())
                {
                    pBuffer->self = Protobuf2THUAI5_C(it->messageofcharacter());
                    pBuffer->teamScore = it->messageofcharacter().score();
                }
                else
                {
                    if (visible(selfX,selfY,it->messageofcharacter())) // 这里不应该是true，还应该加一个视野条件限制，但视野还没有完全确定好
                    {
                        pBuffer->characters.push_back(Protobuf2THUAI5_C(it->messageofcharacter()));
                    }
                }
            }

            else if (it->has_messageofbullet())
            {
                if (visible(selfX, selfY,it->messageofbullet()))
                {
                    pBuffer->bullets.push_back(Protobuf2THUAI5_B(it->messageofbullet()));
                }
            }

            else if (it->has_messageofprop())
            {
                if (visible(selfX, selfY,it->messageofprop()))
                {
                    pBuffer->props.push_back(Protobuf2THUAI5_P(it->messageofprop()));
                }
            }

            else
            {
                std::cerr << "invalid gameobjtype (not character, prop or bullet)" << std::endl;
            }
        }
        /*pBuffer->teamScore = pm2c->gameobjmessage(MESSAGE_OF_CHARACTER).messageofcharacter().score();
        pBuffer->self = Protobuf2THUAI5_C(pm2c->gameobjmessage(MESSAGE_OF_CHARACTER).messageofcharacter());*/


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

void Logic::Update() noexcept
{
    // 交换两个指针的位置
    State* temp = pState;
    pState = pBuffer;
    pBuffer = temp;

    // pBuffer已经指向访问过的，无用的pState
    buffer_updated = false;
    counter_state = counter_buffer;
    current_state_accessed = true;
}

std::shared_ptr<THUAI5::Character> Logic::Protobuf2THUAI5_C(const Protobuf::MessageOfCharacter &c)
{
    std::shared_ptr<THUAI5::Character> character = std::make_shared<THUAI5::Character>();
    character->ActiveSkillType = (THUAI5::ActiveSkillType)c.activeskilltype();
    character->attackRange = c.attackrange();
    character->buff = (THUAI5::BuffType)c.buff();
    character->bulletNum = c.bulletnum();
    character->bulletType = (THUAI5::BulletType)c.bullettype();
    character->canMove = c.canmove();
    character->CD = c.cd();
    character->gemNum = c.gemnum();
    character->guid = c.guid();
    character->isResetting = c.isresetting();
    character->life = c.life();
    character->lifeNum = c.lifenum();
    character->PassiveSkillType = (THUAI5::PassiveSkillType)c.passiveskilltype();
    character->place = (THUAI5::PlaceType)c.place();
    character->playerID = c.playerid();
    character->prop = (THUAI5::PropType)c.prop();
    character->radius = c.radius();
    character->score = c.score();
    character->speed = c.speed();
    character->teamID = c.teamid();
    character->timeUntilCommonSkillAvailable = c.timeuntilcommonskillavailable();
    character->timeUntilUltimateSkillAvailable = c.timeuntilultimateskillavailable();
    character->vampire = c.vampire();
    character->x = c.x();
    character->y = c.y();

    return character;
}

std::shared_ptr<THUAI5::Bullet> Logic::Protobuf2THUAI5_B(const Protobuf::MessageOfBullet &b)
{
    std::shared_ptr<THUAI5::Bullet> bullet = std::make_shared<THUAI5::Bullet>();
    bullet->facingDirection = b.facingdirection();
    bullet->guid = b.guid();
    bullet->parentTeamID = b.parentteamid();
    bullet->place = (THUAI5::PlaceType)b.place();
    bullet->type = (THUAI5::BulletType)b.type();
    bullet->x = b.x();
    bullet->y = b.y();

    return bullet;
}

std::shared_ptr<THUAI5::Prop> Logic::Protobuf2THUAI5_P(const Protobuf::MessageOfProp &p)
{
    std::shared_ptr<THUAI5::Prop> prop = std::make_shared<THUAI5::Prop>();
    prop->facingDirection = p.facingdirection();
    prop->guid = p.guid();
    prop->place = (THUAI5::PlaceType)p.place();
    prop->size = p.size();
    prop->type = (THUAI5::PropType)p.type();
    prop->x = p.x();
    prop->y = p.y();

    return prop;
}

void Logic::Main(const char* address, uint16_t port, int32_t playerID, int32_t teamID, THUAI5::ActiveSkillType activeSkillType, THUAI5::PassiveSkillType passiveSkillType, CreateAIFunc f, int debuglevel, std::string filename)
{
    // 构造AI
    pAI = f();

    // 构造通信组件
    pComm = pDirector->get_comm();

    // 构造API
    pAPI = pDirector->get_api();
   
    std::ofstream Out;
    if (debuglevel)
    {
        if (Out.fail())
        {
            std::cerr << "Failed to open the file!" << std::endl;
        }
    }

    // 构造AI线程函数
    auto AI_execute = [this]()
    {
        std::lock_guard<std::mutex> lock_state(mtx_state);
        if (!current_state_accessed)
        {
            current_state_accessed = true;
            pAI->play(*pAPI);
        }
        else
        {
            std::unique_lock<std::mutex> lock_buffer(mtx_buffer);
            cv_buffer.wait(lock_buffer, [this]() {return buffer_updated; });
            Update();
        }
    };

    // 执行AI线程
    tAI = std::thread(AI_execute);

    // 游戏运行
    if (!pComm->Start(address, port))
    {
        AI_loop = false;
        UnBlockAI();
        tAI.join();
        std::cerr << "Unable to connect to server, failed to start AI thread." << std::endl;
    }
    else
    {
        std::cout << "Connect to the server successfully, AI thread will be start." << std::endl;
        if (tAI.joinable())
        {
            tAI.join();
        }
    }
    Out.close();
    pComm->Join();
}

bool Logic::visible(int x, int y, const Protobuf::MessageOfCharacter& c)const
{
    return true;
}

bool Logic::visible(int x, int y, const Protobuf::MessageOfBullet& b)const
{
    return true;
}

bool Logic::visible(int x, int y, const Protobuf::MessageOfProp& p)const
{
    return true;
}