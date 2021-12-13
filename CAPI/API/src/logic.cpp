#include "../include/logic.h"

BuilderDirector::BuilderDirector(int type)
{
    // 后续会加
    switch (type)
    {
    case 1:
        api_builder = std::make_shared<APIBuilder_1>(0);
        comm_builder = std::make_shared<MultiThreadClientCommunicationBuilder_A>();
        break;
    default:
        api_builder = std::make_shared<APIBuilder_1>(1);
        comm_builder = std::make_shared<MultiThreadClientCommunicationBuilder_A>();
        break;
    }
}

std::shared_ptr<IAPI> BuilderDirector::get_api()
{
    api_builder->set_Empty();
    api_builder->set_getCounter();
    api_builder->set_GetInfo();
    api_builder->set_SendInfo();
    api_builder->set_WaitThread();
    return api_builder->get_api();
}

std::shared_ptr<MultiThreadClientCommunication> BuilderDirector::get_comm()
{
    comm_builder->set_OnClose();
    comm_builder->set_OnConnect();
    comm_builder->set_OnReceive();
    return comm_builder->get_comm();
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
        // 加锁？？
        // TODO
        cv_buffer.notify_one();
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
        MessageStorage.emplace(pm2oc->message());
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
        pm2i->mapserial();
    default:
        break;
    }
}

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
        pBuffer->teamScore = pm2c->gameobjmessage(MESSAGE_OF_CHARACTER).messageofcharacter().score();
        pBuffer->self = Protobuf2THUAI5_C(pm2c->gameobjmessage(MESSAGE_OF_CHARACTER).messageofcharacter());

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
}

std::shared_ptr<THUAI5::Character> Logic::Protobuf2THUAI5_C(const Protobuf::MessageOfCharacter c)
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

std::shared_ptr<THUAI5::Bullet> Logic::Protobuf2THUAI5_B(const Protobuf::MessageOfBullet b)
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

std::shared_ptr<THUAI5::Prop> Logic::Protobuf2THUAI5_P(const Protobuf::MessageOfProp p)
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