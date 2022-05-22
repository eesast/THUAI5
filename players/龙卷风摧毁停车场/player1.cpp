
/*331B*/
//前置说明：
//1. 全部复制到自己的AI.cpp里即可
//2. 大部分代码有注释，可模仿；主要研究策略
//3. 文末有问题的梳理
//4. 大部分注释掉的语句是调试用的输出语句，可以不用管
//5. WPF使用
/*
	在Windows文件夹里，命令行文本可编辑，
	runserver.cmd - 启动服务器，想玩首先双击启动他
					右键编辑，文本里有几个参数，如teamcount，playercount，修改后面的数字即可
	runclient.cmd - 启动选手的exe
					右键编辑，文本里有下列参数（首先建议只保留一个start cmd，初始是8个，就是8个玩家）：
						-t - 所属队伍
						-p - 队员编号
						-P - 端口，7777，不用改
						-I - IP，127.0.0.1，不用改
	runGUIclient.cmd - 观战的exe，也可以用来手操
						右键编辑
							teamID - 大于2022为观战，修改成参赛（如0）就是手操
							playerID - 大于2022为观战，修改成参赛（如1）就是手操
							硬件 - 自己指定
							软件 - 自己指定
	WPF界面，wasd控制移动，鼠标双击发射子弹，其他功能参见//使用说明.pdf
*/

#include <random>
#include <queue>
#include <cmath>
#include<chrono>
#include<iostream>
#include <thread>
#include <algorithm>
#include "../include/AI.h"
#define PI 3.1415926

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;

enum class ROLE :unsigned char {
	NULLROLE = 0,
	Attacker = 1,		//硬件：EmissionAccessory；	软件：Amplification;
	Picker = 2,			//硬件：EmissionAccessory;	软件：Booster;
	Killer = 3,			//硬件：PowerBank;			软件：Invisible;
};
// 机器人分工，选手 !!必须!! 定义此变量来选择机器人角色
const ROLE role = ROLE::Attacker;
const THUAI5::SoftwareType soft[4] = { THUAI5::SoftwareType::NullSoftwareType, THUAI5::SoftwareType::Amplification,
										THUAI5::SoftwareType::Booster, THUAI5::SoftwareType::Invisible };
const THUAI5::HardwareType hard[4] = { THUAI5::HardwareType::NullHardwareType, THUAI5::HardwareType::EmissionAccessory,
										THUAI5::HardwareType::EmissionAccessory, THUAI5::HardwareType::PowerBank };

// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::SoftwareType playerSoftware = soft[(int)role];

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = hard[(int)role];

const double weight[3][5] = { 3, 5, 10, 5, 20,
							  10, 5, 20, 8, 1,
							  5, 3, 8, 5, 20 };
//加速器，电池，CPU，盾，破盾

unsigned char teamrole[4];

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * PI);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

struct coor { int x, y; };			//坐标结构体（x，y）
struct interval { double l, r; };	//区间结构体【l,r】

coor CpuPlace;

bool cmp(interval& a, interval& b) { return a.l < b.l; }

/*欧几里得距离（输入两个坐标，返回float欧几里得距离）*/
inline float E_dis(coor a, coor b) {
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

struct JAMMER_WITH_TIME {	//更详细的信号干扰弹结构体，储存在缓存cache中
	std::shared_ptr<const THUAI5::SignalJammer> j;
	long long msec;			//发射时刻
	coor cor_explode;		//爆炸坐标
	int life;				//寿命
};
/*缓存cache， 保留上一次操作得到的相关信息*/
//1.上一次操作时场上的信号干扰弹信息
//2.上一次操作前的坐标，移动的方向（解决卡墙问题）
//3.上一次操作的时刻，cur_msec
struct CACHE {
	std::vector<JAMMER_WITH_TIME> jam;
	std::vector<std::shared_ptr<const THUAI5::Robot>> robots;
	long long cur_msec;
	int pre_x, pre_y;
	double pre_e;
	bool is_throw;
} cache;

/*地图格子类型*/
unsigned char t[51][51];

/*场上每一个格子到当前位置四周格子的实际距离（上下左右移动，计算了绕墙的情况）*/
unsigned char d[4][51][51];
const int dx[4] = { -1,0,1,0 };
const int dy[4] = { 0,1,0,-1 };

/*用于处理躲避信号干扰弹*/
interval boundary[30];
std::vector<interval> ubound;
int size_bound;


/*一些关于干扰弹的常量，自己重新定义*/
const double speed_jam[4] = { 1,2.5,5,2 };
const int range_jam[4] = { 4000, 2500, 1500, 7000 };
const int attack_range[4] = { -1, 4500, 9000, 7000 };

/*实用的判断函数，判断该格子是否可移动，（目前只包含墙、出生点、越界的情况，可修改）*/
bool check_coor(coor c) {
	int x = c.x, y = c.y;
	if (x < 0 || x >= 50 || y < 0 || y >= 50) return false;
	if (t[x][y] == 1 || (t[x][y] - 5) * (t[x][y] - 12) <= 0) return false;
	if (x >= 0 && x < 50 && y >= 0 && y < 50) return true;
	else return false;
}

coor compute_cor_explode(std::shared_ptr<const THUAI5::SignalJammer> j) {
	double dir = j->facingDirection;
	int s_x = j->x, s_y = j->y;
	int x = s_x, y = s_y;
	int rang = attack_range[(unsigned char)j->type - 1];
	while (check_coor({ x / 1000, y / 1000 }) && E_dis({ s_x,s_y }, { x, y }) < rang) {
		x += 500 * cos(dir);
		y += 500 * sin(dir);
	}
	return coor{ x, y };
}
coor compute_cor_explode(coor s, double dir, THUAI5::SignalJammerType t) {
	int s_x = s.x, s_y = s.y;
	int x = s_x, y = s_y;
	int rang = attack_range[(unsigned char)t - 1];
	while (check_coor({ x / 1000, y / 1000 }) && E_dis({ s_x, s_y }, { x,y }) < rang) {
		x += 500 * cos(dir);
		y += 500 * sin(dir);
	}
	return coor{ x,y };
}

void update_bound(interval b) {
	int size = ubound.size();
	char* check = new char[size + 1];
	for (int j = 0; j < size; j++) {
		check[j] = 0;
		interval& a = ubound[j];
		if (a.r < b.l && a.r>a.l && (b.r > b.l || b.r < b.l && b.r < a.l)) check[j] = 1;
		else if (a.l > b.r && a.r > a.l && b.l < b.r) check[j] = 1;
		else if (a.r<b.l && a.r>a.l && b.r<b.l && b.r>a.l) a.r = std::min(a.r, b.r);
		else if (a.r < a.l && (b.r > b.l || b.r < a.r)) a.r = b.r, a.l = b.l;
		else if (a.r < a.l && b.r<a.l && b.r>a.r) a.l = b.l;
		else if (a.r < a.l && b.r<b.l && b.r>a.l) { double tmp = a.r; a.r = b.r; ubound.push_back({ b.l,tmp }); }
		else if (a.r > b.l && b.r > b.l) a.l = std::max(b.l, a.l), a.r = std::min(a.r, b.r);
		else if (a.r > b.l && b.r < b.l && b.r < a.l) a.l = b.l;
		else if (a.r > b.l && b.r<b.l && b.r>a.l) { double tmp = a.r; a.r = b.r; ubound.push_back({ b.l,tmp }); }
		else if (a.r > b.l && b.r < b.l) a.l = b.l;
	}
	//std::cout << "inside compute settled" << std::endl;
	auto it = ubound.begin();
	for (int j = 0; j < size; j++)
		if (check[j]) it = ubound.erase(it);
		else it++;
	//std::cout << "inside delete settled" << std::endl;
}

/*估价部分，决定下一步移动方向（上下左右*/
void update(float v[4], double weight, int xx, int yy) {
	for (int j = 0; j < 4; j++) {
		if (d[j][xx][yy] == 0) v[j] += 100 * weight;
		else if (d[j][xx][yy] == 1) v[j] += 5 * weight;
		else if (d[j][xx][yy] != -1) v[j] += exp(((double)-d[j][xx][yy]) / 10) * weight;
	}
}

void bfs(unsigned char d[51][51], int x, int y, IAPI& api) {
	bool vis[51][51] = { 0 };
	std::queue<coor> q;
	q.push({ x,y });
	d[x][y] = 0, vis[x][y] = 1;
	while (!q.empty()) {
		coor cur = q.front(); q.pop();
		int xx = cur.x, yy = cur.y, dd = d[cur.x][cur.y];
		if (check_coor({ xx + 1, yy }) && !vis[xx + 1][yy]) {
			q.push({ xx + 1,yy });
			vis[xx + 1][yy] = 1;
			d[xx + 1][yy] = dd + 1;
		}
		if (check_coor({ xx - 1, yy }) && !vis[xx - 1][yy]) {
			q.push({ xx - 1,yy });
			vis[xx - 1][yy] = 1;
			d[xx - 1][yy] = dd + 1;
		}
		if (check_coor({ xx, yy + 1 }) && !vis[xx][yy + 1]) {
			q.push({ xx, yy + 1 });
			vis[xx][yy + 1] = 1;
			d[xx][yy + 1] = dd + 1;
		}
		if (check_coor({ xx, yy - 1 }) && !vis[xx][yy - 1]) {
			q.push({ xx,yy - 1 });
			vis[xx][yy - 1] = 1;
			d[xx][yy - 1] = dd + 1;
		}
	}
}

const int ddx[8] = { -1,-1,-1,0,0,1,1,1 };
const int ddy[8] = { -1,0,1,-1,1,-1,0,1 };

/*移动函数，这里写了两个*/
//1.move_to(self, api, target_x, target_y), 这里暂时只支持1000（一格）的移动
//2.move_tow(self, api, target_angle, target_distance), 可按极坐标方式移动
void move_to(std::shared_ptr<const THUAI5::Robot> self, IAPI& api, int x, int y) {
	int xx = x * 1000 + 500, yy = y * 1000 + 500;
	for (int i = 0; i < 8; i++) {
		if (x + ddx[i] < 0 || x + ddx[i] >= 50 || y + ddy[i] < 0 || y + ddy[i] >= 50) continue;
		if (!check_coor({ x + ddx[i],y + ddy[i] })) xx -= ddx[i] * 5, yy -= ddy[i] * 5;
	}
	int cur_x = self->x, cur_y = self->y;
	double speed = self->speed / 1000;
	double dx = xx - cur_x, dy = yy - cur_y;
	if (dx == 0) {
		if (dy > 0) if (!api.MoveRight((int)dy / speed)) std::cout << "moveright failed!" << std::endl;
		else if (dy < 0) if (!api.MoveLeft(-(int)dy / speed)) std::cout << "moveleft failed!" << std::endl;
		//std::this_thread::sleep_for(std::chrono::milliseconds((int)(abs((int)dy) / speed)/2));
		return;
	}
	else if (dy == 0) {
		if (dx > 0) if (!api.MoveDown((int)dx / speed)) std::cout << "movedown failed!" << std::endl;
		else if (dx < 0) if (!api.MoveUp(-(int)dx / speed)) std::cout << "moveup failed" << std::endl;
		//std::this_thread::sleep_for(std::chrono::milliseconds((int)(abs((int)dx) / speed)/2));
		return;
	}
	double e = atan2(dy, dx);
	if (e < 0) e += 2 * PI;
	if (!api.MovePlayer(sqrt(dy * dy + dx * dx) / speed, e)) std::cout << "move failed!";
	//std::this_thread::sleep_for(std::chrono::milliseconds((int)(1000 / speed)/2));
	return;
}
void move_tow(std::shared_ptr<const THUAI5::Robot> self, IAPI& api, double e, int d = 1000) {
	int xx = self->x / 1000, yy = self->y / 1000;
	cache.pre_e = e;
	if (cache.pre_e > PI) e -= 2 * PI;
	if (fabs(e - PI) < 1e-3) move_to(self, api, xx - 1, yy);
	else if (fabs(e - PI / 2) < 1e-3) move_to(self, api, xx, yy + 1);
	else if (fabs(e) < 1e-3) move_to(self, api, xx + 1, yy);
	else if (fabs(e + PI / 2) < 1e-3) move_to(self, api, xx, yy - 1);
	else {
		int dx = cos(e) > 0 ? 1 : -1;
		int dy = sin(e) > 0 ? 1 : -1;
		if (!check_coor({ xx + dx,yy }) && !check_coor({ xx, yy + dy })) {
			if (fabs(cos(e)) > fabs(sin(e))) dy = -dy, dx = 0;
			else dx = -dx, dy = 0;
		}
		else if (!check_coor({ xx + dx, yy })) dx = 0;
		else if (!check_coor({ xx, yy + dy })) dy = 0;
		if (!dx) e = dy * PI / 2;
		else if (!dy) e = (dx - 1) * PI / 2;
		if (e > PI) e -= 2 * PI;
		//std::cout << "special occasion: move toward e = " << e << "\n";
		//std::cout << "self(x):" << self->x << " self(y)" << self->y << "\n";
		api.MovePlayer(d / (self->speed / 1000), e);
		//std::this_thread::sleep_for(std::chrono::milliseconds((int)(d / (self->speed / 1000)) / 2));
		cache.pre_e = e;
	}
}



/*由server调用的部分*/
void AI::play(IAPI& api)
{
	/*获取self*/
	auto self = api.GetSelfInfo();
	if (self->isResetting) return;

	if (role == ROLE::Picker && api.GetFrameCount() >= 10800) t[CpuPlace.x / 1000][CpuPlace.y / 1000] = 0;

	//std::cout << "时间" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << std::endl;;
	/*第一帧的相关处理，暂时只包括处理地图格子类型，可修改*/
	if (api.GetFrameCount() == 1) {
		std::cout << "my role::" << (int)role << std::endl;

		if (role == ROLE::Picker) CpuPlace = { self->x, self->y };

		/*通信，表明身份*/
		teamrole[(char)role - 1] = self->playerID;
		std::string content = "";
		content += (char)role + '0';
		content += self->playerID + '0';
		for (int i = 0; i < 3; i++) {
			if (i == self->playerID) continue;
			else api.Send(i, content);
		}

		cache.is_throw = 1;
		cache.jam.clear();
		cache.robots.clear();
		for (int i = 0; i < 50; i++) for (int j = 0; j < 50; j++) t[i][j] = (unsigned char)api.GetPlaceType(i, j);
	}

	if (role == ROLE::Picker && api.GetFrameCount() % 50 == 0) {
		std::string content = "";
		content += '0';
		int x = CpuPlace.x / 1000, y = CpuPlace.y / 1000;
		content += x / 10 + '0';	content += x % 10 + '0';
		content += y / 10 + '0';	content += y % 10 + '0';
		for (int i = 0; i < 4; i++) {
			if (i == self->playerID) continue;
			else api.Send(i, content);
		}
	}

	while (api.MessageAvailable()) {
		auto content = api.TryGetMessage();
		if (!content.has_value()) break;
		else {
			if ((*content)[0] != '0') teamrole[(*content)[0] - 1 - '0'] = (*content)[1] - '0';
			else {
				CpuPlace.x = (((*content)[1] - '0') * 10 + (*content)[2]-'0') * 1000 + 500;
				CpuPlace.y = (((*content)[3] - '0') * 10 + (*content)[4]-'0') * 1000 + 500;
			}

		}
	}
	//std::cout << "CpuPlace: x=" << CpuPlace.x << " ,y=" << CpuPlace.y << "\n";


	/*处理卡墙问题（stuck）*/
	if (api.GetFrameCount() % 3 == 0) {
		if (self->x == cache.pre_x && self->y == cache.pre_y) {
			if ((self->y - 500) % 1000 == 0) {
				if (cos(cache.pre_e) > 0 && check_coor({ (int)self->x / 1000 + 1, (int)self->y / 1000 }))
					api.MoveDown(1000000 / self->speed);
				else api.MoveUp(1000000 / self->speed);
			}
			else if ((self->x - 500) % 1000 == 0) {
				if (sin(cache.pre_e) > 0 && check_coor({ (int)self->x / 1000, (int)self->y / 1000 + 1 }))
					api.MoveRight(1000000 / self->speed);
				else api.MoveLeft(1000000 / self->speed);
			}
			else {
				coor s = { self->x, self->y };
				coor t = { (s.x / 1000) * 1000 + 500, (s.y / 1000) * 1000 + 500 };
				int dx = t.x - s.x, dy = t.y - s.y;
				int sp = self->speed / 1000;
				dx = (dx > 0 ? 1 : -1) * ((abs(dx) / sp) + ((abs(dx) % sp) ? 1 : 0));
				dy = (dy > 0 ? 1 : -1) * ((abs(dy) / sp) + ((abs(dy) % sp) ? 1 : 0));
				//std::cout << "第" << api.GetFrameCount() << "帧:" << std::endl;
				//std::cout << "self(x):" << self->x << " self(y):" << self->y << std::endl;
				//std::cout << "get stuck and move dx=" << dx << ";dy=" << dy << std::endl;
				if (dx == dy) api.MovePlayer(sqrt(dx * dx + dy * dy), atan2(double(dy), double(dx)));
				else if (abs(dx) < abs(dy) && api.GetFrameCount() % 2 || abs(dx) > abs(dy) && api.GetFrameCount() % 2 == 0) api.MovePlayer(abs(dy), atan2((double)dy, 0));
				else api.MovePlayer(abs(dx), atan2(0, (double)dx));
				std::this_thread::sleep_for(std::chrono::milliseconds((int)sqrt(dx * dx + dy * dy)));
			}
			return;
		}

		cache.pre_x = self->x;									//更新cache
		cache.pre_y = self->y;
	}
	/*------------------ - */
	//if (api.GetSignalJammers().size() == 0) return;

	int cur_x = self->x / 1000, cur_y = self->y / 1000;			//当前格子

	/*处理距离，用以估价*/
	float v[4];
	v[0] = v[1] = v[2] = v[3] = 0.0;

	for (int i = 0; i < 51; i++) for (int j = 0; j < 51; j++) for (int k = 0; k < 4; k++) d[k][i][j] = -1;

	bfs(d[0], cur_x - 1, cur_y, api);
	bfs(d[1], cur_x, cur_y + 1, api);
	bfs(d[2], cur_x + 1, cur_y, api);
	bfs(d[3], cur_x, cur_y - 1, api);
	/*----------------*/

	/*正式估价*/
	//1. robots部分（待完善）
	//可包含思路：适当远离己方和敌方机器人
	auto robots = api.GetRobots();
	if (api.GetFrameCount() == 1) cache.robots = robots;
	//std::cout << "there are " << robots.size() << " robots." << std::endl;
	size_t size_robots = robots.size();

	/*处理一个地方机器人距自己的最近距离*/
	double r_min = 50 * 1000;

	auto cur_msec_rob = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	for (int i = 0; i < robots.size(); i++) {
		std::shared_ptr<const THUAI5::Robot> r = robots[i], pre_r = NULL;
		if (r->isResetting) continue;

		double d = E_dis({ (int)self->x, (int)self->y }, { (int)r->x, (int)r->y });

		if (r->teamID == self->teamID) {
			if (d < 2000) update(v, -1, ((int)r->x) / 1000, ((int)r->y) / 1000);
			continue;
		}

		r_min = std::min(r_min, d);

		if (role == ROLE::Killer && !self->timeUntilCommonSkillAvailable)
			if (d < 8 * 1000) { api.UseCommonSkill(); update(v, r->cpuNum * 5, r->x / 1000, r->y / 1000); }


		for (int j = 0; j < cache.robots.size(); j++)
			if (cache.robots[j]->teamID == r->teamID && cache.robots[j]->playerID == r->playerID) pre_r = cache.robots[j];
		if (pre_r == NULL) {
			continue;
		}
		double u = E_dis({ (int)r->x, (int)r->y }, { (int)pre_r->x, (int)pre_r->y }) / (cur_msec_rob - cache.cur_msec);
		double e_u = atan2((double)r->y - pre_r->y, (double)r->x - pre_r->x);

		//std::cout << "d is " << E_dis({ (int)r->x, (int)r->y }, { (int)pre_r->x, (int)pre_r->y }) << std::endl;
		//std::cout << "dt is" << (cur_msec_rob - cache.cur_msec) << std::endl;
		//std::cout << "u is " << u << std::endl;


		if (role == ROLE::Attacker && self->signalJammerNum && !self->timeUntilCommonSkillAvailable) {
			if (d < range_jam[(char)THUAI5::SignalJammerType::StrongJammer - 1] - 2000) {
				api.UseCommonSkill();
				double dt = (double)self->attackRange / (speed_jam[(char)self->signalJammerType - 1]);
				double xx = r->x + u * dt * cos(e_u) / 3;
				double yy = r->y + u * dt * sin(e_u) / 3;
				double e = atan2((double)yy - self->y, (double)xx - self->x);
				api.Attack(e - PI / 10);
				api.Attack(e + PI / 10);
			}
		}

		/*激光干扰弹特别攻击方式*/
		if (self->signalJammerType == THUAI5::SignalJammerType::LineJammer) {
			double e = atan2((double)r->y - self->y, (double)r->x - self->x);
			coor explode_cor = compute_cor_explode(coor{ (int)self->x,(int)self->y }, e, THUAI5::SignalJammerType::LineJammer);
			if ((explode_cor.x - r->x) * (explode_cor.x - self->x) > 0 || (explode_cor.y - r->y) * (explode_cor.y - self->y) > 0) continue;
			if (d < range_jam[(char)THUAI5::SignalJammerType::LineJammer - 1]) {
				if (self->signalJammerNum == 4) api.Attack(e), api.Attack(e - PI / 36), api.Attack(e + PI / 36);
				else if (self->signalJammerNum >= 2) api.Attack(e + PI / 36), api.Attack(e - PI / 36);
				else if (self->signalJammerNum >= 1) api.Attack(e);
			}
			continue;
		}

		if (d > 10 * 1000) continue;
		else if (d > self->attackRange && d <= self->attackRange + 1000) {
			double dt = self->attackRange / (speed_jam[(char)self->signalJammerType - 1] / 1000);
			double xx = r->x;
			double yy = r->y;
			double e = atan2(yy - self->y, xx - self->x);
			//std::cout << "case 2:" << std::endl;
			//std::cout << "( " << xx << "," << yy << " )" << std::endl;
			//std::cout << "direction:" << e << std::endl;
			if (e < 0)e += 2 * PI;
			if (self->signalJammerNum > 2) api.Attack(e);
		}
		else if (d <= self->attackRange) {
			double e = atan2((double)r->y - self->y, (double)r->x - self->x);
			double de = PI / 12;
			if (self->signalJammerNum == 1) api.Attack(e);
			else if (self->signalJammerNum >= 2) { api.Attack(e + de); api.Attack(e - de); }
		}
	}
	/*attacking part for attacker*/
	/*if ((char)role == (char)ROLE::Attacker && self->signalJammerNum) {
		std::vector<double> e;
		e.clear();
		for (int i = 0; i < robots.size(); i++) if (robots[i]->teamID != self->teamID) {
			if (E_dis({ (int)self->x,(int)self->y }, { (int)robots[i]->x,(int)robots[i]->y }) > 10 * 1000) continue;
			double ee = atan2((double)robots[i]->y - self->y, (double)robots[i]->x - self->x);
			e.push_back(ee);
		}
		sort(e.begin(), e.end());
		int size_e = e.size();
		if (size_e == 3) {
			if (e[2] - e[0] <= PI / 3) if (api.UseCommonSkill()) api.Attack((e[2] + e[0]) / 2);
			else if (e[0] + 2 * PI - e[1] <= PI / 3) if (api.UseCommonSkill()) api.Attack((e[0] + e[1]) / 2 + PI);
			else if (e[1] - (e[2] - 2 * PI) <= PI / 3) if (api.UseCommonSkill()) api.Attack((e[1] + e[2]) / 2 - PI);
		}
		else if (size_e == 4) {
			if (e[2] - e[0] <= PI / 3) if (api.UseCommonSkill()) api.Attack((e[2] + e[0]) / 2);
			else if (e[3] - e[1] <= PI / 3) if (api.UseCommonSkill()) api.Attack((e[3] + e[1]) / 2);
			else if (e[1] + 2 * PI - e[2] <= PI / 3) if (api.UseCommonSkill()) api.Attack((e[1] + e[2]) / 2 + PI);
		}
	}*/

	if (role == ROLE::Picker) cache.is_throw = r_min > 10 * 1000 ? 1 : 0;

	cache.robots = robots;
	//printf("there are %d robots. \r\n", (int)size_robots);

	//2.props部分（待完善，目前只包含对CPU估价
	auto Props = api.GetProps();
	size_t size_props = Props.size();
	//printf("there are %d props \r\n", (int)size_props);
	for (int i = 0; i < size_props; i++) {
		int xx = Props[i]->x / 1000, yy = Props[i]->y / 1000;
		int num_prop = 1;
		/*picker使用技能*/
		if (Props[i]->type == THUAI5::PropType::CPU && (char)role == (char)ROLE::Picker) {
			if (E_dis({ (int)self->x, (int)self->y }, { (int)Props[i]->x,(int)Props[i]->y }) <= 20 * 1000)
				api.UseCommonSkill();
		}

		if (xx == cur_x && yy == cur_y) {
			if (Props[i]->type == THUAI5::PropType::CPU) {
				if (api.GetFrameCount() < 10800) {
					if (xx == CpuPlace.x && yy == CpuPlace.y) break;
				}
				num_prop = Props[i]->size;
				api.Pick(THUAI5::PropType::CPU);
				continue;
			}
			else {
				if (self->prop != THUAI5::PropType::NullPropType) api.UseProp();
				api.Pick(Props[i]->type);
				continue;
			}
		}

		if (Props[i]->isMoving) continue;
		update(v, num_prop * weight[(char)role - 1][(char)Props[i]->type - 1], xx, yy);
	}
	/*估价结束*/

	/*子弹躲避*/
	auto jam = api.GetSignalJammers();

	/*----------------------------------------------------------------------*/
	//if (jam.size() == 0) return;//用于测试躲避子弹的性能
	/*----------------------------------------------------------------------*/

	auto pre_msec = cache.cur_msec;				//获取上次操作的时间
	cache.cur_msec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	//更新cache里的操作时间

	int pre_jam_size = cache.jam.size();				//上一次操作时的干扰弹数量
	char* check = new char[pre_jam_size + 1];			//判断上次的干扰弹在这一帧里是否还存在
	for (int i = 0; i < pre_jam_size + 1; i++) check[i] = 0;//check初始化

	size_t size_jam = jam.size();						//当前干扰弹数量
	for (int i = 0; i < size_jam; i++) {

		if (jam[i]->parentTeamID == self->teamID) continue;//己方干扰弹，暂不做处理
		if (jam[i]->type == THUAI5::SignalJammerType::LineJammer) continue;//激光干扰弹，暂不作处理

		int xx = jam[i]->x, yy = jam[i]->y;				//xx, yy为干扰弹坐标
		double direct = jam[i]->facingDirection;		//干扰弹方向
		THUAI5::SignalJammerType type = jam[i]->type;	//干扰弹类型

		/*判断是新发射的干扰弹，还是原有的干扰弹*/
		bool is_old = 0;
		//std::cout << "there are " << cache.jam.size() << " pre_jams" << std::endl;
		for (int j = 0; j < cache.jam.size(); j++) {
			if (direct != cache.jam[j].j->facingDirection) continue;
			if (type != cache.jam[j].j->type) continue;
			int xxx = cache.jam[j].j->x, yyy = cache.jam[j].j->y;
			xxx += cos(direct) * speed_jam[(int)type - 1] * (int)(cache.cur_msec - cache.jam[j].msec);
			yyy += sin(direct) * speed_jam[(int)type - 1] * (int)(cache.cur_msec - cache.jam[j].msec);
			if (abs(xxx - xx) <= 50 && abs(yyy - yy) <= 50) {
				/*是旧的干扰弹，更新寿命*/
				check[j] = 1;
				is_old = 1;
				cache.jam[j].life -= cache.cur_msec - pre_msec;
				break;
			}
		}
		if (!is_old) {
			/*不是旧的干扰弹，计算爆炸点位置，计算寿命，加入cache*/
			coor t_cor = compute_cor_explode(jam[i]);
			int t_life = E_dis(t_cor, { xx, yy }) / speed_jam[(int)type - 1];
			cache.jam.emplace_back(JAMMER_WITH_TIME{ jam[i], cache.cur_msec, t_cor, t_life });
		}
	}

	{
		/*删除cache里本次消失（已爆炸）的干扰弹*/
		auto it = cache.jam.begin();
		for (int i = 0; i < pre_jam_size; i++) {
			if (!check[i]) it = cache.jam.erase(it);
			else it++;
		}
	}
	delete[]check; //释放内存

	//std::cout << "there are " << cache.jam.size() << " signal jammers" << std::endl;
	/*for (int i = 0; i < cache.jam.size(); i++) {
		std::cout << "explode cor ( " << cache.jam[i].cor_explode.x << ","  << cache.jam[i].cor_explode.y << std::endl;
	}*/

	/*处理干扰弹对决策的影响，即求出安全的行进方向，放入决策边界ubound里*/
	//1.求出所有干扰弹限制的行进角度
	for (int i = 0; i < cache.jam.size(); i++) {
		std::shared_ptr<const THUAI5::SignalJammer> ja = cache.jam[i].j;
		double dir = ja->facingDirection;
		char ty = (char)(ja->type) - 1;
		int xx = cache.jam[i].cor_explode.x, yy = cache.jam[i].cor_explode.y;
		int life = cache.jam[i].life;
		double r = E_dis({ xx, yy }, { (int)self->x, (int)self->y });
		double r1 = (double)life * self->speed / 1000 * 2 / 3;
		if (r > r1 + range_jam[ty] || r1 > r + range_jam[ty]) {
			/*安全*/
			//std::cout << "=====================safe======================" << std::endl;
			//std::cout << "r:" << r << std::endl;
			//std::cout << "r1:" << r1 << std::endl;
			//std::cout << "range:" << range_jam[ty] << std::endl;
		}
		else if (r + r1 < range_jam[ty]) {
			//std::cout << "====================die=======================" << std::endl;
			if (self->cpuNum > 3) api.UseCPU(self->cpuNum);
		}//无法逃生，die函数还未写
		else {
			/*可逃生*/
			//std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
			//std::cout << "compute angle range" << std::endl;
			//std::cout << "life:" << life << std::endl;
			//std::cout << "explode cor:" << xx << " " << yy << std::endl;
			//std::cout << "self cor:" << self->x << self->y << std::endl;
			//std::cout << "r:" << r << std::endl;
			//std::cout << "r1:" << r1 << std::endl;
			//std::cout << "range:" << range_jam[ty] << std::endl;

			/*这里平面几何*/
			double e = atan2((double)yy - self->y, (double)xx - self->x);
			if (e < 0) e += 2 * PI;
			double de = acos((r1 * r1 + r * r - range_jam[ty] * range_jam[ty]) / (2 * r1 * r));
			double l = e + de;
			if (l > 2 * PI) l -= 2 * PI;
			double r = e - de;
			if (r < 0) r += 2 * PI;
			boundary[++size_bound] = { l,r };
			dir += PI;
			if (dir >= 2 * PI) dir -= 2 * PI;
			boundary[++size_bound] = { dir + PI / 10, dir - PI / 10 };

			//std::cout << "e" << e << std::endl;
			//std::cout << "de" << de << std::endl;
			//std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
		}
	}

	/*左边界升序排列*/
	std::sort(boundary + 1, boundary + size_bound + 1, cmp);


	/*if (size_bound != 0) {
		//std::cout << "========bound=========" << std::endl;
		for (int i = 1; i <= size_bound; i++) //std::cout << "[" << boundary[i].l << "," << boundary[i].r << "]\n";
		std::cout << "====================" << std::endl;
	}*/

	/*ubound更新*/
	ubound.push_back({ 0, 2 * PI });//std::cout << "compute settled" << std::endl;
	for (int i = 1; i <= size_bound; i++) update_bound(boundary[i]);
	size_bound = 0;
	//std::cout << "update settled" << std::endl;

	if (self->cpuNum >= 15) api.UseCPU(self->cpuNum);
	if (self->cpuNum && E_dis(CpuPlace, { self->x, self->y }) < 15 * 1000) {
		if (check_coor({ cur_x - 1, cur_y }) && check_coor({ cur_x + 1, cur_y }) && check_coor({ cur_x, cur_y - 1 }) && check_coor({ cur_x, cur_y + 1 })) {
			coor t = { self->x, self->y };
			double e = atan2((double)CpuPlace.y - t.y, (double)CpuPlace.x - t.x);
			bool check = 0;
			while (check_coor({ t.x / 1000, t.y / 1000 })) {
				t.x += 500 * cos(e);
				t.y += 500 * sin(e);
				if (t.x / 1000 == CpuPlace.x / 1000 && t.y / 1000 == CpuPlace.y / 1000) { check = 1; break; }
			}
			if (check) api.ThrowCPU(E_dis(CpuPlace, { self->x, self->y }) / 3, e, self->cpuNum);
		}
	}
	if (self->cpuNum)update(v, 15 * self->cpuNum, CpuPlace.x / 1000, CpuPlace.y / 1000 - 1);



	/*调试用*/
	//update(v, 50, 41, 1);




	/*最终决策*/
	if (fabs(v[0])<=1e-7 && fabs(v[1])<1e-7 && fabs(v[2]) <= 1e-7 && fabs(v[3]) <= 1e-7) update(v, 10, 26, 26);
	//std::cout << v[0] << " " << v[1] << " " << v[2] << " " << v[3];

	int dec = -1;
	for (int i = 0; i < 4; i++)
		if (check_coor({ cur_x + dx[i],cur_y + dy[i] })) {
			if (dec == -1)  dec = i;
			else if (v[i] >= v[dec]) dec = i;
			else if (v[i] == v[dec])
				if (abs(cur_x + dx[i] - 25) + abs(cur_y - 25 + dy[i]) < abs(cur_x + dx[dec] - 25) + abs(cur_y + dy[i] - 25)) dec = i;
		}	//选出估价最高的方向

	double P;
	switch (dec) {
	case 0: P = PI; break;
	case 1: P = PI / 2; break;
	case 2: P = 0; break;
	case 3: P = 3 * PI / 2; break;
	}

	//std::cout << "the decision is" << dec << std::endl;
	//std::cout << "the ubound is" << ubound.size() << std::endl;
	//for (int i = 0; i < ubound.size(); i++) std::cout << "[ " << ubound[i].l << "," << ubound[i].r << std::endl;

	/*判断该方向能否保证安全*/
	bool ch = 0;
	double e = 100;
	for (int j = 0; j < ubound.size(); j++) {
		if (P >= ubound[j].l && P <= ubound[j].r) {
			/*安全*/
			move_tow(self, api, P);
			ch = 1;
			break;
		}
		else {
			if (fabs(P - e) > fabs(P - ubound[j].l + PI / 10)) e = ubound[j].l + PI / 10;
			if (fabs(P - e) > fabs(P - ubound[j].r - PI / 10)) e = ubound[j].r - PI / 10;
		}
	}
	//std::cout << "ch is" << ch << std::endl;
	/*不安全*/
	if (!ch) move_tow(self, api, e);
	ubound.clear();

}

/*
还未处理的部分：
	1. 对除CPU外其他道具的估价 check
		估价方式：可用d[i][x][y]获得(x,y)到自身位置上下左右四格的距离，进而可根据距离计算出道具对上下左右四格的贡献
				（对CPU采用的是v += e^(-weight*d)的估价函数，简单的说，d越大，贡献价值越小；可对不同的道具采用不同的weight甚至不同的函数）
	2. die函数
		即确定必死的情况下，如何使自己的剩余价值最大化

	3. 攻击函数 check
			自己的几个思路：1）朝道具发射干扰弹；	2）朝敌方直接发射干扰弹； 3）朝墙发射干扰弹

	4. 队友通信
		1）要想最大发挥CPU的作用，最好将所有CPU集中在1人身上，（道具是可投掷的，ThrowCPU(时间,方向,个数), ThrowProp(时间，方向)）投掷精准需要通信
		2) 屏蔽区（草丛）风险很大，可通信
		3) （较困难）角色转换

	5. 分工（确定策略，需讨论）

*/

