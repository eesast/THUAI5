#include "../include/Communication.h"
#include <iostream>
#include <thread>
#include <chrono>

#ifdef COMMUNICATION_DEBUG
const std::string toHexString(const unsigned char* input, const int datasize)
{
	std::string output;
	char ch[3];
	for (int i = 0; i < datasize; ++i)
	{
		snprintf(ch, 3, "%02x", input[i]);
		output += ch;
	}
	return output;
}
#endif

namespace GameMessage
{
	/// <summary>
	/// 供little endian使用的枚举值
	/// </summary>
	enum class PacketType
	{
		MessageToServer = 0,
		MessageToOneClient = 1,
		MessageToClient = 2,
	};

	static const int MessageToServerNum = int(PacketType::MessageToServer);

	/// <summary>
	/// 反序列化
	/// </summary>
	/// <param name="data">需要进行反序列化的数据</param>
	/// <param name="length">序列长度</param>
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
	void Serialize(unsigned char* data, const Protobuf::MessageToServer& m2s)
	{
		// 设置前四位为数据类型
		for (int i = 0; i < 4; i++)
		{
			data[i] = (int(PacketType::MessageToServer) >> (8 * i)) & 0xff;
		}
		int msg_size = m2s.ByteSizeLong();
#ifdef COMMUNICATION_DEBUG
		std::cout << "the length is " << msg_size + 4 << std::endl;
		std::cout << "the content is " << m2s.DebugString() << std::endl;
#endif
		m2s.SerializeToArray(data + 4, msg_size);
#ifdef COMMUNICATION_DEBUG
        std::cout << "after serialize: " << toHexString(data, msg_size + 4) << std::endl;
#endif
	}
};

EnHandleResult ClientCommunication::OnConnect(ITcpClient* pSender, CONNID dwConnID)
{
    comm.OnConnect();
    return HR_OK; 
}

EnHandleResult ClientCommunication::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
    pointer_m2c pm2c = GameMessage::Deserialize(pData, iLength);
	comm.OnReceive(std::move(pm2c));
	return HR_OK;
}

EnHandleResult ClientCommunication::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
    comm.OnClose();
    return HR_OK;
}

bool ClientCommunication::Connect(const char* address, uint16_t port)
{
	std::cout << "Connecting......" << std::endl;
#ifdef COMMUNICATION_DEBUG
    std::cout << "pclient->IsConnected(): " << pclient->IsConnected() << std::endl;
#endif
	while (!pclient->IsConnected())
	{

        bool is_started = pclient->Start(address,port);
#ifdef COMMUNICATION_DEBUG
        std::cout << "is_started: " << is_started << std::endl;
#endif
		if (!is_started)
		{
			std::cerr << "Failed to connect with the server" << std::endl;
			return false;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
    std::cout << "Successfully connect to the server!" << std::endl;
	return true;
}

void ClientCommunication::Send(const Protobuf::MessageToServer& m2s)
{
	unsigned char data[max_length];
	int msgSize = m2s.ByteSizeLong();
    GameMessage::Serialize(data, m2s);
    if (!pclient->Send(data, 4 + msgSize))
	{
		std::cerr << "Failed to send the message. Error code:";
		std::cerr << pclient->GetLastError() << std::endl;
	}
}

void ClientCommunication::Stop()
{
	if (!pclient->HasStarted()) return;
	if (!pclient->Stop())
	{
		std::cerr << "The client wasn`t stopped. Error code:";
		std::cerr << pclient->GetLastError() << std::endl;
	}
}

void MultiThreadClientCommunication::OnConnect()
{
	auto message = subscripter.OnConnect();
	Send(message);
}

void MultiThreadClientCommunication::OnReceive(pointer_m2c p2M)
{
	if (p2M.index() == TYPEM2C)
	{
		counter = 0;
		queue.emplace(p2M);
		UnBlock();
	}
}

void MultiThreadClientCommunication::OnClose()
{
	std::cout << "Connection was closed." << std::endl;
	loop = false;
	UnBlock();
	subscripter.OnClose();
}

void MultiThreadClientCommunication::init()
{
	capi = std::make_unique<ClientCommunication>(*this);
}

void MultiThreadClientCommunication::UnBlock()
{
	{
		std::lock_guard<std::mutex> lock(mtx);
		blocking = false;
	}
	cv.notify_one(); // 唤醒一个线程
}

void MultiThreadClientCommunication::ProcessMessageQueue()
{
	pointer_m2c pm2c;
	while (loop)
	{
		{
			std::unique_lock<std::mutex> lck(mtx); // 本质上和lock_guard类似，但cv类必须用unique_lock
			blocking = queue.empty(); // 如果消息队列为空就阻塞线程
			cv.wait(lck, [this]() {return !blocking; }); // 翻译一下：1.只有当blocking==true时调用wait()时才会阻塞线程 2.在收到notify通知时只有blocking==false时才会解除阻塞
			// 等价于
			/*
			while([this](){return blocking;})
			{
				cv.wait(lck);
			}
			*/
		}
		if (auto pm2c = queue.try_pop(); !pm2c.has_value()) // 接收信息，若获取失败则跳过处理信息的部分
		{
			std::cerr << "failed to pop the message!" << std::endl;
			continue; // 避免处理空信息
		}
		else subscripter.OnReceive(std::move(pm2c.value())); // 处理信息
	}
}

bool MultiThreadClientCommunication::Start(const char* address, uint16_t port)
{
	tPM = std::thread(&MultiThreadClientCommunication::ProcessMessageQueue, this); // 单开一个线程处理信息
	if (!capi->Connect(address, port))
	{
		std::cerr << "unable to connect to server!" << std::endl;
		loop = false; // 终止循环
		UnBlock(); // 若该线程阻塞则迅速解锁
		tPM.join(); // 等待该子线程执行完才可以执行母线程
		// 综合来看，上面两句话的含义是如若无法连接，则迅速把无用的tPM线程执行掉以后再退出
		return false;
	}
	return true;
}

bool MultiThreadClientCommunication::Send(const Protobuf::MessageToServer& m2s)
{
	if (counter >= Limit)
	{
		return false;
	}
	capi->Send(m2s);
	counter++;
	return true;
}

void MultiThreadClientCommunication::Join()
{
	capi->Stop();
	loop = false;
	UnBlock(); // 唤醒当前休眠的tPM线程
	if (tPM.joinable())
	{
		tPM.join(); // 等待其执行完毕
	}
}


