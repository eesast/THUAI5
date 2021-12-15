#pragma once

#ifndef LOGIC_H
#define LOGIC_H

#include <iostream>
#include <thread>
#include <memory>
#include <condition_variable>
#include <tuple>
#include <atomic>
#include <fstream>

#include <Message2Clients.pb.h>
#include <Message2Server.pb.h>
#include <concurrent_queue.hpp>

#include "state.h"
#include "Communication.h"
#include "API.h"
#include "constants.h"
#include "AI.h"

// 使用oneof语法所需要的宏
#define MESSAGE_OF_CHARACTER 1
#define MESSAGE_OF_BULLET 2
#define MESSAGE_OF_PROP 3

class Logic;

/// <summary>
/// 通信组件建造类
/// </summary>
class MultiThreadClientCommunicationBuilder
{
public:
    MultiThreadClientCommunicationBuilder(Logic*& pLogic);
    virtual std::shared_ptr<MultiThreadClientCommunication> get_comm() = 0;
    virtual ~MultiThreadClientCommunicationBuilder() {}

protected:
    Logic* pLogic;
};

class MultiThreadClientCommunicationBuilder_A :public MultiThreadClientCommunicationBuilder
{
public:
    MultiThreadClientCommunicationBuilder_A(Logic*& pLogic) ;
    std::shared_ptr<MultiThreadClientCommunication> get_comm()override;
};

/// <summary>
/// API建造类
/// </summary>
class APIBuilder
{
public:
    APIBuilder(Logic*& pLogic);

    virtual std::shared_ptr<IAPI> get_api() = 0;
    virtual ~APIBuilder() {}

protected:
    Logic* pLogic;
    std::shared_ptr<LogicInterface> pLogicInterface;
};

class APIBuilder_A : public APIBuilder
{
public:
    APIBuilder_A(Logic*& pLogic);
    std::shared_ptr<IAPI> get_api() override;
};


/// <summary>
/// 负责创建对象的主管类
/// </summary>
class BuilderDirector
{
public:
    BuilderDirector(Logic*& pLogic, int type = 1);
    std::shared_ptr<IAPI> get_api();
    std::shared_ptr<MultiThreadClientCommunication> get_comm();

private:
    std::shared_ptr<APIBuilder> api_builder;
    std::shared_ptr<MultiThreadClientCommunicationBuilder> comm_builder;
};

/// <summary>
/// 封装了通信组件和AI对象进行操作
/// </summary>
class Logic
{
public:
    std::unique_ptr<BuilderDirector> pDirector;
    std::shared_ptr<MultiThreadClientCommunication> pComm; // 通信组件指针
    std::shared_ptr<IAI> pAI; // 玩家指针
    std::shared_ptr<IAPI> pAPI; // API指针

    std::thread tAI; // 需要对玩家单开线程 

    // 互斥锁
    std::mutex mtx_ai;
    std::mutex mtx_state;
    std::mutex mtx_buffer;

    // 条件变量
    std::condition_variable cv_buffer;
    std::condition_variable cv_ai;

    // 信息队列（队友发来的字符串）
    thuai::concurrency::concurrent_queue<std::string> MessageStorage;

    // 记录状态和缓冲区数(可能和线程有关)
    int counter_state = 0;
    int counter_buffer = 0;

    // 储存状态：现行状态和缓冲区
    State state[2];

    // 操作储存状态的指针（不是动态内存，不需要开智能指针）
    State* pState;
    State* pBuffer;

    // ID记录
    int teamID;
    int playerID;

    // 此时是否应该循环执行player()
    std::atomic_bool AI_loop = true;

    // buffer更新是否完毕
    bool buffer_updated = true;

    // 是否可以启用当前状态
    bool current_state_accessed = false;

    // 是否应该启动AI
    bool AI_start = false;

    /// <summary>
    /// 执行AI线程
    /// </summary>
    /// <param name="player"></param>
    void PlayerWrapper(std::function<void()> player);

    /// <summary>
    /// 处理信息
    /// </summary>
    /// <param name=""></param>
    void ProcessMessage(pointer_m2c);

    /// <summary>
    /// 处理信息Part1 广播
    /// </summary>
    /// <param name=""></param>
    void ProcessMessageToClient(std::shared_ptr<Protobuf::MessageToClient>);

    /// <summary>
    /// 处理信息Part2 单播
    /// </summary>
    /// <param name=""></param>
    void ProcessMessageToOneClient(std::shared_ptr<Protobuf::MessageToOneClient>);

    /// <summary>
    /// 处理信息Part3 初始化
    /// </summary>
    /// <param name=""></param>
    void ProcessMessageToInitialize(std::shared_ptr<Protobuf::MessageToInitialize>);

    /// <summary>
    /// 加载到buffer
    /// </summary>
    /// <param name=""></param>
    void LoadBuffer(std::shared_ptr<Protobuf::MessageToClient>);

    /// <summary>
    /// 强制解锁状态更新线程
    /// </summary>
    void UnBlockBuffer();

    /// <summary>
    /// 强制解锁AI线程
    /// </summary>
    void UnBlockAI();

    /// <summary>
    /// 更新目前的状态(上锁时调用)
    /// </summary>
    /// <returns></returns>
    void Update() noexcept;

    /// <summary>
    /// 将protobuf类转换为THUAI5命名空间的结构体（人物）
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    std::shared_ptr<THUAI5::Character> Protobuf2THUAI5_C(const Protobuf::MessageOfCharacter&);

    /// <summary>
    /// 将protobuf类转换为THUAI5命名空间的结构体（子弹）
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    std::shared_ptr<THUAI5::Bullet> Protobuf2THUAI5_B(const Protobuf::MessageOfBullet&);

    /// <summary>
    /// 将protobuf类转换为THUAI5命名空间的结构体（道具）
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    std::shared_ptr<THUAI5::Prop> Protobuf2THUAI5_P(const Protobuf::MessageOfProp&);
    
    /// <summary>
    /// 是否可视（有3个重载）
    /// </summary>
    /// <param name="x"></param>
    /// <param name="y"></param>
    /// <returns></returns>
    bool visible(int x, int y, const Protobuf::MessageOfCharacter& c)const;
    bool visible(int x, int y, const Protobuf::MessageOfBullet& b)const;
    bool visible(int x, int y, const Protobuf::MessageOfProp& p)const;

    int TeamID()const { return teamID; }
    int PlayerID()const { return playerID; }
   
    Logic(int teamID, int playerID);
    ~Logic() = default;
    /// <summary>
    /// logic运行的主函数
    /// </summary>
    /// <param name="address">ip地址</param>
    /// <param name="port">监听端口</param>
    /// <param name="playerID">玩家ID</param>
    /// <param name="teamID">队伍ID</param>
    /// <param name="activeSkillType">主动技能（需要玩家手动指定）</param>
    /// <param name="passiveSkillType">被动技能（需要玩家手动指定）</param>
    /// <param name="f">AI函数包装器</param>
    /// <param name="debuglevel">debug模式</param>
    /// <param name="filename">输出信息写入的输出流</param>
    void Main(const char* address,uint16_t port,int32_t playerID,int32_t teamID,THUAI5::ActiveSkillType activeSkillType,THUAI5::PassiveSkillType passiveSkillType,CreateAIFunc f,int debuglevel,std::string filename);
};

#endif