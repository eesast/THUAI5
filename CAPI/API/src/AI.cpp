#include <random>
#include "../include/AI.h"

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;


// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Invisible;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::PowerBank;

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

void AI::play(IAPI& api)
{
	//获得信息
	auto self=api.GetSelfInfo();
	//输出个人CD
	std::cout<<self->CD<<std::endl;

	//e为一个方向角度
	double e=3.1415926*0.5;
	//向e方向攻击
	api.Attack(e);

	//cell转化为grid
	uint32_t gridnumbers=api.CellToGrid(5);
	//grid转化为cell
	uint32_t cellnumbers=api.GridToCell(gridnumbers);

	//获取场上信号干扰器
	auto SignalJammers=api.GetSignalJammers();
	//输出第一个信号干扰器属于的队伍ID和数目
	if (SignalJammers.size()!=0)
	{
		std::cout << SignalJammers[0]->parentTeamID << std::endl;
		std::cout << SignalJammers.size() << std::endl;
	}
	//!!!!!! 注意：千万不要写出这样的代码：
	//! auto SignalJammers=api.GetSignalJammers();
	//! std::cout << SignalJammers[0]->parentTeamID << std::endl;
	//! std::cout << SignalJammers.size() << std::endl;
	//! 当场上没有任何信号干扰器时，SignalJammers的长度将为0，如果此时直接调用SignalJammers[0]->parentTeamID，将会产生空指针，导致程序崩溃！
	//! 编写C++程序的一个好习惯：随时判断一段代码是否会产生空指针，并写出严密的判空机制。

	//获取场上机器人的信息
	auto Robots=api.GetRobots();
	//输出第一个机器人的攻击范围
	if (Robots.size() != 0)
	{
		std::cout << Robots[0]->attackRange << std::endl;
	}

	//获取游戏已经进行的帧数
	auto Count=api.GetFrameCount();

	//获取（25，25）处地点类型
	auto PlaceType=api.GetPlaceType(25,25);
	//判断地点类型是否为CPUFactory
	auto cpufactory=THUAI5::PlaceType::CPUFactory;
	std::cout<<(PlaceType==cpufactory)<<std::endl;

	//返回一个数组，存储了场上所有玩家的GUID（全局唯一标识符）
	auto guids=api.GetPlayerGUIDs();

	//获取场上的道具信息
	auto Props=api.GetProps();
	//判断一号道具是否为cpu
	auto cpu=THUAI5::PropType::CPU;
	if (Props.size() != 0)
	{
		std::cout << (Props[0]->type == cpu) << std::endl;
	}
	
	//返回队伍得分
	auto score=api.GetTeamScore();

	//捡cpu
	api.Pick(cpu);

	//移动
	uint32_t movespeed=300;
	api.MoveDown(3000/movespeed);
	api.MoveLeft(3000/movespeed);
	api.MoveRight(3000/movespeed);
	api.MoveUp(3000/movespeed);
	api.MovePlayer(3000/movespeed,e);

	//asynchronous 为 true 的情况下，选手可以调用此函数，阻塞当前线程，直到下一次消息更新时继续运行。
	api.Wait();

	//查看是否有消息，有则接收消息
	if(api.MessageAvailable())
	{
		auto message=api.TryGetMessage();
	}
	//想2号玩家发消息
	int toPlayerID=2;
	api.Send(toPlayerID,"this is an example");

	//玩家使用CPU,扔CPU/道具，使用技能
	api.UseCPU(self->cpuNum);
	api.ThrowCPU(3000/movespeed,e,0);
	api.ThrowProp(3000/movespeed,e);
	api.UseCommonSkill();
	
	/*
	#获取 thuai5 属性type [code]:

	auto AP=THUAI5::BuffType::AP;
	auto PowerBank=THUAI5::HardwareType::PowerBank;
	auto Wall=THUAI5::PlaceType::Wall;
	auto Booster=THUAI5::PropType::Booster;
	auto Circle=THUAI5::ShapeType::Circle;
	auto FastJammer=THUAI5::SignalJammerType::FastJammer;
	auto PowerEmission=THUAI5::SoftwareType::PowerEmission;

	#Get the (i+1)th Prop/robots and self info [code]:

	auto Robots=api.GetRobots();
	auto Props=api.GetProps();
	auto self=api.GetSelfInfo();
	
	you will see the list ,just choose what you need to use.

	auto Props->
	auto self->
	auto Robots[i]->

	*/
}
