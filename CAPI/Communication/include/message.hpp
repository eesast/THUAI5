#pragma once
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "Message2Clients.pb.h"
#include "Message2Server.pb.h"

#include <google/protobuf/message.h>

#include <variant>
#include <memory>

using pointer_m2c = std::variant<std::shared_ptr<Protobuf::MessageToClient>, std::shared_ptr<Protobuf::MessageToOneClient>, std::shared_ptr<Protobuf::MessageToInitialize>,std::nullptr_t>; // 类型安全的联合体

/// <summary>
/// 供little endian使用的枚举值
/// </summary>
enum class PacketType
{
    MessageToServer = 0,
    MessageToOneClient = 1,
    MessageToClient = 2,
    MessageToInitialize = 3
};

namespace GameMessage 
{
    static const int MessageToServerNum = int(PacketType::MessageToServer);
    /// <summary>
    /// 反序列化
    /// </summary>
    /// <param name="data"></param>
    pointer_m2c Deserialize(const unsigned char* data, int length)
    {  
        /// 解析前四位以获取数据类型
        uint32_t type = (uint32_t)data[0];
        for (int i = 1; i < 4; i++)
        {
            type |= ((uint32_t)data[i]) << (8 * i);
        }
        pointer_m2c pm2c;
        switch (type)
        {
            case int(PacketType::MessageToOneClient) :
            {
                std::shared_ptr<Protobuf::MessageToOneClient> p = std::make_shared<Protobuf::MessageToOneClient>();
                p->ParseFromArray(data + 4, length - 4);
                pm2c = p;
                break;
            }

            case int(PacketType::MessageToClient) :
            {
                std::shared_ptr<Protobuf::MessageToClient> p = std::make_shared<Protobuf::MessageToClient>();
                p->ParseFromArray(data + 4, length - 4);
                pm2c = p;
                break;
            }

            case int(PacketType::MessageToInitialize) :
            {
                std::shared_ptr<Protobuf::MessageToInitialize> p = std::make_shared<Protobuf::MessageToInitialize>();
                p->ParseFromArray(data + 4, length - 4);
                pm2c = p;
                break;
            }
                
            default:
            {
                pm2c = nullptr;
            }
        }
        return pm2c;
    }

    /// <summary>
    /// 序列化
    /// </summary>
    /// <param name="data"></param>
    void Serialize(unsigned char* data,const Protobuf::MessageToServer& m2s)
    {
        // 设置前四位为数据类型
        for (int i = 0; i < 4; i++)
        {
            data[i] = (2 >> (8 * i)) & 0xff;
        }
        int msg_size = m2s.ByteSizeLong();
        m2s.SerializeToArray(data + 4, msg_size);
    }

};

#endif // !MESSAGE_HPP
