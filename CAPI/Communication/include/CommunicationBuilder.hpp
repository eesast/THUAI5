#pragma once
#ifndef COMMUNICATIONBUILDER_HPP
#define COMMUNICATIONBUILDER_HPP

#include <memory>
#include "Communication.h"

/// <summary>
/// 建造者模式，获取不同类型的communication对象，将logic中的构造和逻辑执行分离，增强代码可读性
/// </summary>
class MultiThreadClientCommunicationBuilder
{
public:
    MultiThreadClientCommunicationBuilder()
    {
        this->comm = std::make_shared<MultiThreadClientCommunication>();
    }

    virtual void set_OnConnect() = 0;
    virtual void set_OnReceive() = 0;
    virtual void set_OnClose() = 0;

    std::shared_ptr<MultiThreadClientCommunication> get_comm() const
    {
        return this->comm;
    }
    virtual ~MultiThreadClientCommunicationBuilder() {}

private:
    std::shared_ptr<MultiThreadClientCommunication> comm;
};

/// <summary>
/// 可以根据后续要求加
/// </summary>
class MultiThreadClientCommunicationBuilder_A : public MultiThreadClientCommunicationBuilder
{
public:
    void set_OnConnect()
    {

    }

    void set_OnReceive()
    {

    }

    void set_OnClose()
    {

    }
};


#endif // COMMUNICATIONBUILDER_HPP