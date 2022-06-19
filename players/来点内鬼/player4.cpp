#include <random>
#include "../include/AI.h"
#include<queue>
#include<cmath>
#include<algorithm>
#include <ctime>
#pragma warning(disable:C26495)

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = true;


// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Amplification;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::EnergyConvert;

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

//机器人与道具
struct RobottoProp
{
	std::shared_ptr<const THUAI5::Robot> robot;
	std::shared_ptr<const THUAI5::Prop> prop;
	double dis;
	friend bool operator <(const RobottoProp& a, const RobottoProp& b) {
		return a.dis > b.dis;
	}
};
//自己满血血量
int selfLife;
//机器人目标道具
std::priority_queue<RobottoProp> RobottoProps;

const double PI = 3.1415926;
//移动方向
double angle = 0;
//探图节点
struct node {
	int x;
	int y;
	int prex;
	int prey;
};

struct home {
	int x;
	int y;
	int graphtype;
	int team;
	int homeno;
}home1;


class Enemy
{
public:
	double getnextx()
	{
		return 2 * x - prex;
	}
	double getnexty()
	{
		return 2 * y - prey;
	}
	double x;//这一次的坐标
	double y;
	double prex;//上一次的坐标 确定一条直线
	double prey;
	double nextx;//预测接下来的坐标 假设敌人速度不变
	double nexty;
	double t;
	double speed;//敌人速度
	double angle;//敌人移动角度
}enemy[4];

//自己
Enemy own;
//扔cpu距离
int disforcpu;
//开始时刻
clock_t start;
//炮弹guid
uint64_t _guidofjammer = 0;
uint64_t guidofjammer = 1;

//场上其他人的位置信息
std::set<node> PofOtherRobot;

void isWalling(IAPI& api);
std::shared_ptr<const THUAI5::Prop> uploadcpu(std::shared_ptr<const THUAI5::Robot> self, std::vector<std::shared_ptr<const THUAI5::Prop>> props);
bool iscpu(IAPI& api, std::shared_ptr<const THUAI5::Robot> self, std::vector<std::shared_ptr<const THUAI5::Prop>> props, int x, int y);
std::shared_ptr<const THUAI5::Prop> uploadprop(IAPI& api, std::shared_ptr<const THUAI5::Robot> self, std::vector<std::shared_ptr<const THUAI5::Prop>> props);
double getDtoRobot(std::shared_ptr<const THUAI5::Robot> self, std::shared_ptr<const THUAI5::Robot> other);
double getDtoProp(std::shared_ptr<const THUAI5::Robot> self, std::shared_ptr<const THUAI5::Prop> prop);
std::vector<node> dijkstra(int x, int y, int sx, int sy);
void selfControl(std::shared_ptr<const THUAI5::Robot> self, IAPI& api);
void moveToProp(std::shared_ptr<const THUAI5::Prop> prop, IAPI& api);
void moveToelc(IAPI& api);
void moveTohome(IAPI& api);
double getDirection(uint32_t selfPoisitionX, uint32_t selfPoisitionY, uint32_t aimPositionX, uint32_t aimPositionY);
bool search(std::shared_ptr<const THUAI5::Robot> self, uint32_t selfPoisitionX, uint32_t selfPoisitionY, uint32_t aimPositionX, uint32_t aimPositionY);
bool evade(std::shared_ptr<const THUAI5::Robot> self, IAPI& api);
bool ismove(THUAI5::PlaceType type);
bool ismove(int type);
bool cpuisourhome(IAPI& api, std::shared_ptr<const THUAI5::Prop> prop);
void throwcpu(IAPI& api);
void moveToenemy(IAPI& api);
void awayfromenemy(IAPI& api);
void BeInvisible(IAPI& api);

//判断某地是否可以移动
bool ismove(THUAI5::PlaceType Type) {
	int type = int(Type);
	if (type == 1 || (type > 4 && type < 13)) {
		return false;
	}
	return true;
}
//重载
bool ismove(int type) {
	if (type == 1 || (type > 4 && type < 13)) {
		return false;
	}
	return true;
}

//获取角度
double getDirection(uint32_t selfPoisitionX, uint32_t selfPoisitionY, uint32_t aimPositionX, uint32_t aimPositionY)
{
	double delta_x = double(aimPositionX) - double(selfPoisitionX);
	double delta_y = double(aimPositionY) - double(selfPoisitionY);
	double direction = atan2(delta_y, delta_x);
	if (direction < 0)
	{
		direction = 2 * PI + direction;
	}
	return direction;
}

//攻击距离
bool search(std::shared_ptr<const THUAI5::Robot> self, uint32_t selfPoisitionX, uint32_t selfPoisitionY, uint32_t aimPositionX, uint32_t aimPositionY)
{
	double SEARCH_R = self->attackRange + 1000;
	return SEARCH_R > sqrt((aimPositionX - selfPoisitionX) * (aimPositionX - selfPoisitionX) + (aimPositionY - selfPoisitionY) * (aimPositionY - selfPoisitionY));
}

//得到自身到道具的距离
double getDtoProp(std::shared_ptr<const THUAI5::Robot> self, std::shared_ptr<const THUAI5::Prop> prop) {
	auto selfx = self->x;
	auto selfy = self->y;
	auto propx = prop->x;
	auto propy = prop->y;
	return sqrt((selfx - propx) * (selfx - propx) + (selfy - propy) * (selfy - propy));
}

//得到自身到机器人的距离
double getDtoRobot(std::shared_ptr<const THUAI5::Robot> self, std::shared_ptr<const THUAI5::Robot> other) {
	auto selfx = self->x;
	auto selfy = self->y;
	auto otherx = other->x;
	auto othery = other->y;
	return sqrt((selfx - otherx) * (selfx - otherx) + (selfy - othery) * (selfy - othery));
}

//得到自身到干扰弹的距离
double getDtoJammer(std::shared_ptr<const THUAI5::Robot> self, std::shared_ptr<const THUAI5::SignalJammer> jammer) {
	auto selfx = self->x;
	auto selfy = self->y;
	auto otherx = jammer->x;
	auto othery = jammer->y;
	return sqrt((selfx - otherx) * (selfx - otherx) + (selfy - othery) * (selfy - othery));
}

//保存探图结果
static std::vector<std::vector<int> > isWall(50, std::vector<int>(50, 0));
//判断探图是否结束
static bool isWalled = false;
//探图
void isWalling(IAPI& api) {
	for (int i = 0; i < 50; i++) {
		for (int j = 0; j < 50; j++) {
			isWall[i][j] = (int)api.GetPlaceType(i, j);
		}
	}
	auto Robots = api.GetRobots();
	auto self = api.GetSelfInfo();
	if (self->teamID == 1) {
		home1.homeno = 3;
		disforcpu = 15000;
		home1.team = 1;
		for (auto Robot : Robots) {
			if (self->playerID == 3) {
				home1.x = api.GridToCell(self->x);
				home1.y = api.GridToCell(self->y);
				break;
			}
			if (Robot->teamID != self->teamID) {
				continue;
			}
			if (Robot->playerID == 3) {
				home1.x = api.GridToCell(Robot->x);
				home1.y = api.GridToCell(Robot->y);
				break;
			}
		}
	}
	else {
		home1.homeno = 0;
		disforcpu = 15000;
		home1.team = 0;
		for (auto Robot : Robots) {
			if (self->playerID == 0) {
				home1.x = api.GridToCell(self->x);
				home1.y = api.GridToCell(self->y);
				break;
			}
			if (Robot->teamID != self->teamID) {
				continue;
			}
			if (Robot->playerID == 0) {
				home1.x = api.GridToCell(Robot->x);
				home1.y = api.GridToCell(Robot->y);
				break;
			}
		}
	}
	if (isWall[40][10] != 1 && isWall[40][9] == 1 && isWall[40][11] == 1 && isWall[41][8] == 1 && isWall[41][12] == 1) {//树
		home1.graphtype = 0;
		//树内部
		isWall[40][10] = 1;
		isWall[41][10] = 1;
		isWall[42][10] = 1;
		isWall[41][9] = 1;
		isWall[41][11] = 1;
		//出生点0
		isWall[43][7] = 1;
		//单长墙
		isWall[30][16] = 1;
		isWall[32][16] = 1;
		isWall[30][34] = 1;
		isWall[32][34] = 1;
		//单拐墙
		isWall[13][42] = 1;
		isWall[9][42] = 1;
		//短单墙
		isWall[17][24] = 1;
		//左单道
		isWall[34][1] = 1;
		//右单道
		isWall[11][48] = 1;
	}
	else {
		home1.graphtype = 1;
		disforcpu = 15000;
		//中心“拐”墙
		isWall[20][15] = 1;
		isWall[20][17] = 1;
		isWall[15][15] = 1;
		isWall[15][20] = 1;
		isWall[17][20] = 1;

		isWall[29][15] = 1;
		isWall[29][17] = 1;
		isWall[34][15] = 1;
		isWall[34][20] = 1;
		isWall[32][20] = 1;

		isWall[15][29] = 1;
		isWall[17][29] = 1;
		isWall[15][34] = 1;
		isWall[20][32] = 1;
		isWall[20][34] = 1;

		isWall[29][32] = 1;
		isWall[29][34] = 1;
		isWall[34][34] = 1;
		isWall[32][29] = 1;
		isWall[34][29] = 1;
	}
	isWalled = true;
}

bool cpuisourhome(IAPI& api, std::shared_ptr<const THUAI5::Prop> prop) {
	if (prop->isMoving) {
		auto self = api.GetSelfInfo();
		int nx = home1.x;
		int ny = home1.y;
		/*int d = sqrt((self->x - api.CellToGrid(nx)) * (self->x - api.CellToGrid(nx)) + (self->y - api.CellToGrid(ny)) * (self->y - api.CellToGrid(ny)));
		if (d < disforcpu) {
			return true;
		}*/
		return true;
	}
	return false;
}

//记录目前场上的cpu
std::vector<std::shared_ptr<const THUAI5::Prop>> cpus;
//更新场上cpu数据并返回距离自身最近的cpu指针,否则返回空指针
std::shared_ptr<const THUAI5::Prop> uploadcpu(std::shared_ptr<const THUAI5::Robot> self, std::vector<std::shared_ptr<const THUAI5::Prop>> props) {
	std::vector<std::shared_ptr<const THUAI5::Prop>>().swap(cpus);
	for (auto prop : props)
	{
		if (prop->type == THUAI5::PropType::CPU) {
			cpus.push_back(prop);
		}
	}
	double min = 1e6;
	std::shared_ptr<const THUAI5::Prop> ans = nullptr;
	for (auto cpu : cpus) {
		if (getDtoProp(self, cpu) < min) {
			ans = cpu;
			min = getDtoProp(self, cpu);
		}
	}
	return ans;
}
//判断当前位置是否有cpu
bool iscpu(IAPI& api, std::shared_ptr<const THUAI5::Robot> self, std::vector<std::shared_ptr<const THUAI5::Prop>> props, int x, int y) {
	for (auto prop : props)
	{
		if (prop->type == THUAI5::PropType::CPU) {
			if (api.GridToCell(prop->x) == x && api.GridToCell(prop->y) == y) {
				return true;
			}
		}
	}
	return false;
}

//更新场上prop数据并返回距离自身最近的prop指针,否则返回空指针
std::shared_ptr<const THUAI5::Prop> uploadprop(IAPI& api, std::shared_ptr<const THUAI5::Robot> self, std::vector<std::shared_ptr<const THUAI5::Prop>> props) {

	std::shared_ptr<const THUAI5::Prop> ans = nullptr;
	auto robots = api.GetRobots();
	for (auto prop : props) {
		if (cpuisourhome(api, prop) || !ismove(isWall[api.GridToCell(prop->x)][api.GridToCell(prop->y)]))
			continue;
		RobottoProps.push({ self,prop,getDtoProp(self,prop) });
		for (auto robot : robots) {
			if (robot->teamID != self->teamID) {
				continue;
			}
			RobottoProps.push({ robot,prop,getDtoProp(robot,prop) });
		}
	}
	if (RobottoProps.empty())return ans;

	std::set<std::shared_ptr<const THUAI5::Prop>> Props;
	std::set<std::shared_ptr<const THUAI5::Robot>> Robots;
	while (RobottoProps.size()) {
		auto top = RobottoProps.top();
		RobottoProps.pop();
		if (Robots.count(top.robot) || Props.count(top.prop)) {
			continue;
		}
		if (top.robot->playerID == self->playerID) {
			ans = top.prop;
			break;
		}
		Props.insert(top.prop);
		Robots.insert(top.robot);
		ans = top.prop;
	}
	while (RobottoProps.size()) {
		RobottoProps.pop();
	}
	return ans;
}


//路线
std::vector<node> dijkstra(int x, int y, int sx, int sy) {
	std::vector<node> ans;
	if (x == sx && y == sy)return ans;
	std::vector<std::vector<node>> Node(50, std::vector<node>(50));
	std::vector<std::vector<int>> flag(50, std::vector<int>(50, 0));
	std::queue<node> q;
	q.push({ x,y,0,0 });
	flag[x][y] = 1;
	while (q.size()) {
		node the = q.front();
		q.pop();

		if (the.x + 1 < 50 && ismove(isWall[the.x + 1][the.y]) && !flag[the.x + 1][the.y]) {
			Node[the.x + 1][the.y].x = the.x + 1;
			Node[the.x + 1][the.y].y = the.y;
			Node[the.x + 1][the.y].prex = the.x;
			Node[the.x + 1][the.y].prey = the.y;
			flag[the.x + 1][the.y] = 1;
			q.push(Node[the.x + 1][the.y]);
		}
		if (the.x - 1 >= 0 && isWall[the.x - 1][the.y] != 1 && (isWall[the.x - 1][the.y] < 5 || isWall[the.x - 1][the.y] == 13) && !flag[the.x - 1][the.y]) {
			Node[the.x - 1][the.y].x = the.x - 1;
			Node[the.x - 1][the.y].y = the.y;
			Node[the.x - 1][the.y].prex = the.x;
			Node[the.x - 1][the.y].prey = the.y;
			flag[the.x - 1][the.y] = 1;
			q.push(Node[the.x - 1][the.y]);
		}
		if (the.y + 1 < 50 && isWall[the.x][the.y + 1] != 1 && (isWall[the.x][the.y + 1] < 5 || isWall[the.x][the.y + 1] == 13) && !flag[the.x][the.y + 1]) {
			Node[the.x][the.y + 1].x = the.x;
			Node[the.x][the.y + 1].y = the.y + 1;
			Node[the.x][the.y + 1].prex = the.x;
			Node[the.x][the.y + 1].prey = the.y;
			flag[the.x][the.y + 1] = 1;
			q.push(Node[the.x][the.y + 1]);
		}
		if (the.y - 1 >= 0 && isWall[the.x][the.y - 1] != 1 && (isWall[the.x][the.y - 1] < 5 || isWall[the.x][the.y - 1] == 13) && !flag[the.x][the.y - 1]) {
			Node[the.x][the.y - 1].x = the.x;
			Node[the.x][the.y - 1].y = the.y - 1;
			Node[the.x][the.y - 1].prex = the.x;
			Node[the.x][the.y - 1].prey = the.y;
			flag[the.x][the.y - 1] = 1;
			q.push(Node[the.x][the.y - 1]);
		}
		if (ismove(isWall[the.x + 1][the.y]) && ismove(isWall[the.x][the.y + 1]) && ismove(isWall[the.x + 1][the.y + 1]) && !flag[the.x + 1][the.y + 1]) {
			Node[the.x + 1][the.y + 1].x = the.x + 1;
			Node[the.x + 1][the.y + 1].y = the.y + 1;
			Node[the.x + 1][the.y + 1].prex = the.x;
			Node[the.x + 1][the.y + 1].prey = the.y;
			flag[the.x + 1][the.y + 1] = 1;
			q.push(Node[the.x + 1][the.y + 1]);
		}
		if (ismove(isWall[the.x + 1][the.y]) && ismove(isWall[the.x][the.y - 1]) && ismove(isWall[the.x + 1][the.y - 1]) && !flag[the.x + 1][the.y - 1]) {
			Node[the.x + 1][the.y - 1].x = the.x + 1;
			Node[the.x + 1][the.y - 1].y = the.y - 1;
			Node[the.x + 1][the.y - 1].prex = the.x;
			Node[the.x + 1][the.y - 1].prey = the.y;
			flag[the.x + 1][the.y - 1] = 1;
			q.push(Node[the.x + 1][the.y - 1]);
		}
		if (ismove(isWall[the.x - 1][the.y]) && ismove(isWall[the.x][the.y + 1]) && ismove(isWall[the.x - 1][the.y + 1]) && !flag[the.x - 1][the.y + 1]) {
			Node[the.x - 1][the.y + 1].x = the.x - 1;
			Node[the.x - 1][the.y + 1].y = the.y + 1;
			Node[the.x - 1][the.y + 1].prex = the.x;
			Node[the.x - 1][the.y + 1].prey = the.y;
			flag[the.x - 1][the.y + 1] = 1;
			q.push(Node[the.x - 1][the.y + 1]);
		}
		if (ismove(isWall[the.x - 1][the.y]) && ismove(isWall[the.x][the.y - 1]) && ismove(isWall[the.x - 1][the.y - 1]) && !flag[the.x - 1][the.y - 1]) {
			Node[the.x - 1][the.y - 1].x = the.x - 1;
			Node[the.x - 1][the.y - 1].y = the.y - 1;
			Node[the.x - 1][the.y - 1].prex = the.x;
			Node[the.x - 1][the.y - 1].prey = the.y;
			flag[the.x - 1][the.y - 1] = 1;
			q.push(Node[the.x - 1][the.y - 1]);
		}
	}
	node temp = Node[sx][sy];
	ans.push_back(temp);
	while (temp.prex != x || temp.prey != y) {
		temp = Node[temp.prex][temp.prey];
		ans.push_back(temp);
	}
	reverse(ans.begin(), ans.end());
	return ans;
}

//调整自身位置
void selfControl(std::shared_ptr<const THUAI5::Robot> self, IAPI& api) {
	int propx = api.CellToGrid(api.GridToCell(self->x));
	int propy = api.CellToGrid(api.GridToCell(self->y));
	int cellx = api.GridToCell(self->x);
	int celly = api.GridToCell(self->y);
	own.prex = own.x;
	own.prey = own.y;
	own.x = self->x;
	own.y = self->y;
	auto type1 = api.GetPlaceType(cellx - 1, celly - 1);
	auto type2 = api.GetPlaceType(cellx + 1, celly - 1);
	auto type3 = api.GetPlaceType(cellx - 1, celly + 1);
	auto type4 = api.GetPlaceType(cellx + 1, celly + 1);
	auto type5 = api.GetPlaceType(cellx, celly + 1);
	auto type6 = api.GetPlaceType(cellx, celly - 1);
	auto type7 = api.GetPlaceType(cellx + 1, celly);
	auto type8 = api.GetPlaceType(cellx - 1, celly);
	auto Robots = api.GetRobots();
	if (own.x == own.prex && own.y == own.prey) {
		for (auto robot : Robots) {
			if (getDtoRobot(self, robot) < 2000)
				api.MovePlayer(200, PI + getDirection(own.x, own.y, robot->x, robot->y));
			if (!ismove(type1) && ismove(type6) && ismove(type8))
				api.MovePlayer(100, PI + getDirection(propx, propy, propx - 1000, propy - 1000));
			if (!ismove(type2) && ismove(type6) && ismove(type7))
				api.MovePlayer(100, PI + getDirection(propx, propy, propx + 1000, propy - 1000));
			if (!ismove(type3) && ismove(type5) && ismove(type8))
				api.MovePlayer(100, PI + getDirection(propx, propy, propx - 1000, propy + 1000));
			if (!ismove(type4) && ismove(type5) && ismove(type7))
				api.MovePlayer(100, PI + getDirection(propx, propy, propx + 1000, propy + 1000));
		}
	}
	else {
		angle = getDirection(own.prex, own.prey, own.x, own.y);
	}
	/*
	auto type1 = api.GetPlaceType(cellx - 1, celly - 1);
	auto type2 = api.GetPlaceType(cellx + 1, celly - 1);
	auto type3 = api.GetPlaceType(cellx - 1, celly + 1);
	auto type4 = api.GetPlaceType(cellx + 1, celly + 1);
	auto type5 = api.GetPlaceType(cellx, celly + 1);
	auto type6 = api.GetPlaceType(cellx, celly - 1);
	auto type7 = api.GetPlaceType(cellx + 1, celly);
	auto type8 = api.GetPlaceType(cellx - 1, celly);
	if (angle > 2 * PI - 0.1 || angle < 0.1 || (angle<PI + 0.1 && angle > PI - 0.1)) {
		if (!ismove(type1) && ismove(type2) && ismove(type3) && ismove(type4) && ismove(type5) && ismove(type6) && ismove(type7) && ismove(type8)) {
			api.MoveRight(80);

		}
		if (ismove(type1) && !ismove(type2) && ismove(type3) && ismove(type4) && ismove(type5) && ismove(type6) && ismove(type7) && ismove(type8)) {
			api.MoveRight(80);

		}
		if (ismove(type1) && ismove(type2) && !ismove(type3) && ismove(type4) && ismove(type5) && ismove(type6) && ismove(type7) && ismove(type8)) {
			api.MoveLeft(80);

		}
		if (ismove(type1) && ismove(type2) && ismove(type3) && !ismove(type4) && ismove(type5) && ismove(type6) && ismove(type7) && ismove(type8)) {
			api.MoveLeft(80);

		}
	}
	else {

		if (ismove(type1) && ismove(type2) && !ismove(type3) && ismove(type4) && ismove(type5) && ismove(type6) && ismove(type7) && ismove(type8)) {
			api.MoveDown(80);

		}

		if (!ismove(type1) && ismove(type2) && ismove(type3) && ismove(type4) && ismove(type5) && ismove(type6) && ismove(type7) && ismove(type8)) {
			api.MoveDown(80);;

		}

		if (ismove(type1) && ismove(type2) && ismove(type3) && !ismove(type4) && ismove(type5) && ismove(type6) && ismove(type7) && ismove(type8)) {
			api.MoveUp(80);

		}

		if (ismove(type1) && !ismove(type2) && ismove(type3) && ismove(type4) && ismove(type5) && ismove(type6) && ismove(type7) && ismove(type8)) {
			api.MoveUp(80);

		}
	}*/
}

//向道具移动
void moveToProp(std::shared_ptr<const THUAI5::Prop> prop, IAPI& api) {
	auto self = api.GetSelfInfo();
	int selfx = api.GridToCell(self->x);
	int selfy = api.GridToCell(self->y);
	int propx = api.GridToCell(prop->x);
	int propy = api.GridToCell(prop->y);
	if (selfx == propx && selfy == propy) {
		api.UseProp();
	}
	std::vector<node> L = dijkstra(selfx, selfy, propx, propy);
	if (L.size()) {
		auto nd = L[0];
		api.MovePlayer(sqrt((self->x - api.CellToGrid(nd.x)) * (self->x - api.CellToGrid(nd.x)) + (self->y - api.CellToGrid(nd.y)) * (self->y - api.CellToGrid(nd.y))) * 1000 / self->speed, getDirection(self->x, self->y, api.CellToGrid(nd.x), api.CellToGrid(nd.y)));
	}

}

//扔cpu
void throwcpu(IAPI& api) {
	auto self = api.GetSelfInfo();
	int x = api.GridToCell(self->x);
	int y = api.GridToCell(self->y);
	int nx = home1.x;
	int ny = home1.y;
	//std::cout << nx << "  " << ny << std::endl;
	double d = sqrt((x - nx) * (x - nx) + (y - ny) * (y - ny)) * 1000;
	if (d < disforcpu) {
		if (home1.graphtype == 1)
		{
			if (home1.team == 1) {
				if (self->y - self->x > (ny - nx + 4) * 1000 || self->x > api.CellToGrid(nx) || (x >= 40 && y >= 40)) {
					api.ThrowCPU(d / 3, getDirection(self->x, self->y, api.CellToGrid(nx), api.CellToGrid(ny)), self->cpuNum);
				}
			}
			else {
				if (self->x < api.CellToGrid(nx) || self->x - self->y>(nx - ny + 4) * 1000 || (x <= 9 && y <= 9)) {
					api.ThrowCPU(d / 3, getDirection(self->x, self->y, api.CellToGrid(nx), api.CellToGrid(ny)), self->cpuNum);
				}
			}
		}
		else {
			if (home1.team == 1)
				api.ThrowCPU(d / 3, getDirection(self->x, self->y, api.CellToGrid(nx), api.CellToGrid(ny)), self->cpuNum);
			else {
				if (y <= 16) {
					api.ThrowCPU(d / 3, getDirection(self->x, self->y, api.CellToGrid(nx), api.CellToGrid(ny)), self->cpuNum);
				}
			}
		}
	}
}
//向最近的敌人移动
void moveToenemy(IAPI& api) {
	auto self = api.GetSelfInfo();
	auto player = api.GetRobots();
	int x = api.GridToCell(self->x);
	int y = api.GridToCell(self->y);
	int minNumber = -1;
	double min = 1e6;
	for (int i = 0; i < player.size(); i++)
	{
		if (self->teamID != player[i]->teamID)
		{
			if (getDtoRobot(self, player[i]) < min && !player[i]->isResetting)
			{
				min = getDtoRobot(self, player[i]);
				minNumber = i;
			}
		}
	}
	if (minNumber == -1) return; //未获取敌人信息
	int nx = api.GridToCell(player[minNumber]->x);
	int ny = api.GridToCell(player[minNumber]->y);
	std::vector<node> L = dijkstra(x, y, nx, ny);
	if (L.size()) {
		auto nd = L[0];
		api.MovePlayer(sqrt((self->x - api.CellToGrid(nd.x)) * (self->x - api.CellToGrid(nd.x)) + (self->y - api.CellToGrid(nd.y)) * (self->y - api.CellToGrid(nd.y))) * 1000 / self->speed, getDirection(self->x, self->y, api.CellToGrid(nd.x), api.CellToGrid(nd.y)));
	}
}

//逃离敌人
void awayfromenemy(IAPI& api) {
	auto self = api.GetSelfInfo();
	auto player = api.GetRobots();
	int x = api.GridToCell(self->x);
	int y = api.GridToCell(self->y);
	int minNumber = -1;
	double min = 1e6;
	for (int i = 0; i < player.size(); i++)
	{
		if (self->teamID != player[i]->teamID)
		{
			if (getDtoRobot(self, player[i]) < min && !player[i]->isResetting)
			{
				min = getDtoRobot(self, player[i]);
				minNumber = i;
			}
		}
	}
	if (minNumber == -1) return; //未获取敌人信息
	api.MovePlayer(150, getDirection(player[minNumber]->x, player[minNumber]->y, self->x, self->y)); //此处角度反向
}

//开隐身
void BeInvisible(IAPI& api) {
	auto self = api.GetSelfInfo();
	auto player = api.GetRobots();
	int x = api.GridToCell(self->x);
	int y = api.GridToCell(self->y);
	int minNumber = -1;
	double min = 1e6;
	for (int i = 0; i < player.size(); i++)
	{
		if (self->teamID != player[i]->teamID)
		{
			if (getDtoRobot(self, player[i]) < min && !player[i]->isResetting)
			{
				min = getDtoRobot(self, player[i]);
				minNumber = i;
			}
		}
	}
	if (min < 10000) {
		api.UseCommonSkill();
	}
}

//回家
void moveTohome(IAPI& api) {
	auto self = api.GetSelfInfo();
	int x = api.GridToCell(self->x);
	int y = api.GridToCell(self->y);
	int nx = home1.x;
	int ny = home1.y;
	std::vector<node> L = dijkstra(x, y, nx, ny - 1);
	if (L.size()) {
		auto nd = L[0];
		api.MovePlayer(sqrt((self->x - api.CellToGrid(nd.x)) * (self->x - api.CellToGrid(nd.x)) + (self->y - api.CellToGrid(nd.y)) * (self->y - api.CellToGrid(nd.y))) * 1000 / self->speed, getDirection(self->x, self->y, api.CellToGrid(nd.x), api.CellToGrid(nd.y)));
	}
}

//进草丛
void moveToelc(IAPI& api) {
	auto self = api.GetSelfInfo();
	int x = api.GridToCell(self->x);
	int y = api.GridToCell(self->y);
	int nx = 1;
	int ny = 1;
	double min = 1e6;
	for (int i = 1; i < 49; i++)
	{
		for (int j = 1; j < 49; j++) {
			if (1 < isWall[i][j] && isWall[i][j] < 5) {
				double cur = sqrt((x - i) * (x - i) + (y - j) * (y - j));
				if (cur < min) {
					nx = i;
					ny = j;
					min = cur;
				}
			}
		}
	}
	std::vector<node> L = dijkstra(x, y, nx, ny);
	if (L.size()) {
		auto nd = L[0];
		api.MovePlayer(sqrt((self->x - api.CellToGrid(nd.x)) * (self->x - api.CellToGrid(nd.x)) + (self->y - api.CellToGrid(nd.y)) * (self->y - api.CellToGrid(nd.y))) * 1000 / self->speed, getDirection(self->x, self->y, api.CellToGrid(nd.x), api.CellToGrid(nd.y)));
	}
}


//躲闪
bool evade(std::shared_ptr<const THUAI5::Robot> self, IAPI& api) {
	bool flag = false;
	auto jammers = api.GetSignalJammers();
	double minDis = 9000;
	std::shared_ptr<const THUAI5::SignalJammer> Mjammer = nullptr;
	for (auto jammer : jammers) {
		if (jammer->parentTeamID == self->teamID) {
			continue;
		}
		double dis = getDtoJammer(self, jammer);
		if (dis < minDis) {
			minDis = dis;
			Mjammer = jammer;
			int e = getDirection(Mjammer->x, Mjammer->y, self->x, self->y);
			if (abs(Mjammer->facingDirection - e) < 0.5) break;
		}
	}
	if (Mjammer != nullptr) {
		int e = getDirection(Mjammer->x, Mjammer->y, self->x, self->y);
		if (abs(Mjammer->facingDirection - e) < 0.5) {
			if (self->cpuNum > 2)
				api.UseCPU(self->cpuNum);
			if (playerSoftware == THUAI5::SoftwareType::Invisible)
				api.UseCommonSkill();
			api.UseProp();
			int cellx = api.GridToCell(self->x);
			int celly = api.GridToCell(self->y);
			auto type1 = api.GetPlaceType(cellx - 1, celly - 1);
			auto type2 = api.GetPlaceType(cellx + 1, celly - 1);
			auto type3 = api.GetPlaceType(cellx - 1, celly + 1);
			auto type4 = api.GetPlaceType(cellx + 1, celly + 1);
			auto type5 = api.GetPlaceType(cellx, celly + 1);
			auto type6 = api.GetPlaceType(cellx, celly - 1);
			auto type7 = api.GetPlaceType(cellx + 1, celly);
			auto type8 = api.GetPlaceType(cellx - 1, celly);
			//周围无墙
			if (ismove(type1) && ismove(type2) && ismove(type3) && ismove(type4) && ismove(type5) && ismove(type6) && ismove(type7) && ismove(type8)) {
				//切向走位
				if (e > Mjammer->facingDirection)
					api.MovePlayer(1500000 / self->speed, e + PI / 2);
				else
					api.MovePlayer(1500000 / self->speed, e - PI / 2);
			}
			//周围有墙
			else {
				//切向走位
				if (ismove(type1) && ismove(type6) && ismove(type8)) {
					api.MovePlayer(1500000 / self->speed, PI / 4 + PI);
				}
				else if (ismove(type2) && ismove(type7) && ismove(type6)) {
					api.MovePlayer(1500000 / self->speed, PI / 4 + PI + PI / 2);
				}
				else if (ismove(type3) && ismove(type8) && ismove(type5)) {
					api.MovePlayer(1500000 / self->speed, PI / 4 + PI / 2);
				}
				else if (ismove(type4) && ismove(type7) && ismove(type5)) {
					api.MovePlayer(1500000 / self->speed, PI / 4);
				}
			}
			//法向走位
			api.MovePlayer(500000 / self->speed, e);

			flag = true;
		}
		else {
			api.MovePlayer(500000 / self->speed, e);
			flag = true;
		}
	}
	return flag;
}

//判断是否攻击
bool Attackornot(std::shared_ptr<const THUAI5::Robot> self, uint32_t selfPoisitionX, uint32_t selfPoisitionY, uint32_t aimPositionX, uint32_t aimPositionY)
{
	double ATTACK_R = self->attackRange;
	return ATTACK_R + 1000.0 > sqrt((aimPositionX - selfPoisitionX) * (aimPositionX - selfPoisitionX) + (aimPositionY - selfPoisitionY) * (aimPositionY - selfPoisitionY));
}

//攻击
void attackaround(IAPI& api, std::shared_ptr<const THUAI5::Robot> self)
{
	auto player = api.GetRobots();
	if (!player.empty() && self->signalJammerNum > 0)//
	{
		for (int i = 0; i < player.size(); i++)
		{

			if (self->teamID != player[i]->teamID && !player[i]->isResetting && search(self, self->x, self->y, player[i]->x, player[i]->y))//进入预定范围
			{
				enemy[i].prex = enemy[i].x;//刷新坐标
				enemy[i].prey = enemy[i].y;
				enemy[i].x = player[i]->x;
				enemy[i].y = player[i]->y;
				//非预判
				if (Attackornot(self, self->x, self->y, enemy[i].x, enemy[i].y)) {
					double e = getDirection(self->x, self->y, enemy[i].x, enemy[i].y);
					if (playerSoftware == THUAI5::SoftwareType::Invisible)
						api.UseCommonSkill();
					if (playerSoftware == THUAI5::SoftwareType::Amplification && self->signalJammerNum > 0)
						api.UseCommonSkill();
					api.Attack(e);
				}
				//预判
				if (Attackornot(self, self->x, self->y, enemy[i].getnextx(), enemy[i].getnexty()))//在攻击范围内
				{
					double e = getDirection(self->x, self->y, enemy[i].getnextx(), enemy[i].getnexty());//定角度
					api.Attack(e);
					api.Attack(e);

				}
			}
		}
	}
};

void AI::play(IAPI& api)
{
	std::ios::sync_with_stdio(false);
	//得到个人信息
	auto self = api.GetSelfInfo();
	auto props = api.GetProps();

	if (!isWalled) {
		isWalling(api);
		selfLife = self->life;
		start = clock();
	}

	//使用cpu
	if (self->cpuNum > 4 && !iscpu(api, self, props, home1.x, home1.y)) {
		api.UseCPU(self->cpuNum);
	}

	//开隐身
	if (playerSoftware == THUAI5::SoftwareType::Invisible) {
		BeInvisible(api);
	}

	//解封
	if (self->playerID == home1.homeno) {
		if (((double)(clock() - start)) / CLOCKS_PER_SEC > 9 * 60 + 10) {
			isWall[home1.x][home1.y] = 0;
		}
		else {
			if (self->cpuNum > 1) {
				moveTohome(api);
				throwcpu(api);
				return;
			}
			//扔cpu
			throwcpu(api);
		}
	}
	else
	{
		if (self->cpuNum > 1) {
			moveTohome(api);
			throwcpu(api);
			return;
		}
		//扔cpu
		throwcpu(api);
	}

	std::cout << self->isResetting;

	attackaround(api, self);


	//随机攻击
	if (self->signalJammerNum > 3) {
		api.Attack(getDirection(self->x, self->y, 25 * 1000, 25 * 1000));
	}

	//躲闪
	if (evade(self, api)) {
		return;
	}

	//调整
	selfControl(self, api);

	//获取场上的道具信息
	auto cpu = THUAI5::PropType::CPU;

	//最近的cpu
	/*auto tocpu = uploadcpu(self, props);
	if (tocpu != nullptr) {
		//捡cpu
		api.Pick(cpu);
		moveToProp(tocpu, api);
		return;
	}*/

	//最近的prop
	auto toprop = uploadprop(api, self, props);
	if (toprop != nullptr) {
		moveToProp(toprop, api);
		api.Pick(toprop->type);
		return;
	}

	//往最近的电磁屏蔽区跑
	if (self->cpuNum > 0) {
		moveTohome(api);
	}
	else {
		moveToelc(api);
	}
}



