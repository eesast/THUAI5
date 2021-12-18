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

/// <summary>
/// 封装了通信组件和AI对象进行操作
/// </summary>
class Logic: public ISubscripter, public ILogic
{
private:
    // ID记录
    int teamID;
    int playerID;

    // 记录一场游戏中所有玩家的全部GUID信息
    std::vector<int64_t> playerGUIDS;

    std::unique_ptr<MultiThreadClientCommunication> pComm; // 通信组件指针
    std::unique_ptr<IAI> pAI; // 玩家指针
    std::shared_ptr<IAPI> pAPI; // API指针

    std::thread tAI; // 需要对玩家单开线程 

    // 互斥锁
    mutable std::mutex mtx_ai;
    mutable std::mutex mtx_state;
    mutable std::mutex mtx_buffer;

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

    // 此时是否应该循环执行player()
    std::atomic_bool AI_loop = true;

    // buffer更新是否完毕
    bool buffer_updated = true;

    // 是否可以启用当前状态
    bool current_state_accessed = false;

    // 是否应该启动AI
    bool AI_start = false;

    std::vector<std::shared_ptr<const THUAI5::Character>> GetCharacters() const override;
    std::vector<std::shared_ptr<const THUAI5::Wall>> GetWalls() const override;
    std::vector<std::shared_ptr<const THUAI5::Prop>> GetProps() const override;
    std::vector<std::shared_ptr<const THUAI5::Bullet>> GetBullets() const override;
    std::shared_ptr<const THUAI5::Character> GetSelfInfo() const override;

    virtual uint32_t GetTeamScore() const = 0;
    virtual const std::vector<int64_t> GetPlayerGUIDs() const = 0;

    // 重写的委托
    Protobuf::MessageToServer OnConnect() override;
    virtual void OnReceive(pointer_m2c p2M) override;
    virtual void OnClose() override;

    bool SendInfo(Protobuf::MessageToServer&) override;
    bool Empty() override;
    std::optional<std::string> GetInfo() override;
    bool WaitThread() override;

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
  
public:

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