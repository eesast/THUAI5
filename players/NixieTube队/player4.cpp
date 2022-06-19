#include <random>
#include "../include/AI.h"
#include <vector>
#pragma once
#include <list>
#include <math.h>
/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;


// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Amplification;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::EnergyConvert;

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}


const double kCost1 = 1; //直移一格消耗
const double kCost2 = sqrt(2); //斜移一格消耗

struct Point
{
	int x, y; //点坐标，这里为了方便按照C++的数组来计算，x代表横排，y代表竖列
	double F, G, H; //F=G+H
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
	//void updatePlayerList(std::vector<std::shared_ptr<const THUAI5::Robot>>playerList, std::shared_ptr<const THUAI5::Robot>self);
private:
	Point* findPath(Point& startPoint, Point& endPoint, bool isIgnoreCorner);
	std::vector<Point*> getSurroundPoints(const Point* point, bool isIgnoreCorner) const;
	bool isCanreach(const Point* point, const Point* target, bool isIgnoreCorner) const; //判断某点是否可以用于下一步判断
	Point* isInList(const std::list<Point*>& list, const Point* point) const; //判断开启/关闭列表中是否包含某点
	Point* getLeastFpoint(); //从开启列表中返回F值最小的节点
	//计算FGH值
	double calcG(Point* temp_start, Point* point);
	double calcH(Point* point, Point* end);
	double calcF(Point* point);
private:
	std::vector<std::vector<int>> maze;
	std::list<Point*> openList;  //开启列表
	std::list<Point*> closeList; //关闭列表
	//std::vector<Point>lastPlayerPoints;
};

//void Astar::updatePlayerList(std::vector<std::shared_ptr<const THUAI5::Robot>>playerList, std::shared_ptr<const THUAI5::Robot>self) {
//	for (int i = 0, len = lastPlayerPoints.size(); i < len; ++i) {
//		this->maze[lastPlayerPoints[i].x][lastPlayerPoints[i].y] = 0;
//	}
//	lastPlayerPoints.clear();
//	for (auto itr = playerList.begin(); itr != playerList.end(); ++itr) {
//		if ((*itr)->teamID != self->teamID) continue;
//		if (((*itr)->teamID == self->teamID) && (*itr)->playerID == self->playerID) continue;
//		Point* p = new Point((*itr)->x / 500, (*itr)->y / 500);
//		//std::cout << (*itr)->x / 500 << ' ' << (*itr)->y / 500<<' '<< this->maze.size() << std::endl;
//
//
//		for (int i = (*itr)->x / 500; i < (*itr)->x / 500 + 1 + 1 + 1 + 1; ++i) {
//			for (int j = (*itr)->y / 500; j < (*itr)->y / 500 + 1 + 1 + 1 + 1; ++j) {
//				p->x = i;
//				p->y = j;
//				if (!this->maze[p->x][p->y]) {
//					lastPlayerPoints.push_back(*p);
//					this->maze[p->x][p->y] = 1;
//				}
//			}
//		}
//
//		delete p;
//	}
//}

void Astar::InitAstar(std::vector<std::vector<int>>& _maze)
{
	maze = _maze;
}

double Astar::calcG(Point* temp_start, Point* point)
{
	double extraG = (abs(point->x - temp_start->x) + abs(point->y - temp_start->y)) == 1 ? kCost1 : kCost2;
	double parentG = point->parent == NULL ? 0 : point->parent->G; //如果是初始节点，则其父节点是空
	return parentG + extraG;
}

double Astar::calcH(Point* point, Point* end)
{
	//8个方向移动，故H采用对角距离计算而得
	double x = abs(end->x - point->x);
	double y = abs(end->y - point->y);
	double min = x < y ? x : y;
	return (x + y) * kCost1 - 2 * min * kCost1 + sqrt(2) * min * kCost2;
	//欧几里得距离：
	//return sqrt((double)(end->x - point->x) * (double)(end->x - point->x) + (double)(end->y - point->y) * (double)(end->y - point->y)) * kCost1;
}

double Astar::calcF(Point* point)
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

bool InPath(const int& row, const int& col, const std::list<Point*>& path) {
	for (const auto& p : path) {
		if (row == p->x && col == p->y) {
			return true;
		}
	}
	return false;
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
	for (auto itr = openList.begin(); itr != openList.end(); itr++) {
		int x = (*itr)->x;
		int y = (*itr)->y;
		if (!InPath(x, y, path)) {
			delete* itr;
			*itr = NULL;
		}
	}
	openList.clear();
	for (auto itr = closeList.begin(); itr != closeList.end(); itr++) {
		int x = (*itr)->x;
		int y = (*itr)->y;
		if (!InPath(x, y, path)) {
			delete* itr;
			*itr = NULL;
		}
	}
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

	for (int x = point->x - 1; x <= point->x + 1; x++) {
		for (int y = point->y - 1; y <= point->y + 1; y++) {
			Point* temp = new Point(x, y);
			if (isCanreach(point, temp, isIgnoreCorner))
				surroundPoints.push_back(temp);
			else {
				delete temp;
				temp = NULL;
			}
		}
	}
	return surroundPoints;
}

#include <iostream>


double e = 0;
int cpunum = 0;
int PlaceType[52][52] = { 1 }; //这是为了后面的方便
std::vector<std::vector<int>> map;
Astar astar;
int movetime = 50;
int orix = 0, oriy = 0;
int targetx = 0, targety = 0;

int Goto(int cellx, int celly, IAPI& api, Astar astar) {
	auto self = api.GetSelfInfo();
	int selfx = self->x / 500 + 2;
	int selfy = self->y / 500 + 2;
	int endX = 0, endY = 0;
	for (int m = 2 * (cellx + 1); m <= 2 * (cellx + 1) + 1; m++) {
		for (int n = 2 * (celly + 1); n <= 2 * (celly + 1) + 1; n++) {
			if (map[m][n] != 1) {
				endX = m;
				endY = n;
				break;
			}
		}
	}
	Point start(selfx, selfy);
	Point end(endX, endY);
	std::list<Point*> path = astar.GetPath(start, end, false);
	if (path.empty()) return 0;
	delete path.front();
	path.front() = NULL;
	path.pop_front();
	int x = path.front()->x;
	int y = path.front()->y;
	for (auto itr = path.begin(); itr != path.end(); itr++) {
		delete* itr;
		*itr = NULL;
	}
	if (x > selfx && y == selfy) api.MoveDown(movetime);
	else if (x > selfx && y < selfy) api.MovePlayer(movetime, -3.1415926 / 4);
	else if (x == selfx && y < selfy) api.MoveLeft(movetime);
	else if (x < selfx && y < selfy) api.MovePlayer(movetime, -3.1415926 * 3 / 4);
	else if (x < selfx && y == selfy) api.MoveUp(movetime);
	else if (x < selfx && y > selfy) api.MovePlayer(movetime, 3.1415926 * 3 / 4);
	else if (x == selfx && y > selfy) api.MoveRight(movetime);
	else if (x > selfx && y > selfy) api.MovePlayer(movetime, 3.1415926 / 4);
	return 1;
}

double Distance(int x, int y, IAPI& api) {
	auto self = api.GetSelfInfo();
	return sqrt((self->x - x) * (self->x - x) + (self->y - y) * (self->y - y));
}


void AI::play(IAPI& api)
{
	std::ios::sync_with_stdio(false);
	auto self = api.GetSelfInfo();
	auto Robots = api.GetRobots();
	auto Count = api.GetFrameCount();
	int fightingstate = 0;
	int deliveringstate = 0;
	if (Count == 1) {
		orix = self->x;
		oriy = self->y;
		for (int i = 1; i < 51; i++) {
			for (int j = 1; j < 51; j++) {
				PlaceType[i][j] = (int)api.GetPlaceType(i - 1, j - 1);
			}
		}
		for (int i = 0; i < 103; i++) {
			std::vector<int> B;
			for (int j = 0; j < 103; j++) {
				B.push_back(0);
			}
			map.push_back(B);
		}
		for (int i = 1; i < 51; i++) {
			for (int j = 1; j < 51; j++) {
				if (PlaceType[i][j] == 1 || ((PlaceType[i][j] >= 5 && PlaceType[i][j] <= 12) && (i != orix / 1000 + 1 || j != oriy / 1000 + 1))) {
					for (int m = 2 * i - 1; m <= 2 * (i + 1); m++) {
						for (int n = 2 * j - 1; n <= 2 * (j + 1); n++) {
							map[m][n] = 1;
						}
					}
				}
				if (PlaceType[i][j] == 4 * self->teamID + 7) {
					targetx = i-1;
					targety = j-1;
				}
			}
		}
		astar.InitAstar(map);
	}
	if (Robots.size() != 0)
	{
		int exrange = 0;
		for (auto itr = Robots.begin(); itr != Robots.end(); ++itr) {
			if ((*itr)->teamID != self->teamID) {
				if ((*itr)->signalJammerType == THUAI5::SignalJammerType::CommonJammer) exrange = 3000;
				else if ((*itr)->signalJammerType == THUAI5::SignalJammerType::FastJammer) exrange = 1500;
				else if ((*itr)->signalJammerType == THUAI5::SignalJammerType::LineJammer) exrange = 6000;
				else if ((*itr)->signalJammerType == THUAI5::SignalJammerType::StrongJammer) exrange = 9000;
				if (Distance((*itr)->x, (*itr)->y, api) <= 9000 && self->timeUntilCommonSkillAvailable==0) {
					api.UseCommonSkill();
					::e = atan2(((double)(*itr)->y - (double)self->y), ((double)(*itr)->x - (double)self->x));
					api.Attack(::e);
					::e = ::e - 3.1415926 / 5;
					api.Attack(::e);
					::e = ::e + 3.1415926 / 5 * 2;
					api.Attack(::e);
					::e = ::e - 3.1415926 / 5 * 3;
					api.Attack(::e);
					::e = ::e + 3.1415926 / 5 * 4;
					api.Attack(::e);
					/*if ((*itr)->signalJammerNum == 0 && self->signalJammerNum >= 2) {
						if (PlaceType[(*itr)->x / 1000 + 1][(*itr)->y / 1000 + 1] != 1) {
							if (Goto((*itr)->x / 1000, (*itr)->y / 1000, api, astar) == 0) {
								PlaceType[(*itr)->x / 1000 + 1][(*itr)->y / 1000 + 1] = 1;
							}
						}
					}*/
					/*else if ((*itr)->signalJammerNum > self->signalJammerNum) {
						::e = atan2(((double)self->y - (double)(*itr)->y), ((double)self->x - (double)(*itr)->x));
						api.MovePlayer(50, ::e);
						fightingstate = 0;
					}*/
				}
				else if (Distance((*itr)->x, (*itr)->y, api) <= 9000 && self->timeUntilCommonSkillAvailable!=0) {
					::e = atan2(((double)(*itr)->y - (double)self->y), ((double)(*itr)->x - (double)self->x));
					api.Attack(::e);
					break;
				}
			}
			else {
				if (Distance((*itr)->x, (*itr)->y, api) <= 1500) {
					::e = atan2(((double)self->y - (double)(*itr)->y), ((double)self->x - (double)(*itr)->x));
					api.MovePlayer(50, ::e);
				}
			}
		}
	}
	auto Props = api.GetProps();//获取所有道具信息
	auto cpu = THUAI5::PropType::CPU;
	auto ptype = THUAI5::PropType::Battery;
	//if (self->cpuNum >= 2) {
	//	deliveringstate = 1;
	//}
	if (Props.size() != 0 && fightingstate == 0)
	{
		int mindistance1 = 10000000, mindistance2 = 100000000, x = -1, y = -1, otherx = 0, othery = 0;
		for (auto itr = Props.begin(); itr != Props.end(); ++itr) {
			if ((*itr)->type == cpu) {
				if (Distance((*itr)->x, (*itr)->y, api) < mindistance1  &&  
					sqrt(((*itr)->x-targetx*1000-500)*((*itr)->x-targetx*1000-500) + ((*itr)->y - targety*1000-500) * ((*itr)->y - targety*1000-500))>=2500  &&
					PlaceType[(*itr)->x/1000+1][(*itr)->y/1000+1]!=1) {
					mindistance1 = Distance((*itr)->x, (*itr)->y, api);
					x = (*itr)->x;
					y = (*itr)->y;
				}
			}
			else {
				if (Distance((*itr)->x, (*itr)->y, api) < mindistance2  && 
					PlaceType[(*itr)->x / 1000 + 1][(*itr)->y / 1000 + 1] != 1) {
					mindistance2 = Distance((*itr)->x, (*itr)->y, api);
					otherx = (*itr)->x;
					othery = (*itr)->y;
					ptype = (*itr)->type;
				}
			}
		}
		if (x != -1 && (mindistance2 >= 10000 || mindistance1 <= mindistance2)) {
			if (self->x / 1000 == x / 1000 && self->y / 1000 == y / 1000) {
				api.Pick(cpu);
			}
			else if (deliveringstate == 0 || mindistance1 <= 10000) {
				deliveringstate = 0;
				if (Goto(x / 1000, y / 1000, api, astar) == 0) {
					PlaceType[x / 1000 + 1][y / 1000 + 1] = 1;
				};
			}
		}
		else {
			if (self->x / 1000 == otherx / 1000 && self->y / 1000 == othery / 1000) {
				api.Pick(ptype);
			}
			else if (deliveringstate == 0) {
				if (Goto(otherx / 1000, othery / 1000, api, astar) == 0) {
					PlaceType[otherx / 1000 + 1][othery / 1000 + 1] = 1;
				};
			}
		}
	}
	else if (Props.size() == 0 && fightingstate == 0) {
		Goto(14, 14, api, astar);
	}
	if (deliveringstate == 1) {
		if (Distance(targetx*1000+500, targety*1000+500, api) < 2000) {
			::e = atan2(((double)targety*1000+500 - (double)self->y), ((double)targetx*1000+500 - (double)self->x));
			api.ThrowCPU(666, ::e, self->cpuNum);
			deliveringstate = 0;
		}
		else {
			int diff = self->teamID == 0 ? 1 : -1;
			Goto(targetx+diff, targety, api, astar);
		}
	}
	if (self->prop == THUAI5::PropType::Booster || self->prop == THUAI5::PropType::Shield || self->prop == THUAI5::PropType::Battery || self->prop == THUAI5::PropType::ShieldBreaker) {
		api.UseProp();
	}
	if (self->cpuNum >= 5) {
		api.UseCPU(self->cpuNum);
	}
}