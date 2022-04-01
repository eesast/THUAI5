#pragma once
#ifndef CAPI_H
#define CAPI_H

// proto files
#include <Message2Clients.pb.h>
#include <Message2Server.pb.h>
#include <concurrent_queue.hpp>

// third-party libraries
#include <google/protobuf/message.h>
#include <HPSocket.h>
#include <SocketInterface.h>
#include <HPSocket-SSL.h>
#include <HPTypeDef.h>

// C++ standard libraries
#include <variant> 
#include <type_traits>
#include <functional>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <memory>

// #define COMMUNICATION_DEBUG

// 首先解释一下ClientCommunication和MultiThreadClientCommunication之间的联系：
// 1.ClientCommunication侧重于HPSOCKET中委托和事件的构造
// 2.MultiThreadClientCommunication侧重于对多线程的处理
// 3.ClientCommunication依赖于MultiThreadClientCommunication的接口，而MultiThreadClientCommunication依赖于ClientCommunication的实体
// 在THUAI4中，从MultiThreadClientCommunication(THUAI4中名为Communication)构造ClientCommunication(THUAI4中名为CAPI)中，又用到了大量的回调函数
// 所以不妨将ClientCommunication依赖于MultiThreadClientCommunication的部分抽取为一个接口ICommunication
using pointer_m2c = std::variant<std::shared_ptr<Protobuf::MessageToClient>, std::shared_ptr<Protobuf::MessageToOneClient>, std::nullptr_t>; // 类型安全的联合体
#define TYPEM2C 0
#define TYPEM2OC 1
#define TYPEM2I 2

/// <summary>
/// 信息的委托接口，是指ClientCommunication依赖于MultiThreadClientCommunication的部分
/// </summary>
class ICommunication
{
public:
    virtual void OnConnect() = 0;
    virtual void OnReceive(pointer_m2c p2M) = 0;
    virtual void OnClose() = 0;
};
 

/// <summary>
/// 信息的订阅接口，是指MultiThreadCommunication依赖于Logic的部分
/// </summary>
class ISubscripter
{
public:
    [[nodiscard]] virtual Protobuf::MessageToServer OnConnect() = 0;
    virtual void OnReceive(pointer_m2c p2M) = 0;
    virtual void OnClose() = 0;
};


/// <summary>
/// 通信组件，只定义了基础操作
/// </summary>
class ClientCommunication final: public CTcpClientListener
{
private:
    ICommunication& comm;

    /// <summary>
    /// 信息流的最大长度
    /// </summary>
    static const constexpr int max_length = 1000;

    /// <summary>
    /// tcp-client组件
    /// </summary>
    CTcpPackClientPtr pclient;

    /// <summary>
    /// event
    /// </summary>
    virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID) override;
    virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) override;
    virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) override;

public:

    /// <summary>
    /// 构造函数
    /// </summary>
    ClientCommunication(ICommunication& comm):comm(comm), pclient(this) {}

    /// <summary>
    /// 连接server
    /// </summary>
    /// <param name="address">ip地址</param>
    /// <param name="part">端口</param>
    /// <returns></returns>
    bool Connect(const char* address, uint16_t port);

    /// <summary>
    /// 发送信息
    /// </summary>
    /// <param name="m2s">需要发送的信息</param>
    void Send(const Protobuf::MessageToServer& m2s);

    /// <summary>
    /// 终止client
    /// </summary>
    void Stop();
};

/// <summary>
/// 加入了对信息的多线程处理。
/// </summary>
class MultiThreadClientCommunication:public ICommunication
{
private:
    ISubscripter& subscripter;

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

    std::unique_ptr<ClientCommunication> capi;

    /// <summary>
    /// 唤醒一个线程
    /// </summary>
    void UnBlock();

    /// <summary>
    /// 处理信息（需要单开一个子线程）
    /// </summary>
    void ProcessMessageQueue();

public:
    MultiThreadClientCommunication(ISubscripter& subscripter) :subscripter(subscripter) {}
    ~MultiThreadClientCommunication() {};

    /// <summary>
    /// 在MultiThreadClientCommunication对象构造完毕后设置std::unique_ptr/<ClientCommunication/> capi的内容
    /// </summary>
    void init();

    /// <summary>
    /// 连接Server，成功返回真且启动PM线程，否则返回假且不启动线程
    /// </summary>
    /// <param name="address">ip地址</param>
    /// <param name="port">端口</param>
    /// <returns></returns>
    bool Start(const char* address, uint16_t port);

    /// <summary>
    /// 发送信息
    /// </summary>
    /// <param name="m2s">需要发送的信息</param>
    /// <returns></returns>
    bool Send(const Protobuf::MessageToServer& m2s);

    /// <summary>
    /// 结束线程
    /// </summary>
    void Join();

    void OnConnect() override;
    void OnReceive(pointer_m2c p2M) override;
    void OnClose() override;
};


#endif // !CAPI_H


