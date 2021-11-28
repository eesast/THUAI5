#pragma once

#ifndef LOGIC_H
#define LOGIC_H

#include <iostream>
#include <thread>
#include <memory>
#include <condition_variable>
#include <tuple>
#include <atomic>

#include <Message2Clients.pb.h>
#include <Message2Server.pb.h>

#include "base.h"
#include "state.h"
#include "CAPI.h"
#include <concurrent_queue.hpp>

/// <summary>
/// 封装了通信组件和AI对象进行操作
/// </summary>
class Logic
{
private:
    std::unique_ptr<ClientCommunication> pComm; // 通信组件指针
    //std::unique_ptr<AIBase> pAI; // 玩家指针
    //std::shared_ptr<int> xx; // 玩家状态

    std::thread tAI; // 需要对玩家单开线程

    // 互斥锁
    std::mutex mtx_ai;
    std::mutex mtx_state;
    std::mutex mtx_buffer;

    // 条件变量
    std::condition_variable cv_buffer;
    std::condition_variable cv_ai;

    // 信息队列
    thuai::concurrency::concurrent_queue<std::string> MessageStorage;

    // 记录状态和缓冲区数(可能和线程有关)
    int counter_state = 0;
    int counter_buffer = 0;

    // 此时是否应该循环执行player()
    std::atomic_bool AI_loop = true;

    // buffer更新是否完毕
    std::atomic_bool buffer_updated = true;

    // 储存状态：现行状态和缓冲区
    State state[2];

    // 操作储存状态的指针（不是动态内存，不需要开智能指针）
    State* pState;
    State* pBuffer;

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
   
public:
    Logic();
    ~Logic() = default;
    void Main();
};

#endif