#include <random>
#include "../include/AI.h"
#include<queue>
/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;


// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Amplification;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::PowerBank;

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}
//定义非常多的状态（有限状态机）
enum class status
{
	initial,
	watch,
	sorround,
	idle,
	reset,
	index
};



//从这里开始声明很多全局变量
static bool hasInitMap=false;
static bool IHaveArrived = false;
static status BotStatus=status::initial;
static status LastStatus = status::reset;
static THUAI5::PlaceType map[50][50];
static int vis[50][50];
static int desire[50][50];
static int danger[50][50];
static int wantmovethere[50][50];
static int CPUMap[50][50];
static int lastX=0, lastY=0;
static int steps;
static int lastFrameCount = 0;
struct MapNode
{
	int x, y;
};
static MapNode *path[2500];

static int sorroundbeento=0;
static int hasBeenTo = 1;
static MapNode * dest;
static MapNode conplace[50][50];
static MapNode* Sorroundpath[100] = { &conplace[19][20],&conplace[21][31],&conplace[31][29],&conplace[28][17], &conplace[19][19],&conplace[19][30],&conplace[28][26],&conplace[27][18] };
static bool BFSed=false;
static MapNode* pastDest;
//全局变量声明结束

//从这里开始声明自定义函数
//----------------begin------------------
void InitMap(IAPI&);
bool isWalled(int, int, int, int);
bool BFS(int, int, int, int);
void optim(MapNode*, MapNode*);
void Goto(IAPI&, int, int);
bool arriveAt(int, int, int, int);
int CalculateDesire(int, int);
MapNode* generateDesire(IAPI&);
void CalculateDanger();
double DistanceMeasure(int, int, int, int);
double GetAngle(IAPI&, int);
void JammerAttack(IAPI&, int, int);
void ContinueMyPath(std::shared_ptr<const THUAI5::Robot>, IAPI&);
bool setDest(IAPI&, int, int);
void GoToDest(IAPI&);
void defineStatus(IAPI&);
bool EnemyInSight(IAPI&);
bool RangeAt(int, int, int, int);
//-----------------end-------------------


void InitMap(IAPI& api)
{
	int i, j;
	for (i = 0; i < 50; i++)
		for (j = 0; j < 50; j++)
		{
			map[i][j] = api.GetPlaceType(i, j);
			conplace[i][j].x = i;
			conplace[i][j].y = j;
		}
	api.Wait();
}
bool isWalled(int x1, int y1, int x2, int y2)
{
	double k = (y1 - y2) / (x1 - x2);
	int i, j;
	for (i = x1; i < x2; i++)
	{
		for (j = y1; j < y2; j++)
		{
			if (std::abs(k * i - j + y1 - k * x1) / std::sqrt(k * k + 1) <= 10)
			{
				if (map[i][j] == THUAI5::PlaceType::Wall) return true;
			}
		}
		for (j = y2; j < y1; j++)
		{
			if (std::abs(k * i - j + y1 - k * x1) / std::sqrt(k * k + 1) <= 10)
			{
				if (map[i][j] == THUAI5::PlaceType::Wall) return true;
			}
		}
	}
	for (i = x2; i < x1; i++)
	{
		for (j = y1; j < y2; j++)
		{
			if (std::abs(k * i - j + y1 - k * x1) / std::sqrt(k * k + 1) <= 10)
			{
				if (map[i][j] == THUAI5::PlaceType::Wall) return true;
			}
		}
		for (j = y2; j < y1; j++)
		{
			if (std::abs(k * i - j + y1 - k * x1) / std::sqrt(k * k + 1) <= 10)
			{
				if (map[i][j] == THUAI5::PlaceType::Wall) return true;
			}
		}
	}
	//std::printf("%d,%d-%d,%d no wall!\n",x1,y1,x2,y2);
	return false;
}
/// <summary>
/// BFS搜索路径，在这里已经设置了全局变量dest
/// </summary>
/// <param name="x1">输入0~50的格点！！！不是坐标！</param>
/// <param name="y1">输入0~50的格点！！！不是坐标！</param>
/// <param name="x2">输入0~50的格点！！！不是坐标！</param>
/// <param name="y2">输入0~50的格点！！！不是坐标！</param>
/// <returns></returns>
bool BFS(int x1, int y1, int x2, int y2)
{
	if (BFSed) return true;
	std::queue<MapNode*> bfsq;
	bool exist = false;
	std::memset(vis, 0, sizeof(vis));
	MapNode *current_node = &conplace[x1][y1];
	MapNode *dest_node = &conplace[x2][y2];
	bfsq.push(current_node);
	while (!exist)
	{
		int i, j;
		MapNode *t = bfsq.front();
		//std::printf("dealing %d,%d\n", t->x, t->y);
		bfsq.pop();
		for (i = t->x - 1; i < t->x + 2; i++)
		{
			for (j = t->y - 1; j < t->y + 2; j++)
			{
				if (i < 0 || i>49 || j < 0 || j>49 || map[i][j]==THUAI5::PlaceType::Wall
					||map[i-1][j]== THUAI5::PlaceType::Wall||map[i+1][j] == THUAI5::PlaceType::Wall|| 
					map[i][j-1] == THUAI5::PlaceType::Wall ||map[i][j+1] == THUAI5::PlaceType::Wall) continue;
				if (vis[i][j] == 0)
				{
					vis[i][j] = vis[t->x][t->y] + 1;
					bfsq.push(&conplace[i][j]);
				}
				if (t->x == x2 && t->y == y2)
				{
					exist = true;
					//std::printf("=====find!======\n");
					goto existdeal;
				}
			}
		}
	}
	existdeal:
	if (exist)
	{
		int i, j;
		auto tv = vis[dest_node->x][dest_node->y];
		path[tv] = dest_node;
		steps = tv;
		dest = path[tv];
		while (tv>0)
		{
			MapNode *t = path[tv];
			tv--;
			for (i = t->x - 1; i < t->x + 2; i++)
			{
				bool find = false;
				for (j = t->y - 1; j < t->y + 2; j++)
				{
					if (i < 0 || i>49 || j < 0 || j>49|| map[i][j] == THUAI5::PlaceType::Wall) continue;
					if (vis[i][j] == tv)
					{
						path[tv] = &conplace[i][j];
						find = true;
						break;
					}
				}
				if (find) break;
			}
		}
		//for (int i = 1; path[i] != dest;i++) std::printf("->%d,%d", path[i]->x, path[i]->y);
		return true;
	}
	return false;
}
void optim(MapNode* st, MapNode* ed)
{
	//std::printf("st:%d,%d\ned:%d,%d\n", st->x, st->y, ed->x, ed->y);
	if (st == ed) return;
	MapNode* i;
	for (i=st;i<ed;i++)
	{

		if (!isWalled(ed->x, ed->y, i->x, i->y))
		{
			*(i + 1) = *ed;
			dest = ed;
			break;
		}
	}
}
void Goto(IAPI& api, int destX, int destY)
{
	
	//std::printf("goto %d,%d\n", destX, destY);
	//if (map[destX][destY] == THUAI5::PlaceType::Wall) return;
	auto self = api.GetSelfInfo();
	int sx = self->x;
	int sy = self->y;
	//std::printf("-------------%d,%d------------\n", sx, sy);
	auto delta_x = (double)(destX * 1000  - sx);
	auto delta_y = (double)(destY * 1000  - sy);
	//std::printf("-------------%lf,%lf------------\n", delta_x, delta_y);
	double ang = 0;
	//直接走
	ang = atan2(delta_y, delta_x);
	api.MovePlayer(300, ang+(std::rand()%10-10)/10);
}
bool arriveAt(int x1, int y1, int x2, int y2)
{
	if (std::abs(x1 - 1000 * x2 ) + std::abs(y1 - 1000 * y2 ) < 100) return true;
	return false;
}
bool RangeAt(int x1, int y1, int x2, int y2)
{
	if (std::abs(x1 - 1000 * x2 - 500) + std::abs(y1 - 1000 * y2 - 500) < 10000) return true;
	return false;
}
int CalculateDesire(int i, int j)
{
	int ii, jj,res=0;
	for(ii=i-1;ii<i+2;ii++)
		for (jj = j - 1; jj < j + 2; jj++)
		{
			if (ii < 0 || ii>49 || jj < 0 || jj>49) continue;
			switch (map[ii][jj])
			{
			case THUAI5::PlaceType::CPUFactory:
				res += 200;
				break;
			case THUAI5::PlaceType::Wall:
				break;
			case THUAI5::PlaceType::BlindZone1:
				res += 20;
				break;
			case THUAI5::PlaceType::BlindZone2:
				res += 20;
				break;
			case THUAI5::PlaceType::BlindZone3:
				res += 20;
				break;
			default:
				res += 10;
			}
		}
	return res;
}
MapNode* generateDesire(IAPI& api)
{
	auto self = api.GetSelfInfo();
	MapNode* t = &conplace[0][0];
	memset(desire, 0, sizeof(desire));
	auto props = api.GetProps();
	//std::cout << "props are" << props.size()<<std::endl;
	for (int i = 0; i < props.size(); i++)
	{
		switch (props[i]->type) {
		case(THUAI5::PropType::CPU):
			for (int xi = props[i]->x / 1000 - 1; xi < props[i]->x / 1000 + 3; xi++)
			{
				for (int yi = props[i]->y / 1000 - 1; yi < props[i]->y / 1000 + 3; yi++)
				{
					if (xi >= 0 && yi >= 0 && xi < 50 && yi < 50) desire[xi][yi] += 200 * props[i]->size;
				}
			}
			break;
		}
	}
	//std::cout << "getpropsok" << std::endl;
	auto bots = api.GetRobots();
	for (int i = 0; i < bots.size(); i++)
	{
		if (bots[i]->teamID == self->teamID)
		{
			desire[bots[i]->x / 1000][bots[i]->y / 1000] += 100;
		}
	}
	//std::cout << "getbotsok" << std::endl;
	if (hasInitMap)
	{
		for(int i=0;i<50;i++)
			for (int j = 0; j < 50; j++)
			{
				desire[i][j] += CalculateDesire(i, j);
				desire[i][j] -= std::abs(self->x / 1000 - i) + std::abs(self->y / 1000 - j);
				if (desire[i][j] > desire[t->x][t->y])
				{
					t = &conplace[i][j];
				}
			}
	}
	return t;
}
void CalculateDanger()
{

}
/// <summary>
/// 下面是Barry的算距离呆码
/// </summary>
/// <param name="x1"></param>
/// <param name="x2"></param>
/// <param name="y1"></param>
/// <param name="y2"></param>
/// <returns></returns>
double DistanceMeasure(int x1, int x2, int y1, int y2)
{
	int x = x1 - x2;
	int y = y1 - y2;
	double distance = sqrt(x * x + y * y);
	return distance;
}
/// <summary>
/// 下面是Barry的获取敌人角度呆码
/// </summary>
/// <param name="api"></param>
/// <param name="range"></param>
/// <returns></returns>
double GetAngle(IAPI& api, int range)
{
	auto self = api.GetSelfInfo();
	auto robot = api.GetRobots();
	int x_self = self->x;
	int y_self = self->y;
	int i = 0; double mini_distance = 0;
	int mini_deltax, mini_deltay;
	while (i < robot.size())
	{
		int x, y;
		if (robot[i]->teamID == self->teamID) goto ChangeWhileToFor;
		x = robot[i]->x;
		y = robot[i]->y;
		if (DistanceMeasure(x_self, x, y_self, y) < range+2000)
		{
			if (mini_distance == 0)
			{
				mini_distance = DistanceMeasure(x_self, x, y_self, y);
				mini_deltax = x - x_self;
				mini_deltay = y - y_self;
			}
			else if (DistanceMeasure(x_self, x, y_self, y) < mini_distance)
			{
				{
					mini_distance = DistanceMeasure(x_self, x, y_self, y);
					mini_deltax = x - x_self;
					mini_deltay = y - y_self;
				}
			}
		}
	ChangeWhileToFor:
		i++;
	}
	if (mini_distance == 0)
		return 0;
	else
		return std::atan2(mini_deltay, mini_deltax);
}
/// <summary>
/// 下面是Barry的攻击呆码
/// </summary>
/// <param name="api"></param>
/// <param name="robot_number"></param>
/// <param name="range"></param>
void JammerAttack(IAPI& api, int robot_number, int range)
{
	auto self = api.GetSelfInfo();
	if (GetAngle(api, range) != 0)
	{
		api.UseCommonSkill();
		api.Attack(GetAngle(api, range));
	}
}
void ContinueMyPath(std::shared_ptr<const THUAI5::Robot> self,IAPI& api)
{
	MapNode* maxdesire = generateDesire(api);
		BFSed = BFS((int)self->x / 1000, (int)self->y / 1000, maxdesire->x, maxdesire->y);
		//optim(hasBeenTo, dest);
		hasBeenTo = 1;
		//for (MapNode* p = hasBeenTo; p < dest; p++) std::printf("->%d,%d", p->x, p->y);
		if (arriveAt(self->x, self->y, path[hasBeenTo]->x, path[hasBeenTo]->y))
		{
			//std::printf("arriveat %d,%d\n", hasBeenTo->x, hasBeenTo->y);
			hasBeenTo++;
		}
	Goto(api, path[hasBeenTo]->x, path[hasBeenTo]->y);
}
bool setDest(IAPI& api, int dest_X, int dest_Y)
{
	//if (BFSed) return true;
	auto self = api.GetSelfInfo();
	bool success = BFS((int)(self->x / 1000), (int)(self->y / 1000), dest_X, dest_Y);
	if (!success) return false;
	hasBeenTo = 1;
	//BFSed = true;
	return true;
}
void GoToDest(IAPI& api)
{
	auto self = api.GetSelfInfo();
	if (std::abs(lastX - (int)api.GetSelfInfo()->x) + std::abs(lastY - (int)api.GetSelfInfo()->y) < 100)
	{
		//lastX = api.GetSelfInfo()->x;
		//lastY = api.GetSelfInfo()->y;
		api.MovePlayer(200, std::rand());
		return;
	}
	//lastX = api.GetSelfInfo()->x;
	//lastY = api.GetSelfInfo()->y;
	if (arriveAt(self->x, self->y, path[hasBeenTo]->x, path[hasBeenTo]->y))
	{
		if (path[hasBeenTo] != dest) hasBeenTo++;
		else
		{
			IHaveArrived = true;
		}
	}
	if (!IHaveArrived) Goto(api, path[hasBeenTo]->x, path[hasBeenTo]->y);
}
void defineStatus(IAPI& api)
{
	if (BotStatus != status::idle) return;
	auto self = api.GetSelfInfo();
	if (self->isResetting)
	{
		BotStatus = status::reset;
		return;
	}
	if (EnemyInSight(api))
	{
		BotStatus = status::watch;
		return;
	}
	//if (IHaveArrived)
	//{
	//	BotStatus = status::sorround;
	//	return;
	//}
	if (LastStatus == status::reset)
	{
		BotStatus = status::initial;
		return;
	}
	BotStatus = status::index;
	return;
}
bool EnemyInSight(IAPI& api)
{
	auto robotlist = api.GetRobots();
	auto self = api.GetSelfInfo();
	for (int i = 0; i < robotlist.size(); i++)
	{
		if (robotlist[i]->teamID == self->teamID) continue;
		if (DistanceMeasure(self->x, robotlist[i]->x, self->y, robotlist[i]->y) < std::max(self->attackRange, robotlist[i]->attackRange) + 2000)
		{
			return true;
		}
	}
	return false;
}
void AI::play(IAPI& api)
{
	
	std::ios::sync_with_stdio(false);
	auto self = api.GetSelfInfo();
	if (api.GetFrameCount() -lastFrameCount > 50)
	{
		//std::cout << "update" << std::endl;
		lastX = self->x;
		lastY = self->y;
		auto d = generateDesire(api);
		setDest(api, d->x, d->y);
		lastFrameCount = api.GetFrameCount();
	}
	if (self->cpuNum > 4) api.UseCPU(self->cpuNum);
	defineStatus(api);
	//std::printf("at %d,%d, hasbeento=%d, steps=%d, status=%d\n", self->x, self->y, hasBeenTo, steps, BotStatus);
	api.Pick(THUAI5::PropType::CPU);
	switch (BotStatus)
	{
		//有限状态机的core
	case status::initial:
	{
		if (!hasInitMap)
		{
			InitMap(api);
			hasInitMap = true;
		}
		auto d = generateDesire(api);
		//std::cout << "gdok" << std::endl;
		setDest(api, d->x, d->y);
		//std::cout << "setdestok" << std::endl;
		IHaveArrived = false;
		break;
	}
	case status::watch:
	{
			//有子弹有技能就上去淦！
			api.UseCommonSkill();
			api.MovePlayer(200, GetAngle(api, self->attackRange));
			JammerAttack(api, 0, self->attackRange);
			//if (self->life < 2345)
			//{
			//	if (self->cpuNum > 4) api.UseCPU(self->cpuNum);
			//}
		//向老家方向移动来躲炮弹
		break;
	}
	case status::sorround:
	{
		/*if (RangeAt(self->x, self->y, dest->x, dest->y))
		{
			api.MovePlayer(1000, std::rand());
			break;
		}
		else
		{
			IHaveArrived = false;
			hasBeenTo = steps-2;
			BotStatus = status::index;
			return;
		}
		*/
		auto d = generateDesire(api);
		setDest(api, d->x, d->y);
		//if (arriveAt(self->x, self->y, Sorroundpath[sorroundbeento]->x, Sorroundpath[sorroundbeento]->y))
		//{
		//	sorroundbeento++;
		//	if (sorroundbeento == 8) sorroundbeento = 0;
		//}
		if (std::abs(lastX - (int)api.GetSelfInfo()->x) + std::abs(lastY - (int)api.GetSelfInfo()->y) < 100)
		{
			lastX = api.GetSelfInfo()->x;
			lastY = api.GetSelfInfo()->y;
			api.MovePlayer(100, std::rand());
			return;
		}
		lastX = api.GetSelfInfo()->x;
		lastY = api.GetSelfInfo()->y;
		Goto(api, Sorroundpath[sorroundbeento]->x, Sorroundpath[sorroundbeento]->y);
		//api.MovePlayer(100, std::rand());
		break;
	}
	case status::reset:
	{
		hasBeenTo = 1;
		sorroundbeento = 0;
		IHaveArrived = false;
		break;
	}
	case status::index:
	{
		GoToDest(api);
		break;
	}
	case status::idle:
	{

		break;
	}
	}
	LastStatus = BotStatus;
	BotStatus = status::idle;
	api.Pick(THUAI5::PropType::CPU);
	api.Wait();
	////获得信息
	//auto self=api.GetSelfInfo();
	////输出个人CD
	//if (self->isResetting) 
	//{
	//	IHaveArrived = false;
	//	BFSed = false;
	//	hasBeenTo = &path[1];
	//	return;
	//}
	//std::cout<<self->CD<<std::endl;
	//
	//if (std::abs((int)(self->x - lastX)) + std::abs((int)(self->y - lastY)) < 100&&BFSed)
	//{
	//	//std::printf("Struggling\n-----\n------\n--------\n-----\n-----\n");
	//	api.MovePlayer(500, std::rand());
	//	goto afterGoto;
	//}
	//
	//api.Pick(THUAI5::PropType::Shield);
	//GoToDest(api);
	//api.Pick(THUAI5::PropType::Battery);
	//api.Pick(THUAI5::PropType::CPU);
	
	//api.Pick(THUAI5::PropType::ShieldBreaker);
//afterGoto:
	//api.Pick(THUAI5::PropType::Shield);
	//lastX = self->x;
	//lastY = self->y;
	////不知道第二个参数是干啥的，写个0
	//if(self->signalJammerNum>0)
	//JammerAttack(api, 0, self->attackRange+100);
	//
	//auto Props = api.GetProps();
	//auto cpu = THUAI5::PropType::CPU;
	//if (Props.size() != 0)
	//{
	//	std::cout << (Props[0]->type == cpu) << std::endl;
	//}
	////cell转化为grid
	//uint32_t gridnumbers=api.CellToGrid(5);
	////grid转化为cell
	//uint32_t cellnumbers=api.GridToCell(gridnumbers);
	//
	//获取场上信号干扰器
	//auto SignalJammers=api.GetSignalJammers();
	////输出第一个信号干扰器属于的队伍ID和数目
	//if (SignalJammers.size()!=0)
	//{
	//	std::cout << SignalJammers[0]->parentTeamID << std::endl;
	//	std::cout << SignalJammers.size() << std::endl;
	//}
	////!!!!!! 注意：千万不要写出这样的代码：
	////! auto SignalJammers=api.GetSignalJammers();
	////! std::cout << SignalJammers[0]->parentTeamID << std::endl;
	////! std::cout << SignalJammers.size() << std::endl;
	////! 当场上没有任何信号干扰器时，SignalJammers的长度将为0，如果此时直接调用SignalJammers[0]->parentTeamID，将会产生空指针，导致程序崩溃！
	////! 编写C++程序的一个好习惯：随时判断一段代码是否会产生空指针，并写出严密的判空机制。
	//
	////获取场上机器人的信息
	//auto Robots=api.GetRobots();
	////输出第一个机器人的攻击范围
	//if (Robots.size() != 0)
	//{
	//	std::cout << Robots[0]->attackRange << std::endl;
	//}
	//
	////获取游戏已经进行的帧数
	//auto Count=api.GetFrameCount();

	//获取（25，25）处地点类型
	//auto PlaceType=api.GetPlaceType(25,25);
	////判断地点类型是否为CPUFactory
	//auto cpufactory=THUAI5::PlaceType::CPUFactory;
	////std::cout<<(PlaceType==cpufactory)<<std::endl;
	//
	////返回一个数组，存储了场上所有玩家的GUID（全局唯一标识符）
	//auto guids=api.GetPlayerGUIDs();
	//
	////获取场上的道具信息
	////判断一号道具是否为cpu
	//
	////返回队伍得分
	//auto score=api.GetTeamScore();
	//
	////捡cpu
	////api.Pick(cpu);
	//
	////移动
	////asynchronous 为 true 的情况下，选手可以调用此函数，阻塞当前线程，直到下一次消息更新时继续运行。
	////api.Wait();
	//
	////查看是否有消息，有则接收消息
	//if(api.MessageAvailable())
	//{
	//	auto message=api.TryGetMessage();
	//}
	//想2号玩家发消息
	//int toPlayerID=2;
	//api.Send(toPlayerID,"this is an example");

	//玩家使用CPU,扔CPU/道具，使用技能
	//api.UseCPU(self->cpuNum);
	//api.ThrowCPU(3000/movespeed,e,0);
	//api.ThrowProp(3000/movespeed,e);
	//api.UseCommonSkill();
	
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

