#include <random>
#include "../include/AI.h"
#include <queue>
#include<map>
#include<vector>
#include <thread>
#include<chrono>
#include"../include/structures.h"
using namespace std::literals::chrono_literals;

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;

//MASTER_
// 选手主动技能，选手 !!必须! ! 定义此变量来选择主动技能
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Amplification;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::EnergyConvert;

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}
int birthx0 = -1, birthy0 = -1;
bool back = false;
double deltatime_wyx = 0;
std::chrono::system_clock::time_point starttime_wyx;
std::chrono::system_clock::time_point endtime_wyx;
double deltatime_send = 0;
bool send1 = false;
std::chrono::system_clock::time_point starttime_send;
std::chrono::system_clock::time_point endtime_send;
bool moveornot = true;
bool fetch = false;
bool conflict = false;
bool throwcpu = false;
bool time1 = true;
int xx_gth[4] = { -1,-1,-1,-1 }, yy_gth[4] = { -1,-1,-1,-1 };
double deltatime_gth;
std::chrono::system_clock::time_point startTime_gth, endTime_gth, lastShot_gth, present_gth;
double bulletVelocity_gth, bulletTime_gth;
double bulletVelocities_gth[4] = { 3000,3000,6000,3000 }, bulletRanges_gth[4] = { 4500,900,9000,3000 };
double runThreshold_gth = 1;
int propsGotten = 0;
//MARS更改 20220423   改变了初始化值
int px[5] = { -1,-1,-1,-1,-1 }, py[5] = { -1,-1,-1,-1,-1 };
int propguid;
int otherpropguid[5];
const int maxVerticeNum = 1000;
const int maxEdgeNum = 8000;//为dijkstra服务
int VerticeNum = 0;
int VerticeNum_origin = 0;
int EdgeNum = 0;
int EdgeNum_origin = 0;
bool ViceHaveTask = 0;
int tolerance_deviation = 3;//cell  vice搜索prop的视野  正方形半边长
int tolerance_griddis_max = 3000;//允许的主将与副将的位置差
int tolerance_griddis_min = 1500;//允许的主将与副将的位置差
								 //需要根据具体情况而更改  主将和副将的编号
const uint64_t CAP1 = 0;
const uint64_t CAP2 = 3;
const uint64_t VICE1 = 1;
const uint64_t VICE2 = 2;

bool lastminite = 0;

//MARS20220504
THUAI5::PropType prop_array_for_bfslim[5] = { THUAI5::PropType::CPU ,THUAI5::PropType::Booster ,THUAI5::PropType::Battery ,THUAI5::PropType::Shield ,THUAI5::PropType::ShieldBreaker };
THUAI5::PropType prop_array_for_bfs[5] = { THUAI5::PropType::CPU,THUAI5::PropType::Battery ,THUAI5::PropType::Booster  ,THUAI5::PropType::Shield ,THUAI5::PropType::ShieldBreaker };



uint32_t time_available_mars = 300;
uint32_t originSpeed = 0;




//MARS
bool flag_scan = 0;
int NewMap[50][50] = { 0 };

THUAI5::Robot selfinfo;
void getinfo(IAPI& api) {
	auto temp = api.GetSelfInfo();
	//std::shared_ptr<const THUAI5::Robot> temp(api.GetSelfInfo());
	bool canMove = temp->canMove;                                   // 是否可以移动
	bool isResetting = temp->isResetting;                               // 是否在复活中

	selfinfo.x = temp->x;                                      // x坐标
	selfinfo.y = temp->y;
	//////std::cout << "selfffx" << self.x << " " << "selfffy" << self.y << std::endl;
	// y坐标
	selfinfo.signalJammerNum = temp->signalJammerNum;                       // 信号干扰器数量 
	selfinfo.speed = temp->speed;                                 // 机器人移动速度
	selfinfo.life = temp->life;                                  // 电量（生命值）
	selfinfo.cpuNum = temp->cpuNum;                                // CPU数
	selfinfo.radius = temp->radius;                                // 圆形物体的半径或正方形物体的内切圆半径
	selfinfo.CD = temp->CD;                                    // 回复一个信号干扰器需要的时间
	selfinfo.lifeNum = temp->lifeNum;		                        // 第几次复活
	selfinfo.score = temp->score;                                 // 分数

	selfinfo.teamID = temp->teamID;                                // 队伍ID
	selfinfo.playerID = temp->playerID;                              // 玩家ID
	selfinfo.guid = temp->guid;                                  // 全局唯一ID

	selfinfo.attackRange = temp->attackRange;                             // 攻击范围
	selfinfo.timeUntilCommonSkillAvailable = temp->timeUntilCommonSkillAvailable;           // 普通软件效果的冷却时间 
	selfinfo.timeUntilUltimateSkillAvailable = temp->timeUntilUltimateSkillAvailable;         // 特殊软件效果的冷却时间
	selfinfo.emissionAccessory = temp->emissionAccessory;                       // 强制功率发射配件工作效率

	selfinfo.buff = temp->buff;                     // 所拥有的buff
	selfinfo.prop = temp->prop;                                  // 所持有的道具
	selfinfo.place = temp->place;                                // 机器人所在位置
	selfinfo.signalJammerType = temp->signalJammerType;              // 信号干扰器类型
	selfinfo.hardwareType = temp->hardwareType;                      // 持有的硬件属性（被动技能） 
	selfinfo.softwareType = temp->softwareType;                      // 持有的软件属性（主动技能）
};

struct cellinfo {
	int placetype;
	int props[6];
};

cellinfo enhanced_map[50][50];
// 获取指定格子中心的坐标
int celltogrid(int cell)
{
	return cell * num_of_grid_per_cell + num_of_grid_per_cell / 2;
}

// 获取指定坐标点所位于的格子的 X 序号
int gridtocell(int grid)
{
	return grid / num_of_grid_per_cell;
}
int getplacetype(int i, int j) {
	return enhanced_map[i][j].placetype;
}
void enhanced_scan_map(IAPI& api) {
	for (int i = 0; i < 50; i++) {
		for (int j = 0; j < 50; j++) {
			enhanced_map[i][j].placetype = (int)api.GetPlaceType(i, j);
		}
	}
	auto Props = api.GetProps();
	int Propsize = Props.size();
	for (int i = 0; i < Propsize; i++) {
		if (Props[i]->isMoving)continue;
		enhanced_map[gridtocell(Props[i]->x)][gridtocell(Props[i]->y)].props[((int)Props[i]->type)] += 1;
	}
	if (!lastminite) {
		for (int i = 1; i <= 5; i++) {
			enhanced_map[birthx0 + 1][birthy0 + 1].props[i] = 0;
		}

	}
}


struct vertice;

struct edge
{
	double value;
	edge* next_in_edge, * next_out_edge;//共用一个起点或终点的边
	int from, to;//起点，终点
}edges[maxEdgeNum];//为dijkstra服务
edge edges_origin[maxEdgeNum];
struct cellPosition;

struct gridPosition {
	int x, y;
	bool operator<(const gridPosition& other)const {
		if (this->x < other.x) return 1;
		else if (this->x == other.x && this->y < other.y) return 1;
		return 0;
	}
	operator cellPosition();
	bool operator!=(const gridPosition& other);
};
bool gridPosition::operator!=(const gridPosition& other) {
	if (x != other.x) return 1;
	if (y != other.y) return 1;
	return 0;
}
std::vector<gridPosition> antiroute;//将所需经过的结点反向压入栈，最近的结点在最后

gridPosition ViceLastTask = { -1,-1 };

struct vertice {
	edge* head, * bottom;
	gridPosition GridPosition;
};
static vertice vertices[maxVerticeNum];
static vertice vertices_origin[maxVerticeNum];

struct cellPosition
{
	int x, y;
	bool operator<(const cellPosition& other)const {
		if (this->x < other.x) return 1;
		else if (this->x == other.x && this->y < other.y) return 1;
		return 0;
	}
	operator gridPosition() {
		gridPosition tt;
		tt.x = this->x * num_of_grid_per_cell + num_of_grid_per_cell / 2;
		tt.y = this->y * num_of_grid_per_cell + num_of_grid_per_cell / 2;
		return tt;
	}
};//格点位置信息,cell

gridPosition::operator cellPosition() {
	cellPosition tt;
	tt.x = this->x / num_of_grid_per_cell;
	tt.y = this->y / num_of_grid_per_cell;
	return tt;
}


int Dirx[5] = { 0,-1,0,1,0 };//四种垂直的方向
int Diry[5] = { 0,0,1,0,-1 };
struct bfsResult
{
	cellPosition goalstate;
	int pathLen;//路径长度
};
bfsResult newBfsTo(cellPosition startPos, cellPosition endPos);



void getProp_gth(IAPI& api)
{

	auto self = selfinfo;
	bulletVelocity_gth = bulletVelocities_gth[((int)self.signalJammerType) - 1];
	bulletTime_gth = bulletRanges_gth[((int)self.signalJammerType) - 1] / bulletVelocity_gth;
}
/// <summary>
/// 使用时需要卜佳木的struct cellPosition、struct bfsResult和bfsResult newBfsTo(IAPI& api, cellPosition startPos, cellPosition endPos)
/// </summary>
/// <param name="api">api</param>
/// <returns>攻击则返回真，什么都不做返回假</returns>
bool attack_gth(IAPI& api)
{
	//////printf("\ncall attack\n");
	int attacking = 0;
	auto self = selfinfo;
	endTime_gth = std::chrono::system_clock::now();
	deltatime_gth = std::chrono::duration_cast<std::chrono::microseconds>(endTime_gth - startTime_gth).count() / 1000.0;
	auto Robots = api.GetRobots();
	startTime_gth = std::chrono::system_clock::now();
	double x[4] = { -1,-1,-1,-1 }, y[4] = { -1,-1,-1,-1 };
	double vx[4] = { 0,0,0,0 }, vy[4] = { 0,0,0,0 };
	if (Robots.size() != 0)
	{


		////////std::cout << Robots[0]->attackRange << std::endl;
		for (int i = 0; i < Robots.size(); i++)
		{
			if (!Robots[i]->isResetting) {
				if (Robots[i]->teamID != self.teamID)
				{

					x[Robots[i]->playerID] = Robots[i]->x;
					y[Robots[i]->playerID] = Robots[i]->y;
				}
			}
			
		}
		int targetID = -1;
		double minDistance = 500000;
		for (int i = 0; i < 4; i++)
		{
			if (x[i] >= 0 && xx_gth[i] >= 0)
			{


				vx[i] = (x[i] - xx_gth[i]) / 0.1;
				vy[i] = (y[i] - yy_gth[i]) / 0.1;
				if (sqrt((x[i] - self.x) * (x[i] - self.x) + (y[i] - self.y) * (y[i] - self.y)) < minDistance)
				{

					minDistance = sqrt((x[i] - self.x) * (x[i] - self.x) + (y[i] - self.y) * (y[i] - self.y));
					targetID = i;
				}
			}
		}
		int oKToShoot = 0;
		present_gth = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::microseconds>(present_gth - lastShot_gth).count() / 1000.0 > 1200)
		{


			oKToShoot = 1;
		}
		////////printf("\ntargetID >= 0: %s\nbulletTime_gth * bulletVelocity_gth / 1000==%f\n", targetID >= 0, bulletTime_gth * bulletVelocity_gth / 1000);
		//if (targetID >= 0 && minDistance <= bulletTime_gth * bulletVelocity_gth / 1000 && oKToShoot)
		if (targetID >= 0 && minDistance <= 9000 && oKToShoot)
		{


			int a = self.x, b = self.y, c = x[targetID], d = y[targetID], e = vx[targetID], f = vy[targetID];
			cellPosition pre = { a,b }, des = { c,d };

			//////printf("newbfsto=%d\n", newBfsTo(api, pre, des).pathLen);

			if (newBfsTo(pre, des).pathLen * 1000 <= 1.2 * (abs(a - c) + abs(b - d)))
			{


				double A = (e * e + f * f - bulletVelocity_gth * bulletVelocity_gth), B = 2 * (e * (c - a) + f * (b - d)), C = ((c - a) * (c - a) + (b - d) * (b - d));
				double t = -1, theta;
				if (B * B - 4 * A * C >= 0)
				{
					if ((-B + sqrt(B * B - 4 * A * C)) / (2 * A) > 0)
						t = (-B + sqrt(B * B - 4 * A * C)) / (2 * A);
					else
						t = (-B - sqrt(B * B - 4 * A * C)) / (2 * A);
				}
				if (t > 0)
				{
					double xxx = e * t + c - a, yyy = f * t + d - b;
					if (xxx > 0)
					{
						theta = atan(yyy / xxx);
					}
					else if (xxx == 0)
					{
						if (yyy > 0) theta = 3.14159265 / 2;
						else theta = -3.14159265 / 2;
					}
					else
					{
						theta = 3.14159265 + atan(yyy / xxx);
					}
					api.Attack(theta);
					attacking = 1;
					lastShot_gth = std::chrono::system_clock::now();
				}
			}
		}
		for (int i = 0; i < 4; i++)
		{
			xx_gth[i] = x[i];
			yy_gth[i] = y[i];
		}
	}
	return attacking;
}

/// <summary>
/// 使用时需要卜佳木的struct cellPosition、struct bfsResult和bfsResult newBfsTo(IAPI& api, cellPosition startPos, cellPosition endPos)
/// </summary>
/// <param name="api">api</param>
/// <returns>逃跑则返回真，什么都不做返回假</returns>
bool RUN_gth(IAPI& api)
{

	//THUAI5::Robot self = selfinfo;
	auto jammers = api.GetSignalJammers();
	if (jammers.size() != 0)
	{
		////////printf("1111");

		for (int i = 0; i < jammers.size(); i++)
		{
			if (jammers[i]->parentTeamID != selfinfo.teamID)
			{
				////////printf("2222");

				double l = sqrt((jammers[i]->x - selfinfo.x) * (jammers[i]->x - selfinfo.x) + (jammers[i]->y - selfinfo.y) * (jammers[i]->y - selfinfo.y));
				if (l <= runThreshold_gth * bulletRanges_gth[(int)jammers[i]->type - 1])
				{
					//////printf("\nl=%f\n",l);

					int a = selfinfo.x, b = selfinfo.y, c = jammers[i]->x, d = jammers[i]->y;
					cellPosition pre = { a,b }, des = { c,d };
					if (newBfsTo(pre, des).pathLen * 1000 <= 1.2 * (abs(a - c) + abs(b - d)))
					{
						////////printf("4444");

						double e = jammers[i]->facingDirection;
						e += 0.5 * 3.14159265;
						api.MovePlayer(300, e);
						//////printf("\nmoved\n");
						return 1;
					}
				}
			}
		}
	}
	return 0;
}


//player：武永祥
//尝试使用bfs写出运动系统，并为ai控制留下接口
//2022/4/5
double f(short w, short t) {   //权值函数
	return (double)w * (-(double)t / 100 + 1);
}
bool shoot(int x, int y, int xx, int yy) {
	return false;
}






//MARS
//建图
void ScanMap(int(*NewMap)[50], IAPI& api) {
	//init
	memset(NewMap, 0, sizeof(int) * 50 * 50);
	//up  x-  down x+  left y-  right y+
	for (int i = 0; i < 48; i++)
	{
		for (int j = 0; j < 48; j++)
		{

			for (int k = 0; k < 4; k++) //ul,ur,dl,dr
			{
				int ul = getplacetype(i, j);
				int ur = getplacetype(i, j + 1);
				int dl = getplacetype(i + 1, j);
				int dr = getplacetype(i + 1, j + 1);
				if (dr == (int)THUAI5::PlaceType::BirthPlace1) {
					if (((selfinfo.teamID) * 4 + selfinfo.playerID + 5) != (int)THUAI5::PlaceType::BirthPlace1)
						NewMap[i + 1][j + 1] = -1;
					if (selfinfo.teamID == 0) { birthx0 = i; birthy0 = j; }
					////////printf("birth1  %d %d %d\n", (int)THUAI5::PlaceType::BirthPlace1,i + 1, j + 1);
				}
				if (dr == (int)THUAI5::PlaceType::BirthPlace2) {
					if (((selfinfo.teamID) * 4 + selfinfo.playerID + 5) != (int)THUAI5::PlaceType::BirthPlace2)
						NewMap[i + 1][j + 1] = -1;

					////////printf("birth2  %d %d %d\n", (int)THUAI5::PlaceType::BirthPlace2, i + 1, j + 1);
				}
				if (dr == (int)THUAI5::PlaceType::BirthPlace3) {
					if (((selfinfo.teamID) * 4 + selfinfo.playerID + 5) != (int)THUAI5::PlaceType::BirthPlace3)
						NewMap[i + 1][j + 1] = -1;

					////////printf("birth3  %d %d %d\n", (int)THUAI5::PlaceType::BirthPlace3, i + 1, j + 1);
				}
				if (dr == (int)THUAI5::PlaceType::BirthPlace4) {
					if (((selfinfo.teamID) * 4 + selfinfo.playerID + 5) != (int)THUAI5::PlaceType::BirthPlace4)
						NewMap[i + 1][j + 1] = -1;

					////////printf("birth4  %d %d %d\n", (int)THUAI5::PlaceType::BirthPlace4, i + 1, j + 1);
				}
				if (dr == (int)THUAI5::PlaceType::BirthPlace5) {
					if (((selfinfo.teamID) * 4 + selfinfo.playerID + 5) != (int)THUAI5::PlaceType::BirthPlace5)
						NewMap[i + 1][j + 1] = -1;

					////////printf("birth5  %d %d %d\n", (int)THUAI5::PlaceType::BirthPlace5, i + 1, j + 1);
				}
				if (dr == (int)THUAI5::PlaceType::BirthPlace6) {
					if (((selfinfo.teamID) * 4 + selfinfo.playerID + 5) != (int)THUAI5::PlaceType::BirthPlace6)
						NewMap[i + 1][j + 1] = -1;

					////////printf("birth6  %d %d %d\n", (int)THUAI5::PlaceType::BirthPlace6, i + 1, j + 1);
				}
				if (dr == (int)THUAI5::PlaceType::BirthPlace7) {
					if (((selfinfo.teamID) * 4 + selfinfo.playerID + 5) != (int)THUAI5::PlaceType::BirthPlace7)
						NewMap[i + 1][j + 1] = -1;

					////////printf("birth7  %d %d %d\n", (int)THUAI5::PlaceType::BirthPlace7, i + 1, j + 1);
				}
				if (dr == (int)THUAI5::PlaceType::BirthPlace8) {
					if (((selfinfo.teamID) * 4 + selfinfo.playerID + 5) != (int)THUAI5::PlaceType::BirthPlace8)
						NewMap[i + 1][j + 1] = -1;
					if (selfinfo.teamID == 1) { birthx0 = i; birthy0 = j; }
					////////printf("birth8  %d %d %d\n", (int)THUAI5::PlaceType::BirthPlace8, i + 1, j + 1);
				}

				if (getplacetype(i, j) == (int)THUAI5::PlaceType::BirthPlace1) {
					NewMap[i][j] = 2;
				}
				if (getplacetype(i, j) == (int)THUAI5::PlaceType::BirthPlace8) {
					NewMap[i][j] = 4;
				}

				if ((ul == (int)THUAI5::PlaceType::Wall || NewMap[i][j] == -1) && (ur != (int)THUAI5::PlaceType::Wall && NewMap[i][j + 1] != -1) && (dl != (int)THUAI5::PlaceType::Wall && NewMap[i + 1][j] != -1) && (dr != (int)THUAI5::PlaceType::Wall && NewMap[i + 1][j + 1] != -1)) {
					if (NewMap[i + 1][j + 1] != -1) NewMap[i + 1][j + 1] = 1;
					//if (NewMap[i][j] == -1) //////printf("1111111");
				}
				if ((ul != (int)THUAI5::PlaceType::Wall && NewMap[i][j] != -1) && (ur == (int)THUAI5::PlaceType::Wall || NewMap[i][j + 1] == -1) && (dl != (int)THUAI5::PlaceType::Wall && NewMap[i + 1][j] != -1) && (dr != (int)THUAI5::PlaceType::Wall && NewMap[i + 1][j + 1] != -1)) {
					if (NewMap[i + 1][j] != 1) NewMap[i + 1][j] = 1;
					//if (NewMap[i ][j+1] == -1) //////printf("2222222");
				}
				if ((ul != (int)THUAI5::PlaceType::Wall && NewMap[i][j] != -1) && (ur != (int)THUAI5::PlaceType::Wall && NewMap[i][j + 1] != -1) && (dl == (int)THUAI5::PlaceType::Wall || NewMap[i + 1][j] == -1) && (dr != (int)THUAI5::PlaceType::Wall && NewMap[i + 1][j + 1] != -1)) {
					if (NewMap[i][j + 1] != 1)NewMap[i][j + 1] = 1;
					//if (NewMap[i+1][j ] == -1) //////printf("3333333");
				}
				if ((ul != (int)THUAI5::PlaceType::Wall && NewMap[i][j] != -1) && (ur != (int)THUAI5::PlaceType::Wall && NewMap[i][j + 1] != -1) && (dl != (int)THUAI5::PlaceType::Wall && NewMap[i + 1][j] != -1) && (dr == (int)THUAI5::PlaceType::Wall || NewMap[i + 1][j + 1] == -1)) {
					if (NewMap[i][j] != 1) NewMap[i][j] = 1;
					//if (NewMap[i + 1][j + 1] == -1) //////printf("44444");
				}
			}
		}
	}
	for (int i = 0; i < 50; ++i) {
		for (int j = 0; j < 50; ++j) {
			if (NewMap[i][j] == 1) {
				vertices[VerticeNum++].GridPosition = { celltogrid(i),celltogrid(j) };
				vertices_origin[VerticeNum - 1].GridPosition = vertices[VerticeNum - 1].GridPosition;
			}
		}
	}
	VerticeNum_origin = VerticeNum;
	for (int i = 0; i < VerticeNum; ++i) {
		//////printf("corner%d %d,%d\n", i, vertices[i].GridPosition.x, vertices[i].GridPosition.y);
	}
}




//MARS
//点到直线距离公式  求得的是平方
/*double d(int x1, int x2, int y1, int y2, int x0, int y0) {
return fabs(((double)((y2 - y1) * x0 + (x1 - x2) * y0 + ((x2 * y1) - (x1 * y2)))) * ((double)((y2 - y1) * x0 + (x1 - x2) * y0 + ((x2 * y1) - (x1 * y2))))) / ((pow(y2 - y1, 2) + pow(x1 - x2, 2)));
}*/

//以下均返回的是平方
//点到直线距离
//以下均返回的是平方
//点到直线距离
double dpl(int x1, int x2, int y1, int y2, int x0, int y0) {
	double x00 = double(x0);
	double y00 = double(x0);
	double x11 = double(x1);
	double x22 = double(x2);
	double y11 = double(y1);
	double y22 = double(y2);
	////////std::cout << std::endl << "--" << x1 << " " << x2 << " " << y1 << " " << y2 << " " << x0 << " " << y0 << std::endl;
	if ((x1 - x0) * (y2 - y0) != (x2 - x0) * (y1 - y0) && x00 != x11 && x00 != x22 && x11 != x22) {


		double m = ((y1 - y2) * 1.0 / (x1 - x2));
		double b = (y1 - (y1 - y2) * 1.0 / (x1 - x2) * x1);
		return (m * x0 + b - y0) * (m * x0 + b - y0) / (m * m + 1);

		//return fabs((((y22 - y11) * x00 + (x11 - x22) * y00 + ((x22 * y11) - (x11 * y22)))) * (((y22 - y11) * x00 + (x11 - x22) * y00 + ((x22 * y11) - (x11 * y22))))) * 1.0 / (((x11 - x22) * (x11 - x22) + (y11 - y22) * (y11 - y22)));
	}
	else if ((x1 - x0) * (y2 - y0) != (x2 - x0) * (y1 - y0)) {
		return fabs((((y22 - y11) * x00 + (x11 - x22) * y00 + ((x22 * y11) - (x11 * y22)))) * (((y22 - y11) * x00 + (x11 - x22) * y00 + ((x22 * y11) - (x11 * y22))))) * 1.0 / (((x11 - x22) * (x11 - x22) + (y11 - y22) * (y11 - y22)));
	}
	//return fabs((((y2 - y1) * x0 + (x1 - x2) * y0 + ((x2 * y1) - (x1 * y2)))) * (((y2 - y1) * x0 + (x1 - x2) * y0 + ((x2 * y1) - (x1 * y2))))) * 1.0 / ((pow(y2 - y1, 2) + pow(x1 - x2, 2)));
	else return 0.0;
}
//两点之间距离
double dpp(int x1, int x2, int y1, int y2) {
	return 1.0 * ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}
//找到投影点
double PX(int x1, int x2, int y1, int y2, int x0, int y0) {
	double x00 = double(x0);
	double y00 = double(x0);
	double x11 = double(x1);
	double x22 = double(x2);
	double y11 = double(y1);
	double y22 = double(y2);
	////////printf("call PX between %d,%d and %d,%d     %d %d:\n ", x1,x2,y1,y2,x0,y0);
	////////printf("x1-x0  %d, x2-x0 %d and y1-y0 %d,y2-y0 %d     \n ", x1-x0, x2-x0, y1-y0, y2-y0);

	////////std::cout << "x11" << x11 << " x1" << x1 << std::endl;
	if ((x11 - x00) * (y22 - y00) != (x22 - x00) * (y11 - y00) && x00 != x11 && x00 != x22 && x11 != x22) {
		////////printf("(x1-x0)*(y2-y0)!=(x2-x0)*(y1-y0)%d\n", ((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)));
		double m = ((y1 - y2) * 1.0 / (x1 - x2));
		double b = (y1 - (y1 - y2) * 1.0 / (x1 - x2) * x1);
		////////std::cout << "m b" << m << " " << b << std::endl;
		return (m * y0 + x0 - m * b) / (m * m + 1);
		////////std::cout << "px" << (((y1 - y2) * (y1 - y2) * x1 + (x1 - x2) * (x1 - x2) * x0 + (y0 - y1) * (x1 - x2) * (y1 - y2)) * 1.0 / ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)))<<std::endl;
		////////std::cout << "pxx" << (((y1 - y2) * (y1 - y2) * x11 + (x11 - x2) * (x11 - x2) * x0 + (y0 - y1) * (x11 - x2) * (y1 - y2)) * 1.0 / ((x11 - x2) * (x11 - x2) + (y1 - y2) * (y1 - y2))) << std::endl;

	}
	else if ((x11 - x00) * (y22 - y00) != (x22 - x00) * (y11 - y00)) {
		return (((y11 - y22) * (y11 - y22) * x11 + (x11 - x22) * (x11 - x22) * x00 + (y00 - y11) * (x11 - x22) * (y11 - y22)) * 1.0 / ((x11 - x22) * (x11 - x22) + (y11 - y22) * (y11 - y22)));
	}
	//return ((pow((y1 - y2), 2) * x1 + pow((x1 - x2), 2) * x0 + (y0 - y1) * (x1 - x2) * (y1 - y2)) / (pow((x1 - x2), 2) + pow((y1 - y2), 2)));
	//return ((((y1 - y2) * 1.0 / (x1 - x2)) * y0 + x0 - ((y1 - y2) * 1.0 / ((x1 - x2))) * (y1 - (y1 - y2) * 1.0 / (x1 - x2) * x1)) / ((pow((y1 - y2) * 1.0 / (x1 - x2), 2) + 1)));
	else return 1.0 * x00;
}

//点到线段距离
double d(int x1, int x2, int y1, int y2, int x0, int y0)
{
	double x00 = double(x0);
	double y00 = double(x0);
	double x11 = double(x1);
	double x22 = double(x2);
	double y11 = double(y1);
	double y22 = double(y2);

	//判断最近点是否在线段内
	////////std::cout << std::endl << "PX:" << PX(x1, x2, y1, y2, x0, y0);
	////////std::cout << std::endl << "--" << x1 << " " << x2 << " " << y1 << " " << y2 << " " << x0 << " " << y0 << std::endl;
	if (PX(x1, x2, y1, y2, x0, y0) <= std::max(x11, x22) && PX(x1, x2, y1, y2, x0, y0) >= std::min(x11, x22)) {
		////////std::cout << "    pl    ";
		return dpl(x1, x2, y1, y2, x0, y0);
	}
	else {
		//不在线段内，则判断到端点的距离
		////////std::cout << "    pp    ";
		return std::min(dpp(x0, x1, y0, y2), dpp(x0, x2, y0, y2));
	}
}



//MARS
//判断能否直达
bool Directly(const gridPosition& selfPos_grid, const gridPosition& target) {
	//////printf("call directly between %d,%d and %d,%d: ", gridtocell(selfPos_grid.x), gridtocell(selfPos_grid.y), gridtocell(target.x), gridtocell(target.y));
	bool flag = 1;
	double r = 1.0 * 500 * 500;
	int x_self = std::min(selfPos_grid.x, target.x);
	int y_self = std::min(selfPos_grid.y, target.y);
	int x_target = std::max(selfPos_grid.x, target.x);
	int y_target = std::max(selfPos_grid.y, target.y);
	//一下均以grid为单位
	////////std::cout << "```````````````````" << selfPos_grid.x << "  " << target.x << "  " << selfPos_grid.y << "  " << target.y << std::endl;
	for (int i = gridtocell(x_self) - 1; i <= gridtocell(x_target) + 1; i++) {   //应为最外周是墙，所以不会越界
		for (int j = gridtocell(y_self) - 1; j <= gridtocell(y_target) + 1; j++) {
			int x_c = celltogrid(i);
			int y_c = celltogrid(j);
			////////std::cout << "(" << i << "," << j << ")   " << (int)getplacetype(i, j) << std::endl;

			if (getplacetype(i, j) == (int)THUAI5::PlaceType::Wall || NewMap[i][j] == -1 || NewMap[i][j] == 2 && getplacetype(i, j) != ((selfinfo.teamID) * 4 + selfinfo.playerID + 5) || NewMap[i][j] == 4 && getplacetype(i, j) != ((selfinfo.teamID) * 4 + selfinfo.playerID + 5))
			{
				////////std::cout << std::endl << "wall (" << i << "," << j << ")  " << std::endl;
				double d1 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c + num_of_grid_per_cell / 2, y_c + num_of_grid_per_cell / 2);
				double d2 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c + num_of_grid_per_cell / 2, y_c - num_of_grid_per_cell / 2);
				double d3 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c - num_of_grid_per_cell / 2, y_c + num_of_grid_per_cell / 2);
				double d4 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c - num_of_grid_per_cell / 2, y_c - num_of_grid_per_cell / 2);
				double d5 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c, y_c);

				////////std::cout << std::endl << "d: " << d1 << " " << d2 << " " << d3 << " " << d4 << " " << d5 << std::endl;
				if (d1 - r < 0 || d2 - r < 0 || d3 - r < 0 || d4 - r < 0 || d5 < 500000.0) {
					////////std::cout << "aaaaaaaaaaaaa" << std::endl;
					flag = 0;
				}
			}
		}
	}
	//if (flag) //////printf("yes\n");
	//else //////printf("no\n");
	////////std::cout << flag << std::endl;
	return flag;
}


bool Directly_throw(const gridPosition& selfPos_grid, const gridPosition& target) {
	//////printf("call directly between %d,%d and %d,%d: ", gridtocell(selfPos_grid.x), gridtocell(selfPos_grid.y), gridtocell(target.x), gridtocell(target.y));

	bool flag = 1;
	double r = 1.0 * 500 * 500;
	int x_self = std::min(selfPos_grid.x, target.x);
	int y_self = std::min(selfPos_grid.y, target.y);
	int x_target = std::max(selfPos_grid.x, target.x);
	int y_target = std::max(selfPos_grid.y, target.y);
	//一下均以grid为单位
	////////std::cout << "```````````````````" << selfPos_grid.x << "  " << target.x << "  " << selfPos_grid.y << "  " << target.y << std::endl;
	for (int i = gridtocell(x_self) - 1; i <= gridtocell(x_target) + 1; i++) {   //应为最外周是墙，所以不会越界
		for (int j = gridtocell(y_self) - 1; j <= gridtocell(y_target) + 1; j++) {
			int x_c = celltogrid(i);
			int y_c = celltogrid(j);
			////////std::cout << "(" << i << "," << j << ")   " << (int)getplacetype(i, j) << std::endl;

			if (getplacetype(i, j) == (int)THUAI5::PlaceType::Wall || NewMap[i][j] == -1 || NewMap[i][j] == 4 - 2 * (selfinfo.teamID))
			{
				////////std::cout << std::endl << "wall (" << i << "," << j << ")  " << std::endl;
				double d1 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c + num_of_grid_per_cell / 2, y_c + num_of_grid_per_cell / 2);
				double d2 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c + num_of_grid_per_cell / 2, y_c - num_of_grid_per_cell / 2);
				double d3 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c - num_of_grid_per_cell / 2, y_c + num_of_grid_per_cell / 2);
				double d4 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c - num_of_grid_per_cell / 2, y_c - num_of_grid_per_cell / 2);
				double d5 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c, y_c);

				////////std::cout << std::endl << "d: " << d1 << " " << d2 << " " << d3 << " " << d4 << " " << d5 << std::endl;
				if (d1 - r < 0 || d2 - r < 0 || d3 - r < 0 || d4 - r < 0 || d5 < 500000.0) {
					////////std::cout << "aaaaaaaaaaaaa" << std::endl;
					flag = 0;
				}
			}
		}
	}
	//if (flag) //////printf("yes\n");
	//else //////printf("no\n");
	////////std::cout << flag << std::endl;
	return flag;
}
/*bool Directly(gridPosition selfPos_grid, gridPosition target, IAPI& api) {
////////printf("call directly between %d,%d and %d,%d\n", gridtocell(selfPos_grid.x), gridtocell(selfPos_grid.y), gridtocell(target.x), gridtocell(target.y));
bool flag = 1;
double r = 500 * 500;
double epsilon = 2;
double x_self = std::min(selfPos_grid.x, target.x);
double y_self = std::min(selfPos_grid.y, target.y);
double x_target = std::max(selfPos_grid.x, target.x);
double y_target = std::max(selfPos_grid.y, target.y);
//一下均以grid为单位
for (int i = gridtocell(x_self) - 1; i <= gridtocell(x_target) + 1; i++) {   //应为最外周是墙，所以不会越界
for (int j = gridtocell(y_self) - 1; j <= gridtocell(y_target) + 1; j++) {
double x_c = celltogrid(i);
double y_c = celltogrid(j);
////////std::cout << "(" << i << "," << j << ")   " << (int)getplacetype(i, j) << std::endl;

if (getplacetype(i, j) == THUAI5::PlaceType::Wall || NewMap[i][j] == -1)
{
double d1 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c + num_of_grid_per_cell / 2, y_c + num_of_grid_per_cell / 2);
double d2 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c + num_of_grid_per_cell / 2, y_c - num_of_grid_per_cell / 2);
double d3 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c - num_of_grid_per_cell / 2, y_c + num_of_grid_per_cell / 2);
double d4 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c - num_of_grid_per_cell / 2, y_c - num_of_grid_per_cell / 2);
double d5 = d(selfPos_grid.x, target.x, selfPos_grid.y, target.y, x_c, y_c);
////////std::cout << "wall (" << i << "," << j << ")  " << std::endl;
////////std::cout << std::endl << "d: " << d1 << " " << d2 << " " << d3 << " " << d4 << " " << d5 << std::endl;
if (d1 - r < 0 || d2 - r < 0 || d3 - r < 0 || d4 - r < 0 || d5 < 500000)
flag = 0;
}
}
}
return flag;
}*/

void constructGraph() {
	for (int i = 0; i < EdgeNum; i++)
	{
		int beginNode = edges[i].from;
		int endNode = edges[i].to;
		edges[i].next_in_edge = vertices[beginNode].head;
		vertices[beginNode].head = &edges[i];
		edges[i].next_out_edge = vertices[endNode].bottom;
		vertices[endNode].bottom = &edges[i];
	}
	for (int i = 0; i < VerticeNum; i++) {
		vertices_origin[i] = vertices[i];
	}
	/*for (int i = 0; i < EdgeNum; i++)
	{
	edges_origin[i].next_in_edge = edges[i].next_in_edge;
	edges_origin[i].next_out_edge = edges[i].next_out_edge;
	}*/

}

double lengthofGrids(const gridPosition& t1, const gridPosition& t2) {
	long double squareSum = ((double)t1.x - (double)t2.x) * ((double)t1.x - (double)t2.x) + ((double)t1.y - (double)t2.y) * ((double)t1.y - (double)t2.y);
	return sqrt(squareSum);
}

void learnEdges(IAPI& api) {
	for (int i = 0; i < VerticeNum; ++i) {
		for (int j = 0; j < i; ++j) {
			if (i == j) continue;
			////////printf("already constructGraph\n");
			////////printf("send in (%d,%d)and (%d,%d)", vertices[i].GridPosition.x, vertices[i].GridPosition.y, vertices[j].GridPosition.x, vertices[j].GridPosition.y);
			if (!Directly(vertices[i].GridPosition, vertices[j].GridPosition)) continue;
			////////printf("already constructGraph1\n");

			edges[EdgeNum].value = lengthofGrids(vertices[i].GridPosition, vertices[j].GridPosition);
			edges[EdgeNum].from = i;
			edges[EdgeNum].to = j;
			////printf("new value=%f\n", edges[EdgeNum].value);
			EdgeNum++;
			edges[EdgeNum].value = edges[EdgeNum - 1].value;
			edges[EdgeNum].from = j;
			edges[EdgeNum].to = i;
			EdgeNum++;
		}
	}
	EdgeNum_origin = EdgeNum;
	for (int i = 0; i < EdgeNum; ++i) {
		edges[i].next_in_edge = nullptr;
		edges[i].next_out_edge = nullptr;
	}
}

double Dijkstra(const int& start, const int& finalss) {
	//////printf("call Dij!!! for (%d,%d)\n", vertices[start].GridPosition.x, vertices[start].GridPosition.y);  //MARS改 改变函数类型
	double leastDistance[maxVerticeNum];
	int nearestNode[maxVerticeNum];
	int allFixed = 0;//所有已被确定的点的个数
	bool wthFixed[maxVerticeNum] = { 0 };
	for (int i = 0; i < VerticeNum; i++)
	{
		nearestNode[i] = start;
		leastDistance[i] = 10000000;
	}
	leastDistance[start] = 0;
	while (true) {
		if (allFixed == VerticeNum) { break; }
		double minleastDistance = 10000000;
		int Node2Expand = -1;
		for (int i = 0; i < VerticeNum; i++)
		{
			if (!wthFixed[i]) {
				if (leastDistance[i] < minleastDistance)
				{
					minleastDistance = leastDistance[i];
					Node2Expand = i;
				}
			}
		}
		if ((int)minleastDistance == 10000000)
		{
			break;
		}
		wthFixed[Node2Expand] = 1;
		allFixed++;
		for (edge* e = vertices[Node2Expand].head; e != nullptr; e = e->next_in_edge) {
			double newdistance = leastDistance[Node2Expand] + e->value;
			if (newdistance < leastDistance[e->to]) {
				leastDistance[e->to] = newdistance;
				////printf("newdiatance=%f\n", newdistance);
				nearestNode[e->to] = Node2Expand;
			}
			////////printf("cccccccccccccccccccccccccccc\n");
		}
		////////printf("allfixed=%d\n", allFixed);
	}
	/*for (edge* e = vertices[start].head; e != nullptr; e = e->next_in_edge) {
	//////printf("start %d,%d connect to %d,%d\n", vertices[start].GridPosition.x, vertices[start].GridPosition.y, vertices[e->to].GridPosition.x, vertices[e->to].GridPosition.y);
	}
	for (edge* e = vertices[finalss].head; e != nullptr; e = e->next_in_edge) {
	//////printf("final %d,%d connect to %d,%d\n", vertices[finalss].GridPosition.x, vertices[finalss].GridPosition.y, vertices[e->to].GridPosition.x, vertices[e->to].GridPosition.y);
	}*/
	if (!wthFixed[finalss])
	{
		//////printf("NO PATH\n");
		/*for (int i = 0; i < VerticeNum - 1; ++i) {
		if(Directly(vertices[finalss].GridPosition,vertices[i].GridPosition,api))
		}*/
		return -1;   //MARS改  增加返回值
	}
	//int anti_route[maxVerticeNum];
	int flag = finalss;
	int len = 0;
	while (flag != start)
	{
		antiroute.push_back(vertices[flag].GridPosition);
		//////printf("(%d,%d)->", vertices[flag].GridPosition.x, vertices[flag].GridPosition.y);
		flag = nearestNode[flag];
	}
	//////printf("\n");
	////////printf("antiroute size=%d\n", antiroute.size());
	//gridPosition curTarget = vertices[anti_route[len - 2]].GridPosition;
	/*cout << "[" << start;
	for (int i = 1; i <= len; ++i) {
	cout << "->" << anti_route[len - i];
	}
	cout << " " << leastDistance[final] << "]" << endl;*/
	//printf("dijlen=%f\n", leastDistance[finalss]);
	return leastDistance[finalss];
}

double SearchSlopeRoute(gridPosition begin, gridPosition end) {
	int startNode = -1, finalNode = -1;
	//////printf("VerticeNum=%d,EdgeNum=%d\n", VerticeNum, EdgeNum);
	for (int i = 0; i < VerticeNum; ++i) {
		if (begin.x == vertices[i].GridPosition.x && begin.y == vertices[i].GridPosition.y) {
			startNode = i; break;
		}
	}
	for (int i = 0; i < VerticeNum; ++i) {
		if (end.x == vertices[i].GridPosition.x && end.y == vertices[i].GridPosition.y) {
			finalNode = i; break;
		}
	}
	if (startNode == -1) {
		startNode = VerticeNum; VerticeNum++;
		vertices[startNode].GridPosition = begin;
		vertices[startNode].bottom = nullptr;
		vertices[startNode].head = nullptr;
		for (int i = 0; i < VerticeNum - 1; ++i) {
			if (!Directly(vertices[i].GridPosition, begin)) continue;
			edges[EdgeNum].value = lengthofGrids(vertices[i].GridPosition, begin);
			edges[EdgeNum].from = i;
			edges[EdgeNum].to = startNode;
			edges[EdgeNum].next_in_edge = vertices[i].head;
			vertices[i].head = &edges[EdgeNum];
			edges[EdgeNum].next_out_edge = vertices[startNode].bottom;
			vertices[startNode].bottom = &edges[EdgeNum];
			EdgeNum++;
			edges[EdgeNum].value = edges[EdgeNum - 1].value;
			edges[EdgeNum].from = startNode;
			edges[EdgeNum].to = i;
			edges[EdgeNum].next_out_edge = vertices[i].bottom;
			vertices[i].bottom = &edges[EdgeNum];
			edges[EdgeNum].next_in_edge = vertices[startNode].head;
			vertices[startNode].head = &edges[EdgeNum];
			EdgeNum++;
		}

	}
	//////printf("VerticeNum=%d,EdgeNum=%d\n", VerticeNum, EdgeNum);
	if (finalNode == -1) {
		finalNode = VerticeNum; VerticeNum++;
		vertices[finalNode].GridPosition = end;
		vertices[finalNode].bottom = nullptr;
		vertices[finalNode].head = nullptr;
		////////printf("%d,%d\n", end.x, end.y);
		for (int i = 0; i < VerticeNum - 1; ++i) {
			if (!Directly(vertices[i].GridPosition, end)) continue;
			edges[EdgeNum].value = lengthofGrids(vertices[i].GridPosition, end);
			edges[EdgeNum].from = i;
			edges[EdgeNum].to = finalNode;
			edges[EdgeNum].next_in_edge = vertices[i].head;
			vertices[i].head = &edges[EdgeNum];
			edges[EdgeNum].next_out_edge = vertices[finalNode].bottom;
			vertices[finalNode].bottom = &edges[EdgeNum];
			EdgeNum++;
			edges[EdgeNum].value = edges[EdgeNum - 1].value;
			edges[EdgeNum].from = finalNode;
			edges[EdgeNum].to = i;
			edges[EdgeNum].next_out_edge = vertices[i].bottom;
			vertices[i].bottom = &edges[EdgeNum];
			edges[EdgeNum].next_in_edge = vertices[finalNode].head;
			vertices[finalNode].head = &edges[EdgeNum];
			EdgeNum++;
		}
	}
	//////printf("VerticeNum=%d,EdgeNum=%d\n", VerticeNum, EdgeNum);
	double r = Dijkstra(startNode, finalNode);
	//if (Dijkstra(startNode, finalNode) < 0) return;    //MARS改 增加判断
	/*for (int i = 0; i < VerticeNum; ++i) {
	if (vertices[i].GridPosition.x != 15500 || vertices[i].GridPosition.y != 25500)continue;
	for (edge* e = vertices[i].head; e != nullptr; e = e->next_in_edge) {
	//////printf("15,25 connect to %d,%d\n", gridtocell(vertices[e->to].GridPosition.x), gridtocell(vertices[e->to].GridPosition.y));
	}
	}
	for (int i = 0; i < VerticeNum; ++i) {
	if (vertices[i].GridPosition.x != 25500 || vertices[i].GridPosition.y != 27500)continue;
	for (edge* e = vertices[i].head; e != nullptr; e = e->next_in_edge) {
	//////printf("25,27 connect to %d,%d\n", gridtocell(vertices[e->to].GridPosition.x), gridtocell(vertices[e->to].GridPosition.y));
	}
	}
	for (int i = 0; i < VerticeNum; ++i) {
	if (vertices[i].GridPosition.x != 21500 || vertices[i].GridPosition.y != 27500)continue;
	for (edge* e = vertices[i].head; e != nullptr; e = e->next_in_edge) {
	//////printf("21,27 connect to %d,%d\n", gridtocell(vertices[e->to].GridPosition.x), gridtocell(vertices[e->to].GridPosition.y));
	}
	}*/
	//将图恢复
	/*std::map<edge*, int> trashcan;
	while (VerticeNum > VerticeNum_origin) {
	for (edge* e = vertices[VerticeNum].head; e != nullptr; e = e->next_in_edge) {
	if (trashcan[e] != 1) {
	trashcan[e]=1;
	}
	}
	for (edge* e = vertices[VerticeNum].bottom; e != nullptr; e = e->next_out_edge) {
	if (trashcan[e] != 1) {
	trashcan[e] = 1;
	}
	}
	}
	std::map<edge*, int>::iterator iter;
	for (iter = trashcan.begin(); iter != trashcan.end(); iter++) {
	delete iter->first;
	}*/
	EdgeNum = EdgeNum_origin;
	VerticeNum = VerticeNum_origin;
	for (int i = 0; i < VerticeNum_origin; ++i) {
		vertices[i] = vertices_origin[i];
	}
	/*for (int i = 0; i < EdgeNum_origin; i++)
	{
	edges[i].next_in_edge = edges_origin[i].next_in_edge;
	edges[i].next_out_edge = edges_origin[i].next_out_edge;
	}*/
	return r;
}

bool legalPos(const cellPosition& tt) {
	//判断一个位置是否合法
	//////printf("ttx %d,tty %d\n", tt.x, tt.y);
	if (tt.x <= 0 || tt.x > 50 || tt.y <= 0 || tt.y > 50) return 0;
	auto PlaceType = getplacetype(tt.x, tt.y);//不能出格
											  //////printf("pt %d\n", PlaceType);						  ////////printf("here!\n");
	if (PlaceType == 1) return 0;//不能卡墙里
	return 1;
}

bfsResult newBfsTo(cellPosition startPos, cellPosition endPos) {
	//搜到某一点的最短“方格”距离
	//////printf("call newBfsto this time!\n");
	short dirMap[52][52];
	std::queue<bfsResult> fringe;//队列
	std::map<cellPosition, int> closed;//已经展开过的点
	fringe.push({ startPos,0 });
	closed[startPos] = 1;
	bool success = 0;
	////////printf("hahaha\n");

	////////printf("Props.size()=%d\n", (int)Props.size());

	while (!fringe.empty()) {
		////////printf("fringe.size()=%d\n", (int)fringe.size());
		bfsResult memberToExpand = fringe.front();
		fringe.pop();
		////////printf("x=%d,y=%d\n", memberToExpand.goalstate.x,memberToExpand.goalstate.y);
		THUAI5::PropType type;
		if (memberToExpand.goalstate.x == endPos.x && memberToExpand.goalstate.y == endPos.y) {
			success = 1;
		}
		if (success) {
			//////printf("now pathlen=%d\n", memberToExpand.pathLen);
			cellPosition cur = memberToExpand.goalstate;
			for (int i = 0; i < memberToExpand.pathLen + 1; ++i) {
				//////printf("(%d,%d)<-", cur.x, cur.y);
				//将路径反向打印出来，不一定有用，可以注释掉
				if (dirMap[cur.x][cur.y] == 1) cur.x++;
				else if (dirMap[cur.x][cur.y] == 2) cur.y--;
				else if (dirMap[cur.x][cur.y] == 3) cur.x--;
				else if (dirMap[cur.x][cur.y] == 4) cur.y++;
			}
			//////printf("\n");
			return memberToExpand;
		}
		std::map<cellPosition, int>::iterator iter;
		for (int i = 1; i <= 4; ++i) {
			cellPosition newPos;
			newPos.x = memberToExpand.goalstate.x + Dirx[i];
			newPos.y = memberToExpand.goalstate.y + Diry[i];
			if (!legalPos(newPos)) {
				////////printf("illegal\n"); 
				continue;
			}
			iter = closed.find(newPos);
			if (iter != closed.end()) continue;//不要二进宫

			dirMap[newPos.x][newPos.y] = i;

			closed[newPos] = 1;
			bfsResult newNode;
			newNode.goalstate = newPos;
			newNode.pathLen = memberToExpand.pathLen + 1;
			fringe.push(newNode);
		}

	}
	return { {-1,-1},-1 };//没搜到的情况
}

bfsResult newBfs_limit(cellPosition startPos, cellPosition masterPos, THUAI5::PropType goalType) {
	////////printf("call newBfs time!\n");
	short dirMap[52][52];
	std::queue<bfsResult> fringe;//队列
	std::map<cellPosition, int> closed;//已经展开过的点
	fringe.push({ startPos,0 });
	closed[startPos] = 1;
	bool success = 0;
	int x1 = masterPos.x - tolerance_deviation;
	int x2 = masterPos.x + tolerance_deviation;
	int y1 = masterPos.y - tolerance_deviation;
	int y2 = masterPos.y + tolerance_deviation;
	////////printf("hahaha\n");

	////////printf("Props.size()=%d\n", (int)Props.size());

	while (!fringe.empty()) {
		////////printf("fringe.size()=%d\n", (int)fringe.size());
		bfsResult memberToExpand = fringe.front();
		fringe.pop();
		////////printf("x=%d,y=%d\n", memberToExpand.goalstate.x,memberToExpand.goalstate.y);
		if (enhanced_map[memberToExpand.goalstate.x][memberToExpand.goalstate.y].props[(int)goalType]) {
			success = 1;
		}
		if (success) {
			//////printf("now pathlen=%d\n", memberToExpand.pathLen);
			cellPosition cur = memberToExpand.goalstate;
			for (int i = 0; i < memberToExpand.pathLen + 1; ++i) {
				////////printf("(%d,%d)<-", cur.x, cur.y);
				//将路径反向打印出来，不一定有用，可以注释掉
				if (dirMap[cur.x][cur.y] == 1) cur.x++;
				else if (dirMap[cur.x][cur.y] == 2) cur.y--;
				else if (dirMap[cur.x][cur.y] == 3) cur.x--;
				else if (dirMap[cur.x][cur.y] == 4) cur.y++;
			}
			//////printf("\n");
			return memberToExpand;
		}
		std::map<cellPosition, int>::iterator iter;
		for (int i = 1; i <= 4; ++i) {
			cellPosition newPos;
			newPos.x = memberToExpand.goalstate.x + Dirx[i];
			newPos.y = memberToExpand.goalstate.y + Diry[i];

			if (newPos.x<x1 || newPos.x > x2 || newPos.y < y1 || newPos.y > y2) {
				continue;
			}
			if (!legalPos(newPos)) {
				////////printf("illegal\n"); 
				continue;
			}

			iter = closed.find(newPos);
			if (iter != closed.end()) continue;//不要二进宫

			dirMap[newPos.x][newPos.y] = i;

			closed[newPos] = 1;
			bfsResult newNode;
			newNode.goalstate = newPos;
			newNode.pathLen = memberToExpand.pathLen + 1;
			fringe.push(newNode);
		}

	}
	return { {-1,-1},-1 };//没搜到的情况
}

bfsResult newBfs(THUAI5::PropType goalType, cellPosition startPos) {
	//////printf("call newBfs this time!\n");
	short dirMap[52][52];
	std::queue<bfsResult> fringe;//队列
	std::map<cellPosition, int> closed;//已经展开过的点
	fringe.push({ startPos,0 });
	closed[startPos] = 1;
	bool success = 0;
	////////printf("hahaha\n");

	////////printf("Props.size()=%d\n", (int)Props.size());

	while (!fringe.empty()) {
		////////printf("fringe.size()=%d\n", (int)fringe.size());
		bfsResult memberToExpand = fringe.front();
		fringe.pop();
		////////printf("x=%d,y=%d\n", memberToExpand.goalstate.x,memberToExpand.goalstate.y);
		if (enhanced_map[memberToExpand.goalstate.x][memberToExpand.goalstate.y].props[(int)goalType]) {
			success = 1;
			//////printf("iii len=%d\n", memberToExpand.pathLen);
		}
		if (success) {
			//////printf("now pathlen=%d\n", memberToExpand.pathLen);
			cellPosition cur = memberToExpand.goalstate;
			for (int i = 0; i < memberToExpand.pathLen + 1; ++i) {
				//////printf("(%d,%d)<-", cur.x, cur.y);
				//将路径反向打印出来，不一定有用，可以注释掉
				if (dirMap[cur.x][cur.y] == 1) cur.x++;
				else if (dirMap[cur.x][cur.y] == 2) cur.y--;
				else if (dirMap[cur.x][cur.y] == 3) cur.x--;
				else if (dirMap[cur.x][cur.y] == 4) cur.y++;
			}
			//////printf("\n");
			return memberToExpand;
		}
		std::map<cellPosition, int>::iterator iter;
		for (int i = 1; i <= 4; ++i) {
			cellPosition newPos;
			newPos.x = memberToExpand.goalstate.x + Dirx[i];
			newPos.y = memberToExpand.goalstate.y + Diry[i];
			if (!legalPos(newPos)) {
				//////printf("illegal\n"); 
				continue;
			}
			iter = closed.find(newPos);
			if (iter != closed.end()) continue;//不要二进宫

			dirMap[newPos.x][newPos.y] = i;

			closed[newPos] = 1;
			bfsResult newNode;
			newNode.goalstate = newPos;
			newNode.pathLen = memberToExpand.pathLen + 1;
			fringe.push(newNode);
		}

	}
	return { {-1,-1},-1 };//没搜到的情况
}

bfsResult newBfsForPlace(THUAI5::PlaceType goalType, cellPosition startPos) {
	//////printf("call newBfs this time!\n");
	short dirMap[52][52];
	std::queue<bfsResult> fringe;//队列
	std::map<cellPosition, int> closed;//已经展开过的点
	fringe.push({ startPos,0 });
	closed[startPos] = 1;
	bool success = 0;
	////////printf("hahaha\n");

	////////printf("Props.size()=%d\n", (int)Props.size());

	while (!fringe.empty()) {
		////////printf("fringe.size()=%d\n", (int)fringe.size());
		bfsResult memberToExpand = fringe.front();
		fringe.pop();
		////////printf("x=%d,y=%d\n", memberToExpand.goalstate.x,memberToExpand.goalstate.y);
		if (enhanced_map[memberToExpand.goalstate.x][memberToExpand.goalstate.y].placetype == (int)goalType) {
			success = 1;
		}
		if (success) {
			//////printf("now pathlen=%d\n", memberToExpand.pathLen);
			cellPosition cur = memberToExpand.goalstate;
			for (int i = 0; i < memberToExpand.pathLen + 1; ++i) {
				//////printf("(%d,%d)<-", cur.x, cur.y);
				//将路径反向打印出来，不一定有用，可以注释掉
				if (dirMap[cur.x][cur.y] == 1) cur.x++;
				else if (dirMap[cur.x][cur.y] == 2) cur.y--;
				else if (dirMap[cur.x][cur.y] == 3) cur.x--;
				else if (dirMap[cur.x][cur.y] == 4) cur.y++;
			}
			//////printf("\n");
			return memberToExpand;
		}
		std::map<cellPosition, int>::iterator iter;
		for (int i = 1; i <= 4; ++i) {
			cellPosition newPos;
			newPos.x = memberToExpand.goalstate.x + Dirx[i];
			newPos.y = memberToExpand.goalstate.y + Diry[i];
			if (!legalPos(newPos)) {
				////////printf("illegal\n"); 
				continue;
			}
			iter = closed.find(newPos);
			if (iter != closed.end()) continue;//不要二进宫

			dirMap[newPos.x][newPos.y] = i;

			closed[newPos] = 1;
			bfsResult newNode;
			newNode.goalstate = newPos;
			newNode.pathLen = memberToExpand.pathLen + 1;
			fringe.push(newNode);
		}

	}
	return { {-1,-1},-1 };//没搜到的情况
}

//
uint32_t moveTo(gridPosition target, uint32_t time_available, IAPI& api) {
	//在特定时间内，超指定位置走动，并返回行走的时间（可借此推断有无到达目的地），距离小于50就算达到了
	auto self = selfinfo;

	gridPosition selfPos_grid = { self.x,self.y };
	cellPosition selfPos = { gridtocell(selfPos_grid.x),gridtocell(selfPos_grid.y) };
	if (lengthofGrids(selfPos_grid, target) <= 400) {
		return 0;
	}
	gridPosition curTarget;//当前移动目标
	SearchSlopeRoute(selfPos_grid, target);
	if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }

	//curTarget = antiroute.back(); antiroute.pop_back();
	uint32_t time_used = 0;
	//bool flag_quit = 0;
	while (1) {
		if (time_used > time_available - 1) break;
		auto self = selfinfo;
		selfPos_grid.x = self.x; selfPos_grid.y = self.y;
		uint32_t speed = self.speed / 1000;
		double lentomove = lengthofGrids(selfPos_grid, curTarget);
		////////printf("speed=%d,lentomove=%f\n", speed, lentomove);
		if (lengthofGrids(selfPos_grid, target) <= 50) {
			break;
		}
		uint32_t newtime = (uint32_t)lentomove / speed + 5;
		long double e = 0;
		e = atan2((long double)curTarget.y - (long double)selfPos_grid.y, (long double)curTarget.x - (long double)selfPos_grid.x);

		if (time_used + newtime > time_available) {
			newtime = time_available - time_used;
		}
		else {
			if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }
		}
		time_used += newtime;
		if (curTarget.y == selfPos_grid.y && curTarget.x < selfPos_grid.x) { //////printf("MoveUp\n"); 
			api.MoveUp(newtime);
		}
		else if (curTarget.y == selfPos_grid.y && curTarget.x > selfPos_grid.x) { //////printf("MoveDown\n"); 
			api.MoveDown(newtime);
		}
		else if (curTarget.y < selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveLeft\n"); 
			api.MoveLeft(newtime);
		}
		else if (curTarget.y > selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveRight\n"); 
			api.MoveRight(newtime);
		}
		else api.MovePlayer(newtime, e);
	}
	antiroute.clear();
}

uint32_t MoveToMaster(IAPI& api, gridPosition CapPos, gridPosition selfPos_grid, uint32_t time_available) {
	if (conflict)return 0;
	if (!moveornot)return 0;
	//////printf("\nmovetomaster\n");
	//////printf("\ncappos%d,%d\n", CapPos.x, CapPos.y);
	//在特定时间内，超指定位置走动，并返回行走的时间（可借此推断有无到达目的地），距离小于50就算达到了
	auto self = selfinfo;


	cellPosition selfPos = { gridtocell(selfPos_grid.x),gridtocell(selfPos_grid.y) };
	if (((selfPos_grid.x - CapPos.x) * (selfPos_grid.x - CapPos.x) + (selfPos_grid.y - CapPos.y) * (selfPos_grid.y - CapPos.y)) <= tolerance_griddis_min * tolerance_griddis_min) //千万不能调用lengthofgrid  类型不同！！会有问题
	{
		//////printf("\ntooo near\n");
		return 0;
	}
	gridPosition target = CapPos;

	gridPosition curTarget;//当前移动目标
	SearchSlopeRoute(selfPos_grid, target);
	//////printf("antiroute size=%d\n", antiroute.size());
	if (antiroute.size() < 1) {          //MARS改，增加判断
										 //////printf("aaaaaaaaaaaaaaaaa\n");
		return 0;
	}
	if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }

	//curTarget = antiroute.back(); antiroute.pop_back();
	uint32_t time_used = 0;
	while (1) {
		//if (antiroute.size() == 0) break;
		if (((selfPos_grid.x - CapPos.x) * (selfPos_grid.x - CapPos.x) + (selfPos_grid.y - CapPos.y) * (selfPos_grid.y - CapPos.y)) <= tolerance_griddis_min * tolerance_griddis_min) break;
		if (time_used > time_available - 1) break;

		SearchSlopeRoute(selfPos_grid, target);
		//////printf("antiroute size=%d\n", antiroute.size());
		if (antiroute.size() < 1) {      //MARS改，增加判断
										 ////////printf("aaaaaaaaaaaaaaaaa\n");
			return 0;
		}
		if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }

		//curTarget = antiroute.back(); antiroute.pop_back();


		auto self = selfinfo;
		selfPos_grid.x = self.x; selfPos_grid.y = self.y;
		uint32_t speed = self.speed / 1000;
		//self.
		double lentomove = lengthofGrids(selfPos_grid, curTarget);
		//////printf("speed=%d,lentomove=%f\n", speed, lentomove);
		uint32_t newtime = lentomove / speed + 5;
		long double e = 0;
		double pie = 3.1415926;
		e = atan2((long double)curTarget.y - (long double)selfPos_grid.y, (long double)curTarget.x - (long double)selfPos_grid.x);

		if (time_used + newtime > time_available) {
			newtime = time_available - time_used;
		}
		else
		{
			if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }
		}
		time_used += newtime;
		if (curTarget.y == selfPos_grid.y && curTarget.x < selfPos_grid.x) { //////printf("MoveUp\n"); 
			api.MoveUp(newtime);
		}
		else if (curTarget.y == selfPos_grid.y && curTarget.x > selfPos_grid.x) { //////printf("MoveDown\n"); 
			api.MoveDown(newtime);
		}
		else if (curTarget.y < selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveLeft\n"); 
			api.MoveLeft(newtime);
		}
		else if (curTarget.y > selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveRight\n"); 
			api.MoveRight(newtime);
		}
		else api.MovePlayer(newtime, e);
	}

	antiroute.clear();
	return 1;
	//gridPosition curTarget;//当前移动目标
	//SearchSlopeRoute(selfPos_grid, CapPos, api);
	//curTarget = antiroute.back(); antiroute.pop_back();
	//uint32_t time_used = 0;
	////bool flag_quit = 0;
	//while (1) {
	//	

	//	if (time_used > time_available - 1) break;
	//	auto self = selfinfo;
	//	selfPos_grid.x = self.x; selfPos_grid.y = self.y;
	//	uint32_t speed = self.speed / 1000;
	//	double lentomove = lengthofGrids(selfPos_grid, curTarget);
	//	////////printf("speed=%d,lentomove=%f\n", speed, lentomove);
	//	uint32_t newtime = (uint32_t)lentomove / speed + 1;
	//	long double e = 0;
	//	e = atan2((long double)curTarget.y - (long double)selfPos_grid.y, (long double)curTarget.x - (long double)selfPos_grid.x);

	//	if (time_used + newtime > time_available) {
	//		newtime = time_available - time_used;
	//	}
	//	else {
	//		if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }
	//	}
	//	time_used += newtime;
	//	if (curTarget.y == selfPos_grid.y && curTarget.x < selfPos_grid.x) { //////printf("MoveUp\n"); api.MoveUp(newtime); }
	//	else if (curTarget.y == selfPos_grid.y && curTarget.x > selfPos_grid.x) { //////printf("MoveDown\n"); api.MoveDown(newtime); }
	//	else if (curTarget.y < selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveLeft\n"); api.MoveLeft(newtime); }
	//	else if (curTarget.y > selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveRight\n"); api.MoveRight(newtime); }
	//	else api.MovePlayer(newtime, e);
	//}
	//antiroute.clear();
	//




}


int moveForTarget(IAPI& api, THUAI5::PropType type, uint32_t time_available) {
	//在特定时间内，搜一特定类型并前进,并返回能收集多少个
	int typeNum = 0;
	auto Props = api.GetProps();
	auto self = selfinfo;

	gridPosition selfPos_grid = { self.x,self.y };
	cellPosition selfPos = { gridtocell(selfPos_grid.x),gridtocell(selfPos_grid.y) };
	bfsResult BFS = newBfs(type, selfPos);
	cellPosition target_cell = BFS.goalstate;
	gridPosition target = { celltogrid(target_cell.x),celltogrid(target_cell.y) };
	if (lengthofGrids(selfPos_grid, target) <= 400 && !throwcpu) {
		api.Pick(type); //捡CPU！！！
		typeNum++;
		BFS = newBfs(type, selfPos);
		target_cell = BFS.goalstate;
		////////printf("%d,%d\n", target_cell.x, target_cell.y);
		target = { celltogrid(target_cell.x),celltogrid(target_cell.y) };
	}
	gridPosition curTarget;//当前移动目标
	if (BFS.pathLen >= 0) {
		SearchSlopeRoute(selfPos_grid, target);
		if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }

		//curTarget = antiroute.back(); antiroute.pop_back();
	}
	uint32_t time_used = 0;
	while (1) {
		if (BFS.pathLen < 0) break;
		if (time_used > time_available - 1) break;
		auto self = selfinfo;
		selfPos_grid.x = self.x; selfPos_grid.y = self.y;
		uint32_t speed = self.speed / 1000;
		double lentomove = lengthofGrids(selfPos_grid, curTarget);
		////////printf("speed=%d,lentomove=%f\n", speed, lentomove);
		if (lengthofGrids(selfPos_grid, target) <= 400 && !throwcpu) {
			api.Pick(type);
			typeNum++; break;//捡CPU！！！
		}
		uint32_t newtime = lentomove / speed + 5;
		long double e = 0;
		e = atan2((long double)curTarget.y - (long double)selfPos_grid.y, (long double)curTarget.x - (long double)selfPos_grid.x);

		if (time_used + newtime > time_available) {
			newtime = time_available - time_used;
		}
		else {
			if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }
		}
		time_used += newtime;
		if (curTarget.y == selfPos_grid.y && curTarget.x < selfPos_grid.x) { //////printf("MoveUp\n");
			api.MoveUp(newtime);
		}
		else if (curTarget.y == selfPos_grid.y && curTarget.x > selfPos_grid.x) { //////printf("MoveDown\n"); 
			api.MoveDown(newtime);
		}
		else if (curTarget.y < selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveLeft\n"); 
			api.MoveLeft(newtime);
		}
		else if (curTarget.y > selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveRight\n"); 
			api.MoveRight(newtime);
		}
		else api.MovePlayer(newtime, e);
	}
	antiroute.clear();
	return typeNum;
}


//MARS更改 20220423   直接将整个函数替换即可
int FindCAP(IAPI& api) {
	auto self = selfinfo;
	auto Robots = api.GetRobots();
	bool havemycap = 0;
	bool haveteammate = 0;
	bool havecap = 0;
	uint64_t MYCAP = -1;
	uint64_t OTHERCAP = -1;
	//const std::shared_ptr<const THUAI5::Robot> Cap = nullptr;
	if (self.playerID == VICE1) {
		MYCAP = CAP1;
		OTHERCAP = CAP2;
	}
	else if (self.playerID == VICE2) {
		MYCAP = CAP2;
		OTHERCAP = CAP1;
	}

	int CapID = -1;
	if (Robots.size() != 0)
	{
		for (int i = 0; i < Robots.size(); i++)
		{
			if (!Robots[i]->isResetting) {
				if (Robots[i]->teamID == self.teamID && Robots[i]->playerID == self.playerID) {
					continue;
				}//自己

				if (havemycap == 0) {
					if (Robots[i]->teamID != self.teamID) continue;
					else {

						haveteammate = 1;
						if (Robots[i]->playerID == MYCAP) {
							havemycap = 1;
							havecap = 1;

							CapID = Robots[i]->playerID;
							break;
						}
						if (havemycap == 0 && Robots[i]->playerID == OTHERCAP) {
							havecap = 1;

							CapID = Robots[i]->playerID;
						}
					}
				}
				else continue;
			}
			if (!havecap && haveteammate) {
				for (int i = 0; i < Robots.size(); i++)
				{
					if (Robots[i]->teamID == self.teamID && Robots[i]->playerID == self.playerID) {
						continue;
					}//自己
					else if (Robots[i]->teamID != self.teamID) continue;
					else if (Robots[i]->teamID == self.teamID && Robots[i]->playerID > self.playerID) {
						havecap = 1;

						CapID = Robots[i]->playerID;
						break;
					}
					else continue;
				}
			}
			
		}
	}
	//////printf("robot_num %d \ncappos%d,%d", Robots.size(), CapPos.x, CapPos.y);
	return CapID;//以后可以考虑直接返回Cap
}
bool Calculate(IAPI& api) {       //判断第i个机器人是回到base还是继续捡拾cpu true则返回
	auto self = selfinfo;
	gridPosition now = { self.x,self.y };
	gridPosition birth = { celltogrid(birthx0) + 1000,celltogrid(birthy0) + 1000 };
	if (Directly_throw(now, birth)) {
		double t, angle;
		double s = dpp(now.x, birth.x, now.y, birth.y);
		t = std::sqrt(s) * 1.0 / 3;
		if (t < 5000)api.ThrowCPU(t, std::atan2((double)(birth.y - now.y), (double)(birth.x - now.x)), self.cpuNum);
	}
	if (self.cpuNum >= 3)return true;
	else return false;
}
//实现多个机器人的目标协同
void Broadcast(int guid, IAPI& api) {
	std::string messtui = std::to_string(guid);
	auto self = selfinfo;
	std::string id = std::to_string(self.playerID);
	messtui = "2" + id + messtui;
	for (int i = 0; i <= 3; i++) {
		if (self.playerID == i)continue;
		api.Send(i, messtui);
	}
}
//判断是否有cpu被选为目标，如果有，是哪个
int Determination(IAPI& api, std::string messa) {
	int cpuguid = 0, h = 1;
	while (h < messa.length()) {
		cpuguid += messa[h++] - '0';
		if (h < messa.length())cpuguid *= 10;
	}
	return cpuguid;
}
void addCorner(const gridPosition& tt, IAPI& api) {
	for (int i = 0; i < VerticeNum; ++i) {
		if (tt.x == vertices[i].GridPosition.x && tt.y == vertices[i].GridPosition.y) {
			return;
		}
	}
	//VerticeNum++;
	vertices[VerticeNum].GridPosition = tt;
	vertices[VerticeNum].bottom = nullptr;
	vertices[VerticeNum].head = nullptr;
	for (int i = 0; i < VerticeNum - 1; ++i) {
		if (!Directly(vertices[i].GridPosition, tt)) continue;
		edges[EdgeNum].value = lengthofGrids(vertices[i].GridPosition, tt);
		edges[EdgeNum].from = i;
		edges[EdgeNum].to = VerticeNum;
		edges[EdgeNum].next_in_edge = vertices[i].head;
		vertices[i].head = &edges[EdgeNum];
		edges[EdgeNum].next_out_edge = vertices[VerticeNum].bottom;
		vertices[VerticeNum].bottom = &edges[EdgeNum];
		EdgeNum++;
		edges[EdgeNum].value = edges[EdgeNum - 1].value;
		edges[EdgeNum].from = VerticeNum;
		edges[EdgeNum].to = i;
		edges[EdgeNum].next_out_edge = vertices[i].bottom;
		vertices[i].bottom = &edges[EdgeNum];
		edges[EdgeNum].next_in_edge = vertices[VerticeNum].head;
		vertices[VerticeNum].head = &edges[EdgeNum];
		EdgeNum++;
	}
	VerticeNum++;
}
//行进方向冲突时的解决办法
void Conflict(IAPI& api, uint64_t id) {
	auto self = selfinfo;
	auto robot = api.GetRobots();
	int p = 0;
	if (!robot.size())return;
	if (fetch)return;
	for (int i = 0; i < robot.size(); i++) {
		if (robot[i]->guid == id) {
			p = i; break;
		}
	}
	int x1 = self.x;
	int y1 = self.y;
	int x2 = robot[p]->x;
	int y2 = robot[p]->y;
	api.MovePlayer(50, std::atan2((long double)(y2 - y1), (long double)(x2 - x1)) + 3.1415926);
}
bool Crash(int32_t x1, int32_t x2, int32_t y1, int32_t y2) {
	if ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) <= 2000000)return true;
	else return false;
}
void Correspondence(IAPI& api) {  //实现通信功能的总接口
	int cpunumi = 0;
	auto self = selfinfo;
	auto robot = api.GetRobots();
	long double atan = 0;
	std::string type = "3";
	std::string id = std::to_string(self.playerID);
	std::string idx = std::to_string(self.x);
	std::string fen = "*";
	std::string idy = std::to_string(self.y);
	std::string sum = type + id + idx + fen + idy;
	endtime_send = std::chrono::system_clock::now();
	deltatime_send = std::chrono::duration_cast<std::chrono::microseconds>(endtime_send - starttime_send).count() / 1000.0;
	if (deltatime_send > 1000) {
		for (int j = 0; j <= 3; j++) {
			if (j == self.playerID)continue;
			if (sum != "30")api.Send(j, sum);
		}
		Broadcast(propguid, api);
		starttime_send = std::chrono::system_clock::now();
		deltatime_send = 0;
	}
	for (int i = 0; i < robot.size(); i++) {
		if (robot[i]->guid == self.guid)continue;
		int32_t x1 = self.x;
		int32_t y1 = self.y;
		int32_t x2 = robot[i]->x;
		int32_t y2 = robot[i]->y;
		int id = robot[i]->guid;
		if (Crash(x1, x2, y1, y2)) {
			conflict = true;
			Conflict(api, id);
		}
		else conflict = false;
	}
	back = Calculate(api); //printf("%d\n", back);
	if (back) {
		gridPosition birth = { celltogrid(birthx0) + 1000,celltogrid(birthy0) + 1000 };
		moveTo(birth, 100, api);
	}
	if (self.cpuNum >= 10)api.UseCPU(self.cpuNum);
	std::string messa;
	int guidd = 0;
	while (api.MessageAvailable()) {
		auto message = api.TryGetMessage();
		if (message.has_value())messa = message.value();
		if (messa[0] == '2') {
			guidd = Determination(api, messa);
			int head = messa[0] - '0';
			otherpropguid[head] = guidd;
		}
		if (messa[0] == '3') {
			int id = messa[1] - '0';
			int xx = 0, yy = 0, h = 2;
			while (messa[h] != '*') {
				xx += messa[h] - '0';
				h++;
				if (messa[h] != '*')xx *= 10;
			}
			h++;
			while (h < messa.length()) {
				yy += messa[h] - '0';
				h++;
				if (h < messa.length())yy *= 10;
			}
			//////printf("debug:xx=%d,yy=%d\n", xx, yy);
			px[id] = xx;
			py[id] = yy;
		}
		if (messa[0] == '4') {
			fetch = true;
			moveornot = false;
			for (int i = 1; i < messa.length(); i++) {
				cpunumi += messa[i] - '0';
				if (i < messa.length() - 1)cpunumi *= 10;
			}
		}
	}
	return;
}


void move_mars_noBFS(IAPI& api, gridPosition target, gridPosition selfPos_grid, gridPosition curTarget) {

	////printf("movemarsNO called\n");
	uint32_t time_used = 0;
	while (1) {
		//if (antiroute.size() == 0) break;
		if (lengthofGrids(target, selfPos_grid) < 400) break;
		if (time_used > time_available_mars - 1) break;
		auto self = selfinfo;
		selfPos_grid.x = self.x; selfPos_grid.y = self.y;
		uint32_t speed = self.speed / 1000;
		//self.
		double lentomove = lengthofGrids(selfPos_grid, curTarget);
		//////printf("speed=%d,lentomove=%f\n", speed, lentomove);
		if (lengthofGrids(selfPos_grid, target) <= 400 && !throwcpu) {
			api.Pick(THUAI5::PropType::CPU); break;//捡CPU！！！
		}
		uint32_t newtime = lentomove / speed + 5;
		long double e = 0;
		double pie = 3.1415926;
		e = atan2((long double)curTarget.y - (long double)selfPos_grid.y, (long double)curTarget.x - (long double)selfPos_grid.x);

		if (time_used + newtime > time_available_mars) {
			newtime = time_available_mars - time_used;
		}
		else
		{
			if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }
		}
		time_used += newtime;
		if (moveornot && !conflict && !back) {
			if (curTarget.y == selfPos_grid.y && curTarget.x < selfPos_grid.x) {
				//////printf("MoveUp\n"); 
				api.MoveUp(newtime);
			}
			else if (curTarget.y == selfPos_grid.y && curTarget.x > selfPos_grid.x) {
				//////printf("MoveDown\n"); 
				api.MoveDown(newtime);
			}
			else if (curTarget.y < selfPos_grid.y && curTarget.x == selfPos_grid.x) {
				//////printf("MoveLeft\n");
				api.MoveLeft(newtime);
			}
			else if (curTarget.y > selfPos_grid.y && curTarget.x == selfPos_grid.x) {
				//////printf("MoveRight\n"); 
				api.MoveRight(newtime);
			}
			else api.MovePlayer(newtime, e);
		}
	}

	antiroute.clear();
}


void move_mars_haveBFS(IAPI& api, gridPosition target, gridPosition selfPos_grid, gridPosition curTarget, bfsResult BFS) {

	////printf("movemarsYES called\n");
	uint32_t time_used = 0;
	while (1) {
		//if (antiroute.size() == 0) break;
		if (BFS.pathLen < 0) break;
		if (time_used > time_available_mars - 1) break;
		auto self = selfinfo;
		selfPos_grid.x = self.x; selfPos_grid.y = self.y;
		uint32_t speed = self.speed / 1000;
		//self.
		double lentomove = lengthofGrids(selfPos_grid, curTarget);
		//////printf("speed=%d,lentomove=%f\n", speed, lentomove);
		if (lengthofGrids(selfPos_grid, target) <= 400 && !throwcpu) {
			api.Pick(THUAI5::PropType::CPU); break;//捡CPU！！！
		}
		uint32_t newtime = lentomove / speed + 5;
		long double e = 0;
		double pie = 3.1415926;
		e = atan2((long double)curTarget.y - (long double)selfPos_grid.y, (long double)curTarget.x - (long double)selfPos_grid.x);

		if (time_used + newtime > time_available_mars) {
			newtime = time_available_mars - time_used;
		}
		else
		{
			if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }
		}
		time_used += newtime;
		if (moveornot && !conflict && !back) {
			if (curTarget.y == selfPos_grid.y && curTarget.x < selfPos_grid.x) { //////printf("MoveUp\n");
				api.MoveUp(newtime);
			}
			else if (curTarget.y == selfPos_grid.y && curTarget.x > selfPos_grid.x) { //////printf("MoveDown\n"); 
				api.MoveDown(newtime);
			}
			else if (curTarget.y < selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveLeft\n");
				api.MoveLeft(newtime);
			}
			else if (curTarget.y > selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveRight\n"); 
				api.MoveRight(newtime);
			}
			else api.MovePlayer(newtime, e);
		}
	}

	antiroute.clear();
}




void search_for_BL(gridPosition target, gridPosition selfPos_grid, gridPosition& curTarget, double lentoTarget) {

	int selfPlaceType = getplacetype(gridtocell(selfPos_grid.x), gridtocell(selfPos_grid.y));
	uint32_t speed = selfinfo.speed / 1000;
	////std::cout << "speed " << speed << std::endl << "oooospeed " << originSpeed << std::endl;;
	if (speed == originSpeed && (selfPlaceType < 2 || selfPlaceType>4)) {
		//找最近的草丛
		//printf("speed now=%d, placeType=%d, look for grass\n", speed, (int)selfPlaceType);
		gridPosition newTarget = target;
		int favorlen = lentoTarget / 4;
		//printf("len2Target=%f\n", lentoTarget);
		bfsResult BFSforBL1 = newBfsForPlace(THUAI5::PlaceType::BlindZone1, (cellPosition)selfPos_grid);
		if (BFSforBL1.pathLen * 1000 < favorlen) {
			newTarget = BFSforBL1.goalstate;
			favorlen = BFSforBL1.pathLen;
			//printf("len1=%d\n", BFSforBL1.pathLen);
		}
		bfsResult BFSforBL2 = newBfsForPlace(THUAI5::PlaceType::BlindZone2, (cellPosition)selfPos_grid);
		if (BFSforBL2.pathLen * 1000 < favorlen) {
			newTarget = BFSforBL2.goalstate;
			favorlen = BFSforBL2.pathLen;
			//printf("len2=%d\n", BFSforBL2.pathLen);
		}
		bfsResult BFSforBL3 = newBfsForPlace(THUAI5::PlaceType::BlindZone3, (cellPosition)selfPos_grid);
		if (BFSforBL3.pathLen * 1000 < favorlen) {
			newTarget = BFSforBL3.goalstate;
			favorlen = BFSforBL3.pathLen;
			//printf("len3=%d\n", BFSforBL3.pathLen);
		}
		if (newTarget != target) {
			antiroute.clear();
			//printf("search for new target %d,%d\n", newTarget.x, newTarget.y);
			SearchSlopeRoute(selfPos_grid, newTarget);
			if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }
		}
	}
	else if (speed == originSpeed && selfPlaceType > 1 && selfPlaceType < 5) {
		gridPosition newTarget = target;
		int favorlen = lentoTarget / 4;
		bfsResult BFSforLand = newBfsForPlace(THUAI5::PlaceType::Land, (cellPosition)selfPos_grid);
		if (BFSforLand.pathLen * 1000 < favorlen) {
			newTarget = BFSforLand.goalstate;
			favorlen = BFSforLand.pathLen;
			//printf("len1=%d\n", BFSforLand.pathLen);
		}
		bfsResult BFSforFac = newBfsForPlace(THUAI5::PlaceType::CPUFactory, (cellPosition)selfPos_grid);
		if (BFSforFac.pathLen * 1000 < favorlen) {
			newTarget = BFSforFac.goalstate;
			favorlen = BFSforFac.pathLen;
			//printf("len1=%d\n", BFSforFac.pathLen);
		}
		if (newTarget != target) {
			antiroute.clear();
			//printf("search for new target %d,%d\n", newTarget.x, newTarget.y);
			SearchSlopeRoute(selfPos_grid, newTarget);
			if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }
		}
	}
}

cellPosition anti_birthCPU = { -1,-1 };
int hsb = 0;
bool havebirthCPU() {
	for (int i = 0; i < 50; i++) {
		for (int j = 0; j < 50; j++) {
			if ((enhanced_map[i][j].placetype >= (1 - selfinfo.teamID) * 4 + 5) && (enhanced_map[i][j].placetype <= (1 - selfinfo.teamID) * 4 + 5 + 3) && enhanced_map[i][j].props[3] >= 5) {
				anti_birthCPU = { i,j };
				//std::cout << i << j;
				return true;
			}
		}
	}
	return false;
}



#include<iomanip>
void AI::play(IAPI& api)
{
	if (((selfinfo.teamID == 0 && selfinfo.playerID == 0) || (selfinfo.teamID == 1 && selfinfo.playerID == 3)) && api.GetFrameCount() >= 11100) {
		lastminite = 1;
	}
	if (lastminite) {
		gridPosition birth = { celltogrid(birthx0) + 1000,celltogrid(birthy0) + 1000 };
		moveTo(birth, 100, api);

		if (gridtocell(selfinfo.x) == birthx0 + 1 && gridtocell(selfinfo.y) == birthy0 + 1) {
			while (api.Pick(THUAI5::PropType::CPU)) { hsb++; }
			if (enhanced_map[birthx0][birthy0].props[3] == 0 || api.GetFrameCount() >= 11900) {
				api.UseCPU(hsb);
			}
		}

		return;
	}

	if (((selfinfo.teamID == 0 && (selfinfo.playerID == 1 || selfinfo.playerID == 2)) || (selfinfo.teamID == 1 && (selfinfo.playerID == 1 || selfinfo.playerID == 2))) && api.GetFrameCount() >= 11100) {
		//std::cout << "11111111";
		if (havebirthCPU()) {
			//gridPosition birthttt = { 50000 - (celltogrid(birthx0)) + selfinfo.playerID * 1000,50000 - (celltogrid(birthy0)) + selfinfo.playerID * 1000 };
			if (dpp(selfinfo.x, anti_birthCPU.x, selfinfo.y, anti_birthCPU.y) > 1000 * 1000) {
				cellPosition now_target = { anti_birthCPU.x + 4 * (1.5 - selfinfo.playerID),anti_birthCPU.y + 4 * (1.5 - selfinfo.playerID) };
				moveTo({ celltogrid(now_target.x),celltogrid(now_target.y), }, 100, api);
			}
			attack_gth(api);
			return;

		}

	}

	for (int i = 0; i < 50; ++i) {
		for (int j = 0; j < 50; ++j) {
			for (int k = 0; k < 6; ++k) {
				enhanced_map[i][j].props[k] = 0;
			}

		}
	}
	enhanced_scan_map(api);
	getinfo(api);
	auto self = selfinfo;
	//////std::cout << "selfx" << self.x << " " << "selfy" << self.y << std::endl;
	//auto self = selfinfo;
	api.UseCommonSkill();
	if (!propsGotten)
	{
		getProp_gth(api);
		propsGotten = 1;
	}
	attack_gth(api);
	if (!self.isResetting)Correspondence(api);
	if (RUN_gth(api))return;

	//for (int i = 0; i <= 49; i++) {
	//	for (int j = 0; j <= 49; j++) {
	//		////std::cout << std::setw(5) << enhanced_map[i][j].placetype << " ";
	//	}
	//	////std::cout << std::endl;
	//}



	bool bfslimfound = 0;
	auto Props = api.GetProps();
	////////printf("Props.size()=%d\n", (int)Props.size());
	if (!flag_scan) {
		ScanMap(NewMap, api);
		//////printf("already scan\n");
		learnEdges(api);
		//////printf("already learnEdges\n");
		constructGraph();
		//////printf("already constructGraph\n");
		flag_scan = 1;
		originSpeed = self.speed / 1000;
	}
	int id = self.playerID;
	//////printf("debug:%d %d %d\n", id, px[0], py[0]);
	if (int(self.prop) != 0) {
		//////printf("\nint(self.prop) != 0\n");
		for (int p = 0; p < Props.size(); p++) {
			if ((int)Props[p]->type != 3) {
				//////printf("usingiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii");
				api.UseProp();
			}
		}
	}

	bool isVice = 0;
	bool haveCap = 0;
	bool canmove = 0;  //MARS改，增加判断  一切有关canmove的都是判断
	gridPosition CapPos = { -1,-1 };

	//MARS更改 20220423   加上这句
	int CapID = -1;
	//MARS更改 20220423   这个判断语句直接整体替换即可
	////printf("\nmyid%d\n", self.playerID);
	if (self.playerID == VICE1 || self.playerID == VICE2) {
		CapID = FindCAP(api);
		isVice = 1;
		////printf("capID  %d\n", CapID);
		if (CapID != -1) {
			CapPos.x = px[CapID];
			CapPos.y = py[CapID];
			if (CapPos.x != -1) {
				haveCap = 1;
			}
		}
		//////printf("pxy   %d  %d\n", px[CapID], py[CapID]);
		//////printf("havecap  %d\n", haveCap);
		//////printf("cappos   %d  %d\n", CapPos.x, CapPos.y);
	}
	//////printf("havecap? %d\n", haveCap);
	//////printf("\ncappos%d,%d\n", CapPos.x, CapPos.y);

	gridPosition selfPos_grid = { self.x,self.y };
	cellPosition selfPos = { gridtocell(selfPos_grid.x),gridtocell(selfPos_grid.y) };

	//gridPosition target = { -1,-1 };
	gridPosition curTarget;//当前移动目标


	if (isVice && haveCap && !ViceHaveTask) {
		tolerance_griddis_max = 3500;
		tolerance_deviation = 5;
		if (dpp(self.x, CapPos.x, self.y, CapPos.y) > tolerance_griddis_max * tolerance_griddis_max) {
			bfslimfound = 0;
			//////printf("\ntoofar!!!!\n");
			if (haveCap && !back) {

				MoveToMaster(api, CapPos, selfPos_grid, time_available_mars);
			}
		}
		else {
			bfsResult BFS = { {-1,-1},-1 };
			//MARS20220504
			for (int i = 0; i < 5; i++) {
				if (!bfslimfound) {
					BFS = newBfs_limit(selfPos, CapPos, prop_array_for_bfslim[i]);
					if (BFS.pathLen >= 0) {
						bfslimfound = 1;
					}
				}

			}
			////////printf("now bfslimfound %d", bfslimfound);
			cellPosition target_cell = BFS.goalstate;
			////////printf("%d,%d\n", target_cell.x, target_cell.y);
			if (BFS.pathLen != -1) {
				ViceHaveTask = 1;
				gridPosition target = { celltogrid(target_cell.x),celltogrid(target_cell.y) };
				ViceLastTask = target;
			}
		}
	}

	else if (isVice && haveCap && ViceHaveTask) {
		tolerance_deviation = 6;
		tolerance_griddis_max = 5500;
		if (dpp(self.x, CapPos.x, self.y, CapPos.y) > tolerance_griddis_max * tolerance_griddis_max) {
			//////printf("\ntoofar!!!!\n");
			bfslimfound = 0;
			if (haveCap) {
				MoveToMaster(api, CapPos, selfPos_grid, time_available_mars);
			}
		}
		bfsResult BFS = { {-1,-1},-1 };
		for (int i = 0; i < 5; i++) {
			if (!bfslimfound) {
				BFS = newBfs_limit(selfPos, CapPos, prop_array_for_bfslim[i]);
				if (BFS.pathLen >= 0) {
					bfslimfound = 1;
				}
			}

		}

		////////printf("now bfslimfound %d", bfslimfound);
		cellPosition target_cell = BFS.goalstate;
		////////printf("%d,%d\n", target_cell.x, target_cell.y);
		gridPosition target = { celltogrid(target_cell.x),celltogrid(target_cell.y) };
		//判断是否接受新的坐标
		if (lengthofGrids(CapPos, target) > lengthofGrids(ViceLastTask, target)) {
			target = ViceLastTask;
			target_cell = { gridtocell(target.x),gridtocell(target.y) };
		}
		ViceLastTask = target;



		if (lengthofGrids(selfPos_grid, target) <= 400) {
			//将此处的捡完
			//auto Props = api.GetProps();
			//for (int p = 0; p < Props.size(); p++) {
			//	////////printf("%d,%d\n", Props[p]->x, gridtocell(Props[p]->x));
			//	while (gridtocell(Props[p]->x) == gridtocell(self.x) && gridtocell(Props[p]->y) == gridtocell(self.y)) {
			//		api.Pick(Props[p]->type);
			//	}
			//}
			for (int p = 0; p < Props.size(); p++) {
				////////printf("%d,%d\n", Props[p]->x, gridtocell(Props[p]->x));
				if (gridtocell(Props[p]->x) == gridtocell(self.x) && gridtocell(Props[p]->y) == gridtocell(self.y)) {

					api.Pick(Props[p]->type);
					////////printf("\npicked!!!!!!\n");
				}
			}
			bfslimfound = 0;
			ViceHaveTask = 0;
		}
		if (ViceHaveTask) {
			if (lengthofGrids(target, selfPos_grid) >= 400) {
				SearchSlopeRoute(selfPos_grid, target);
				//////printf("antiroute size=%d\n", antiroute.size());
				if (antiroute.size() >= 1) {
					canmove = 1;
				}
				//antiroute.pop_back();
				//if(antiroute.back()==s)
				if (canmove == 1) {
					if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }
					//curTarget = antiroute.back(); antiroute.pop_back();
				}
			}
			if (canmove == 1) {
				move_mars_noBFS(api, target, selfPos_grid, curTarget);
			}

		}
	}

	else if (!isVice || !haveCap) {
		//////printf("0000\n");
		bfsResult BFS = { {-1,-1},-1 };
		for (int i = 0; i < 5; i++) {
			//////printf("5555\n");
			if (!bfslimfound) {
				BFS = newBfs(prop_array_for_bfs[i], selfPos);
				if (BFS.pathLen >= 0) {
					//////printf("4444\n");
					bfslimfound = 1;
				}
			}
			////printf("bfslimfound=%d\n", bfslimfound);

		}
		cellPosition target_cell = BFS.goalstate;
		//printf("target cell=%d,%d\n", target_cell.x, target_cell.y);
		gridPosition target = { celltogrid(target_cell.x),celltogrid(target_cell.y) };
		if (lengthofGrids(selfPos_grid, target) <= 400) {
			auto Props = api.GetProps();
			for (int p = 0; p < Props.size(); p++) {
				////////printf("%d,%d\n", Props[p]->x, gridtocell(Props[p]->x));
				if (gridtocell(Props[p]->x) == gridtocell(self.x) && gridtocell(Props[p]->y) == gridtocell(self.y) && !throwcpu) {
					api.Pick(Props[p]->type);
				}
				bfslimfound = 0;
			}
			//////printf("3333\n");
			for (int i = 0; i < 5; i++) {
				if (!bfslimfound) {
					BFS = newBfs(prop_array_for_bfs[i], selfPos);
					if (BFS.pathLen >= 0) {
						//////printf("4444\n");
						bfslimfound = 1;
					}
				}

			}
			target_cell = BFS.goalstate;
			////////printf("%d,%d\n", target_cell.x, target_cell.y);
			target = { celltogrid(target_cell.x),celltogrid(target_cell.y) };
		}
		gridPosition curTarget;//当前移动目标
		double lentoTarget = -1;
		if (BFS.pathLen >= 0) {
			lentoTarget = SearchSlopeRoute(selfPos_grid, target);
			//////printf("antiroute size=%d\n", antiroute.size());
			//antiroute.pop_back();
			//if(antiroute.back()==s)
			if (antiroute.size() >= 1) {
				canmove = 1;
			}
			if (canmove == 1) {
				if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }

				//curTarget = antiroute.back(); antiroute.pop_back();
			}
		}
		if (canmove && !back) {
			//////printf("22222\n"); 
			search_for_BL(target, selfPos_grid, curTarget, lentoTarget);
			move_mars_haveBFS(api, target, selfPos_grid, curTarget, BFS);
		}
	}
}


/*if (!flag_scan) {
ScanMap(NewMap, api);
flag_scan = 1;
}*/
/*//////printf("ID: %d\n", selfinfo.playerID);
for (int i = 0; i < 49; i++) {
for (int j = 0; j < 49; j++) {
//////std::cout << std::setw(5) << NewMap[i][j] << " ";
}
//////std::cout << std::endl;
}*/



/*
//////std::cout << "Directly(29421, 25500, 15981, 22500)  1   " << Directly({ 29421,15981 }, { 25500, 22500 }, api) << "****" << std::endl << std::endl << std::endl;
//////std::cout << "Directly(21625, 19625, 22669, 20919)  1   " << Directly({ 21625,22669 }, { 19625, 20919 }, api) << "****" << std::endl << std::endl << std::endl;
//////std::cout << "Directly(30500, 40500, 27500, 10500)  0   " << Directly({ 30500,27500 }, { 40500, 10500 }, api) << "****" << std::endl << std::endl << std::endl;
//////std::cout << "Directly(41500, 39500, 10500, 10500)  1   " << Directly({ 41500,10500 }, { 39500,  10500 }, api) << "****" << std::endl << std::endl << std::endl;
//////std::cout << "Directly(30500, 39500, 16500, 10500)  1   " << Directly({ 30500, 16500 }, { 39500,  10500 }, api) << "****" << std::endl << std::endl << std::endl;
//////std::cout << "Directly(26500, 30500, 26500, 16500)  1   " << Directly({ 26500, 26500 }, { 30500, 16500 }, api) << "****" << std::endl << std::endl << std::endl;
//////std::cout << "Directly(19500,5500,20500,39500)  1   " << Directly({ 19500,20500 }, { 5500, 39500 }, api) << "****" << std::endl << std::endl << std::endl;
//////std::cout << "Directly(14500, 10500, 42500, 42500)  1   " << Directly({ 14500,42500 }, { 10500, 42500 }, api) << "****" << std::endl << std::endl << std::endl;
//////std::cout << "Directly(14500,12500,42500,42500)  1   " << Directly({ 14500,42500 }, { 12500, 42500 }, api) << "****" << std::endl << std::endl << std::endl;
//////std::cout << "Directly(14500,12500,42500,42500)  0   " << Directly({ 4500, 16000 }, { 4500,20000 }, api) << "****" << std::endl << std::endl << std::endl;
*/


//switch (i) {
//case 1:
//	if (!bfslimfound) {
//		////////printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabfslimcalled   %d\n", i);
//		BFS = newBfs_limit(api, selfPos, CapPos, THUAI5::PropType::Battery);
//		////////printf("newbfslimresult: %d  %d\n", BFS.goalstate.x, BFS.goalstate.y);
//		if (BFS.pathLen >= 0) {
//			bfslimfound = 1;
//		}
//		break;
//	}
//case 2:
//	if (!bfslimfound) {
//		////////printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabfslimcalled   %d\n", i);
//		BFS = newBfs_limit(api, selfPos, CapPos, THUAI5::PropType::Booster);
//		////////printf("newbfslimresult: %d  %d\n", BFS.goalstate.x, BFS.goalstate.y);
//		if (BFS.pathLen >= 0) {
//			bfslimfound = 1;
//		}
//		break;
//	}
//case 3:
//	if (!bfslimfound) {
//		////////printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabfslimcalled   %d\n", i);
//		BFS = newBfs_limit(api, selfPos, CapPos, THUAI5::PropType::CPU);
//		////////printf("newbfslimresult: %d  %d\n", BFS.goalstate.x, BFS.goalstate.y);
//		if (BFS.pathLen >= 0) {
//			bfslimfound = 1;
//		}
//		break;
//	}
//case 4:
//	if (!bfslimfound) {
//		////////printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabfslimcalled   %d\n", i);
//		BFS = newBfs_limit(api, selfPos, CapPos, THUAI5::PropType::Shield);
//		////////printf("newbfslimresult: %d  %d\n", BFS.goalstate.x, BFS.goalstate.y);
//		if (BFS.pathLen >= 0) {
//			bfslimfound = 1;
//		}
//		break;
//	}
//case 5:
//	if (!bfslimfound) {
//		////////printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabfslimcalled   %d\n", i);
//		BFS = newBfs_limit(api, selfPos, CapPos, THUAI5::PropType::ShieldBreaker);
//		////////printf("newbfslimresult: %d  %d\n", BFS.goalstate.x, BFS.goalstate.y);
//		if (BFS.pathLen >= 0) {
//			bfslimfound = 1;
//		}
//		break;
//	}
//}





//	uint32_t time_used = 0;
//	while (1) {
//		//if (antiroute.size() == 0) break;
//		if (lengthofGrids(target, selfPos_grid) < 400) break;
//		if (time_used > 69) break;
//		auto self = selfinfo;
//		selfPos_grid.x = self.x; selfPos_grid.y = self.y;
//		uint32_t speed = self.speed / 1000;
//		//self.
//		double lentomove = lengthofGrids(selfPos_grid, curTarget);
//		//////printf("speed=%d,lentomove=%f\n", speed, lentomove);
//		if (lengthofGrids(selfPos_grid, target) <= 400 && !throwcpu) {
//			api.Pick(THUAI5::PropType::CPU); break;//捡CPU！！！
//		}
//		uint32_t newtime = lentomove / speed + 1;
//		long double e = 0;
//		double pie = 3.1415926;
//		e = atan2((long double)curTarget.y - (long double)selfPos_grid.y, (long double)curTarget.x - (long double)selfPos_grid.x);

//		if (time_used + newtime > 70) {
//			newtime = 70 - time_used;
//		}
//		else
//		{
//			if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }
//		}
//		time_used += newtime;
//		if (moveornot && !conflict) {
//			if (curTarget.y == selfPos_grid.y && curTarget.x < selfPos_grid.x) { //////printf("MoveUp\n"); 
//				api.MoveUp(newtime);
//			}
//			else if (curTarget.y == selfPos_grid.y && curTarget.x > selfPos_grid.x) { //////printf("MoveDown\n"); 
//				api.MoveDown(newtime);
//			}
//			else if (curTarget.y < selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveLeft\n");
//				api.MoveLeft(newtime);
//			}
//			else if (curTarget.y > selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveRight\n"); 
//				api.MoveRight(newtime);
//			}
//			else api.MovePlayer(newtime, e);
//		}
//	}

//	antiroute.clear();
//}






//uint32_t time_used = 0;
//while (1) {
//	//if (antiroute.size() == 0) break;
//	if (BFS.pathLen < 0) break;
//	if (time_used > 69) break;
//	auto self = selfinfo;
//	selfPos_grid.x = self.x; selfPos_grid.y = self.y;
//	uint32_t speed = self.speed / 1000;
//	//self.
//	double lentomove = lengthofGrids(selfPos_grid, curTarget);
//	//////printf("speed=%d,lentomove=%f\n", speed, lentomove);
//	if (lengthofGrids(selfPos_grid, target) <= 400 && !throwcpu) {
//		api.Pick(THUAI5::PropType::CPU); break;//捡CPU！！！
//	}
//	uint32_t newtime = lentomove / speed + 1;
//	long double e = 0;
//	double pie = 3.1415926;
//	e = atan2((long double)curTarget.y - (long double)selfPos_grid.y, (long double)curTarget.x - (long double)selfPos_grid.x);
//
//	if (time_used + newtime > 70) {
//		newtime = 70 - time_used;
//	}
//	else
//	{
//		if (!antiroute.empty()) { curTarget = antiroute.back(); antiroute.pop_back(); }
//	}
//	time_used += newtime;
//	if (moveornot && !conflict) {
//		if (curTarget.y == selfPos_grid.y && curTarget.x < selfPos_grid.x) { //////printf("MoveUp\n");
//			api.MoveUp(newtime);
//		}
//		else if (curTarget.y == selfPos_grid.y && curTarget.x > selfPos_grid.x) { //////printf("MoveDown\n"); 
//			api.MoveDown(newtime);
//		}
//		else if (curTarget.y < selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveLeft\n");
//			api.MoveLeft(newtime);
//		}
//		else if (curTarget.y > selfPos_grid.y && curTarget.x == selfPos_grid.x) { //////printf("MoveRight\n"); 
//			api.MoveRight(newtime);
//		}
//		else api.MovePlayer(newtime, e);
//	}
//}
//
//antiroute.clear();
////int bfs(IAPI& api) {
//////	//////printf("debug:bfs");
//////	short qx[1000], qy[1000], qt[1000];//bfs搜索队列
//////	short cellmap[52][52];
//////	auto Props = api.GetProps();
//////	auto Robots = api.GetRobots();
//////	auto self = selfinfo;
//////	THUAI5::PropType type;
//////	auto SignalJammers = api.GetSignalJammers();
//////	//先初始化地图信息
//////	for (int i = 1; i <= 50; i++) {
//////		for (int j = 1; j <= 50; j++) {
//////			auto PlaceType = getplacetype(i, j);
//////			bool prop = false;
//////			int enemy = 0;
//////			int bullet = 0;
//////			if (SignalJammers.size()) {
//////				for (int p = 0; p < SignalJammers.size(); p++) {
//////					if (SignalJammers[p]->parentTeamID == self.teamID)continue;
//////					if (shoot(self.x, self.y, SignalJammers[p]->x, SignalJammers[p]->y))bullet++;
//////				}
//////			}
//////			if (Props.size()) {
//////				for (int p = 0; p < Props.size(); p++) {
//////					if (gridtocell(Props[p]->x) == i && gridtocell(Props[p]->y) == j) {
//////						prop = true;
//////						type = Props[p]->type;
//////					}
//////				}
//////			}
//////			if (Robots.size()) {
//////				for (int p = 0; p < Robots.size(); p++) {
//////					if (Robots[p]->teamID == self.teamID)continue;
//////					if ((Robots[p]->x - celltogrid(i)) * (Robots[p]->x - celltogrid(i)) + (Robots[p]->y - celltogrid(j)) * (Robots[p]->y - celltogrid(j)) <= (Robots[p]->attackRange) * (Robots[p]->attackRange)) {
//////						enemy++;
//////						//进入了enemy个敌人的攻击范围，用来计算“危险程度”，从而令机器人做出有效的规避。
//////					}
//////				}
//////			}
//////			//搜索时会根据值来优先选择路线，因此不同物品的优先值可以在此处修改。
//////			if (prop && type == THUAI5::PropType::CPU)cellmap[i][j] = 100;    //代表此处存在cpu
//////			else if (prop && type == THUAI5::PropType::Booster)cellmap[i][j] = 50;     //代表此处存在加速器
//////			else if (prop && type == THUAI5::PropType::Battery)cellmap[i][j] = 50;     //代表此处存在电池
//////			else if (prop && type == THUAI5::PropType::Shield)cellmap[i][j] = 40;     //代表此处存在护盾
//////			else if (prop && type == THUAI5::PropType::ShieldBreaker)cellmap[i][j] = 10;     //代表此处存在破盾
//////			else cellmap[i][j] = 0;
//////			if (enemy)cellmap[i][j] += -100 * enemy;    //代表进入可观测到的敌人的攻击范围
//////			if (PlaceType == THUAI5::PlaceType::CPUFactory)cellmap[i][j] += 40;     //代表进入cpu工厂
//////			if (PlaceType >= THUAI5::PlaceType::BlindZone1 && PlaceType <= THUAI5::PlaceType::BlindZone3)cellmap[i][j] += 10;     //代表进入草丛
//////			if (PlaceType == THUAI5::PlaceType::Wall)cellmap[i][j] += -10000;   //不要撞墙
//////			if (bullet)cellmap[i][j] += -300 * bullet;   //代表目标位置有子弹
//////		}
//////	}
//	int h = 1, t = 1;//h=head头指针，t=tail尾指针
//					 //目前只考虑四个方向运动的情况
//	short dx[5] = { 0,-1,0,1,0 };  //方向分别为：左，下，右，上
//	short dy[5] = { 0,0,1,0,-1 };
//	double decide[5] = { 0,0,0,0,0 };
//	int t0 = 100;//行走一个格子需要的时间，需要结合速度
//	int direction = 1; int maxdecide = 0;
//	for (int r = 1; r <= 4; r++) {     //分别计算向四个方向行走的得分最大值，选择最优方向移动
//		qx[h] = self.x + dx[r], qy[h] = self.y + dy[r], qt[h] = 0;
//		for (; h <= t; h++) {
//			for (int i = 1; i <= 4; i++) {
//				short xx = qx[h] + dx[i];
//				short yy = qy[h] + dy[i];
//				short tt = qt[h] + t0;
//				if (xx <= 0 || xx > 50 || yy <= 0 || yy > 50)continue;
//				decide[r] += f(cellmap[xx][yy], tt);
//				qx[++t] = xx, qy[++t] = yy, qt[++t] = tt;
//			}
//		}
//		if (decide[r] > maxdecide) {
//			direction = r;
//			maxdecide = decide[r];
//		}
//	}
//	return direction;
//}
