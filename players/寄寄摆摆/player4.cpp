#include<cstdio>
#include <random>
#include "../include/AI.h"
#include <queue>
#include<cstring>
#include<map>
#include<vector>
#include <thread>
#include<chrono>
using namespace std::literals::chrono_literals;

const static int M = 50;
const static int N = 1000;
const static double PI = 3.14159265358979323846;


/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;

//MASTER
// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Invisible;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::EnergyConvert;

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

int map[50][50] = { 0 };
//定义安全窝点
int32_t safe_x, safe_y;
//记录上一帧的寻路目标CPU（也可能是道具）的GUID值
static uint64_t TempGuid = 114514;
//记录上一帧的位置
static uint32_t TempX = 0, TempY = 0;
//虚拟道具
const THUAI5::Prop VirtualProp = { 0,0,0,0,0,THUAI5::PropType::NullPropType,THUAI5::PlaceType::Land,false };
//计数器
int32_t cnt=0;
//敌方teamID
int32_t enemyTeamID;

//距离函数
double d(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	int64_t t1 = (int64_t)(x1 - x2) * (int64_t)(x1 - x2) + (int64_t)(y1 - y2) * (int64_t)(y1 - y2);
	double t2 = pow((double)t1, 0.5);
	return t2;
}

void AI::play(IAPI& api)
{
	//设定帧数
	std::ios::sync_with_stdio(false);

	//获取自身信息
	auto self = api.GetSelfInfo();
	//获取场上所有可见的道具的信息
	auto Props = api.GetProps();
	//获取场上其他所有机器人的信息
	auto robots = api.GetRobots();
	//获取场上其他友军的信息
	std::vector<std::shared_ptr<const THUAI5::Robot>> friends;

	for (int i = 0; i < (int)robots.size(); ++i)
	{
		if (robots[i]->teamID == self->teamID)
		{
			friends.emplace_back(robots[i]);
		}
	}

	//获取场上所有可见的子弹的信息
	auto jammers = api.GetSignalJammers();
	//获取场上所有可见的敌军子弹的信息
	std::vector<std::shared_ptr<const THUAI5::SignalJammer>> enemyJammers;

	for (int i = 0; i < (int)jammers.size(); ++i)
	{
		if (jammers[i]->parentTeamID != self->teamID)
		{
			enemyJammers.emplace_back(jammers[i]);
		}
	}

	if (api.GetFrameCount() == 1)
	{
		for(int i=0;i<50;++i)
			for (int j = 0; j < 50; ++j)
			{
				if(api.GetPlaceType(i, j)==THUAI5::PlaceType::Wall)
				map[i][j] = 1;
			}
		if (self->teamID == 0)
		{
			enemyTeamID = 1;
			for (int i = 0; i < 50; ++i)
				for (int j = 0; j < 50; ++j)
				{
					if (api.GetPlaceType(i, j) == THUAI5::PlaceType::BirthPlace1)
					{
						safe_x = i;
						safe_y = j;
						break;
					}
				}
		}
		else if (self->teamID == 1)
		{
			enemyTeamID = 0;
			for (int i = 0; i < 50; ++i)
				for (int j = 0; j < 50; ++j)
				{
					if (api.GetPlaceType(i, j) == THUAI5::PlaceType::BirthPlace5)
					{
						safe_x = i;
						safe_y = j;
						break;
					}
				}
		}
	}

	//自动攻击
	auto shoot = [&api, &self, &robots]()
	{
		std::vector<THUAI5::Robot> enemy;
		for (int i = 0; i < (int)robots.size(); ++i)
		{
			if (robots[i]->teamID != self->teamID)
			{
				enemy.emplace_back(*robots[i]);
			}
		}
		for (int j = 0; j < (int)enemy.size(); ++j)
		{
			if (d(enemy[j].x, enemy[j].y, self->x, self->y) <= self->attackRange-2000.0 && self->signalJammerNum > 1)
			{
				if (self->timeUntilCommonSkillAvailable == 0) { api.UseCommonSkill(); }
				for (int i = 0;i< (int)self->signalJammerNum ;++i)
					api.Attack(atan2(enemy[j].y - self->y, enemy[j].x - self->x));
			}
		}
		std::vector<THUAI5::Robot>().swap(enemy);
	};

	//定义一种BFS寻路法
	static int book[M][M] = { 0 };			//动态记录地图信息
	static std::vector<int> MoveOrder = { 0 };			//记录由BFS给出的序列
	auto vectorBFSLoca = [&api, &self](int32_t target_x, int32_t target_y)		//由BFS算法生成寻路序列，向某坐标点移动
	{
		int nx[4][2] = { 0,1,1,0,0,-1,-1,0 };
		int32_t d_x = target_x / N, d_y = target_y / N;
		struct node
		{
			int x;
			int y;
			///用步数当坐标的下标
			int step;
			///记录每次走过的坐标
			int a[100];
		};
		int i, s = 0;
		node u, v, w;
		std::queue<node> q;
		memset(book, 0, sizeof(book));
		u.x = self->x / N; u.y = self->y / N; u.step = 0;
		u.a[0] = 0;
		///起点坐标
		book[u.x][u.y] = 1;
		q.push(u);
		while (!q.empty())
		{
			v = q.front();
			q.pop();
			if (v.x == d_x && v.y == d_y)///找到终点
			{
				///将到终点的这个结构体所存的路径输出
				for (i = v.step; i >= 0; --i)
				{
					if (v.a[i] == 4)
					{
						MoveOrder.emplace_back(4);
					}
					if (v.a[i] == 1)
					{
						MoveOrder.emplace_back(1);
					}
					if (v.a[i] == 2)
					{
						MoveOrder.emplace_back(2);
					}
					if (v.a[i] == 3)
					{
						MoveOrder.emplace_back(3);
					}
				}
				break;
			}
			///继承这个结构体前面走过的坐标
			///实质上就是记录上一步的坐标
			///（到最后就把整个路径的坐标记录了下来）
			w = v;
			for (i = 0; i < 4; i++)
			{
				w.x = v.x + nx[i][0];
				w.y = v.y + nx[i][1];
				if (w.x >= 0 && w.x < 50 && w.y >= 0 && w.y < 50 && book[w.x][w.y] == 0 && map[w.x][w.y] != 1)
				{
					book[w.x][w.y] = 1;
					w.step = v.step + 1;
					///步数加1的同时记录坐标
					w.a[w.step] = i + 1;
					q.push(w);
				}
			}
		}
		return 0;
	};
	auto vectorBFSProp = [&api, &self](THUAI5::Prop destinationCPU)		//由BFS算法生成寻路序列，向某道具移动
	{
		int nx[4][2] = { 0,1,1,0,0,-1,-1,0 };
		int32_t d_x = destinationCPU.x / N, d_y = destinationCPU.y / N;
		struct node
		{
			int x;
			int y;
			///用步数当坐标的下标
			int step;
			///记录每次走过的坐标
			int a[100];
		};
		int i, s = 0;
		node u, v, w;
		std::queue<node> q;
		memset(book, 0, sizeof(book));
		u.x = self->x / N; u.y = self->y / N; u.step = 0;
		u.a[0] = 0;
		///起点坐标
		book[u.x][u.y] = 1;
		q.push(u);
		while (!q.empty())
		{
			v = q.front();
			q.pop();
			if (v.x == d_x && v.y == d_y)///找到终点
			{
				///将到终点的这个结构体所存的路径输出
				for (i = v.step; i >= 0; --i)
				{
					if (v.a[i] == 4)
					{
						MoveOrder.emplace_back(4);
					}
					if (v.a[i] == 1)
					{
						MoveOrder.emplace_back(1);
					}
					if (v.a[i] == 2)
					{
						MoveOrder.emplace_back(2);
					}
					if (v.a[i] == 3)
					{
						MoveOrder.emplace_back(3);
					}
				}
				break;
			}
			///继承这个结构体前面走过的坐标
			///实质上就是记录上一步的坐标
			///（到最后就把整个路径的坐标记录了下来）
			w = v;
			for (i = 0; i < 4; i++)
			{
				w.x = v.x + nx[i][0];
				w.y = v.y + nx[i][1];
				if (w.x >= 0 && w.x < 50 && w.y >= 0 && w.y < 50 && book[w.x][w.y] == 0 && map[w.x][w.y] != 1)
				{
					book[w.x][w.y] = 1;
					w.step = v.step + 1;
					///步数加1的同时记录坐标
					w.a[w.step] = i + 1;
					q.push(w);
				}
			}
		}
		return 0;
	};
	auto MoveInBFS = [&api, &self, &vectorBFSProp](int velocity = 70)		//消耗掉BFS寻路序列的最后一个元素，并作出相应的移动
	{
		switch (MoveOrder[(int)MoveOrder.size() - 1])
		{
		case 4:
		{
			api.MoveUp(velocity); break;
			//std::this_thread::sleep_for(std::chrono::milliseconds(velocity)); break;
		}
		case 1:
		{
			api.MoveRight(velocity); break;
			//std::this_thread::sleep_for(std::chrono::milliseconds(velocity)); break;
		}
		case 2:
		{
			api.MoveDown(velocity); break;
			//std::this_thread::sleep_for(std::chrono::milliseconds(velocity)); break;
		}
		case 3:
		{
			api.MoveLeft(velocity); break;
			//std::this_thread::sleep_for(std::chrono::milliseconds(velocity)); break;
		}
		default: break;
		}
		MoveOrder.pop_back();
	};

	//获取距离最近的CPU信息，若场上无CPU则返回距离最近的道具信息
	auto nearestCPU = [&api, &self](std::vector<std::shared_ptr<const THUAI5::Prop>>& Props)->THUAI5::Prop
	{
		if ((int)Props.size() > 0)
		{		
			std::vector<std::shared_ptr<const THUAI5::Prop>> PropsCPU{};				//用以存储场上所有可见的CPU的信息
			for (int i = 0; i < (int)Props.size(); ++i)								//录入场上所有可见的CPU的信息
			{
				if (Props[i]->type == THUAI5::PropType::CPU)
				{
					if (Props[i]->place==THUAI5::PlaceType::BirthPlace1 || Props[i]->place == THUAI5::PlaceType::BirthPlace2|| 
						Props[i]->place == THUAI5::PlaceType::BirthPlace3|| Props[i]->place == THUAI5::PlaceType::BirthPlace4||
						Props[i]->place == THUAI5::PlaceType::BirthPlace5|| Props[i]->place == THUAI5::PlaceType::BirthPlace6||
						Props[i]->place == THUAI5::PlaceType::BirthPlace7|| Props[i]->place == THUAI5::PlaceType::BirthPlace8)
					{
						return *Props[i];
					}
					PropsCPU.emplace_back(Props[i]);
				}
			}
			if ((int)PropsCPU.size() > 0)							//若场上的确存在可见的CPU，则寻找出其中最近的那个
			{
				THUAI5::Prop TempCPU = *PropsCPU[0];
				for (int i = 1; i < (int)PropsCPU.size(); ++i)
				{
					int32_t d_original = d(self->x, self->y, TempCPU.x, TempCPU.y);
					int32_t d_new = d(self->x, self->y, PropsCPU[i]->x, PropsCPU[i]->y);
					if (d_new < d_original)
					{
						TempCPU = *PropsCPU[i];
					}
				}
				return (const THUAI5::Prop)TempCPU;
			}
			else
			{
				THUAI5::Prop TempProp = *Props[0];
				for (int i = 1; i < (int)Props.size(); ++i)
				{
					int32_t d_original = d(self->x, self->y, TempProp.x, TempProp.y);
					int32_t d_new = d(self->x, self->y, Props[i]->x, Props[i]->y);
					if (d_new < d_original)
					{
						TempProp = *Props[i];
					}
				}
				return (const THUAI5::Prop)TempProp;
			}
		}
		return VirtualProp;
	};

	//返回距离最近的友军
	auto nearestFriend = [&api, &self](std::vector<std::shared_ptr<const THUAI5::Robot>>& friends)
	{
		std::shared_ptr<const THUAI5::Robot> temp = friends[0];
		for (int i = 0; i < (int)friends.size(); ++i)
		{
			if (friends[i]->teamID == self->teamID)
			{
				if (d(self->x, self->y, friends[i]->x, friends[i]->y) < d(self->x, self->y, temp->x, temp->y))
					temp = friends[i];
			}
		}
		return temp;
	};

	//返回距离最近的敌军子弹
	auto nearestJammer = [&api, &self](std::vector<std::shared_ptr<const THUAI5::SignalJammer>>& enemyJammers)
	{
		if ((int)enemyJammers.size() > 0)
		{
			std::shared_ptr<const THUAI5::SignalJammer> temp = enemyJammers[0];
			for (int i = 0; i < (int)enemyJammers.size(); ++i)
			{
				if (d(self->x, self->y, enemyJammers[i]->x, enemyJammers[i]->y) < d(self->x, self->y, temp->x, temp->y))
					temp = enemyJammers[i];
			}
			return temp;
		}
		const THUAI5::SignalJammer VirtualJammer = { 256,256,1919810,enemyTeamID };
		std::shared_ptr<const THUAI5::SignalJammer> VirtualJammerPtr = std::make_shared<const THUAI5::SignalJammer>(VirtualJammer);
		return VirtualJammerPtr;
	};




	if (api.GetFrameCount()>1)
	{
		if ((int)Props.size() > 0)
		{
			//读取BFS寻路序列，并作出相应的移动
			if ((int)MoveOrder.size() <= 1)			//在序列弹药消耗完时，填充BFS寻路序列
			{	
				TempGuid = nearestCPU(Props).guid;
				vectorBFSProp(nearestCPU(Props));
			}
			if (TempGuid != nearestCPU(Props).guid)			//检测距离最近的CPU（也可能是道具）是否改变，如果改变则重定向
			{
				MoveOrder.clear();
				TempGuid = nearestCPU(Props).guid;
				vectorBFSProp(nearestCPU(Props));
			}

			if (cnt >= 5)
			{
				api.MovePlayer(2 * 1000 * 1000 / self->speed, 2.0 * PI * (rand() / double(RAND_MAX)));
				cnt = 0;
			}
			else if (d(self->x, self->y, nearestJammer(enemyJammers)->x, nearestJammer(enemyJammers)->y) <= 6)		//躲子弹
			{
				api.MovePlayer(1000 * 1000 / self->speed, atan2((double)(self->y - nearestJammer(enemyJammers)->y), (double)(self->x - nearestJammer(enemyJammers)->x)));
			}
			else { MoveInBFS(1000 * 1000 / self->speed); }		//消耗序列中的一发弹药，进行BFS寻路移动


			//发起攻击
			shoot();

			self = api.GetSelfInfo();

			if (self->prop != THUAI5::PropType::NullPropType)
			{
				if (self->prop != THUAI5::PropType::CPU)
				{
					api.UseProp();
				}
				else if (self->prop == THUAI5::PropType::CPU)
				{
					if (self->life <= 4500 || self->cpuNum >= 25)
					{
						api.UseCPU(self->cpuNum);
					}
				}
			}

			api.Pick(THUAI5::PropType::CPU);
			if (self->prop == THUAI5::PropType::NullPropType) { api.Pick(THUAI5::PropType::Battery); }
			if (self->prop == THUAI5::PropType::NullPropType) { api.Pick(THUAI5::PropType::Shield); }
			if (self->prop == THUAI5::PropType::NullPropType) { api.Pick(THUAI5::PropType::Booster); }
			if (self->prop == THUAI5::PropType::NullPropType) { api.Pick(THUAI5::PropType::ShieldBreaker); }

			auto postSelf = api.GetSelfInfo();
			TempX = postSelf->x, TempY = postSelf->y;
			if (d(postSelf->x, postSelf->y, TempX, TempY) == 0)		//卡墙
			{
				cnt++;
			}
		}
		else
		{
			if (cnt >= 5)		//卡墙
			{
				api.MovePlayer(2 * 1000 * 1000 / self->speed, 2.0 * PI * (rand() / double(RAND_MAX)));
				cnt = 0;
			}
			else if (d(self->x, self->y, nearestJammer(enemyJammers)->x, nearestJammer(enemyJammers)->y) <= 6)		//躲子弹
			{
				api.MovePlayer(1000 * 1000 / self->speed, atan2((double)(self->y - nearestJammer(enemyJammers)->y), (double)(self->x - nearestJammer(enemyJammers)->x)));
			}
			else { api.MovePlayer(1000 * 1000 / self->speed, 2.0 * PI * (rand() / double(RAND_MAX))); }

			//发起攻击
			shoot();

			self = api.GetSelfInfo();

			if (self->prop != THUAI5::PropType::NullPropType)
			{
				if (self->prop != THUAI5::PropType::CPU)
				{
					api.UseProp();
				}
				else if (self->prop == THUAI5::PropType::CPU)
				{
					if ( self->life <= 4500 || self->cpuNum>=25 )
					{
						api.UseCPU(self->cpuNum);
					}
				}
			}

			api.Pick(THUAI5::PropType::CPU);
			if (self->prop == THUAI5::PropType::NullPropType) { api.Pick(THUAI5::PropType::Battery); }
			if (self->prop == THUAI5::PropType::NullPropType) { api.Pick(THUAI5::PropType::Shield); }
			if (self->prop == THUAI5::PropType::NullPropType) { api.Pick(THUAI5::PropType::Booster); }
			if (self->prop == THUAI5::PropType::NullPropType) { api.Pick(THUAI5::PropType::ShieldBreaker); }

			auto postSelf = api.GetSelfInfo();
			TempX = postSelf->x, TempY = postSelf->y;
			if (d(postSelf->x, postSelf->y, TempX, TempY) == 0)		//卡墙
			{
				cnt++;
			}
		}
	}
}