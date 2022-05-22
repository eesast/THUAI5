#include <random>
#include <string>
#include <time.h>
#include "../include/AI.h"
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
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}
/*

class RTN {
public:
	double angle;
	double weight;
	RTN(const double& A = 0, const double& W = 0) :angle(A), weight(W) {	}
};*/

class RTN {
public:
	double angle;
	int time;
	double weight;
	double possibility;
	RTN() {
		angle = 0; time = 0; weight = 0; possibility = 0;
	}
	RTN(double ngl, int t, double wght, double p) {
		angle = ngl; time = t; weight = wght; possibility = p;
	}
	RTN(double ngl, int t, double wght) {
		angle = ngl; time = t; weight = wght; possibility = -1;
	}
	void show() {
		std::cout << "angle: " << angle << " time: " << time << " weight: " << weight << " possibility: " << possibility << std::endl;
	}
};

class JMRinfo {
public:
	int range, damage, speed, explosion;
	//range为射程，damage为伤害值，speed为飞行速度，explosion为爆炸半径。其中，激光干扰弹爆炸半径取矩形外接圆半径。
	JMRinfo()
	{
		range = 0; damage = 0; speed = 0; explosion = 0;
	}
	JMRinfo(THUAI5::SignalJammerType jammertype)
	{
		if (jammertype == THUAI5::SignalJammerType::LineJammer)
		{
			range = 900; damage = 2000; speed = 1000; explosion = 2062;
		}
		else if (jammertype == THUAI5::SignalJammerType::FastJammer)
		{
			range = 9000; damage = 1500; speed = 5000; explosion = 1500;
		}
		else if (jammertype == THUAI5::SignalJammerType::CommonJammer)
		{
			range = 4500; damage = 2500; speed = 2500; explosion = 2500;
		}
		else if (jammertype == THUAI5::SignalJammerType::StrongJammer)
		{
			range = 2000; damage = 7000; speed = 2000; explosion = 7000;
		}
	}
	friend JMRinfo GetJmrInfo(THUAI5::SignalJammerType jammertype);
};

JMRinfo GetJmrInfo(THUAI5::SignalJammerType jammertype) {
	JMRinfo temp(jammertype);
	return temp;
}

double distance(int x, int y, double angle, int xx, int yy) {
	return(abs(xx * tan(angle) - yy - tan(x) - y) / sqrt(tan(angle) * tan(angle)) + 1);
}//计算点到直线距离的函数
double distance(int x, int y, int xx, int yy) {
	return(sqrt((xx - x) * (xx - x) + (yy - y) * (yy - y)));
}//计算两点之间距离函数
double angle(int x, int y, int xx, int yy) {
	if (yy >= y) return(acos((xx - x) / distance(x, y, xx, yy)));
	else return(4 * asin(1) - acos((xx - x) / distance(x, y, xx, yy)));
}//计算两点之间角度的函数
double xtoy(int x, int y, double angle, int xx) {
	return(y + tan(angle) * (xx - x));
}//已知直线与x坐标，返回相应的y坐标

RTN EfrJ(std::shared_ptr<const THUAI5::Robot>& SelfInfo, std::vector<std::shared_ptr<const THUAI5::SignalJammer>>& sig);
RTN Attack(IAPI& api);

class Explosion {
public:
	int x;
	int y;
	int range;
	int frame;
	unsigned long long guid;
	Explosion(const int& X = 0, const int& Y = 0, const int& R = 0, const int& F = 0, const unsigned long long& G = 0) :x(X), y(Y), range(R), frame(F), guid(G) {}
};

// 未考虑伤害

class node {
private:
	Explosion e;
	node* p;
public:
	node(node* P, const int& X = 0, const int& Y = 0, const int& R = 0, const int& F = 0, const unsigned long long& G = 0) :e(X, Y, R, F, G), p(P) {}
	Explosion& get() {
		return e;
	}
	int getFrame() {
		return e.frame;
	}
	unsigned long long getGuid() {
		return e.guid;
	}
	void connect(node* P) {
		p = P;
	}
	node* next() {
		return p;
	}
};

class chain {
private:
	node* HEAD;
public:
	chain() {
		HEAD = NULL;
	}
	~chain() {
		node* p = HEAD;
		while (HEAD != NULL) {
			HEAD = HEAD->next();
			delete p;
			p = HEAD;
		}
	}
	void add(const int& X = 0, const int& Y = 0, const int& R = 0, const int& F = 0, const unsigned long long& G = 0) {
		if (HEAD == NULL) {
			HEAD = new node(NULL, X, Y, R, F, G);
		}
		else if (HEAD->getFrame() >= F) {
			if (HEAD->getGuid() != G) {
				HEAD = new node(HEAD, X, Y, R, F, G);
			}
		}
		else {
			node* p = HEAD;
			node* q = HEAD->next();
			while (q != NULL) {
				if (F <= (*q).getFrame()) {
					break;
				}
				p = p->next();
				q = q->next();
			}
			if (HEAD->getGuid() != G) {
				q = new node(q, X, Y, R, F, G);
				(*p).connect(q);
			}
		}
	}
	void clean(const int& current) {
		if (HEAD == NULL) {
			return;
		}
		while (HEAD->getFrame() <= current) {
			node* p = HEAD;
			HEAD = HEAD->next();
			delete p;
			if (HEAD == NULL) {
				break;
			}
		}
	}
	node* get() {
		return HEAD;
	}
};

class Target {
public:
	int x;
	int y;
	char type;
	Target(const int& X = 0, const int& Y = 0, const char& C = 'c') :x(X), y(Y), type(C) {}
};

bool initialized = false;
char map[50][50] = { 0 };
int myTeamID = 0;
int myPlayerID = 0;
int positionX = 0;
int positionY = 0;
double mySpeed = 3;
// 未更新速度
chain explosions;
clock_t start = clock();
// 记录上一轮的角度，合成、异步获取信息计算，提高占空比

char route[200];

inline int ToGrid(const int& x) {
	return x / 1000;
}

inline double Angle(const double& x, const double& y) {
	if (x > 0) {
		return atan(y / x);
	}
	else if (x < 0) {
		return atan(y / x) + asin(1) * 2;
	}
	else if (y > 0) {
		return asin(1);
	}
	else {
		return -asin(1);
	}
}

inline double Distance(const int& x, const int& y) {
	return sqrt(x * x + y * y);
}

Target FindTarget(IAPI& api, const int& forEnemy, const int& forCPU, const int& forProp);

int Guide(const Target& T);

RTN Attack(IAPI& api);


void AI::play(IAPI& api) {

	if (!initialized) {
		for (int i = 0; i < 50; i++) {
			for (int j = 0; j < 50; j++) {
				map[i][j] = (int)api.GetPlaceType(i, j);
			}
		}
		auto myself = api.GetSelfInfo();
		myTeamID = myself->teamID;
		myPlayerID = myself->playerID;
		mySpeed = myself->speed/1000.;
		positionX = myself->x;
		positionY = myself->y;
		initialized = true;
	}

	
	//std::cout << "clock: " << (int)clock() << std::endl;
	//std::cout << "time: " << double((clock() - start) / CLOCKS_PER_SEC) << std::endl;
	//if ((int)clock() % 1000 <= 20) {
	auto myself = api.GetSelfInfo();
	positionX = myself->x;
	positionY = myself->y;
	int dx = ToGrid(positionX)*1000+500 - positionX;
	int dy = ToGrid(positionY)*1000+500 - positionY;
	api.MovePlayer(Distance(dx, dy) / mySpeed, Angle(dx, dy));
	
	/*
	auto signalJammers = api.GetSignalJammers();
	int i = 0, loop = (int)signalJammers.size();
	THUAI5::SignalJammerType type;
	int range, damage, distance, x, y, t;
	double speed;
	while (i < loop) {
		type = signalJammers[i]->type;
		if (type == THUAI5::SignalJammerType::LineJammer){
			distance = 900; damage = 2000; speed = 1; range = 4000;
		}
		else if (type == THUAI5::SignalJammerType::FastJammer){
			distance = 9000; damage = 1500; speed = 5; range = 1500;
		}
		else if (type == THUAI5::SignalJammerType::CommonJammer){
			distance = 4500; damage = 2500; speed = 2.5; range = 2500;
		}
		else if (type == THUAI5::SignalJammerType::StrongJammer){
			distance = 2000; damage = 7000; speed = 2; range = 7000;
		}
		// 未考虑隔墙打
		x = int(signalJammers[i]->x + cos(signalJammers[i]->facingDirection) * distance);
		y = int(signalJammers[i]->y + sin(signalJammers[i]->facingDirection) * distance);
		t = (int)clock() + int((double)distance / speed);
		explosions.add(x, y, range, t, signalJammers[i]->guid);
	}
	explosions.clean((int)clock());*/

	Target target = FindTarget(api, 0, 1, 0);
	//std::cout << "target:" << target.x << "\t" << target.y << std::endl;
	//std::cout << "My:" << positionX << "\t" << positionY << std::endl;
	int step = Guide(target);
	//std::cout << "route:" << route << std::endl;
	//std::cout << step << std::endl;
	for (int i = 0; i < step; i++) {
		if (route[i] == 'w') {
			if (api.MoveUp(1000 / mySpeed)) {
				positionX -= 1000;
			}
			else {
				i--; continue;
			}
		}
		else if (route[i] == 's') {
			if (api.MoveDown(1000 / mySpeed)) {
				positionX += 1000;
			}
			else {
				i--; continue;
			}
		}
		else if (route[i] == 'a') {
			if (api.MoveLeft(1000 / mySpeed)) {
				positionY -= 1000;
			}
			else {
				i--; continue;
			}
		}
		else if (route[i] == 'd') {
			if (api.MoveRight(1000 / mySpeed)) {
				positionY += 1000;
			}
			else {
				i--; continue;
			}
		}
		if (i % 3 == 0) {
			auto self = api.GetSelfInfo();
			auto signalJammers = api.GetSignalJammers();
			RTN attack = Attack(api);
			if (attack.weight >= 0.25) api.Attack(attack.angle);
			RTN escape = EfrJ(self, signalJammers);
			if (escape.weight) {
				if (escape.possibility == 1) {
					api.UseCPU(self->cpuNum);
				}
				else {
					api.MovePlayer(escape.time, escape.angle);
					positionX += int(cos(escape.angle) * escape.time * mySpeed);
					positionY += int(sin(escape.angle) * escape.time * mySpeed);
					int dx = ToGrid(positionX) * 1000 + 500 - positionX;
					int dy = ToGrid(positionY) * 1000 + 500 - positionY;
					api.MovePlayer(Distance(dx, dy) / mySpeed, Angle(dx, dy));
					return;
				}
			}
			if (target.type == 'e') {
				return;
			}
		}
	}
	if (target.type == 'c') {
		api.Pick(THUAI5::PropType::CPU);
	}
	else {
		api.Pick(THUAI5::PropType::Booster);
	}
	/*
	double angle = 0;
	std::cout << "l: "<< route.dest << std::endl;
	for (int i = 2; i < route.dest; i++) {
		angle = Angle(333.3 * route.HEAD[i].x + 166.67 - positionX, 333.3 * route.HEAD[i].y + 166.67 - positionY);
		api.MovePlayer(333. / mySpeed, angle);
		positionX = 333.3 * route.HEAD[i].x + 166.67;
		positionY = 333.3 * route.HEAD[i].y + 166.67;
		if (i % 6 == 0) {
			auto self = api.GetSelfInfo();
			signalJammers = api.GetSignalJammers();
			RTN escape = EfrJ(self, signalJammers);
			if (escape.weight) {
				if (escape.possibility == 1) {
					api.UseCPU(self->cpuNum);
				}
				else {
					api.MovePlayer(escape.time, escape.angle);
					return;
				}
			}
			RTN attack = Attack(api);
			if (attack.weight >= 0.4) api.Attack(attack.angle);
		}
		api.Pick(THUAI5::PropType::CPU);
	}*/

}

Target FindTarget(IAPI& api, const int& forEnemy, const int& forCPU, const int& forProp) {
	double enemy_max = 0, cpu_max = 0, prop_max = 0;
	int enemy_x = 0, enemy_y = 0;
	int cpu_x = 0, cpu_y = 0;
	int prop_x = 0, prop_y = 0;
	char type = 'c';
	if (forEnemy) {
		auto robots = api.GetRobots();
		int i = 0, loop = (int)robots.size();
		double distance, weight;
		while (i < loop) {
			if (robots[i]->teamID != myTeamID) {
				distance = Distance(robots[i]->x - positionX, robots[i]->y - positionY);
				weight = pow(2, 3 / (distance / 1000.0 - 1));// 权重
				if (weight > enemy_max) {
					enemy_max = weight;
					enemy_x = robots[i]->x;
					enemy_y = robots[i]->y;
				}
			}
			i++;
		}
		enemy_max *= forEnemy;
	}
	if (forCPU || forProp) {
		auto props = api.GetProps();
		int i = 0, loop = (int)props.size();
		double distance, weight;
		while (i < loop) {
			if (props[i]->type == THUAI5::PropType::CPU) {
				if (forCPU) {
					distance = Distance(props[i]->x - positionX, props[i]->y - positionY);
					weight = pow(2, 10 / (distance / 1000.0));// 权重
					if (weight > cpu_max) {
						cpu_max = weight;
						cpu_x = props[i]->x;
						cpu_y = props[i]->y;
					}
				}
			}
			else {
				if (forProp) {
					distance = Distance(props[i]->x - positionX, props[i]->y - positionY);
					weight = pow(2, 10 / (distance / 1000.0));// 权重
					if (weight > prop_max) {
						prop_max = weight;
						prop_x = props[i]->x;
						prop_y = props[i]->y;
					}
				}
			}
			i++;
		}
		cpu_max *= forCPU;
		prop_max *= forProp;
		if (prop_max > cpu_max) {
			cpu_max = prop_max;
			cpu_x = prop_x;
			cpu_y = prop_y;
			type = 'p';
		}
	}
	if (enemy_max > cpu_max) {
		return Target(enemy_x, enemy_y, 'e');
	}
	else {
		return Target(cpu_x, cpu_y, type);
	}
}

int Guide(const Target& T) {
	int endX = ToGrid(T.x);
	int endY = ToGrid(T.y);
	int startX = ToGrid(positionX);
	int startY = ToGrid(positionY);
	
	char label[50][50] = { 0 };
	label[startX][startY] = 1;
	label[endX][endY] = -1;
	bool found = false;
	int plusX = 0, plusY = 0;
	int minusX = 0, minusY = 0;
	for (int i = 0; i < 100; i++) {
		route[i] = 0;
	}
	if (startX == endX && startY == endY) {
		return 0;
	}
	if (startX == endX) {
		if (startY == endY + 1) {
			route[0] = 'a';
			return 1;
		}
		if (startY == endY - 1) {
			route[0] = 'd';
			return 1;
		}
	}
	if (startY == endY) {
		if (startX == endX + 1) {
			route[0] = 'w';
			return 1;
		}
		if (startX == endX - 1) {
			route[0] = 's';
			return 1;
		}
	}
	int step = 1;
	for (; !found && step <= 200; step++) {
		for (int i = 1; !found && i < 49; i++) {
			for (int j = 1; !found && j < 49; j++) {
				if (map[i][j] != 1 && label[i][j] == 0) {
					if (abs(startX - i) + abs(startY - j) <= step) {
						if (label[i - 1][j] == step || label[i + 1][j] == step || label[i][j - 1] == step || label[i][j + 1] == step) {
							label[i][j] = step + 1;
							if (label[i - 1][j] == -step) {
								plusX = i; plusY = j;
								minusX = i - 1; minusY = j;
								route[step] = 'w';
								found = true; break;
							}
							if (label[i + 1][j] == -step) {
								plusX = i; plusY = j;
								minusX = i + 1; minusY = j;
								route[step] = 's';
								found = true; break;
							}
							if (label[i][j - 1] == -step) {
								plusX = i; plusY = j;
								minusX = i; minusY = j - 1;
								route[step] = 'a';
								found = true; break;
							}
							if (label[i][j + 1] == -step) {
								plusX = i; plusY = j;
								minusX = i; minusY = j + 1;
								route[step] = 'd';
								found = true; break;
							}
						}
					}
					if (abs(endX - i) + abs(endY - j) <= step) {
						if (label[i - 1][j] == -step || label[i + 1][j] == -step || label[i][j - 1] == -step || label[i][j + 1] == -step) {
							label[i][j] = -step - 1;
							if (label[i - 1][j] == step + 1) {
								minusX = i; minusY = j;
								plusX = i - 1; plusY = j;
								route[step] = 's';
								found = true; break;
							}
							if (label[i + 1][j] == step + 1) {
								minusX = i; minusY = j;
								plusX = i + 1; plusY = j;
								route[step] = 'w';
								found = true; break;
							}
							if (label[i][j - 1] == step + 1) {
								minusX = i; minusY = j;
								plusX = i; plusY = j - 1;
								route[step] = 'd';
								found = true; break;
							}
							if (label[i][j + 1] == step + 1) {
								minusX = i; minusY = j;
								plusX = i; plusY = j + 1;
								route[step] = 'a';
								found = true; break;
							}
						}
					}
				}
			}
		}
	}
	if (!found) {
		for (int i = 0; i < 100; i++) {
			route[i] = 0;
		}
		return 0;
	}
	int mid = label[plusX][plusY];
	int len = mid - label[minusX][minusY] - 1;
	for (int n = mid - 1; n; n--) {
		if (label[plusX - 1][plusY] == n) {
			route[n - 1] = 's';
			plusX--;
		}
		else if (label[plusX + 1][plusY] == n) {
			route[n - 1] = 'w';
			plusX++;
		}
		else if (label[plusX][plusY - 1] == n) {
			route[n - 1] = 'd';
			plusY--;
		}
		else if (label[plusX][plusY + 1] == n) {
			route[n - 1] = 'a';
			plusY++;
		}
	}
	for (int n = label[minusX][minusY] + 1; n; n++) {
		if (label[minusX - 1][minusY] == n) {
			route[len + n] = 'w';
			minusX--;
		}
		else if (label[minusX + 1][minusY] == n) {
			route[len + n] = 's';
			minusX++;
		}
		else if (label[minusX][minusY - 1] == n) {
			route[len + n] = 'a';
			minusY--;
		}
		else if (label[minusX][minusY + 1] == n) {
			route[len + n] = 'd';
			minusY++;
		}
	}
	return len;
}

RTN Attack(IAPI& api) {
	auto robots = api.GetRobots();
	int loop = (int)robots.size();
	auto SelfInfo = api.GetSelfInfo();
	int MyTeamID = SelfInfo->teamID;
	int d = 0, temp = 0, max = -5000;
	//int jammerType = (int)SelfInfo->signalJammerType;
	JMRinfo Jinfo(SelfInfo->signalJammerType);
	RTN ans;
	for (int i = 0; i < loop; i++) {
		if (robots[i]->teamID != MyTeamID) {
			d = distance(SelfInfo->x, SelfInfo->y, robots[i]->x, robots[i]->y);
			temp = Jinfo.range - abs(SelfInfo->attackRange - d) + 500 - SelfInfo->attackRange * robots[i]->speed / Jinfo.speed;
			if (temp > max) {
				max = temp;
				ans.weight = pow(2., temp / 1000);
				ans.angle = angle(SelfInfo->x, SelfInfo->y, robots[i]->x, robots[i]->y);
			}
		}
	}
	return ans;
}

RTN calculate(std::shared_ptr < const THUAI5::SignalJammer>& sig, std::shared_ptr<const THUAI5::Robot>& SelfInfo)
{
	RTN areturn;
	int k, dx = 0, dy = 0, flag = 0, xxx, yyy;
	//xxx,yyy为爆炸点坐标
	JMRinfo Jinfo(sig->type);
	double djm = distance(SelfInfo->x, SelfInfo->y, sig->x, sig->y), dem = 0, tarrival = 0, length = Jinfo.range;//导弹飞行过程中遇到墙，则需要对这个变量的值进行改动，故存下来。
	double dp = distance(sig->x, sig->y, sig->facingDirection, SelfInfo->x, SelfInfo->y);//dp表示自身所在点到导弹轨迹的距离
	if (djm > (Jinfo.explosion + Jinfo.range)) { ; }//排除显然打不到的情况
	else
	{
		//判断信号干扰弹行进过程中是否会遇到墙
		areturn.weight = 1;
		yyy = sig->y; xxx = sig->x;
		if (cos(sig->facingDirection) == 0)
		{
			while (((yyy - sig->y) * sin(sig->facingDirection) < Jinfo.range) && flag == 0)
			{
				if (map[sig->x / 1000][yyy / 1000] == 1) { flag = 1; }//如果这一点对应的单元格是墙,则跳出循环
				yyy += sin(sig->facingDirection) * 1000;
			}
		}
		else if (sin(sig->facingDirection) == 0)
		{
			while (((xxx - sig->x) * cos(sig->facingDirection) < Jinfo.range) && flag == 0)
			{
				if (map[sig->x / 1000][yyy / 1000] == 1) { flag = 1; }//如果这一点对应的单元格是墙,则跳出循环
				xxx += cos(sig->facingDirection) * 1000;
			}
		}
		else
		{
			k = sig->facingDirection / asin(1);
			switch ((int)k)
			{
			case 1: {dx = 1; dy = 1; break; }
			case 2: {dx = -1; dy = 1; break; }
			case 3: {dx = -1; dy = -1; break; }
			case 4: {dx = 1; dy = -1; break; }
			}
		}
		while ((((xxx - (sig->x + Jinfo.range * cos(sig->facingDirection))) * dx < 0) && ((yyy - (sig->x + Jinfo.range * sin(sig->facingDirection))) * dy < 0)) && flag == 0) {
			if ((dy * ((xtoy(sig->x, sig->y, sig->facingDirection, (xxx / 1000 + dx) * 1000) - yyy) - (yyy / 1000 + dy) * 1000)) < 0) {
				xxx = (xxx / 1000 + dx) * 1000;
			}
			else { yyy = (yyy / 1000 + dy) * 1000; }
			if (map[sig->x / 1000][yyy / 1000] == 1) { flag = 1; }
		}

		if (flag == 0) {//如果行进过程中不会遇到墙
			xxx = sig->x + Jinfo.range * cos(sig->facingDirection); yyy = sig->y + Jinfo.range * cos(sig->facingDirection);//爆炸点坐标
			dem = distance(xxx, yyy, SelfInfo->x, SelfInfo->y); tarrival = Jinfo.range / Jinfo.speed;
		}
		else//如果行进过程中会遇到墙
		{
			dem = distance(xxx, yyy, SelfInfo->x, SelfInfo->y); length = distance(xxx, yyy, sig->x, sig->y); tarrival = length / Jinfo.speed;
		}
		//以下为判断是否能躲避的计算部分
		if (dp > 650 && dem > Jinfo.explosion) { ; }
		if (dp <= 650 && djm < length)
		{
			areturn.angle = (sig->facingDirection > angle(sig->x, sig->y, SelfInfo->x, SelfInfo->y)) ? (sig->facingDirection + 3.141926 / 2) : (sig->facingDirection + 3 / 2);
			if (dem > Jinfo.explosion) { areturn.time = 1000 * (60 - dp) / SelfInfo->speed; areturn.possibility = (areturn.time > tarrival) ? 0 : 1; }
			else {
				areturn.time = 1000 * sqrt(Jinfo.explosion * Jinfo.explosion - dem * dem - dp * dp) / SelfInfo->speed; areturn.possibility = (areturn.time > tarrival) ? 0 : 1;
			}
		}
		if (dp > 650 && dem < Jinfo.explosion)
		{
			areturn.angle = angle(xxx, yyy, SelfInfo->x, SelfInfo->y);
			areturn.time = 1000 * (Jinfo.explosion - dem) / SelfInfo->speed;
			areturn.possibility = (areturn.time > tarrival) ? 0 : 1;
			areturn.possibility = ((Jinfo.damage) > SelfInfo->life) ? (areturn.possibility) : (areturn.possibility / 2);
		}
	}
	return areturn;
}

RTN EfrJ(std::shared_ptr<const THUAI5::Robot>& SelfInfo, std::vector<std::shared_ptr<const THUAI5::SignalJammer>>& sig) {//Escape from Jammers
	int JammerNumber = (int)sig.size(), i, m = 0, p_near = 0, t_min = 0;
	RTN nearestJammer, temp;
	RTN alljammers[50];
	double sum_possibility = 0, sum_weight = 0;
	if (JammerNumber == 0) {
		return nearestJammer;
	}
	for (i = 0; i < JammerNumber; i++) {
		if (sig[i]->parentTeamID != SelfInfo->teamID) {    //在判断导弹是敌方时才进行以下计算
			auto guidthis = sig[i]->guid;
			alljammers[m] = calculate(sig[i], SelfInfo);
			sum_possibility += alljammers[m].possibility;
			sum_weight += alljammers[m].weight;
			if (sum_possibility >= 1) {
				temp.possibility = 1;
				return temp;
			}
			else if (alljammers[m].time < t_min && alljammers[m].time>0) {
				t_min = alljammers[m].time;
				p_near = m;
			}
		}
		m++;
	}
	if (sum_weight == 0) return temp;
	else return alljammers[p_near];
}
