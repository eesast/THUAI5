#include <random>
#include "../include/AI.h"
#include<cmath>
#include <vector>
#include <list>
#include <iostream>
#define N 50
#define pi 3.141592654
const int kCost1 = 10; //直移一格消耗
const int kCost2 = 14; //斜移一格消耗
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
//---------------------------------------------------------------------------------------------------------------------
//以下部分为网上的A*寻路算法改


struct Point
{
	int x, y; //点坐标，这里为了方便按照C++的数组来计算，x代表横排，y代表竖列
	int F, G, H; //F=G+H
	Point* parent; //parent的坐标，这里没有用指针，从而简化代码
	Point(int _x, int _y) :x(_x), y(_y), F(0), G(0), H(0), parent(NULL)  //变量初始化
	{
	}
};

class Astar
{
public:
	void InitAstar(std::vector<std::vector<int>>& _maze);
	std::list<Point*> GetPath(Point& startPoint, Point& endPoint, bool isIgnoreCorner);

private:
	Point* findPath(Point& startPoint, Point& endPoint, bool isIgnoreCorner);
	std::vector<Point*> getSurroundPoints(const Point* point, bool isIgnoreCorner) const;
	bool isCanreach(const Point* point, const Point* target, bool isIgnoreCorner) const; //判断某点是否可以用于下一步判断
	Point* isInList(const std::list<Point*>& list, const Point* point) const; //判断开启/关闭列表中是否包含某点
	Point* getLeastFpoint(); //从开启列表中返回F值最小的节点
	//计算FGH值
	int calcG(Point* temp_start, Point* point);
	int calcH(Point* point, Point* end);
	int calcF(Point* point);
private:
	std::vector<std::vector<int>> maze;
	std::list<Point*> openList;  //开启列表
	std::list<Point*> closeList; //关闭列表
};
void Astar::InitAstar(std::vector<std::vector<int>>& _maze)
{
	maze = _maze;
}

int Astar::calcG(Point* temp_start, Point* point)
{
	int extraG = (abs(point->x - temp_start->x) + abs(point->y - temp_start->y)) == 1 ? kCost1 : kCost2;
	int parentG = point->parent == NULL ? 0 : point->parent->G; //如果是初始节点，则其父节点是空
	return parentG + extraG;
}

int Astar::calcH(Point* point, Point* end)
{
	//用简单的欧几里得距离计算H，这个H的计算是关键，还有很多算法，没深入研究^_^
	return sqrt((double)(end->x - point->x) * (double)(end->x - point->x) + (double)(end->y - point->y) * (double)(end->y - point->y)) * kCost1;
}

int Astar::calcF(Point* point)
{
	return point->G + point->H;
}

Point* Astar::getLeastFpoint()
{
	if (!openList.empty())
	{
		auto resPoint = openList.front();
		for (auto& point : openList)
			if (point->F < resPoint->F)
				resPoint = point;
		return resPoint;
	}
	return NULL;
}

Point* Astar::findPath(Point& startPoint, Point& endPoint, bool isIgnoreCorner)
{
	openList.push_back(new Point(startPoint.x, startPoint.y)); //置入起点,拷贝开辟一个节点，内外隔离
	while (!openList.empty())
	{
		auto curPoint = getLeastFpoint(); //找到F值最小的点
		openList.remove(curPoint); //从开启列表中删除
		closeList.push_back(curPoint); //放到关闭列表
		//1,找到当前周围八个格中可以通过的格子
		auto surroundPoints = getSurroundPoints(curPoint, isIgnoreCorner);
		for (auto& target : surroundPoints)
		{
			//2,对某一个格子，如果它不在开启列表中，加入到开启列表，设置当前格为其父节点，计算F G H
			if (!isInList(openList, target))
			{
				target->parent = curPoint;

				target->G = calcG(curPoint, target);
				target->H = calcH(target, &endPoint);
				target->F = calcF(target);

				openList.push_back(target);
			}
			//3，对某一个格子，它在开启列表中，计算G值, 如果比原来的大, 就什么都不做, 否则设置它的父节点为当前点,并更新G和F
			else
			{
				int tempG = calcG(curPoint, target);
				if (tempG < target->G)
				{
					target->parent = curPoint;

					target->G = tempG;
					target->F = calcF(target);
				}
			}
			Point* resPoint = isInList(openList, &endPoint);
			if (resPoint)
				return resPoint; //返回列表里的节点指针，不要用原来传入的endpoint指针，因为发生了深拷贝
		}
	}

	return NULL;
}

std::list<Point*> Astar::GetPath(Point& startPoint, Point& endPoint, bool isIgnoreCorner)
{
	Point* result = findPath(startPoint, endPoint, isIgnoreCorner);
	std::list<Point*> path;
	//返回路径，如果没找到路径，返回空链表
	while (result)
	{
		path.push_front(result);
		result = result->parent;
	}

	// 清空临时开闭列表，防止重复执行GetPath导致结果异常
	openList.clear();
	closeList.clear();

	return path;
}

Point* Astar::isInList(const std::list<Point*>& list, const Point* point) const
{
	//判断某个节点是否在列表中，这里不能比较指针，因为每次加入列表是新开辟的节点，只能比较坐标
	for (auto p : list)
		if (p->x == point->x && p->y == point->y)
			return p;
	return NULL;
}

bool Astar::isCanreach(const Point* point, const Point* target, bool isIgnoreCorner) const
{
	if (target->x<0 || target->x>maze.size() - 1
		|| target->y<0 || target->y>maze[0].size() - 1
		|| maze[target->x][target->y] == 1
		|| target->x == point->x && target->y == point->y
		|| isInList(closeList, target)) //如果点与当前节点重合、超出地图、是障碍物、或者在关闭列表中，返回false
		return false;
	else
	{
		if (abs(point->x - target->x) + abs(point->y - target->y) == 1) //非斜角可以
			return true;
		else
		{
			//斜对角要判断是否绊住
			if (maze[point->x][target->y] == 0 && maze[target->x][point->y] == 0)
				return true;
			else
				return isIgnoreCorner;
		}
	}
}

std::vector<Point*> Astar::getSurroundPoints(const Point* point, bool isIgnoreCorner) const
{
	std::vector<Point*> surroundPoints;

	for (int x = point->x - 1; x <= point->x + 1; x++)
		for (int y = point->y - 1; y <= point->y + 1; y++)
			if (isCanreach(point, new Point(x, y), isIgnoreCorner))
				surroundPoints.push_back(new Point(x, y));

	return surroundPoints;
}

bool InPath(const int& row, const int& col, const std::list<Point*>& path) {
	for (const auto& p : path) {
		if (row == p->x && col == p->y) {
			return true;
		}
	}
	return false;
}


//--------------------------------------------------------------------------------------------------------------------
bool obstacle(int x0, int a0, int x1, int a1, IAPI& api);
bool roadclear(int a, int b, IAPI& api);
double distance(int a, int b, int c, int d);
void avoidobstacle(double a,IAPI& api);
void movealongpath(IAPI& api);
void avoidjammers(IAPI& api);
THUAI5::PlaceType map1[50][50];
int mapinfo[50][50],addr[50][50];
int runtimes = 0;
int x0=0, a0=0, x1=10, a1=20;
int x0_=0, a0_=0, x1_, a1_;
int planBtimes=0;
int distancetomove = 0;
int lr = 0, ud = 0;
int xl=16, al=20;
int xl_, al_;
bool onway=true;
int xc, ac;
int count = 0;

int xp=0, ap=0;
int stucktimes = 0;
int xj, aj;
bool w = 0, p = 1;
int xrobot=25000, arobot=20000;
double targetdistance = 100000;
bool deviation = 0,modealert=0;
int target;
int xtm0=0, atm0=0, xtm1=0, atm1=0;
int ifalert(IAPI& api);
int xinv[3]={0,0,0}, ainv[3]={0,0,0};
void AI::play(IAPI& api)
{
	api.Pick(THUAI5::PropType::CPU);
	int cpuonboard = 0,propsnum=0;
	int xcpu=10000000, ycpu=100000000;
	int xprop=100000000, aprop=100000000;
	//捡道具
	api.Pick(THUAI5::PropType::Shield);
	api.Pick(THUAI5::PropType::Battery);
	api.Pick(THUAI5::PropType::ShieldBreaker);
	api.Pick(THUAI5::PropType::Booster);
	int i = 0, j = 0, k = 0;
	auto self = api.GetSelfInfo();
	x0_ = self->x, a0_ = self->y;
	x0 = api.GridToCell(self->x), a0 = api.GridToCell(self->y);
	auto Robots = api.GetRobots();
	auto Props = api.GetProps();
	auto players = api.GetPlayerGUIDs();
	




	if (runtimes == 0)
	{
		for (i = 0;i < N;i++)
		{
			for (j = 0;j < N;j++)
			{
				map1[i][j] = api.GetPlaceType(i, j);

				if (map1[i][j] == THUAI5::PlaceType::BirthPlace4 && self->teamID == 0)
				{
					xtm0 = i - 1;atm0 = j - 1;
				}
				if (map1[i][j] == THUAI5::PlaceType::BirthPlace7 && self->teamID == 1)
				{
					xtm1 = i + 1;atm1 = j + 1;
				}

				if (map1[i][j] == THUAI5::PlaceType::Wall)
					mapinfo[i][j] = 1;
				else if (map1[i][j] == THUAI5::PlaceType::NullPlaceType)
					mapinfo[i][j] = 1;
				else if (map1[i][j] == THUAI5::PlaceType::BirthPlace1 && self->teamID == 0 && self->playerID == 0)
					mapinfo[i][j] = 0;
				else if (map1[i][j] == THUAI5::PlaceType::BirthPlace2 && self->teamID == 0 && self->playerID == 1)
					mapinfo[i][j] = 0;
				else if (map1[i][j] == THUAI5::PlaceType::BirthPlace3 && self->teamID == 0 && self->playerID == 2)
					mapinfo[i][j] = 0;
				else if (map1[i][j] == THUAI5::PlaceType::BirthPlace4 && self->teamID == 0 && self->playerID == 3)
					mapinfo[i][j] = 0;
				else if (map1[i][j] == THUAI5::PlaceType::BirthPlace5 && self->teamID == 1 && self->playerID == 0)
					mapinfo[i][j] = 0;
				else if (map1[i][j] == THUAI5::PlaceType::BirthPlace6 && self->teamID == 1 && self->playerID == 1)
					mapinfo[i][j] = 0;
				else if (map1[i][j] == THUAI5::PlaceType::BirthPlace7 && self->teamID == 1 && self->playerID == 2)
					mapinfo[i][j] = 0;
				else if (map1[i][j] == THUAI5::PlaceType::BirthPlace8 && self->teamID == 1 && self->playerID == 3)
					mapinfo[i][j] = 0;
				else if (map1[i][j] == THUAI5::PlaceType::BirthPlace1 || map1[i][j] == THUAI5::PlaceType::BirthPlace2 || map1[i][j] == THUAI5::PlaceType::BirthPlace3 || map1[i][j] == THUAI5::PlaceType::BirthPlace4 ||
					map1[i][j] == THUAI5::PlaceType::BirthPlace5 || map1[i][j] == THUAI5::PlaceType::BirthPlace6 || map1[i][j] == THUAI5::PlaceType::BirthPlace7 || map1[i][j] == THUAI5::PlaceType::BirthPlace8)
					mapinfo[i][j] = 1;
				else mapinfo[i][j] = 0;
			}
		}
	}

	if(self->teamID==0&&x0==(xtm0+1)&&a0==(atm0+1))
	{
		for (i = 0;i<10;i++)
		{
			api.Pick(THUAI5::PropType::CPU);
		}
	}
	if (self->teamID == 1 && x0 == (xtm1-1) && a0 == (atm1-1))
	{
		for (i = 0;i < 10;i++)
		{
			api.Pick(THUAI5::PropType::CPU);
		}
	}



	//隐身杀手攻击：
	//----------------------------------------------------------------------------------------------


	for (i = 0;i < Robots.size();i++)
	{
		if (Robots[i]->teamID != self->teamID&&Robots[i]->isResetting==0)
		{
			if (distance(x0_, a0_, Robots[i]->x, Robots[i]->y) <= 20000)
			{
				api.UseProp();
			}
			if (distance(x0_, a0_, Robots[i]->x, Robots[i]->y) <= 1500)
			{
					double theta = atan2(abs((double)Robots[i]->y - (double)a0_), abs((double)Robots[i]->x - (double)x0_));
					if (Robots[i]->x <= x0_ && Robots[i]->y >= a0_)
						theta = pi - theta;
					else if (Robots[i]->x <= x0_ && Robots[i]->y <= a0_)
						theta = theta + pi;
					else if (Robots[i]->x >= x0_ && Robots[i]->y <= a0_)
						theta = 2 * pi - theta;
					api.Attack(theta);
					api.Attack(theta+pi/20);
					api.Attack(theta-pi/20);
			}
			if (self->timeUntilCommonSkillAvailable == 0 && distance(x0_, a0_, Robots[i]->x, Robots[i]->y) <= 5000)
			{
				api.UseCommonSkill();
			}
			if (distance(x0_, a0_, Robots[i]->x, Robots[i]->y) <= 5000 && self->timeUntilCommonSkillAvailable <= 23000 && self->timeUntilCommonSkillAvailable >= 6000 &&self->signalJammerNum>2&& roadclear(api.GridToCell(Robots[i]->x), api.GridToCell(Robots[i]->y), api))
			{
				double theta = atan2(abs((double)Robots[i]->y - (double)a0_), abs((double)Robots[i]->x - (double)x0_));
				if (Robots[i]->x <= x0_ && Robots[i]->y >= a0_)
					theta = pi - theta;
				else if (Robots[i]->x <= x0_ && Robots[i]->y <= a0_)
					theta = theta + pi;
				else if (Robots[i]->x >= x0_ && Robots[i]->y <= a0_)
					theta = 2 * pi - theta;
				api.Attack(theta+pi/12);
				api.Attack(theta-pi/12);
			}
		
			if (distance(x0_, a0_, Robots[i]->x, Robots[i]->y) <= 2500 && self->timeUntilCommonSkillAvailable <= 23000 && self->timeUntilCommonSkillAvailable >= 6000 && self->signalJammerNum >1 && !roadclear(api.GridToCell(Robots[i]->x), api.GridToCell(Robots[i]->y), api))
			{
				double theta = atan2(abs((double)Robots[i]->y - (double)a0_), abs((double)Robots[i]->x - (double)x0_));
				if (Robots[i]->x <= x0_ && Robots[i]->y >= a0_)
					theta = pi - theta;
				else if (Robots[i]->x <= x0_ && Robots[i]->y <= a0_)
					theta = theta + pi;
				else if (Robots[i]->x >= x0_ && Robots[i]->y <= a0_)
					theta = 2 * pi - theta;
				api.Attack(theta);
			}
		}
	}
	//-------------------------------------------------------------------------------------------------------------

	
	




	if(x0==xp&&a0==ap)
	{
		stucktimes += 1;
	}
	else { stucktimes = 0; }
	xp = x0;ap = a0;
	addr[x0][a0] = 0;
	xc = api.CellToGrid(x0); ac = api.CellToGrid(a0);

	if (runtimes <= 5)
	{
		if (self->teamID == 0)
		{
			if (self->playerID == 0)
				x1 = 16, a1 = 18;
			else if (self->playerID == 1)
				x1 = 16, a1 = 20;
			else if (self->playerID == 2)
				x1 = 16, a1 = 30;
			else if (self->playerID == 3)
				x1 = 16, a1 = 32;
		}
		if (self->teamID == 1)
		{
			if (self->playerID == 0)
				x1 = 32, a1 = 18;
			else if (self->playerID == 1)
				x1 = 32, a1 = 20;
			else if (self->playerID == 2)
				x1 = 32, a1 = 30;
			else if (self->playerID == 3)
				x1 = 32, a1 = 32;
		}
		x1_ = api.CellToGrid(x1);a1_ = api.CellToGrid(a1);
		onway = true;
	}

	
	
	

	for (i = 0;i < Props.size();i++)
	{
		if (Props[i]->type == THUAI5::PropType::CPU&&Props[i]->isMoving==0&&map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)]!=THUAI5::PlaceType::BirthPlace1
			&& map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace2 && map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace3
			&& map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace4 && map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace5
			&& map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace6 && map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace7
			&& map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace8)
		{
			if (!(api.GridToCell(Props[i]->x) == 42 && api.GridToCell(Props[i]->y) == 10) && !(api.GridToCell(Props[i]->x) == 41 && api.GridToCell(Props[i]->y) == 10) && !(api.GridToCell(Props[i]->x) == 40 && api.GridToCell(Props[i]->y) == 10))
			{
				cpuonboard += 1;
				if (distance(xcpu, ycpu, x0_, a0_) > distance(Props[i]->x, Props[i]->y, x0_, a0_))
				{
					xcpu = Props[i]->x, ycpu = Props[i]->y;
				}
			}
		}
		else if (  Props[i]->isMoving == 0 && map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace1
			&& map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace2 && map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace3
			&& map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace4 && map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace5
			&& map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace6 && map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace7
			&& map1[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] != THUAI5::PlaceType::BirthPlace8&& (Props[i]->type == THUAI5::PropType::Shield || Props[i]->type == THUAI5::PropType::Battery || Props[i]->type == THUAI5::PropType::Booster || Props[i]->type == THUAI5::PropType::ShieldBreaker))
		{
			propsnum += 1;
			if (distance(xprop, aprop, x0_, a0_) > distance(Props[i]->x, Props[i]->y, x0_, a0_))
			{
				xprop = Props[i]->x, aprop = Props[i]->y;
			}
			if(api.GridToCell(Props[i]->x)==x0&& api.GridToCell(Props[i]->y) == a0)
			{
				api.UseProp();
			}
		}
	}
	


//最最核心的移动部分：


if (ifalert(api) == 1)
{
	avoidjammers(api);
	//此处是直接潜伏靠近
}
else if(ifalert(api) == 2)
{for (i = 0;i < Robots.size();i++)
	{
		if (Robots[i]->teamID != self->teamID && Robots[i]->isResetting == 0&&roadclear(api.GridToCell(Robots[i]->x),api.GridToCell(Robots[i]->y), api))
		{
			if (distance(x0_, a0_, Robots[i]->x, Robots[i]->y) < targetdistance)
			{
				targetdistance = distance(x0_, a0_, Robots[i]->x, Robots[i]->y);
				xrobot = Robots[i]->x; arobot = Robots[i]->y;
			}
		}
	}
	if ( self->timeUntilCommonSkillAvailable == 0&& distance(x0_, a0_, xrobot, arobot) <= 13000 )
	{
		api.UseCommonSkill();
	}
	double theta = atan2(abs((double)arobot - (double)a0_), abs((double)xrobot - (double)x0_));
	if (xrobot <= x0_ && arobot >= a0_)
		theta = pi - theta;
	else if (xrobot <= x0_ && arobot <= a0_)
		theta = theta + pi;
	else if (xrobot >= x0_ && arobot <= a0_)
		theta = 2 * pi - theta;
	api.MovePlayer(75, theta);
	targetdistance = 100000;
	onway = false;
}
else 
{

	if(stucktimes>10)
	{
		onway = false;
		stucktimes = 0;
	}
	if (onway==false&&api.GetFrameCount() > 11000 && self->teamID == 0 && self->playerID == 3)
	{
		x1 = xtm0+1, a1 = atm0+1;
		x1_ = api.CellToGrid(x1);a1_ = api.CellToGrid(a1);
		onway = true;
		planBtimes = 0;
		distancetomove = 0;
	}
	else if (onway == false && api.GetFrameCount() > 11000 && self->teamID == 1 && self->playerID == 2)
	{
		x1 = xtm1 - 1, a1 = atm1 - 1;
		x1_ = api.CellToGrid(x1);a1_ = api.CellToGrid(a1);
		onway = true;
		planBtimes = 0;
		distancetomove = 0;
	}
	else if (onway == false && self->cpuNum >= 1 && self->teamID == 0 && (x0 != xtm0 || a0 != atm0))
	{
		x1 = xtm0, a1 = atm0;
		x1_ = api.CellToGrid(x1);a1_ = api.CellToGrid(a1);
		onway = true;
		planBtimes = 0;
		distancetomove = 0;
	}
	else if (onway == false && self->cpuNum >= 1 && self->teamID == 1 && (x0 != xtm1 || a0 != atm1))
		{
			x1 = xtm1, a1 = atm1;
			x1_ = api.CellToGrid(x1);a1_ = api.CellToGrid(a1);
			onway = true;
			planBtimes = 0;
			distancetomove = 0;
		}

	else if (onway == false && cpuonboard > 0)
	{
		x1_ = xcpu, a1_ = ycpu;
		x1 = api.GridToCell(xcpu), a1 = api.GridToCell(ycpu);
		onway = true;
		planBtimes = 0;
		distancetomove = 0;
	}
	else if (onway == false && propsnum > 0  && api.GridToCell(xprop) != 1 && api.GridToCell(xprop) != 48)
	{
		x1_ = xprop, a1_ = aprop;
		x1 = api.GridToCell(xprop), a1 = api.GridToCell(aprop);
		onway = true;
		planBtimes = 0;
		distancetomove = 0;
	}
	else if (onway == false)
	{
		if (self->teamID == 0)
		{
			if (self->playerID == 0)
				x1 = 16, a1 = 18;
			else if (self->playerID == 1)
				x1 = 16, a1 = 20;
			else if (self->playerID == 2)
				x1 = 16, a1 = 30;
			else if (self->playerID == 3)
				x1 = 16, a1 = 32;
		}
		if (self->teamID == 1)
		{
			if (self->playerID == 0)
				x1 = 32, a1 = 18;
			else if (self->playerID == 1)
				x1 = 32, a1 = 20;
			else if (self->playerID == 2)
				x1 = 32, a1 = 30;
			else if (self->playerID == 3)
				x1 = 32, a1 = 32;
		}
		x1_ = api.CellToGrid(x1);
		a1_ = api.CellToGrid(a1);
		onway = true;
		planBtimes = 0;
		distancetomove = 0;
	}

	if (onway == true)
	{
		if (planBtimes == 0)
		{
			obstacle(x0, a0, x1, a1, api); 
			planBtimes += 1;
			/*if (self->teamID == 0 && (self->playerID == 0 || self->playerID == 1))
			{
				for (i = 0;i < 50;i++)
				{
					for (j = 0;j < 50;j++)
					{
						std::cout << addr[i][j] << " ";
					}
					std::cout << std::endl;
				}
			}*/
			movealongpath(api);
			xl_ = api.CellToGrid(xl), al_ = api.CellToGrid(al);
			distancetomove = distance(x0_, a0_, xl_, al_);
			std::cout << xl << " "<<al<<std::endl;
			std::cout << x1 << " " << a1 << std::endl;
		}
		else {
			double theta = atan2(abs((double)al_ - (double)a0_), abs((double)xl_ - (double)x0_));
			if (xl_ <= x0_ && al_ >= a0_)
				theta = pi - theta;
			else if (xl_ <= x0_ && al_ <= a0_)
				theta = theta + pi;
			else if (xl_ >= x0_ && al_ <= a0_)
				theta = 2 * pi - theta;

			avoidobstacle(theta, api);

			if (distancetomove < 400 || deviation)
			{
				if (distancetomove >=0 && deviation == 0)
				{
					movealongpath(api);
					xl_ = api.CellToGrid(xl), al_ = api.CellToGrid(al);
					api.MovePlayer((distancetomove / 5 + 1), theta);
				}
				if (deviation == true)
				{
					deviation = 0;
				}
				distancetomove = distance(x0_, a0_, xl_, al_);
			}
			/*else if (distancetomove > 3000)
			{
				planBtimes = 0;
				distancetomove = 0;
			}*/
			else {
				api.MovePlayer(80, theta);
				distancetomove -= 400;
			}

		}
		if (x0 == x1 && a0 == a1) { onway = false;planBtimes = 0; }
	}
}

if (self->teamID == 0 && self->cpuNum > 0 && distance(x0_, a0_, api.CellToGrid(xtm0 + 1), api.CellToGrid(atm0 + 1)) < 15000 && distance(x0_, a0_, api.CellToGrid(xtm0 + 1), api.CellToGrid(atm0 + 1)) > 1500 && roadclear(xtm0 + 1, atm0 + 1, api))
{
	double theta = atan2(abs((double)api.CellToGrid(atm0 + 1) - (double)a0_), abs((double)api.CellToGrid(xtm0 + 1) - (double)x0_));
	if (api.CellToGrid(xtm0 + 1) <= x0_ && api.CellToGrid(atm0 + 1) >= a0_)
		theta = pi - theta;
	else if (api.CellToGrid(xtm0 + 1) <= x0_ && api.CellToGrid(atm0 + 1) <= a0_)
		theta = theta + pi;
	else if (api.CellToGrid(xtm0 + 1) >= x0_ && api.CellToGrid(atm0 + 1) <= a0_)
		theta = 2 * pi - theta;
	api.ThrowCPU(distance(api.CellToGrid(xtm0 + 1), api.CellToGrid(atm0 + 1), x0_, a0_) / 3, theta, self->cpuNum);
	onway = false;
}
else if (self->teamID == 1 && self->cpuNum > 0 && distance(x0_, a0_, api.CellToGrid(xtm1 - 1), api.CellToGrid(atm1 - 1)) < 15000 && distance(x0_, a0_, api.CellToGrid(xtm1 - 1), api.CellToGrid(atm1 - 1)) > 1500 && roadclear(xtm1 - 1, atm1 - 1, api))
{
	double theta = atan2(abs((double)api.CellToGrid(atm1 - 1) - (double)a0_), abs((double)api.CellToGrid(xtm1 - 1) - (double)x0_));
	if (api.CellToGrid(xtm1 - 1) <= x0_ && api.CellToGrid(atm1 - 1) >= a0_)
		theta = pi - theta;
	else if (api.CellToGrid(xtm1 - 1) <= x0_ && api.CellToGrid(atm1 - 1) <= a0_)
		theta = theta + pi;
	else if (api.CellToGrid(xtm1 - 1) >= x0_ && api.CellToGrid(atm1 - 1) <= a0_)
		theta = 2 * pi - theta;
	api.ThrowCPU(distance(api.CellToGrid(xtm1 - 1), api.CellToGrid(atm1 - 1), x0_, a0_) / 3, theta, self->cpuNum);
	onway = false;
}
runtimes += 1;
}

bool obstacle(int x0, int a0, int x1, int a1,IAPI& api)
{
	int i, j;
	//初始化地图，用二维矩阵代表地图，1表示障碍物，0表示可通
	std::vector<std::vector<int>> map;
	for (i = 0;i < 50;i++)
	{
		std::vector<int> B;
		for (j = 0;j < 50;j++)
		{
			B.push_back(mapinfo[i][j]);
		}
		map.push_back(B);
	}
	Astar astar;
	astar.InitAstar(map);

	//设置起始和结束点
	Point start(x0,a0);
	Point end(x1,a1);
	std::list<Point*> path = astar.GetPath(start, end, false);

	for (int row = 0; row < map.size(); ++row) {
		for (int col = 0; col < map[0].size(); ++col) {
			if (InPath(row, col, path)) {
				if (map[row][col] != 0) {}
				else {
					addr[row][col] = 2;
				}
			}
			else {
				addr[row][col] = map[row][col];
			}
		}
	}
	
	return true;
}

void movealongpath(IAPI& api)
{
	auto self = api.GetSelfInfo();
	x0 = api.GridToCell(self->x), a0 = api.GridToCell(self->y);
	x0_ = self->x, a0_ = self->y;
		addr[x0][a0] = 0;
		if (addr[x0 - 1][a0] == 2)
			xl = x0 - 1,al=a0;
		else if (addr[x0 + 1][a0] == 2)
			xl = x0 + 1, al = a0;
	else if (addr[x0][a0 + 1] == 2)
			xl = x0 , al = a0+1;
	else if (addr[x0][a0 - 1] == 2)
			xl = x0 , al = a0-1;
	else if (addr[x0 - 1][a0 + 1] == 2)
			xl = x0 -1, al = a0+1;
	else if (addr[x0 - 1][a0 - 1] == 2)
			xl = x0 - 1, al = a0-1;
	else if (addr[x0 + 1][a0 + 1] == 2)
			xl = x0 + 1, al = a0+1;
	else if (addr[x0 + 1][a0 - 1] == 2)
			xl = x0 + 1, al = a0-1;

}
void avoidobstacle( double a ,IAPI& api)
{
	int i;
	auto Robots = api.GetRobots();

	if (mapinfo[ x0+1][a0+1]==1&&mapinfo[x0+1][a0]==0&&mapinfo[x0][a0+1]==0&&mapinfo[x0-1][a0+1]==0&&mapinfo[x0-1][a0]==0
	&&mapinfo[x0-1][a0-1]==0&&mapinfo[x0][a0-1]==0&&mapinfo[x0+1][a0-1]==0)
{ 
	if (distance(api.CellToGrid(x0 + 1), api.CellToGrid(a0 + 1), x0_, a0_) < 1250)
	{
		double theta = atan2(abs((double)api.CellToGrid(a0 + 1) - (double)a0_), abs((double)api.CellToGrid(x0 + 1) - (double)x0_));
		if (api.CellToGrid(x0 + 1) <= x0_ && api.CellToGrid(a0 + 1) >= a0_)
			theta = pi - theta;
		else if (api.CellToGrid(x0 + 1) <= x0_ && api.CellToGrid(a0 + 1) <= a0_)
			theta = theta + pi;
		else if (api.CellToGrid(x0 + 1) >= x0_ && api.CellToGrid(a0 + 1) <= a0_)
			theta = 2 * pi - theta;
		if (theta > 0 && theta < pi)
		{
			api.MovePlayer(50, pi + theta);
			api.MovePlayer(50, a);
			deviation = 1;
		}
		else
		{
			api.MovePlayer(50, theta - pi);
			api.MovePlayer(50, a);
			deviation = 1;
		}
	}
}
else  if (mapinfo[x0 + 1][a0 + 1] == 0 && mapinfo[x0 + 1][a0] == 1 && mapinfo[x0][a0 + 1] == 0 && mapinfo[x0 - 1][a0 + 1] == 0 && mapinfo[x0 - 1][a0] == 0
	&& mapinfo[x0 - 1][a0 - 1] == 0 && mapinfo[x0][a0 - 1] == 0 && mapinfo[x0 + 1][a0 - 1] == 0)
{
	if (distance(api.CellToGrid(x0 + 1), api.CellToGrid(a0), x0_, a0_) < 1250)
	{
		double theta = atan2(abs((double)api.CellToGrid(a0) - (double)a0_), abs((double)api.CellToGrid(x0 + 1) - (double)x0_));
		if (api.CellToGrid(x0 + 1) <= x0_ && api.CellToGrid(a0 ) >= a0_)
			theta = pi - theta;
		else if (api.CellToGrid(x0 + 1) <= x0_ && api.CellToGrid(a0 ) <= a0_)
			theta = theta + pi;
		else if (api.CellToGrid(x0 + 1) >= x0_ && api.CellToGrid(a0 ) <= a0_)
			theta = 2 * pi - theta;
		if (theta > 0 && theta < pi)
		{
			api.MovePlayer(50, pi + theta);
			api.MovePlayer(50, a);
			deviation = 1;
		}
		else
		{
			api.MovePlayer(50, theta - pi);
			api.MovePlayer(50, a);
			deviation = 1;
		}
	}
}
else  if (mapinfo[x0 + 1][a0 + 1] == 0 && mapinfo[x0 + 1][a0] == 0 && mapinfo[x0][a0 + 1] == 1 && mapinfo[x0 - 1][a0 + 1] == 0 && mapinfo[x0 - 1][a0] == 0
	&& mapinfo[x0 - 1][a0 - 1] == 0 && mapinfo[x0][a0 - 1] == 0 && mapinfo[x0 + 1][a0 - 1] == 0)
{
	if (distance(api.CellToGrid(x0 ), api.CellToGrid(a0 + 1), x0_, a0_) < 1250)
	{
		double theta = atan2(abs((double)api.CellToGrid(a0 + 1) - (double)a0_), abs((double)api.CellToGrid(x0 ) - (double)x0_));
		if (api.CellToGrid(x0 ) <= x0_ && api.CellToGrid(a0 + 1) >= a0_)
			theta = pi - theta;
		else if (api.CellToGrid(x0 ) <= x0_ && api.CellToGrid(a0 + 1) <= a0_)
			theta = theta + pi;
		else if (api.CellToGrid(x0 ) >= x0_ && api.CellToGrid(a0 + 1) <= a0_)
			theta = 2 * pi - theta;
		if (theta > 0 && theta < pi)
		{
			api.MovePlayer(50, pi + theta);
			api.MovePlayer(50, a);
			deviation = 1;
		}
		else
		{
			api.MovePlayer(50, theta - pi);
			api.MovePlayer(50, a);
			deviation = 1;
		}
	}
}
else  if (mapinfo[x0 + 1][a0 + 1] == 0 && mapinfo[x0 + 1][a0] == 0 && mapinfo[x0][a0 + 1] == 0 && mapinfo[x0 - 1][a0 + 1] == 1 && mapinfo[x0 - 1][a0] == 0
	&& mapinfo[x0 - 1][a0 - 1] == 0 && mapinfo[x0][a0 - 1] == 0 && mapinfo[x0 + 1][a0 - 1] == 0)
{
	if (distance(api.CellToGrid(x0 - 1), api.CellToGrid(a0 + 1), x0_, a0_) < 1250)
	{
		double theta = atan2(abs((double)api.CellToGrid(a0 + 1) - (double)a0_), abs((double)api.CellToGrid(x0 -1) - (double)x0_));
		if (api.CellToGrid(x0 - 1) <= x0_ && api.CellToGrid(a0 + 1) >= a0_)
			theta = pi - theta;
		else if (api.CellToGrid(x0 - 1) <= x0_ && api.CellToGrid(a0 + 1) <= a0_)
			theta = theta + pi;
		else if (api.CellToGrid(x0 - 1) >= x0_ && api.CellToGrid(a0 + 1) <= a0_)
			theta = 2 * pi - theta;
		if (theta > 0 && theta < pi)
		{
			api.MovePlayer(50, pi + theta);
			api.MovePlayer(50, a);
			deviation = 1;
		}
		else
		{
			api.MovePlayer(50, theta - pi);
			api.MovePlayer(50, a);
			deviation = 1;
		}
	}
}
else  if (mapinfo[x0 + 1][a0 + 1] == 0 && mapinfo[x0 + 1][a0] == 0 && mapinfo[x0][a0 + 1] == 0 && mapinfo[x0 - 1][a0 + 1] == 0 && mapinfo[x0 - 1][a0] == 1
	&& mapinfo[x0 - 1][a0 - 1] == 0 && mapinfo[x0][a0 - 1] == 0 && mapinfo[x0 + 1][a0 - 1] == 0)
{
	if (distance(api.CellToGrid(x0 -1), api.CellToGrid(a0 ), x0_, a0_) < 1250)
	{
		double theta = atan2(abs((double)api.CellToGrid(a0 ) - (double)a0_), abs((double)api.CellToGrid(x0 - 1) - (double)x0_));
		if (api.CellToGrid(x0 - 1) <= x0_ && api.CellToGrid(a0 ) >= a0_)
			theta = pi - theta;
		else if (api.CellToGrid(x0 - 1) <= x0_ && api.CellToGrid(a0 ) <= a0_)
			theta = theta + pi;
		else if (api.CellToGrid(x0 - 1) >= x0_ && api.CellToGrid(a0 ) <= a0_)
			theta = 2 * pi - theta;
		if (theta > 0 && theta < pi)
		{
			api.MovePlayer(50, pi + theta);
			api.MovePlayer(50, a);
			deviation = 1;
		}
		else
		{
			api.MovePlayer(50, theta - pi);
			api.MovePlayer(50, a);
			deviation = 1;
		}
	}
}
else  if (mapinfo[x0 + 1][a0 + 1] == 0 && mapinfo[x0 + 1][a0] == 0 && mapinfo[x0][a0 + 1] == 0 && mapinfo[x0 - 1][a0 + 1] == 0 && mapinfo[x0 - 1][a0] == 0
	&& mapinfo[x0 - 1][a0 - 1] == 1 && mapinfo[x0][a0 - 1] == 0 && mapinfo[x0 + 1][a0 - 1] == 0)
{
	if (distance(api.CellToGrid(x0 - 1), api.CellToGrid(a0 - 1), x0_, a0_) < 1250)
	{
		double theta = atan2(abs((double)api.CellToGrid(a0 - 1) - (double)a0_), abs((double)api.CellToGrid(x0 - 1) - (double)x0_));
		if (api.CellToGrid(x0 - 1) <= x0_ && api.CellToGrid(a0 - 1) >= a0_)
			theta = pi - theta;
		else if (api.CellToGrid(x0 - 1) <= x0_ && api.CellToGrid(a0 - 1) <= a0_)
			theta = theta + pi;
		else if (api.CellToGrid(x0 - 1) >= x0_ && api.CellToGrid(a0 - 1) <= a0_)
			theta = 2 * pi - theta;
		if (theta > 0 && theta < pi)
		{
			api.MovePlayer(50, pi + theta);
			api.MovePlayer(50, a);
			deviation = 1;
		}
		else
		{
			api.MovePlayer(50, theta - pi);
			api.MovePlayer(50, a);
			deviation = 1;
		}
	}
}
else if (mapinfo[x0 + 1][a0 + 1] == 0 && mapinfo[x0 + 1][a0] == 0 && mapinfo[x0][a0 + 1] == 0 && mapinfo[x0 - 1][a0 + 1] == 0 && mapinfo[x0 - 1][a0] == 0
	&& mapinfo[x0 - 1][a0 - 1] == 0 && mapinfo[x0][a0 - 1] == 1 && mapinfo[x0 + 1][a0 - 1] == 0)
{
	if (distance(api.CellToGrid(x0 ), api.CellToGrid(a0 - 1), x0_, a0_) < 1250)
	{
		double theta = atan2(abs((double)api.CellToGrid(a0 - 1) - (double)a0_), abs((double)api.CellToGrid(x0 ) - (double)x0_));
		if (api.CellToGrid(x0 ) <= x0_ && api.CellToGrid(a0 - 1) >= a0_)
			theta = pi - theta;
		else if (api.CellToGrid(x0 ) <= x0_ && api.CellToGrid(a0 - 1) <= a0_)
			theta = theta + pi;
		else if (api.CellToGrid(x0 ) >= x0_ && api.CellToGrid(a0 - 1) <= a0_)
			theta = 2 * pi - theta;
		if (theta > 0 && theta < pi)
		{
			api.MovePlayer(50, pi + theta);
			api.MovePlayer(50, a);
			deviation = 1;
		}
		else
		{
			api.MovePlayer(50, theta - pi);
			api.MovePlayer(50, a);
			deviation = 1;
		}
	}
}
else  if (mapinfo[x0 + 1][a0 + 1] == 0 && mapinfo[x0 + 1][a0] == 0 && mapinfo[x0][a0 + 1] == 0 && mapinfo[x0 - 1][a0 + 1] == 0 && mapinfo[x0 - 1][a0] == 0
	&& mapinfo[x0 - 1][a0 - 1] == 0 && mapinfo[x0][a0 - 1] == 0 && mapinfo[x0 + 1][a0 - 1] == 1)
{
	if (distance(api.CellToGrid(x0 + 1), api.CellToGrid(a0 - 1), x0_, a0_) < 1250)
	{
		double theta = atan2(abs((double)api.CellToGrid(a0 - 1) - (double)a0_), abs((double)api.CellToGrid(x0 + 1) - (double)x0_));
		if (api.CellToGrid(x0 + 1) <= x0_ && api.CellToGrid(a0 - 1) >= a0_)
			theta = pi - theta;
		else if (api.CellToGrid(x0 + 1) <= x0_ && api.CellToGrid(a0 - 1) <= a0_)
			theta = theta + pi;
		else if (api.CellToGrid(x0 + 1) >= x0_ && api.CellToGrid(a0 - 1) <= a0_)
			theta = 2 * pi - theta;
		if (theta > 0 && theta < pi)
		{
			api.MovePlayer(50, pi + theta);
			api.MovePlayer(50, a);
			deviation = 1;
		}
		else
		{
			api.MovePlayer(50, theta - pi);
			api.MovePlayer(50, a);
			deviation = 1;
		}
	}
}

	for (i = 0;i < Robots.size();i++)
	{
		if (distance(x0_, a0_, Robots[i]->x, Robots[i]->y) <= 1200)
		{
			double theta = atan2(abs((double)Robots[i]->y - (double)a0_), abs((double)Robots[i]->x - (double)x0_));
			if (Robots[i]->x <= x0_ && Robots[i]->y >= a0_)
				theta = pi - theta;
			else if (Robots[i]->x <= x0_ && Robots[i]->y <= a0_)
				theta = theta + pi;
			else if (Robots[i]->x >= x0_ && Robots[i]->y <= a0_)
				theta = 2 * pi - theta;


			if (roadclear(api.GridToCell(x0_ +2000 * cos(pi + theta)), api.GridToCell(a0_ + 2000* sin(pi + theta)), api))
			{
				api.MovePlayer(100, pi + theta);deviation = 1;
			}
			else if (roadclear(api.GridToCell(x0_ + 1500 * cos(1.25*pi + theta)), api.GridToCell(a0_ + 1500 * sin(1.25*pi +theta)), api))
				{
					api.MovePlayer(100,1.25* pi + theta);deviation = 1;
				}
			else if (roadclear(api.GridToCell(x0_ + 1500 * cos(0.75*pi +  theta)), api.GridToCell(a0_ + 1500 * sin(0.75*pi +  theta)), api))
			{
				api.MovePlayer(100, 0.75*pi + theta);deviation = 1;
			}
			else if (roadclear(api.GridToCell(x0_ + 1500 * cos(0.5*pi + theta)), api.GridToCell(a0_ + 1500 * sin(0.5*pi +  theta)), api))
			{
				api.MovePlayer(100, 0.5*pi +  theta);deviation = 1;
			}
			else if (roadclear(api.GridToCell(x0_ + 1500 * cos(1.5 * pi + theta)), api.GridToCell(a0_ + 1500 * sin(1.5 * pi + theta)), api))
			{
				api.MovePlayer(100, 1.5 * pi + theta);deviation = 1;
			}
		}
	}

}
double distance(int a, int b, int c, int d)
{
	return(sqrt(((double)a - (double)c) * ((double)a - (double)c) + ((double)b - (double)d) * ((double)b - (double)d)));
}

void avoidjammers(IAPI& api)
{
	auto self = api.GetSelfInfo();
	int i, j, k = 0, num = 0;
	auto jammers = api.GetSignalJammers();
	double angle[10] = { 20,20,20,20,20,20,20,20,20,20 }, escape = 0;
	int multi[10] = { 0,0,0,0,0,0,0,0,0,0 };
	int near = 0, away = 0;
	for (i = 0;i < jammers.size();i++)
	{
		if (jammers[i]->parentTeamID != self->teamID && jammers[i]->type == THUAI5::SignalJammerType::StrongJammer && distance(x0_, a0_, jammers[i]->x, jammers[i]->y) <= 9000)
		{
			angle[k] = jammers[i]->facingDirection;
			if (roadclear(api.GridToCell(x0_ + 2000 * cos(angle[k])), api.GridToCell(a0_ + 2000 * sin(angle[k])), api))
			{
				api.MovePlayer(80, angle[k]);
			}
			else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] + 0.25 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] + 0.25 * pi)), api))
			{
				api.MovePlayer(80, angle[k] + 0.25 * pi);
			}
			else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] - 0.25 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] - 0.25 * pi)), api))
			{
				api.MovePlayer(80, angle[k] - 0.25 * pi);
			}
			else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] + 0.5 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] + 0.5 * pi)), api))
			{
				api.MovePlayer(80, angle[k] + 0.5 * pi);
			}
			else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] - 0.5 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] - 0.5 * pi)), api))
			{
				api.MovePlayer(80, angle[k] - 0.5 * pi);
			}
			else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] + 0.75 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] + 0.75 * pi)), api))
			{
				api.MovePlayer(80, angle[k] + 0.75 * pi);
			}
			else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] - 0.75 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] - 0.75 * pi)), api))
			{
				api.MovePlayer(80, angle[k] - 0.75 * pi);
			}
			else {
				api.Attack(angle[k]);api.Attack(angle[k]);api.Attack(angle[k]);
				api.ThrowCPU(1000, angle[k], self->cpuNum);
			}
		}


		if (jammers[i]->parentTeamID != self->teamID && distance(x0_, a0_, jammers[i]->x, jammers[i]->y) <= 5000)
		{
			for (k = 0;k < 10;k++)
			{
				if (abs(jammers[i]->facingDirection - angle[k]) < pi / 8)
				{
					angle[k] = 0.5 * (angle[k] + jammers[i]->facingDirection);
					multi[k] += 1;num += 1;
					if (distance(jammers[i]->x, jammers[i]->y, x0_, a0_) <= 2500)
					{
						near += 1;
					}
					else away += 1;
					break;
				}
				if (2 * pi - abs(jammers[i]->facingDirection - angle[k]) < pi / 8)

				{
					if (0.5 * (angle[k] + jammers[i]->facingDirection) >= pi)
					{
						angle[k] = 0.5 * (angle[k] + jammers[i]->facingDirection) - pi;
						multi[k] += 1;num += 1;
						break;
						if (distance(jammers[i]->x, jammers[i]->y, x0_, a0_) <= 2500)
						{
							near += 1;
						}
						else away += 1;
					}
					else
					{
						angle[k] = 0.5 * (angle[k] + jammers[i]->facingDirection) + pi;
						multi[k] += 1;num += 1;
						break;
					}
				}
				if (abs(jammers[i]->facingDirection - angle[k]) > 2 * pi)
				{
					angle[k] = jammers[i]->facingDirection;
					break;
				}
			}
		}
		if (num == 1)
		{
			for (k = 0;k < 10;k++)
			{
				if (multi[k] >= 1)
				{
					if (near <= away)
					{
						if (roadclear(api.GridToCell(x0_ + 2000 * cos(angle[k])), api.GridToCell(a0_ + 2000 * sin(angle[k])), api))
						{
							api.MovePlayer(80, angle[k]);
						}
						else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] + 0.25 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] + 0.25 * pi)), api))
						{
							api.MovePlayer(80, angle[k] + 0.25 * pi);
						}
						else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] - 0.25 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] - 0.25 * pi)), api))
						{
							api.MovePlayer(80, angle[k] - 0.25 * pi);
						}
						else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] + 0.5 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] + 0.5 * pi)), api))
						{
							api.MovePlayer(80, angle[k] + 0.5 * pi);
						}
						else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] - 0.5 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] - 0.5 * pi)), api))
						{
							api.MovePlayer(80, angle[k] - 0.5 * pi);
						}
						else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] + 0.75 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] + 0.75 * pi)), api))
						{
							api.MovePlayer(80, angle[k] + 0.75 * pi);
						}
						else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] - 0.75 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] - 0.75 * pi)), api))
						{
							api.MovePlayer(80, angle[k] - 0.75 * pi);
						}
						else {
							api.Attack(angle[k]);api.Attack(angle[k]);api.Attack(angle[k]);
							api.ThrowCPU(1000, angle[k], self->cpuNum);
						}
					}
					else
					{
						if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] + 0.5 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] + 0.5 * pi)), api))
						{
							api.MovePlayer(80, angle[k] + 0.5 * pi);
						}
						else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] - 0.5 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] - 0.5 * pi)), api))
						{
							api.MovePlayer(80, angle[k] - 0.5 * pi);
						}
						else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] + 0.75 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] + 0.75 * pi)), api))
						{
							api.MovePlayer(80, angle[k] + 0.75 * pi);
						}
						else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] - 0.75 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] - 0.75 * pi)), api))
						{
							api.MovePlayer(80, angle[k] - 0.75 * pi);
						}
						else if (roadclear(api.GridToCell(x0_ + 2000 * cos(angle[k])), api.GridToCell(a0_ + 2000 * sin(angle[k])), api))
						{
							api.MovePlayer(80, angle[k]);
						}
						else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] + 0.25 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] + 0.25 * pi)), api))
						{
							api.MovePlayer(80, angle[k] + 0.25 * pi);
						}
						else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] - 0.25 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] - 0.25 * pi)), api))
						{
							api.MovePlayer(80, angle[k] - 0.25 * pi);
						}
						else {
							api.Attack(angle[k]);api.Attack(angle[k]);api.Attack(angle[k]);
							api.ThrowCPU(1000, angle[k], self->cpuNum);
						}
					}


				}
			}
		}
		else if (num >= 2)
		{
			for (k = 0;k < 10;k++)
			{
				if (multi[k] >= 1)
				{
					escape += angle[k];
				}
			}
			escape = escape / num;
			if (near <= away)
			{
				if (roadclear(api.GridToCell(x0_ + 2000 * cos(escape)), api.GridToCell(a0_ + 2000 * sin(escape)), api))
				{
					api.MovePlayer(80, escape);
				}
				else if (roadclear(api.GridToCell(x0_ + 1500 * cos(escape + 0.25 * pi)), api.GridToCell(a0_ + 1500 * sin(escape + 0.25 * pi)), api))
				{
					api.MovePlayer(80, escape + 0.25 * pi);
				}
				else if (roadclear(api.GridToCell(x0_ + 1500 * cos(escape - 0.25 * pi)), api.GridToCell(a0_ + 1500 * sin(escape - 0.25 * pi)), api))
				{
					api.MovePlayer(80, escape - 0.25 * pi);
				}
				else if (roadclear(api.GridToCell(x0_ + 1500 * cos(escape + 0.5 * pi)), api.GridToCell(a0_ + 1500 * sin(escape + 0.5 * pi)), api))
				{
					api.MovePlayer(80, escape + 0.5 * pi);
				}
				else if (roadclear(api.GridToCell(x0_ + 1500 * cos(escape - 0.5 * pi)), api.GridToCell(a0_ + 1500 * sin(escape - 0.5 * pi)), api))
				{
					api.MovePlayer(80, escape - 0.5 * pi);
				}
				else if (roadclear(api.GridToCell(x0_ + 1500 * cos(escape + 0.75 * pi)), api.GridToCell(a0_ + 1500 * sin(escape + 0.75 * pi)), api))
				{
					api.MovePlayer(80, escape + 0.75 * pi);
				}
				else if (roadclear(api.GridToCell(x0_ + 1500 * cos(escape - 0.75 * pi)), api.GridToCell(a0_ + 1500 * sin(escape - 0.75 * pi)), api))
				{
					api.MovePlayer(80, escape - 0.75 * pi);
				}
				else {
					api.ThrowCPU(1000, escape, self->cpuNum);
				}
			}
			else
			{
		 if (roadclear(api.GridToCell(x0_ + 1500 * cos(escape + 0.5 * pi)), api.GridToCell(a0_ + 1500 * sin(escape + 0.5 * pi)), api))
				{
				api.MovePlayer(80, escape + 0.5 * pi);
				}
		else if (roadclear(api.GridToCell(x0_ + 1500 * cos(escape - 0.5 * pi)), api.GridToCell(a0_ + 1500 * sin(escape - 0.5 * pi)), api))
				{
				api.MovePlayer(80, escape - 0.5 * pi);
				}
		else if (roadclear(api.GridToCell(x0_ + 1500 * cos(escape + 0.75 * pi)), api.GridToCell(a0_ + 1500 * sin(escape + 0.75 * pi)), api))
				{
				api.MovePlayer(80, escape + 0.75 * pi);
				}
		else if (roadclear(api.GridToCell(x0_ + 1500 * cos(escape - 0.75 * pi)), api.GridToCell(a0_ + 1500 * sin(escape - 0.75 * pi)), api))
				{
				api.MovePlayer(80, escape - 0.75 * pi);
				}
				else if (roadclear(api.GridToCell(x0_ + 2000 * cos(escape)), api.GridToCell(a0_ + 2000 * sin(escape)), api))
				{
					api.MovePlayer(80, escape);
				}
				else if (roadclear(api.GridToCell(x0_ + 1500 * cos(escape + 0.25 * pi)), api.GridToCell(a0_ + 1500 * sin(escape + 0.25 * pi)), api))
				{
					api.MovePlayer(80, escape + 0.25 * pi);
				}
				else if (roadclear(api.GridToCell(x0_ + 1500 * cos(escape - 0.25 * pi)), api.GridToCell(a0_ + 1500 * sin(escape - 0.25 * pi)), api))
		       {
					api.MovePlayer(80, escape - 0.25 * pi);
				}
				else {
					api.ThrowCPU(1000, escape, self->cpuNum);
				}
			}
		}
		else {
			for (k = 0;k < 10;k++)
			{
				double theta = atan2(abs((double)jammers[i]->y - (double)a0_), abs((double)jammers[i]->x - (double)x0_));
				if (jammers[i]->x <= x0_ && jammers[i]->y >= a0_)
					theta = pi - theta;
				else if (jammers[i]->x <= x0_ && jammers[i]->y <= a0_)
					theta = theta + pi;
				else if (jammers[i]->x >= x0_ && jammers[i]->y <= a0_)
					theta = 2 * pi - theta;
				if (theta < angle[k] && roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] - 0.5 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] - 0.5 * pi)), api))
					api.MovePlayer(80, angle[k] - 0.5 * pi);
				else if (theta >= angle[k] && roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] + 0.5 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] + 0.5 * pi)), api))
					api.MovePlayer(80, angle[k] + 0.5 * pi);
				else if (roadclear(api.GridToCell(x0_ + 2000 * cos(angle[k])), api.GridToCell(a0_ + 2000 * sin(angle[k])), api))
					api.MovePlayer(80, angle[k]);
				else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] + 0.75 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] + 0.75 * pi)), api))
				{
					api.MovePlayer(80, angle[k] + 0.75 * pi);
				}
				else if (roadclear(api.GridToCell(x0_ + 1500 * cos(angle[k] - 0.75 * pi)), api.GridToCell(a0_ + 1500 * sin(angle[k] - 0.75 * pi)), api))
				{
					api.MovePlayer(80, angle[k] - 0.75 * pi);
				}
			}
		}
	}
}

bool roadclear(int a, int b, IAPI& api)
{
	int i;
	double theta = atan2(abs((double)api.CellToGrid(b) - (double)a0_), abs(api.CellToGrid(a) - (double)x0_));
	if (api.CellToGrid(a) <= x0_ && api.CellToGrid(b) >= a0_)
		theta = pi - theta;
	else if (api.CellToGrid(a) <= x0_ && api.CellToGrid(b) <= a0_)
		theta = theta + pi;
	else if (api.CellToGrid(a) >= x0_ && api.CellToGrid(b) <= a0_)
		theta = 2 * pi - theta;

	for (i = 1;500 * i < distance(api.CellToGrid(a), api.CellToGrid(b), x0_, a0_);i++)
	{
		if (mapinfo[api.GridToCell(x0_ + 500 * i * cos(theta))][api.GridToCell(a0_ + 500 * i * sin(theta))] == 1 && map1[api.GridToCell(x0_ + 500 * i * cos(theta))][api.GridToCell(a0_ + 500 * i * sin(theta))] != THUAI5::PlaceType::BirthPlace4 && map1[api.GridToCell(x0_ + 500 * i * cos(theta))][api.GridToCell(a0_ + 500 * i * sin(theta))] != THUAI5::PlaceType::BirthPlace7)
			return 0;
		else if (mapinfo[api.GridToCell(x0_ + 500 * i * cos(theta - pi / (2*i+1)))][api.GridToCell(a0_ + 500 * i * sin(theta - pi / (2 * i + 1)))] == 1)
			return 0;
		else if (mapinfo[api.GridToCell(x0_ + 500 * i * cos(theta + pi / (2*i+1)))][api.GridToCell(a0_ + 500 * i * sin(theta + pi / (2 * i + 1)))] == 1)
			return 0;
	}
	return 1;
}

int ifalert(IAPI& api)
{
	auto self = api.GetSelfInfo();
	int i, j;
	auto jammers = api.GetSignalJammers();
	auto Robots = api.GetRobots();
	for (i = 0;i < jammers.size();i++)
	{
		if (jammers[i]->parentTeamID != self->teamID && ((jammers[i]->type == THUAI5::SignalJammerType::StrongJammer && distance(x0_, a0_, jammers[i]->x, jammers[i]->y) < 10000) || (jammers[i]->type == THUAI5::SignalJammerType::LineJammer && distance(x0_, a0_, jammers[i]->x, jammers[i]->y) < 4500) ||
			(jammers[i]->type == THUAI5::SignalJammerType::CommonJammer && distance(x0_, a0_, jammers[i]->x, jammers[i]->y) < 6000) || (jammers[i]->type == THUAI5::SignalJammerType::FastJammer && distance(x0_, a0_, jammers[i]->x, jammers[i]->y) < 3000)))
			return 1;
	}
	for (i = 0;i < Robots.size();i++)
	{
		if (self->teamID != Robots[i]->teamID && Robots[i]->isResetting == 0 && roadclear(api.GridToCell(Robots[i]->x), api.GridToCell(Robots[i]->y), api) && distance(Robots[i]->x, Robots[i]->y, x0_, a0_) < 20000 && self->signalJammerNum >= 1 && (self->timeUntilCommonSkillAvailable == 0 || self->timeUntilCommonSkillAvailable >= 24000))
		{
			return 2;
		}
	}
	return 0;
}