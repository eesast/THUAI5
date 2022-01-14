#include "../include/logic.h"

// 这样写可以吗
extern const THUAI5::ActiveSkillType playerActiveSkill;
extern const THUAI5::PassiveSkillType playerPassiveSkill;

/// <summary>
/// 辅助函数：将Proto类转换为THUAI类
/// </summary>
namespace Proto2THUAI
{
    /// <summary>
    /// 将protobuf类转换为THUAI5命名空间的结构体（人物）
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    std::shared_ptr<THUAI5::Character> Protobuf2THUAI5_C(const Protobuf::MessageOfCharacter& c)
    {
        std::shared_ptr<THUAI5::Character> character = std::make_shared<THUAI5::Character>();
        character->activeSkillType = (THUAI5::ActiveSkillType)c.activeskilltype();
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
        character->passiveSkillType = (THUAI5::PassiveSkillType)c.passiveskilltype();
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

    /// <summary>
    /// 将protobuf类转换为THUAI5命名空间的结构体（子弹）
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    std::shared_ptr<THUAI5::Bullet> Protobuf2THUAI5_B(const Protobuf::MessageOfBullet& b)
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

    /// <summary>
    /// 将protobuf类转换为THUAI5命名空间的结构体（道具）
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    std::shared_ptr<THUAI5::Prop> Protobuf2THUAI5_P(const Protobuf::MessageOfProp& p)
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
}

/// <summary>
/// 辅助函数：判断是否可见
/// </summary>
namespace Vision
{
    /*
    * 人物是否可见的判定机制如下：
    * 1.若人物不在草丛里，则看不到技能隐身和在草丛里的玩家
    * 2.若人物在草丛里，则可以看得到与自己位于同一草丛的玩家，但是看不到技能隐身的玩家
    */

    static bool visible(std::shared_ptr<THUAI5::Character> self, const Protobuf::MessageOfCharacter& c)
    {
        int64_t dx = self->x - c.x();
        int64_t dy = self->y - c.y();
        uint64_t distanceSquared = dx * dx + dy * dy;
        if (!(distanceSquared <= Constants::Map::sightRadiusSquared))
        {
            return false;
        }
        if (c.isinvisible())
        {
            return false;
        }
        if (c.place() == Protobuf::PlaceType::Grass1 ||c.place() == Protobuf::PlaceType::Grass2 || c.place() == Protobuf::PlaceType::Grass3) // 人物在草丛中
        {
            if (self->place == (THUAI5::PlaceType)c.place())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    static bool visible(std::shared_ptr<THUAI5::Character> self, const Protobuf::MessageOfBullet& b)
    {
        int64_t dx = self->x - b.x();
        int64_t dy = self->y - b.y();
        uint64_t distanceSquared = dx * dx + dy * dy;
        return distanceSquared <= Constants::Map::sightRadiusSquared;
    }

    static bool visible(std::shared_ptr<THUAI5::Character> self, const Protobuf::MessageOfProp& p)
    {
        int64_t dx = self->x - p.x();
        int64_t dy = self->y - p.y();
        uint64_t distanceSquared = dx * dx + dy * dy;
        return distanceSquared <= Constants::Map::sightRadiusSquared;
    }
} // 目前还没有引入草丛机制！

int Logic::GetCounter() const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    return counter_state;
}

std::vector<std::shared_ptr<const THUAI5::Character>> Logic::GetCharacters() const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    std::vector<std::shared_ptr<const THUAI5::Character>> temp;
    temp.assign(pState->characters.begin(), pState->characters.end());
    return temp;
}

std::vector<std::shared_ptr<const THUAI5::Wall>> Logic::GetWalls() const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    std::vector<std::shared_ptr<const THUAI5::Wall>> temp;
    temp.assign(pState->walls.begin(), pState->walls.end());
    return temp;
}

std::vector<std::shared_ptr<const THUAI5::Bullet>> Logic::GetBullets() const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    std::vector<std::shared_ptr<const THUAI5::Bullet>> temp;
    temp.assign(pState->bullets.begin(), pState->bullets.end());
    return temp;
}

std::vector<std::shared_ptr<const THUAI5::Prop>> Logic::GetProps() const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    std::vector<std::shared_ptr<const THUAI5::Prop>> temp;
    temp.assign(pState->props.begin(), pState->props.end());
    return temp;
}

std::shared_ptr<const THUAI5::Character> Logic::GetSelfInfo() const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    return pState->self;
}

uint32_t Logic::GetTeamScore()const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    return pState->teamScore;
}

const std::vector<int64_t> Logic::GetPlayerGUIDs() const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    return pState->guids;
}

Protobuf::MessageToServer Logic::OnConnect()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    Protobuf::MessageToServer message;
    message.set_messagetype(Protobuf::MessageType::AddPlayer);
    message.set_playerid(playerID);
    message.set_teamid(teamID);
    message.set_askill1((Protobuf::ActiveSkillType)playerActiveSkill);
    message.set_pskill((Protobuf::PassiveSkillType)playerPassiveSkill);
    return message;
}

void Logic::OnReceive(pointer_m2c p2M)
{
    ProcessMessage(p2M);
}

void Logic::OnClose()
{
    AI_loop = false;
    UnBlockBuffer();
}

bool Logic::SendInfo(Protobuf::MessageToServer& m2s)
{
    m2s.set_playerid(playerID);
    m2s.set_teamid(teamID);
    return pComm->Send(m2s);
}

bool Logic::Empty()
{
    return MessageStorage.empty();
}

std::optional<std::string> Logic::GetInfo()
{
    return MessageStorage.try_pop();
}

bool Logic::WaitThread()
{
    std::unique_lock<std::mutex> lck_buffer(mtx_buffer);
    cv_buffer.wait(lck_buffer, [this]() {return buffer_updated; });
    Update();
    return true;
}

Logic::Logic(int teamID, int playerID) :teamID(teamID), playerID(playerID)
{
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
    // case 2:
    //     ProcessMessageToInitialize(std::get<std::shared_ptr<Protobuf::MessageToInitialize>>(p2m));
    //     break;
    default:
        std::cerr << "No info type matches std::variant!" << std::endl;
    }
}

// 子过程 
 
void Logic::ProcessMessageToClient(std::shared_ptr<Protobuf::MessageToClient> pm2c)
{
    switch (pm2c->messagetype())
    {
    case Protobuf::MessageType::StartGame:
        LoadBuffer(pm2c); // 加载信息到buffer

        // 对guid进行重新载入（只载入玩家的guid信息）
        playerGUIDS.clear();
        for (auto it = pm2c->gameobjmessage().begin(); it != pm2c->gameobjmessage().end(); it++)
        {
            if (it->has_messageofcharacter())
            {
                playerGUIDS.push_back(it->messageofcharacter().guid());
            }
        }
        pState->guids = playerGUIDS;
        pBuffer->guids = playerGUIDS;

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
            counter_buffer = -1;
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
        std::cout << "Valid player: " <<pm2oc->teamid() << " " << pm2oc->playerid() << std::endl;
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

        // 1.清除原有信息
        pBuffer->characters.clear();
        pBuffer->walls.clear();
        pBuffer->props.clear();
        pBuffer->bullets.clear();

        // 2.信息不能全盘接受，要根据现有的视野范围接受（话说是这么用吗...）
        for (auto it = pm2c->gameobjmessage().begin(); it != pm2c->gameobjmessage().end(); it++)
        {
            if (it->has_messageofcharacter())
            {
                // 此信息是属于自身的
                if (it->messageofcharacter().teamid() == teamID && it->messageofcharacter().playerid() == playerID)
                {
                    pBuffer->self = Proto2THUAI::Protobuf2THUAI5_C(it->messageofcharacter());
                    pBuffer->teamScore = it->messageofcharacter().score();
                }
                else
                {
                    if (Vision::visible(pState->self,it->messageofcharacter()))
                    {
                        pBuffer->characters.push_back(Proto2THUAI::Protobuf2THUAI5_C(it->messageofcharacter()));
                    }
                }
            }

            else if (it->has_messageofbullet())
            {
                if (Vision::visible(pState->self,it->messageofbullet()))
                {
                    pBuffer->bullets.push_back(Proto2THUAI::Protobuf2THUAI5_B(it->messageofbullet()));
                }
            }

            else if (it->has_messageofprop())
            {
                if (Vision::visible(pState->self,it->messageofprop()))
                {
                    pBuffer->props.push_back(Proto2THUAI::Protobuf2THUAI5_P(it->messageofprop()));
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
    current_state_accessed = false;
}

void Logic::Main(const char* address, uint16_t port, int32_t playerID, int32_t teamID, THUAI5::ActiveSkillType activeSkillType, THUAI5::PassiveSkillType passiveSkillType, CreateAIFunc f, int debuglevel, std::string filename)
{
    // 构造AI
    pAI = f();

    // 构造通信组件
    pComm = std::make_unique<MultiThreadClientCommunication>(*this);
    pComm->init(); // 千万不要忘记这一步!

    // 构造API
    std::ofstream Out;
    if (debuglevel)
    {
        Out.open(filename);
        if (Out.fail())
        {
            std::cerr << "Failed to open the file!" << std::endl;
        }
        pAPI = std::make_unique<DebugAPI>(*this);
    }
    else
    {
        pAPI = std::make_unique<API>(*this);
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
    tAI = std::thread(&Logic::PlayerWrapper, this, AI_execute);

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
