#pragma once

#ifndef LOGIC_H
#define LOGIC_H

#include<iostream>
#include<thread>
#include<memory>
#include<condition_variable>
#include<tuple>
#include<atomic>

#include"base.h"
#include"CAPI.h"
#include"proto/Message2Clients.pb.h"
#include"concurrent_queue.hpp"



/// <summary>
/// 封装了通信组件和AI对象进行操作
/// </summary>
class Logic
{
private:
    std::unique_ptr<ClientCommunication> pComm; // 通信组件指针
    std::unique_ptr<AIBase> pAI; // 玩家指针
    std::shared_ptr<int> xx; // 玩家状态

    std::thread tAI; // 需要对玩家单开线程

    // 互斥锁
    std::mutex mtx_ai;
    std::mutex mtx_state; 
    std::mutex mtx_buffer;

    // 条件变量
    std::condition_variable cv_buffer;
    std::condition_variable cv_ai;
    
    // 信息队列
    concurrency::concurrent_queue<std::string> MessageStorage;

    // 记录状态和信息数(可能和线程有关)
    std::atomic<int> counter_state = 0;
    std::atomic<int> counter_buffer = 0;

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
    /// 解锁状态
    /// </summary>
    void UnBlockBuffer();
    
    /// <summary>
    /// 解锁AI线程
    /// </summary>
    void UnBlockAI();
    
public:
    Logic();
    ~Logic() = default;
    void Main();
};

#endif