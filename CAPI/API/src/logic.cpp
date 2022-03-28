#include "../include/logic.h"
#include "../include/utils.hpp"

extern const bool asynchronous;
extern const THUAI5::SoftwareType playerSoftware;
extern const THUAI5::HardwareType playerHardware;



int Logic::GetCounter() const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    return counter_state;
}

std::vector<std::shared_ptr<const THUAI5::Robot>> Logic::GetRobots() const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    std::vector<std::shared_ptr<const THUAI5::Robot>> temp;
    temp.assign(pState->robots.begin(), pState->robots.end());
    return temp;
}

std::vector<std::shared_ptr<const THUAI5::SignalJammer>> Logic::GetSignalJammers() const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    std::vector<std::shared_ptr<const THUAI5::SignalJammer>> temp;
    temp.assign(pState->jammers.begin(), pState->jammers.end());
    return temp;
}

std::vector<std::shared_ptr<const THUAI5::Prop>> Logic::GetProps() const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    std::vector<std::shared_ptr<const THUAI5::Prop>> temp;
    temp.assign(pState->props.begin(), pState->props.end());
    return temp;
}

std::shared_ptr<const THUAI5::Robot> Logic::GetSelfInfo() const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    return pState->self;
}

THUAI5::PlaceType Logic::GetPlaceType(int CellX,int CellY) const
{
    std::unique_lock<std::mutex> lock(mtx_buffer);
    return pState->gamemap[CellX][CellY];
}

uint32_t Logic::GetTeamScore() const
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
    message.set_askill1((Protobuf::ActiveSkillType)playerSoftware);
    message.set_pskill((Protobuf::PassiveSkillType)playerHardware);
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
    Update();
    return true;
}

Logic::Logic(int teamID, int playerID, THUAI5::SoftwareType softwareType, THUAI5::HardwareType hardwareType) : teamID(teamID), playerID(playerID), softwareType(softwareType), hardwareType(hardwareType)
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

void Logic::LoadBuffer(std::shared_ptr<Protobuf::MessageToClient> pm2c)
{
    // 更新buffer内容
    {
        std::lock_guard<std::mutex> lck(mtx_buffer);

        // 1.清除原有信息
        pBuffer->robots.clear();
        pBuffer->props.clear();
        pBuffer->jammers.clear();

        // 2.信息不能全盘接受，要根据现有的视野范围接受
        auto gameObjMessage = pm2c->gameobjmessage();
        for (auto it = gameObjMessage.begin(); it != gameObjMessage.end(); it++)
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
                        pBuffer->robots.push_back(Proto2THUAI::Protobuf2THUAI5_C(it->messageofcharacter()));
                    }
                }
            }

            else if (it->has_messageofbullet())
            {
                if (Vision::visible(pState->self,it->messageofbullet()))
                {
                    pBuffer->jammers.push_back(Proto2THUAI::Protobuf2THUAI5_B(it->messageofbullet()));
                }
            }

            else if (it->has_messageofprop())
            {
                if (Vision::visible(pState->self,it->messageofprop()))
                {
                    pBuffer->props.push_back(Proto2THUAI::Protobuf2THUAI5_P(it->messageofprop()));
                }
            }

            else if(it->has_messageofmap())
            {
                auto Row = it->messageofmap().row();
                int row_id = 0;
                for(auto it_row=Row.begin();it_row!=Row.end();it_row++)
                {
                    auto Col = it_row->col();
                    int col_id = 0;
                    for(auto it_col = Col.begin();it_col!=Col.end();it_col++)
                    {
                        pBuffer->gamemap[row_id][col_id] = Proto2THUAI::Protobuf2THUAI5_M(*it_col);
                        col_id++;
                    }
                    row_id++;
                }
            }

            else if(it->has_messageofbombedbullet() || it->has_messageofpickedprop())
            {
                
            }

            else
            {
                std::cerr << "invalid gameobjtype (not character, prop or bullet)" << std::endl;
            }
        }

        if (asynchronous)
        {
            {
                std::lock_guard<std::mutex> lck(mtx_state);
                State* temp = pState;
                pState = pBuffer;
                pBuffer = pState;
            }
            freshed = true;  
        }
        else
        {
            buffer_updated = true;
        }

        counter_buffer += 1;
    }
    // 唤醒由于buffer未更新而被阻塞的线程
    cv_buffer.notify_one();
}

void Logic::PlayerWrapper(std::function<void()> player)
{
    {
        std::unique_lock<std::mutex> lock(mtx_ai);
        cv_ai.wait(lock, [this]() {return AI_start; }); 
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
    {
        std::unique_lock<std::mutex> lck_buffer(mtx_buffer);

        // 缓冲区被使用才需要更新，否则等待下一帧
        cv_buffer.wait(lck_buffer, [this]()
                       { return buffer_updated; });

        // 交换两个指针的位置
        State *temp = pState;
        pState = pBuffer;
        pBuffer = temp;

        // pBuffer已经指向访问过的，无用的pState
        buffer_updated = false;
        counter_state = counter_buffer;
        current_state_accessed = false;
    }
}

void Logic::Wait() noexcept
{
    freshed = false;
    {
        std::unique_lock<std::mutex> lck_state(mtx_state);
        cv_buffer.wait(lck_state, [this]()
                       { return freshed.load(); });
    }
}

void Logic::Main(const char* address, uint16_t port, CreateAIFunc f, int debuglevel, std::string filename)
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
        if (filename != "")
        {
            Out.open(filename);
            if (Out.fail())
            {
                std::cerr << "Failed to open the file!" << std::endl;
            }
            pAPI = std::make_unique<DebugAPI>(*this, Out);
        }
        else
        {
            pAPI = std::make_unique<DebugAPI>(*this, std::cout);
        }
    }
    else
    {
        pAPI = std::make_unique<API>(*this);
    }

    // 构造AI线程函数
    auto AI_execute = [this]()
    {
        if (asynchronous) // 这里是不是用if constexpr会更好
        {
            Wait();
            pAPI->StartTimer();
            pAI->play(*pAPI);
            pAPI->EndTimer();
        }
        else
        {
            Update();
            pAPI->StartTimer();
            pAI->play(*pAPI);
            pAPI->EndTimer();
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
            std::cout << "Join the AI thread." << std::endl;
            tAI.join();
        }
    }
    Out.close();
    pComm->Join();
}
