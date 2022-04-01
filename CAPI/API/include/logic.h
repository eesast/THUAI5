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

#define LOGIC_DEBUG

/// <summary>
/// 封装了通信组件和AI对象进行操作
/// </summary>
class Logic: public ISubscripter, public ILogic
{
private:
    // ID记录
    int teamID;
    int playerID;

    // 技能记录
    THUAI5::SoftwareType softwareType;
    THUAI5::HardwareType hardwareType;

    // 记录一场游戏中所有玩家的全部GUID信息
    std::vector<int64_t> playerGUIDS;

    std::unique_ptr<MultiThreadClientCommunication> pComm; // 通信组件指针
    std::unique_ptr<IAI> pAI; // 玩家指针
    std::shared_ptr<IAPI_For_Logic> pAPI; // API指针

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

    // asynchronous = true 时控制内容更新的变量
    std::atomic_bool freshed = false;

    int GetCounter() const override;
    std::vector<std::shared_ptr<const THUAI5::Robot>> GetRobots() const override;
    std::vector<std::shared_ptr<const THUAI5::Prop>> GetProps() const override;
    std::vector<std::shared_ptr<const THUAI5::SignalJammer>> GetSignalJammers() const override;
    std::shared_ptr<const THUAI5::Robot> GetSelfInfo() const override;
    THUAI5::PlaceType GetPlaceType(int CellX, int CellY) const override;

    uint32_t GetTeamScore() const override;
    const std::vector<int64_t> GetPlayerGUIDs() const override;

    // 重写的委托
    Protobuf::MessageToServer OnConnect() override;
    void OnReceive(pointer_m2c p2M) override;
    void OnClose() override;

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
    /// 更新状态(仅在asynchronous = false时调用！！)
    /// </summary>
    /// <returns></returns>
    void Update() noexcept;

    /// <summary>
    /// 更新状态(仅在asynchronous = true时调用！！)
    /// </summary>
    /// <returns></returns>
    void Wait() noexcept;
  
public:

    Logic(int teamID, int playerID,THUAI5::SoftwareType softwareType, THUAI5::HardwareType hardwareType);
    ~Logic() = default;
    void Main(const char* address, uint16_t port, CreateAIFunc f, int debuglevel, std::string filename);
};

#endif
