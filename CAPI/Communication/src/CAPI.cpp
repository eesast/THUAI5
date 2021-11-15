#include "CAPI.h"
#include <iostream>
#include <thread>
#include <chrono>

CAPI::CAPI(std::function<void()> onconnect, std::function<void()> onclose, std::function<void(pointer_m2c)> onreceive) : __OnConnect(onconnect), __OnReceive(onreceive), __OnClose(onclose), pclient(this) {}

EnHandleResult CAPI::OnConnect(ITcpClient* pSender, CONNID dwConnID)
{
    __OnConnect();
    return HR_OK;
}

/// <summary>
/// 收发字节流前四比特是type(little endian)，随后是Message的字节流
/// </summary>
/// <param name="pSender"></param>
/// <param name="dwConnID"></param>
/// <param name="pData"></param>
/// <param name="iLength"></param>
/// <returns></returns>
EnHandleResult CAPI::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
    pointer_m2c pm2c = GameMessage::Deserialize(pData, iLength);
	__OnReceive(std::move(pm2c));
	return HR_OK;
}

EnHandleResult CAPI::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
    __OnClose();
    return HR_OK;
}

bool CAPI::Connect(const char* address, uint16_t port)
{
	std::cout << "Connecting......" << std::endl;
	while (!pclient->IsConnected())
	{
		if (!pclient->Start(address,port))
		{
			std::cout << "Failed to connect with the server" << std::endl;
			return false;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	return true;
}

void CAPI::Send(const Protobuf::MessageToServer& m2s)
{
	unsigned char data[max_length];
	int msgSize = m2s.ByteSizeLong();
	GameMessage::Serialize(data,m2s);
	if (!pclient->Send(data, msgSize))
	{
		std::cout << "Failed to send the message. Error code:";
		std::cout << pclient->GetLastError() << std::endl;
	}
}

void CAPI::Stop()
{
	if (!pclient->HasStarted()) return;
	if (!pclient->Stop())
	{
		std::cout << "The client wasn`t stopped. Error code:";
		std::cout << pclient->GetLastError() << std::endl;
	}
}


ClientCommunication::ClientCommunication(std::function<void(pointer_m2c)> OnReceive, std::function<void() > OnConnect, std::function<void() > CloseHandler = nullptr) :__OnReceive(OnReceive), __OnClose(CloseHandler), 
capi(OnConnect, 
	[this]() 
	{
		std::cout << "Connect between server and client was closed." << std::endl;
		loop = false;
		UnBlock();
		if (__OnClose) // 这里的回调函数是可选的
		{
			__OnClose();
		}
	}, 
	[this](pointer_m2c pm2c) 
	{
		// 收到MessageToClient时将计数器清空
		if (pm2c.index() == 0)
		{
			counter = 0;
		}
		queue.push(pm2c);
		UnBlock(); // 唤醒线程
	})
{}

void ClientCommunication::UnBlock()
{
	blocking = false;
	cv.notify_one(); // 唤醒一个线程
}

void ClientCommunication::ProcessMessage()
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
		if (!queue.try_pop(pm2c)) // 接收信息，若获取失败则跳过处理信息的部分
		{
			std::cout << "failed to pop the message" << std::endl;
			continue; // 避免处理空信息
		}
		__OnReceive(std::move(pm2c)); // 处理信息
	}
}

bool ClientCommunication::Start(const char* address, uint16_t port)
{
	tPM = std::thread(&ClientCommunication::ProcessMessage, this); // 单开一个线程处理信息
	if (!capi.Connect(address, port))
	{
		std::cout << "unable to connect to server!" << std::endl;
		loop = false; // 终止循环
		UnBlock(); // 若该线程阻塞则迅速解锁
		tPM.join(); // 等待该子线程执行完才可以执行母线程
		// 综合来看，上面两句话的含义是如若无法连接，则迅速把无用的tPM线程执行掉以后再退出
		return false;
	}
	return true;
}

bool ClientCommunication::Send(const Protobuf::MessageToServer& m2s)
{
	if (counter >= Limit)
	{
		return false;
	}
	capi.Send(m2s);
	counter++;
	return true;
}

void ClientCommunication::Join()
{
	capi.Stop();
	loop = false;
	UnBlock(); // 唤醒当前休眠的tPM线程
	if (tPM.joinable())
	{
		tPM.join(); // 等待其执行完毕
	}
}

ClientCommunication::~ClientCommunication()
{
	Join();
}