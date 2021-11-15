#pragma once
#ifndef CAPI_H

// proto files
#include "Message2Clients.pb.h"
#include "Message2Server.pb.h"
#include "concurrent_queue.hpp"
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

/// <summary>
/// CAPI通信组件
/// THUAI4使用了模板。但我感觉这种做法只是一种对于不确定信息类的妥协，所以暂时删除了模板
/// </summary>
class CAPI final: public CTcpClientListener
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
    /// delegate
    /// </summary>
    const std::function<void()> __OnConnect;
    const std::function<void(pointer_m2c)> __OnReceive;
    const std::function<void()> __OnClose;

    /// <summary>
    /// event
    /// </summary>
    virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID);
    virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
    virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);

public:
    /// <summary>
    /// 构造函数
    /// </summary>
    /// <param name="onconnect">OnConnect 触发时调用的回调函数</param>
    /// <param name="onclose">OnClose 触发时调用的回调函数</param>
    /// <param name="onreceive">OnReceive 触发时调用的回调函数</param>
    CAPI(std::function<void()> onconnect, std::function<void()> onclose, std::function<void(pointer_m2c)> onreceive);

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
/// 对于CAPI的多线程处理。内含一个CAPI类
/// ClientCommunication这个名字不是特别满意
/// 此处我最大的不解之处在于，这个类才是真正对标C# Communication组件的类。但可以看到这个类对信息做了大量的并发处理，而显然C#类中并没有。试问C#通信组件是如何妥善处理这一点的？
/// </summary>
class ClientCommunication
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

    concurrency::concurrent_queue<pointer_m2c> queue;

    CAPI capi;
    
    /// <summary>
    /// delegate
    /// </summary>
    const std::function<void(pointer_m2c)> __OnReceive;
    const std::function<void()> __OnClose;

    /// <summary>
    /// 唤醒一个线程
    /// </summary>
    void UnBlock();

    /// <summary>
    /// 处理信息（需要单开一个子线程）
    /// </summary>
    void ProcessMessage();

public:
    ClientCommunication(std::function<void(pointer_m2c)> OnReceive, std::function<void() > OnConnect, std::function<void() > CloseHandler = nullptr);
    ~ClientCommunication();

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


