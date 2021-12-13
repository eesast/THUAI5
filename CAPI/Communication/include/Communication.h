#pragma once
#ifndef CAPI_H

// proto files
#include <Message2Clients.pb.h>
#include <Message2Server.pb.h>
#include <concurrent_queue.hpp>
#include "message.hpp"

// third-party libraries
#include <google/protobuf/message.h>
#include <HPSocket.h>
#include <SocketInterface.h>

// C++ standard libraries
#include <variant> 
#include <type_traits>
#include <functional>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <memory>

/// <summary>
/// 通信组件，只定义了基础操作
/// </summary>
class ClientCommunication final: public CTcpClientListener
{
private:
    /// <summary>
    /// 信息流的最大长度
    /// </summary>
    static const constexpr int max_length = 1000;

    /// <summary>
    /// tcp-client组件
    /// </summary>
    CTcpPackClientPtr pclient;

    /// <summary>
    /// 可以理解为委托，即下面OnConnect、Onclose、Onreceive所真正需要执行的回调函数可由函数对象自定义
    /// </summary>
    std::function<void()> __OnConnect;
    std::function<void(pointer_m2c)> __OnReceive;
    std::function<void()> __OnClose;

    /// <summary>
    /// event
    /// </summary>
    virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID) override;
    virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) override;
    virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) override;

public:
    /// <summary>
    /// 委托的设置
    /// </summary>
    /// <param name=""></param>
    void set_OnConnect(std::function<void()>);
    void set_OnReceive(std::function<void(pointer_m2c)>);
    void set_OnClose(std::function<void()>);

    /// <summary>
    /// 构造函数
    /// </summary>
    ClientCommunication() {};

    /// <summary>
    /// 连接server
    /// </summary>
    /// <param name="address"></param>
    /// <param name="part"></param>
    /// <returns></returns>
    bool Connect(const char* address, uint16_t part);

    /// <summary>
    /// 发送信息
    /// </summary>
    /// <param name="m2s"></param>
    void Send(const Protobuf::MessageToServer& m2s);

    /// <summary>
    /// 终止client
    /// </summary>
    void Stop();
};

/// <summary>
/// 加入了对信息的多线程处理。
/// </summary>
class MultiThreadClientCommunication
{
private:
    /// <summary>
    /// 每收到一次message2client就允许发50条消息
    /// </summary>
    constexpr static inline int Limit = 50; 

    /// <summary>
    /// 消息的计数器
    /// </summary>
    std::atomic_int counter = 0;

    /// <summary>
    /// 在没有消息时线程阻塞节约资源
    /// </summary>
    std::atomic_bool blocking = false;

    /// <summary>
    /// 是否循环运行
    /// </summary>
    std::atomic_bool loop = true;

    std::thread tPM;
    std::mutex mtx;
    std::condition_variable cv;

    thuai::concurrency::concurrent_queue<pointer_m2c> queue;

    ClientCommunication capi;
    
    /// <summary>
    /// 相比于ClientCommunication的委托新增了一些功能
    /// </summary>
    std::function<void(pointer_m2c)> __AdvancedOnReceive;
    std::function<void()> __AdvancedOnClose;

    /// <summary>
    /// 唤醒一个线程
    /// </summary>
    void UnBlock();

    /// <summary>
    /// 处理信息（需要单开一个子线程）
    /// </summary>
    void ProcessMessage();

public:
    MultiThreadClientCommunication() {}
    ~MultiThreadClientCommunication();

    void set_AdvancedOnReceive(std::function<void(pointer_m2c)>);

    /// <summary>
    /// 连接Server，成功返回真且启动PM线程，否则返回假且不启动线程
    /// </summary>
    /// <param name="address"></param>
    /// <param name="port"></param>
    /// <returns></returns>
    bool Start(const char* address, uint16_t port);

    /// <summary>
    /// 发送信息
    /// </summary>
    /// <param name="m2s"></param>
    /// <returns></returns>
    bool Send(const Protobuf::MessageToServer& m2s);

    /// <summary>
    /// 结束线程
    /// </summary>
    void Join();
};


#endif // !CAPI_H


