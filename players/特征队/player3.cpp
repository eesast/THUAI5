#include <random>
#include "../include/AI.h"
#include<math.h>
#include<ctime>
#define STEP 300000

#define pi 3.1415926539


#include<iostream>
/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */
// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;
// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Amplification;
// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::EmissionAccessory;
namespace
{	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

double E = 0;//用于旋转躲避的全局变量函数，表征转动方向。





class coordination
{
public:
	static int count;
	static int IS;
	int x;
	int y;
	coordination(int X = 0, int Y = 0) :x(X), y(Y) {};
	bool operator==(coordination& b) { return (x == b.x && y == b.y); }
	coordination& operator()(int x, int y) { coordination origin(x, y);return origin; }
};






coordination myself;

int coordination::count = 0;

int coordination::IS = 0;


coordination Past_and_Present[5];

coordination* p = Past_and_Present;


int nearest_enemy_xy[2];//最近敌人，第一个为x坐标，第二个为y坐标

int nearest_teammate_xy[2];//最近队友，第一个为x坐标，第二个为y坐标



int Stop = 996;










class theWall
{
public:
	int L;
	int R;
	int U;
	int D;
	coordination central;
	coordination corner[4];	/*coordination UL;	coordination UR;	coordination DL;	coordination DR;*/
	theWall(int l = 0, int r = 0, int u = 0, int d = 0) :L(l), R(r), U(u), D(d)
	{
		corner[0](U, L);
		corner[1](U, R);
		corner[2](D, L);
		corner[3](D, R);
		central((int)((double)(U + D) / 2), (int)(double(L + R) / 2));
	};
	/*void display_for_debug()
	{
		std::cout << "UL="; corner[0].display();
		std::cout << "UR="; corner[1].display();
		std::cout << "DL="; corner[2].display();
		std::cout << "DR="; corner[3].display();
	}*/
}wall[8] =
{ theWall(12375,29500,4375,8500),theWall(42375,48500,8125,14750),theWall(20500,24500,17500,19500),theWall(27500,32625,17500,19625),
theWall(22400,27500,21500,25500),theWall(1500,8625,30500,38500),theWall(16500,34700,30500,32500),theWall(6080,14581,39500,44625) };

int nextplace[2] = { 0,0 };


bool go = false;



inline double d(coordination p, coordination q)//距离
{
	return (sqrt((p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y)));
}



struct cpu_position {
	int32_t x;
	int32_t y;
	bool hasCpu;
};
struct Walked {
	//每个点的六个数代表横坐标、纵坐标、上下左右状态;0表示不可前往，1反之
	int32_t cordinate[500][6];
	int num;
};
clock_t start = clock();
clock_t thisTurn = clock();
struct Walked walked = { {{0}},0 };
//最终目的和临时目的的绝对坐标
int32_t finalGoalx;
int32_t finalGoaly;
int32_t tempGoalx;
int32_t tempGoaly;
//是否到过
bool hasCpu = 0;

bool beenTo(int32_t cellX, int32_t cellY) {
	for (int i = 0; i < walked.num;i++) {
		if ((walked.cordinate[i][0] == cellX) && (walked.cordinate[i][1] == cellY))
		{
			//std::cout << "beenTo" << cellX<<"," << cellY << std::endl;
			return true;
		}
	}
	return false;
}
cpu_position getNearCpu(IAPI& api) {
	auto Props = api.GetProps();
	auto propsNum = Props.size();
	auto cpu = THUAI5::PropType::CPU;
	int32_t cpux = 0, cpuy = 0;
	bool hasCpu = false;
	//循环查找cpu
	if (propsNum) {
		auto self = api.GetSelfInfo();
		auto mex = self->x;
		auto mey = self->y;
		//寻找最近的CPU
		double minDis = 50000 * 2.0;
		for (int i = 0; i < propsNum; i++) {
			auto x = api.GridToCell(Props[i]->x);
			auto y = api.GridToCell(Props[i]->y);
			if (Props[i]->type == cpu
				&& (!((y >= 10 && y <= 12)))) {
				hasCpu = true;
				auto newCpux = Props[i]->x;
				auto newCpuy = Props[i]->y;
				int32_t deltax, deltay;
				if (mex > newCpux)
					deltax = mex - newCpux;
				else
					deltax = newCpux - mex;
				if (mey > newCpuy)
					deltay = mey - newCpuy;
				else
					deltay = newCpuy - mey;
				auto newDis = sqrt((double)pow(deltax, 2) + (double)pow(deltay, 2));
				if (newDis < minDis) {
					minDis = newDis;
					cpux = newCpux;
					cpuy = newCpuy;
				}
			}
		}
	}
	cpu_position result = { cpux,cpuy,hasCpu };
	return result;
}







//周围是否有墙
bool WallAround(int32_t cellX, int32_t cellY, IAPI& api) {
	bool result = false;
	auto wall = THUAI5::PlaceType::Wall;
	auto self = api.GetSelfInfo();
	auto mex = self->x;
	auto mey = self->y;
	for (int i = cellX - 1;i <= cellX + 1;i++)
		for (int j = cellY - 1; j <= cellY + 1; j++) {
			int dis = (int)sqrt(pow((mex - api.CellToGrid(i)), 2) + pow((mey - api.CellToGrid(j)), 2));
			if (dis <= 1208 && api.GetPlaceType(i, j) == wall)
				result = true;
		}
	return result;
}
bool hasWallAcross(int32_t x, int32_t y, IAPI& api) {
	auto self = api.GetSelfInfo();
	auto mex = api.GridToCell(self->x);
	auto mey = api.GridToCell(self->y);
	auto goalx = api.GridToCell(x);
	auto goaly = api.GridToCell(y);
	auto maxx = (mex >= goalx) ? mex : goalx;
	auto minx = (mex < goalx) ? mex : goalx;
	auto maxy = (mey >= goaly) ? mey : goaly;
	auto miny = (mey < goaly) ? mey : goaly;
	bool result = false;
	for (auto i = minx; i <= maxx; i++) {
		for (auto j = miny; j <= maxy; j++) {
			if (api.GetPlaceType(i, j) == THUAI5::PlaceType::Wall)
				result = true;
		}
	}
	return result;
}


int checkStuck(IAPI& api)  //0 没卡 1 卡墙  2卡敌人
{	p = Past_and_Present;
	coordination* q = p + 1;
	int flag = 0;
	for (int i = 1;i < 6;i++, q++)
	{
		if (!(*q==p[0]))
			return 0;
	}
	//printf("\n\n\n\nJAMMED!JAMMED!JAMMED!JAMMED!JAMMED!JAMMED!JAMMED!\n\n\n\n");

	coordination EnemyNearby = (nearest_enemy_xy[0], nearest_enemy_xy[1]);
	coordination TeammateNearby = (nearest_teammate_xy[0], nearest_teammate_xy[1]);
	//Update my info(for debug)
	auto self = api.GetSelfInfo();
	auto mex = self->x;
	auto mey = self->y;
	Past_and_Present[coordination::count] = (mex, mey);
	coordination::count++;
	coordination::count %= 6;
	double enemyDistance = d(myself, EnemyNearby);
	double e_Enemey = atan2(nearest_enemy_xy[1] - mey, nearest_enemy_xy[0] - mex);
	double teammateDistance = d(myself, TeammateNearby);
	double e_Teammate = atan2((nearest_teammate_xy[1] - mey)   , (nearest_teammate_xy[0] - mex));


	if (enemyDistance > 3000 && teammateDistance > 3000)
	{
		//std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\nIT's the WALL!!  IT's the WALL!!  IT's the WALL!!  \n\n\n\n\n\n\n\n\n\n\n\n\n";


		return 1;
	}
	else if (enemyDistance < teammateDistance)
	{
		std::default_random_engine F;
		std::uniform_real_distribution<double> f(-3.14159, 3.14159);
		api.Attack(f(F));

		/*api.Attack(e_Enemey);*/
		api.MovePlayer(75, -1 * e_Enemey);

		//api.MovePlayer(200, -1 * e_Enemey);
		std::default_random_engine G;
		std::uniform_real_distribution<double> g(-3.14159, 3.14159);
		api.Attack(g(G));


		//std::cout << "\n\nJust shot him" << '\n\n';
		api.MovePlayer(500, -1 * e_Enemey);
		//api.MovePlayer(500, e_Enemey);
		//std::cout << "\n\nRetreat!!" << '\n\n';
		return 2;
	}
	else
	{

		//std::cout << "\n\nIt's our ally!!!!It's our ally!!!!It's our ally!!!!" << '\n\n';
		//api.MovePlayer(500, -1 * e_Teammate);
		//api.MovePlayer(500, e_Teammate + pi / 2);
		//std::cout << "\n\nRetreat!!" << '\n\n';
		return 2;
	}
		
	
	

}

//coordination& avoidCorner(coordination p, IAPI& api)                      //将最接近我第几个墙，和第几个角，存在next[0][1]中。
//{	//找到最近的墙的最近的角。
//	double D = d(p, wall[0].central);    /*std::cout << "\nd(p,wall[" << 0 << "]=" << d(p, wall[0].central);*/
//	int i = 1, I = 0;
//	for (;i < 8;i++)                    /*std::cout << "\nd(p,wall[" << i << "]=" << d(p, wall[i].central);*/
//	{
//		if (d(p, wall[i].central) < D)
//		{
//			I = i;			                         /*std::cout << "\nI=" << I;*/
//			D = d(p, wall[i].central);			/*std::cout << "\nD=" << D;*/
//		}
//	}
//	double D2 = d(p, wall[I].corner[0]);
//	/*std::cout << "\n\nnearest wall:   " << I;	std::cout << "\nd(p,wall[" << I << "],corner[" << 0 << "]=" << d(p, wall[I].corner[0]);*/
//
//	int j = 1, J = 0;
//	for (;j < 4;j++)
//	{	//std::cout << "\nd(p,wall[" << I << "],corner["<<j<<"]="<< d(p, wall[I].corner[j]);
//		if (d(p, wall[I].corner[j]) < D2)
//		{
//			J = j;	/*		std::cout << "\nJ=" << J;*/
//			D2 = d(p, wall[I].corner[j]);
//
//			if (D2 < 1500)//近距离阈值（fordebug）
//			{	//规避最近的角：	
//				//std::cout << "\n\nToo near!\nToo near!\nToo near!" << "'\nWall NO." << I << "\nCorner" << J << "\nDistance:  " << D2<<"\n\n";
//				double v = atan2(-p.y+wall[I].corner[J].y, -p.x+wall[I].corner[J].x );
//				////掩护火力
//				api.Attack(-1*v);api.Attack(-1 * v);api.Attack(-1 * v);api.Attack(-1 * v);
//				////if(J==0) //在左上角附近
//				//// 统一操作：
//				////先往后退
//				//api.MovePlayer(400, -1 * v);
//				//printf("\n\nBACK!!BACK!!!");
//				//api.MovePlayer(400,  v);
//				//printf("\n\nGo!!GO!!");
//				////再顺时针转
//				//v += 1.57;
//				//if (v > 3.1416)
//				//	v -= 2 * 3.14159;
//				//api.MovePlayer(250, v);				
//				//printf("\n\nTWIRL!Twirl!!!");
//				//printf("\n\nTWIRL!Twirl!!!");
//				return wall[I].corner[J];
//			
//			}
//
//		}
//	}
//	coordination b(0, 0);
//	return b;
//}

//！！！！！！！！！！！！！！！！！！！找最近的敌人和队友！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！



void get_enemy_xy(IAPI& api)
{
	auto Robots = api.GetRobots();
	auto RobotsNum = Robots.size();
	nearest_enemy_xy[0] = 100000;
	nearest_enemy_xy[1] = 100000;
	//循环查找敌人
	if (RobotsNum) {
		auto self = api.GetSelfInfo();
		auto team = self->teamID;
		auto mex = self->x;
		auto mey = self->y;
		//寻找最近的敌人
		double minDis = 50000 * 2.0;
		for (int i = 0; i < RobotsNum; i++) {
			if (Robots[i]->teamID != team) {
				auto tempx = (int)Robots[i]->x;
				auto tempy = (int)Robots[i]->y;
				int32_t deltax, deltay;
				if (mex > tempx)
					deltax = mex - tempx;
				else
					deltax = tempx - mex;
				if (mey > tempy)
					deltay = mey - tempy;
				else
					deltay = tempy - mey;
				auto newDis = sqrt((double)pow(deltax, 2) + (double)pow(deltay, 2));
				if (newDis < minDis) {
					minDis = newDis;
					nearest_enemy_xy[0] = tempx;
					nearest_enemy_xy[1] = tempy;
				}
			}
		}
	}
}

void   get_teammate_xy   (IAPI& api)

{   auto Robots = api.GetRobots();
	auto RobotsNum = Robots.size();
	nearest_teammate_xy[0] = 100000;
	nearest_teammate_xy[1] = 100000;
	//循环查找队友
	if (RobotsNum) {
		auto self = api.GetSelfInfo();
		auto team = self->teamID;
		auto mex = self->x;
		auto mey = self->y;
		//寻找最近的队友
		double minDis = 50000 * 2.0;
		for (int i = 0; i < RobotsNum; i++) {
			if (Robots[i]->teamID == team) {
				auto tempx = (int)Robots[i]->x;
				auto tempy = (int)Robots[i]->y;
				int32_t deltax, deltay;
				if (mex > tempx)
					deltax = mex - tempx;
				else
					deltax = tempx - mex;
				if (mey > tempy)
					deltay = mey - tempy;
				else
					deltay = tempy - mey;
				auto newDis = sqrt((double)pow(deltax, 2) + (double)pow(deltay, 2));
				if (newDis < minDis) {
					minDis = newDis;
					nearest_teammate_xy[0] = tempx;
					nearest_teammate_xy[1] = tempy;
				}
			}
		}
	}
}




inline void pick_or_kill(IAPI& api,bool TargetType)
{
	//std::cout << "\n\n\nPick or kill!!!!!!!!!!!!!!!!!!!!!!!!!1";
	//Update my info(for debug)
	auto self = api.GetSelfInfo();
	auto mex = self->x;
	auto mey = self->y;
	Past_and_Present[coordination::count] = (mex, mey);
	coordination::count++;
	coordination::count %= 6;
	
	if (TargetType)
	{  
		get_enemy_xy(api);
		double e = atan2(nearest_enemy_xy[1] - mey, nearest_enemy_xy[0] - mex);
		coordination EnemyNearby = (nearest_enemy_xy[0], nearest_enemy_xy[1]);
		double enemyDistance = d(myself, EnemyNearby);
		if (self->timeUntilCommonSkillAvailable == 0
			&& (enemyDistance < 3000)) {
			api.UseCommonSkill();
		}
		if ((self->timeUntilCommonSkillAvailable >= 25000)
			&& (self->signalJammerNum != 0)
			&& (enemyDistance < 3000)) {
			api.Attack(e);
			api.Attack(e);
		}
		else {
			if ((enemyDistance < 4000) && (self->signalJammerNum != 0)) {
				api.Attack(e);
				api.Attack(e);
			}
			else {
				api.MovePlayer(75, e + pi / 2);
			}
		}
		return;
	}
	api.Pick(THUAI5::PropType::CPU);
	if((api.GetSelfInfo()->cpuNum>=5)||(self->life<=3000))
		api.UseCPU(api.GetSelfInfo()->cpuNum);

}
//！！！！！！！！！！！！！！！！！！！找最近的敌人和队友！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！


//Target Type: 0   CPU   1  Enemy
bool proceed(int cpux, int cpuy, IAPI& api,bool TargetType) 
{
	auto self = api.GetSelfInfo();
	auto mex = self->x;
	auto mey = self->y;
	coordination me(mex, mey);
	coordination target(cpux, cpuy);
	auto meCorx = api.GridToCell(mex);
	auto meCory = api.GridToCell(mey);
	auto cpuCorx = api.GridToCell(cpux);
	auto cpuCory = api.GridToCell(cpuy);
	auto speed = self->speed;

	
	int deltax, deltay;
	deltax = (int)cpux - (int)mex;
	deltay = (int)cpuy - (int)mey;
	double e;
	e = atan2(deltay, deltax);
	double distance = sqrt((double)pow(cpux - mex, 2) + (double)pow(cpuy - mey, 2));

	api.Pick(THUAI5::PropType::CPU);
	api.UseCPU(api.GetSelfInfo()->cpuNum);
	//判断当前是否卡住。根据卡在对手、自己人、墙上分别做出反应。
	


	if (!hasWallAcross(cpux, cpuy, api)) 
	{	tempGoalx = cpux;
		tempGoaly = cpuy;
		api.MovePlayer(75, e);
		pick_or_kill(api, TargetType);
		//pick_or_kill(api, TargetType);//
		if (checkStuck(api) != 0) {//真卡死了

			if (mex < cpux)
			{
				api.MoveDown(50);
				//pick_or_kill(api, TargetType);
			}
			else
			{
				api.MoveUp(50);
				//pick_or_kill(api, TargetType);

			}

			if (mey < cpuy)
			{
				api.MoveRight(50);
				//pick_or_kill(api, TargetType);

			}
			else
			{
				api.MoveLeft(50);
				//pick_or_kill(api, TargetType);

			}
			/*api.MovePlayer(25, pi * (-0.5+(mey<cpuy)));
			api.Pick(THUAI5::PropType::CPU);
			api.UseCPU(api.GetSelfInfo()->cpuNum);
			api.Wait();

			api.MovePlayer(25, pi * (-0.5 + (mey < cpuy)));
			api.Pick(THUAI5::PropType::CPU);
			api.UseCPU(api.GetSelfInfo()->cpuNum);
			api.Wait();*/
			

			if (mex < cpux)
			{
				api.MoveDown(50);

				//pick_or_kill(api, TargetType);

			}
			else
			{
				api.MoveUp(50);
				//pick_or_kill(api, TargetType);

			}
		}
		return true;
	}




	else
	{	walked.cordinate[walked.num][0] = cpuCorx;
		walked.cordinate[walked.num][1] = cpuCory;
		if ((beenTo(cpuCorx - 1, cpuCory)) || api.GetPlaceType(cpuCorx - 1, cpuCory) == THUAI5::PlaceType::Wall)
			walked.cordinate[walked.num][2] = 0;
		else
			walked.cordinate[walked.num][2] = 1;
		if ((beenTo(cpuCorx + 1, cpuCory)) || api.GetPlaceType(cpuCorx + 1, cpuCory) == THUAI5::PlaceType::Wall)
			walked.cordinate[walked.num][3] = 0;
		else
			walked.cordinate[walked.num][3] = 1;
		if ((beenTo(cpuCorx, cpuCory - 1)) || api.GetPlaceType(cpuCorx, cpuCory - 1) == THUAI5::PlaceType::Wall)
			walked.cordinate[walked.num][4] = 0;
		else
			walked.cordinate[walked.num][4] = 1;
		if ((beenTo(cpuCorx, cpuCory + 1)) || api.GetPlaceType(cpuCorx, cpuCory + 1) == THUAI5::PlaceType::Wall)
			walked.cordinate[walked.num][5] = 0;
		else
			walked.cordinate[walked.num][5] = 1;
		walked.num++;

		int available = 0;
		for (int i = 2; i < 6; i++) {
			if (walked.cordinate[walked.num - 1][i] == 1)
				available++;
		}

		while (available > 0) 
		{
			//不耗太长时间
			if ((clock() - thisTurn) > 200) {
				if ((getNearCpu(api).x != finalGoalx) || (getNearCpu(api).y != finalGoaly))
					return true;
				thisTurn = clock();
			}

			//下一步的格点坐标
			auto nextx = cpuCorx;
			auto nexty = cpuCory;
			double mindis = 1000000;
			//选了那个点，其中2,3,4,5代表上下左右
			int chosen = 0;
			//上
			if ((walked.cordinate[walked.num - 1][2] == 1)
				&& (mindis > sqrt((double)pow(cpuCorx - 1 - meCorx, 2) + (double)pow(cpuCory - meCory, 2))
					+ sqrt((double)pow(cpuCorx - 1 - api.GridToCell(finalGoalx), 2) + (double)pow(cpuCory - api.GridToCell(finalGoaly), 2))))
			{
				mindis = sqrt((double)pow(cpuCorx - 1 - meCorx, 2) + (double)pow(cpuCory - meCory, 2))
					+ sqrt((double)pow(cpuCorx - 1 - api.GridToCell(finalGoalx), 2) + (double)pow(cpuCory - api.GridToCell(finalGoaly), 2));
				nextx = cpuCorx - 1;
				nexty = cpuCory;
				chosen = 2;
			}
			//下
			if ((walked.cordinate[walked.num - 1][3] == 1)
				&& (mindis > sqrt((double)pow(cpuCorx + 1 - meCorx, 2) + (double)pow(cpuCory - meCory, 2))
					+ sqrt((double)pow(cpuCorx + 1 - api.GridToCell(finalGoalx), 2) + (double)pow(cpuCory - api.GridToCell(finalGoaly), 2)))) {
				mindis = sqrt((double)pow(cpuCorx + 1 - meCorx, 2) + (double)pow(cpuCory - meCory, 2))
					+ sqrt((double)pow(cpuCorx + 1 - api.GridToCell(finalGoalx), 2) + (double)pow(cpuCory - api.GridToCell(finalGoaly), 2));
				nextx = cpuCorx + 1;
				nexty = cpuCory;
				chosen = 3;
			}
			//左
			if ((walked.cordinate[walked.num - 1][4] == 1)
				&& (mindis > sqrt((double)pow(cpuCorx - meCorx, 2) + (double)pow(cpuCory - 1 - meCory, 2))
					+ sqrt((double)pow(cpuCorx - api.GridToCell(finalGoalx), 2) + (double)pow(cpuCory - 1 - api.GridToCell(finalGoaly), 2)))) {
				mindis = sqrt((double)pow(cpuCorx - meCorx, 2) + (double)pow(cpuCory - 1 - meCory, 2))
					+ sqrt((double)pow(cpuCorx - api.GridToCell(finalGoalx), 2) + (double)pow(cpuCory - 1 - api.GridToCell(finalGoaly), 2));
				nextx = cpuCorx;
				nexty = cpuCory - 1;
				chosen = 4;
			}
			//右
			if ((walked.cordinate[walked.num - 1][5] == 1)
				&& (mindis > sqrt((double)pow(cpuCorx - meCorx, 2) + (double)pow(cpuCory + 1 - meCory, 2))
					+ sqrt((double)pow(cpuCorx - api.GridToCell(finalGoalx), 2) + (double)pow(cpuCory + 1 - api.GridToCell(finalGoaly), 2)))) {
				mindis = sqrt((double)pow(cpuCorx - meCorx, 2) + (double)pow(cpuCory + 1 - meCory, 2))
					+ sqrt((double)pow(cpuCorx - api.GridToCell(finalGoalx), 2) + (double)pow(cpuCory + 1 - api.GridToCell(finalGoaly), 2));
				nextx = cpuCorx;
				nexty = cpuCory + 1;
				chosen = 5;
			}
			if (chosen == 0) 
			{
				
				return false;
			}
			//std::cout << "Time:" << clock() - start << std::endl;
			//std::cout << "going to " << nextx << ',' << nexty << std::endl;
			//std::cout << "Distance:" << mindis << std::endl;
			walked.cordinate[walked.num - 1][chosen] = 0;
			if (proceed(api.CellToGrid(nextx), api.CellToGrid(nexty), api, TargetType))
				return true;

			//重新检查可前往点数
			available = 0;
			for (int i = 2; i < 6; i++) {
				if (walked.cordinate[walked.num][i] == 1)
					available++;
			}
		}
		walked.cordinate[walked.num][0] = 0;
		walked.cordinate[walked.num][1] = 0;
		walked.cordinate[walked.num][2] = 0;
		walked.cordinate[walked.num][3] = 0;
		walked.cordinate[walked.num][4] = 0;
		walked.cordinate[walked.num][5] = 0;
		walked.num--;
	}
}

void proceedFinal(int targetX, int targetY, IAPI& api,bool TargetType)
{
	auto self = api.GetSelfInfo();
	auto mex = self->x;
	auto mey = self->y;
	coordination me(mex, mey);
	//tooNear(me);
	double e = atan2((int)targetY - (int)mey, (int)targetX - (int)mex);
	//如果目标没变且还没到中间位置，则继续前往中间位置
	if (targetX == finalGoalx && targetY == finalGoaly	&& (sqrt(pow((mex - tempGoalx), 2) + pow((mey - tempGoaly), 2)) > 500))
	{
		proceed(tempGoalx, tempGoaly, api, TargetType);
		return;
	}

	else 
	{
		finalGoalx = targetX;
		finalGoaly = targetY;
		if (!proceed(targetX, targetY, api, TargetType))
			exit(-1);
		else
			walked.num = 0;
		return;
	}
	
}


void position_initialization(IAPI& api)
{
	auto self = api.GetSelfInfo();
	auto mex = self->x;
	auto mey = self->y;
	if (self->teamID == 0 && self->playerID == 0)
	{
		if (mex < 9000)
		{
			api.MovePlayer(75, 0.25);
		}
		else
			go = true;
	}
	if (self->teamID == 0 && self->playerID == 1)
	{
		if (mey > 12000)
		{
			api.MoveLeft(75);
		}
		else if (mex < 9000)
		{
			api.MoveDown(75);
		}
		else
			go = true;
	}
	if (self->teamID == 0 && self->playerID == 2)
	{
		if (mey < 31000)
		{
			api.MoveRight(75);
		}
		else if (mex < 13000)
		{
			api.MoveDown(75);
		}
		else
			go = true;

	}
	if (self->teamID == 0 && self->playerID == 3)
	{
		if (mey > 34000)
		{
			api.MoveLeft(75);
		}
		else if (mex < 13000)
		{
			api.MoveDown(75);
		}
		else
			go = true;

	}
	if (self->teamID == 1 && self->playerID == 0)
	{
		if (mey < 16000)
		{
			api.MoveRight(75);
		}
		else if (mex > 30000)
		{
			api.MoveUp(75);
		}
		else
			go = true;
	}
	if (self->teamID == 1 && self->playerID == 1)
	{
		if (mey > 16000)
		{
			api.MoveLeft(75);
		}
		else if (mex > 34000)
		{
			api.MoveUp(75);
		}
		else
			go = true;
	}
	if (self->teamID == 1 && self->playerID == 2)
	{
		if (mex > 32000)
		{
			api.MovePlayer(75, 5 * pi / 6);
		}
		else
			go = true;
	}
	if (self->teamID == 1 && self->playerID == 3)
	{
		if (mex > 34000)
		{
			api.MoveUp(75);
		}
		else
			go = true;
	}
}


void AI::play(  IAPI&   api)
{
	thisTurn = clock();
	std::ios::sync_with_stdio(false);


	//Update my info(for debug)
	auto self = api.GetSelfInfo();
	auto mex = self->x;
	auto mey = self->y;
	Past_and_Present[coordination::count] = (mex, mey);
	coordination::count++;
	coordination::count %= 6;
	//for (int i = 0;i < 5;i++)		std::cout << "\ncoordination:" << "  x=  " << Past_and_Present[i].x << "  y=  " << Past_and_Present[i].y;

	//Update cpu info
	cpu_position nearest;
	nearest = getNearCpu(api);
	auto cpux = nearest.x;
	auto cpuy = nearest.y;
	bool hasCpu = nearest.hasCpu;

	//Update enemy& teammates' info
	get_enemy_xy(api);
	get_teammate_xy(api);
	
	if (self->isResetting && (go == true)) {
		go = false;
	}
	
	if (!go)
		position_initialization(api);
	else
	{
		//control center,main body of function.
		coordination EnemyNearby = (nearest_enemy_xy[0], nearest_enemy_xy[1]);
		coordination TeammateNearby = (nearest_teammate_xy[0], nearest_teammate_xy[1]);

		double enemyDistance = d(myself, EnemyNearby);
		double TeammateDistance = d(myself, TeammateNearby);


		if (enemyDistance <10000)
		{
			proceedFinal(nearest_enemy_xy[0], nearest_enemy_xy[1], api, 1);
			return;
		}

		else if (hasCpu)
		{
			proceedFinal(cpux, cpuy, api, 0);
			return;
		}


		else
		{
			for (int i = 0;i < 3;i++)
			{
				std::default_random_engine F;
				std::uniform_real_distribution<double> f(-3.14159, 3.14159);

				api.MovePlayer(100, f(F));
				api.Attack(f(F));
			}
			return;

		}
	}
}
