#include <random>
#include<cmath>
#include "../include/AI.h"
#include<thread>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;


// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Invisible;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::EnergyConvert;

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

static int step = 1;
int eated = 0;

struct trace
{
	int x[8] = { 0 };
	int y[8] = { 0 };
	int bul = 0;
	int range = 0;
	int life = 0;
}tra[8];

int judgewall(IAPI& api, int* map[], double selta, double distance, double r);

struct bullet
{
	THUAI5::SignalJammerType type;
	int guid;
	int x = 0, y = 0;
	int orgx = 0, orgy = 0;
	int team;
	double e;
}bul[250];

int bultop = 0, bulbot = 0;

double distance(double x1, double y1, double x2, double y2)
{
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

void pause(int time)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(time));
	return;
}

void maxarea(int* map[], int* x, int* y, int f, int g)
{
	int a = 0;
	if (step == 1)
	{
		for (int i = 0; i < 50; i++)
		{
			for (int j = 0; j < 50; j++)
			{
				if (map[i][j] == 4)
				{
					f = i;
					g = j;
					*x = i;
					*y = j;
					a = 1;
					break;
				}
			}
			if (a == 1)
			{
				break;
			}
		}
	}
	map[f][g] = 0;
	if (map[f + 1][g] == 4)
	{
		f = f + 1;
		*x = *x + 1;
		step++;
		maxarea(map, x, y, f, g);
	}
	if (f - 1 >= 0 && map[f - 1][g] == 4)
	{
		f = f - 1;
		*x = *x - 1;
		step++;
		maxarea(map, x, y, f, g);
	}
	if (map[f][g + 1] == 4)
	{
		g = g + 1;
		*y = *y + 1;
		step++;
		maxarea(map, x, y, f, g);
	}
	if (g - 1 >= 0 && map[f][g - 1] == 4)
	{
		g = g - 1;
		*y = *y - 1;
		step++;
		maxarea(map, x, y, f, g);
	}
}

int stuck(int ID)
{
	int flag = 1;
	for (int i = 1; i < 4; i++)
	{
		if (tra[ID].x[i] != tra[ID].x[0])
		{
			flag = 0;
		}
		if (tra[ID].y[i] != tra[ID].y[0])
		{
			flag = 0;
		}
	}
	return flag;
}

void record(IAPI& api)
{
	int i, k, j, ID;
	auto bullets = api.GetSignalJammers();
	int bulnum = bullets.size();
	int mem[200] = { 0 };
	int mem2[200] = { 0 };
	for (ID = 0; ID < 8; ID++)
	{
		for (j = 7; j > 0; j--)
		{
			tra[ID].x[j] = tra[ID].x[j - 1];
			tra[ID].y[j] = tra[ID].y[j - 1];
		}
		tra[ID].x[0] = 0;
		tra[ID].y[0] = 0;
	}
	for (i = 0; i < api.GetRobots().size(); i++)
	{
		ID = int(api.GetRobots()[i]->playerID) + int(api.GetRobots()[i]->teamID) * 4;
		if (api.GetRobots()[i]->isResetting == 0)
		{
			tra[ID].x[0] = (api.GetRobots()[i]->x);
			tra[ID].y[0] = (api.GetRobots()[i]->y);
		}
		tra[ID].bul = (api.GetRobots()[i]->signalJammerNum);
		if (api.GetRobots()[i]->signalJammerType == THUAI5::SignalJammerType::CommonJammer)
		{
			tra[ID].range = 7000;
		}
		if (api.GetRobots()[i]->signalJammerType == THUAI5::SignalJammerType::FastJammer)
		{
			tra[ID].range = 10500;
		}
		if (api.GetRobots()[i]->signalJammerType == THUAI5::SignalJammerType::LineJammer)
		{
			tra[ID].range = 5000;
		}
		if (api.GetRobots()[i]->signalJammerType == THUAI5::SignalJammerType::StrongJammer)
		{
			tra[ID].range = 14000;
		}
		tra[ID].life = (api.GetRobots()[i]->life);

	}
	ID = api.GetSelfInfo()->playerID + api.GetSelfInfo()->teamID * 4;
	tra[ID].x[0] = (api.GetSelfInfo()->x);
	tra[ID].y[0] = (api.GetSelfInfo()->y);
	for (i = 0; i < bulnum; i++)
	{
		for (j = bulbot; j != bultop; j = (j + 1) % 200)
		{
			if (bul[j].guid == bullets[i]->guid)
			{
				bul[j].x = bullets[i]->x;
				bul[j].y = bullets[i]->y;
				mem[i] = 1;
				mem2[j] = 1;
			}
		}
	}
	for (j = bulbot; j != bultop; j = (j + 1) % 200)
	{
		if (mem2[j] == 0)
		{
			bul[j].x = 0;
			bul[j].y = 0;
		}
	}
	for (i = bulbot; bul[i].x == 0 && bul[i].y == 0 && i != bultop; i = (i + 1) % 200);
	bulbot = i;
	for (i = 0; i < bulnum; i++)
	{
		if (mem[i] == 0)
		{
			bultop = (bultop + 1) % 200;
			bul[bultop].x = bullets[i]->x;
			bul[bultop].y = bullets[i]->y;
			bul[bultop].type = bullets[i]->type;
			bul[bultop].orgx = bullets[i]->x;
			bul[bultop].orgy = bullets[i]->y;
			bul[bultop].team = bullets[i]->parentTeamID;
			bul[bultop].e = bullets[i]->facingDirection;
			bul[bultop].guid = bullets[i]->guid;
		}
	}
}

void adjust(int* map[], int x, int y, int movespeed, int ID, IAPI& api)
{
	pause(300);
	double sum = 0, i = 0, count = 0;
	int step = 400;
	auto self = api.GetSelfInfo();
	if (x + 1 < 50)
	{
		if (map[x + 1][y] == 5 || map[x + 1][y] == 6)
		{
			api.MovePlayer(step / movespeed, 3.1415926 + i);
			pause(int(step / movespeed));
			count++;
		}
	}
	i = i + 3.1415926 / 4;
	if (x + 1 < 50 && y + 1 < 50)
	{
		if (map[x + 1][y + 1] == 5 || map[x + 1][y + 1] == 6)
		{
			api.MovePlayer(step / movespeed, 3.1415926 + i);
			pause(int(step / movespeed));
			count++;
		}
	}
	i = i + 3.1415926 / 4;
	if (y + 1 < 50)
	{
		if (map[x][y + 1] == 5 || map[x][y + 1] == 6)
		{
			api.MovePlayer(step / movespeed, 3.1415926 + i);
			pause(int(step / movespeed));
			count++;
		}
	}
	i = i + 3.1415926 / 4;
	if (x - 1 >= 0 && y + 1 < 50)
	{
		if (map[x - 1][y + 1] == 5 || map[x - 1][y + 1] == 6)
		{
			api.MovePlayer(step / movespeed, 3.1415926 + i);
			pause(int(step / movespeed));
			count++;
		}
	}
	i = i + 3.1415926 / 4;
	if (x - 1 >= 0)
	{
		if (map[x - 1][y] == 5 || map[x - 1][y] == 6)
		{
			api.MovePlayer(step / movespeed, i - 3.1415926);
			pause(int(step / movespeed));
			count++;
		}
	}
	i = i + 3.1415926 / 4;
	if (x - 1 >= 0 && y - 1 >= 0)
	{
		if (map[x - 1][y - 1] == 5 || map[x - 1][y - 1] == 6)
		{
			api.MovePlayer(step / movespeed, i - 3.1415926);
			pause(int(step / movespeed));
			count++;
		}
	}
	i = i + 3.1415926 / 4;
	if (y - 1 >= 0)
	{
		if (map[x][y - 1] == 5 || map[x][y - 1] == 6)
		{
			api.MovePlayer(step / movespeed, i - 3.1415926);
			pause(int(step / movespeed));
			count++;
		}
	}
	i = i + 3.1415926 / 4;
	if (x + 1 < 50 && y - 1 >= 0)
	{
		if (map[x + 1][y - 1] == 5 || map[x + 1][y - 1] == 6)
		{
			api.MovePlayer(step / movespeed, i - 3.1415926);
			pause(int(step / movespeed));
			count++;
		}
	}
	i = i + 3.1415926 / 4;
	if (count == 0)
	{
		api.MovePlayer(step / movespeed, 3.1415926);
		pause(int(step / movespeed));
	}
}

void adjust1(double speed, IAPI& api)
{
	pause(300);
	double dx, dy;
	double lim = 1;
	auto self = api.GetSelfInfo();
	if (self->y % 1000 != 500)
	{
		dy = (int(api.GridToCell(self->y) * 1000 + 500) - int(self->y)) / speed;
		if (0 < dy)
		{
			if (dy < lim)
			{
				dy = lim;
			}
			api.MovePlayer(int(dy), 3.1415926 / 2);
			pause(int(dy));
		}
		if (dy < 0)
		{
			if (-lim < dy)
			{
				dy = -lim;
			}
			api.MovePlayer(int(-dy), 3.1415926 * 3 / 2);
			pause(int(-dy));
		}

	}
	if (self->x % 1000 != 500)
	{
		dx = (int(api.GridToCell(self->x) * 1000 + 500) - int(self->x)) / speed;
		if (0 < dx)
		{
			if (dx < lim)
			{
				dx = lim;
			}
			api.MovePlayer(int(dx), 0);
			pause(int(dx));
		}
		if (dx < 0)
		{
			if (-lim < dx)
			{
				dx = -lim;
			}
			api.MovePlayer(int(-dx), 3.1415926);
			pause(int(-dx));
		}

	}
}

void attack(IAPI& api, int* map[])
{
	auto self = api.GetSelfInfo();
	double xself = self->x;
	double yself = self->y;
	double xcell = tra[0].x[0];
	double ycell = tra[0].y[0];
	double dis;
	double dismin = 50000;
	int tegrat = 0;
	double xt;
	double yt;
	if (api.GetSelfInfo()->teamID == 0)
	{
		for (int i = 4; i <= 7; i++)
		{
			xt = tra[i].x[0];
			yt = tra[i].y[0];
			dis = sqrt((xt - xself) * (xt - xself) + (yt - yself) * (yt - yself));
			if (dis < dismin)
			{
				dismin = dis;
				tegrat = i;
			}
		}
	}
	if (api.GetSelfInfo()->teamID == 1)
	{
		for (int i = 0; i <= 3; i++)
		{
			xt = tra[i].x[0];
			yt = tra[i].y[0];
			dis = sqrt((xt - xself) * (xt - xself) + (yt - yself) * (yt - yself));
			if (dis < dismin)
			{
				dismin = dis;
				tegrat = i;
			}
		}
	}
	double speed = 20 * (sqrt((tra[tegrat].x[0] - tra[tegrat].x[1]) * (tra[tegrat].x[0] - tra[tegrat].x[1]) + (tra[tegrat].y[0] - tra[tegrat].y[1]) * (tra[tegrat].y[0] - tra[tegrat].y[1])));
	double distancedifference = sqrt((xself - tra[tegrat].x[0]) * (xself - tra[tegrat].x[0]) + (yself - tra[tegrat].y[0]) * (yself - tra[tegrat].y[0]));
	double ddx = tra[tegrat].x[0] - xself;
	double ddy = tra[tegrat].y[0] - yself;
	double sx = tra[tegrat].x[0] - tra[tegrat].x[1];
	double sy = tra[tegrat].y[0] - tra[tegrat].y[1];
	double selta1 = 0;
	double selta2 = 0;
	double selta;//机器人发射炮弹的方向，以向下为极坐标起始方位逆时针旋转，如selta=3.1415926则向上开炮
	if (ddx > 0)
	{
		selta1 = atan(ddy / ddx);
	}
	if (ddx < 0)
	{
		selta1 = 3.1415926 + atan(ddy / ddx);
	}
	if (ddx == 0)
	{
		if (ddy > 0)
		{
			selta1 = 1.5707963;
		}
		if (ddy < 0)
		{
			selta1 = 4.7123889;
		}
	}
	if (sx > 0)
	{
		selta2 = 1.5707963 - atan(sy / sx);
	}
	if (sx < 0)
	{
		selta2 = 4.7123889 - atan(sy / sx);
	}
	if (sx == 0 && sy != 0)
	{
		if (sy > 0)
		{
			selta2 = 1.570963;
		}
		if (sy < 0)
		{
			selta2 = 4.7123889;
		}
	}
	if (sx == 0 && sy == 0)
	{
		if (judgewall(api, map, selta1, distancedifference / 1000, 500) == 0)
		{
			if (distancedifference <= 6000)
			{
				api.Attack(selta1);
			}
		}
		return;
	}
	double selta3 = 3.1415926 - abs(selta1 - selta2);
	if (abs(speed * sin(selta3) / 3000) <= 1 && distancedifference <= 6000)//此处<=4500随子弹速度有所修改
	{
		double selta4 = asin((speed * sin(selta3)) / 5000);//如炮弹为快速炮弹，则此处3000应改为5000
		if (selta1 > selta2)
		{
			selta = selta1 - selta4;
		}
		if (selta1 <= selta2)
		{
			selta = selta1 + selta4;
		}
		if (judgewall(api, map, selta, distancedifference / 1000, 500) == 0)
		{
			api.Attack(selta);
		}
	}
}

class Vector {
public:
	int x;
	int y;
};

int judgewall(IAPI& api, int* map[], double selta, double distance, double r)
{
	auto self = api.GetSelfInfo();
	int x = self->x, y = self->y;
	if (distance == 0)
		return 0;
	for (int i = 1000; i <= 1000 * distance; i += 200)
	{
		double m = x + (r)*cos(selta - 1.5707963) + i * cos(selta);
		double n = y + (r)*sin(selta - 1.5707963) + i * sin(selta);
		m = (int)m; n = (int)n;
		if (m <= 50000 && n <= 50000)
		{
			m = api.GridToCell(m);
			n = api.GridToCell(n);
			int mm = m, nn = n;
			if (map[mm][nn] == 5)
				return 1;
		}
	}
	for (int i = 1000; i <= 1000 * distance; i += 200)
	{
		double m = x - (r)*cos(selta - 1.5707963) + i * cos(selta);
		double n = y - (r)*sin(selta - 1.5707963) + i * sin(selta);
		m = (int)m; n = (int)n;
		if (m <= 50000 && n <= 50000)
		{
			m = api.GridToCell(m);
			n = api.GridToCell(n);
			int mm = m, nn = n;
			if (map[mm][nn] == 5)
				return 1;
		}

	}
	for (int i = 1000; i <= 1000 * distance; i += 200)
	{
		double m = x + i * cos(selta);
		double n = y + i * sin(selta);
		m = (int)m; n = (int)n;
		if (m <= 50000 && n <= 50000)
		{
			m = api.GridToCell(m);
			n = api.GridToCell(n);
			int mm = m, nn = n;
			if (map[mm][nn] == 5)
				return 1;
		}

	}
	return 0;
}
//|| map[mm][nn] == 6
int judgewall3(IAPI& api, int* map[], double selta, double distance, double x, double y, double r)
{
	auto self = api.GetSelfInfo();
	if (distance == 0)
		return 0;
	for (int i = 1000; i <= 1000 * distance; i += 200)
	{
		double m = x + (r)*cos(selta - 1.5707963) + i * cos(selta);
		double n = y + (r)*sin(selta - 1.5707963) + i * sin(selta);
		m = (int)m; n = (int)n;
		if (m <= 50000 && n <= 50000)
		{
			m = api.GridToCell(m);
			n = api.GridToCell(n);
			int mm = m, nn = n;
			if (map[mm][nn] == 5 || map[mm][nn] == 6)
				return 1;
		}
	}
	for (int i = 1000; i <= 1000 * distance; i += 200)
	{
		double m = x - (r)*cos(selta - 1.5707963) + i * cos(selta);
		double n = y - (r)*sin(selta - 1.5707963) + i * sin(selta);
		m = (int)m; n = (int)n;
		if (m <= 50000 && n <= 50000)
		{
			m = api.GridToCell(m);
			n = api.GridToCell(n);
			int mm = m, nn = n;
			if (map[mm][nn] == 5 || map[mm][nn] == 6)
				return 1;
		}

	}
	for (int i = 1000; i <= 1000 * distance; i += 200)
	{
		double m = x + i * cos(selta);
		double n = y + i * sin(selta);
		m = (int)m; n = (int)n;
		if (m <= 50000 && n <= 50000)
		{
			m = api.GridToCell(m);
			n = api.GridToCell(n);
			int mm = m, nn = n;
			if (map[mm][nn] == 5 || map[mm][nn] == 6)
				return 1;
		}

	}
	return 0;
}

int judgewall2(IAPI& api, int* map[], double selta, double distance, double x, double y, double r)
{
	auto self = api.GetSelfInfo();
	if (distance == 0)
		return 0;
	for (int i = 1000; i <= 1000 * distance; i += 200)
	{
		double m = x + (r)*cos(selta - 1.5707963) + i * cos(selta);
		double n = y + (r)*sin(selta - 1.5707963) + i * sin(selta);
		m = (int)m; n = (int)n;
		if (m <= 50000 && n <= 50000)
		{
			m = api.GridToCell(m);
			n = api.GridToCell(n);
			int mm = m, nn = n;
			if (map[mm][nn] == 5)
				return 1;
		}
	}
	for (int i = 1000; i <= 1000 * distance; i += 200)
	{
		double m = x - (r)*cos(selta - 1.5707963) + i * cos(selta);
		double n = y - (r)*sin(selta - 1.5707963) + i * sin(selta);
		m = (int)m; n = (int)n;
		if (m <= 50000 && n <= 50000)
		{
			m = api.GridToCell(m);
			n = api.GridToCell(n);
			int mm = m, nn = n;
			if (map[mm][nn] == 5)
				return 1;
		}

	}
	for (int i = 1000; i <= 1000 * distance; i += 200)
	{
		double m = x + i * cos(selta);
		double n = y + i * sin(selta);
		m = (int)m; n = (int)n;
		if (m <= 50000 && n <= 50000)
		{
			m = api.GridToCell(m);
			n = api.GridToCell(n);
			int mm = m, nn = n;
			if (map[mm][nn] == 5)
				return 1;
		}

	}
	return 0;
}

int judgewall4(IAPI& api, int* map[], double selta, double distance, double r)
{
	auto self = api.GetSelfInfo();
	int x = self->x, y = self->y;
	if (distance == 0)
		return 0;
	for (int i = 1000; i <= 1000 * distance; i += 200)
	{
		double m = x + (r)*cos(selta - 1.5707963) + i * cos(selta);
		double n = y + (r)*sin(selta - 1.5707963) + i * sin(selta);
		m = (int)m; n = (int)n;
		if (m <= 50000 && n <= 50000)
		{
			m = api.GridToCell(m);
			n = api.GridToCell(n);
			int mm = m, nn = n;
			if (map[mm][nn] == 5 || map[mm][nn] == 6)
				return 1;
		}
	}
	for (int i = 1000; i <= 1000 * distance; i += 200)
	{
		double m = x - (r)*cos(selta - 1.5707963) + i * cos(selta);
		double n = y - (r)*sin(selta - 1.5707963) + i * sin(selta);
		m = (int)m; n = (int)n;
		if (m <= 50000 && n <= 50000)
		{
			m = api.GridToCell(m);
			n = api.GridToCell(n);
			int mm = m, nn = n;
			if (map[mm][nn] == 5 || map[mm][nn] == 6)
				return 1;
		}

	}
	for (int i = 1000; i <= 1000 * distance; i += 200)
	{
		double m = x + i * cos(selta);
		double n = y + i * sin(selta);
		m = (int)m; n = (int)n;
		if (m <= 50000 && n <= 50000)
		{
			m = api.GridToCell(m);
			n = api.GridToCell(n);
			int mm = m, nn = n;
			if (map[mm][nn] == 5 || map[mm][nn] == 6)
				return 1;
		}

	}
	return 0;
}

double locate(int x, int y, int aimx, int aimy, int* map[], IAPI& api)
{
	int  top = 0, bot = 0, flag = 0, dep = 0;
	int* queuex = (int*)malloc(sizeof(int) * 30000);
	int* queuey = (int*)malloc(sizeof(int) * 30000);
	int* depth = (int*)malloc(sizeof(int) * 30000);
	double* root = (double*)malloc(sizeof(double) * 30000);
	int pass[50][50] = { 0 };
	double sum = 0;
	double e = 0;
	*queuex = x;
	*queuey = y;
	*depth = 0;
	pass[x][y] = 1;
	double dis = distance(aimx, aimy, queuex[bot], queuey[bot]);
	if (queuex[bot] < aimx && queuey[bot] <= aimy)
	{
		e = atan(double(queuey[bot] - (aimy)) / double(queuex[bot] - (aimx)));
	}
	else if (queuex[bot] < aimx && queuey[bot] > aimy)
	{
		e = (3.1415926 * 2) + atan(double(queuey[bot] - (aimy)) / double(queuex[bot] - (aimx)));
	}
	else if (queuex[bot] > aimx)
	{
		e = 3.1415926 + atan(double(queuey[bot] - (aimy)) / double(queuex[bot] - (aimx)));
	}
	else if (queuey[bot] < aimy)
	{
		e = 3.1415926 / 2;
	}
	else
	{
		e = 3 * 3.1415926 / 2;
	}
	if (judgewall2(api, map, e, dis, queuex[bot] * 1000 + 500, queuey[bot] * 1000 + 500, 500) == 0
		&& judgewall2(api, map, e, dis, queuex[bot] * 1000 + 500, queuey[bot] * 1000 + 500, 1000) == 0)
	{
		free(queuex);
		free(queuey);
		free(depth);
		free(root);
		return e;
	}
	for (; (flag == 0 || dep >= depth[bot]) && bot <= top; bot++)
	{
		double dis = distance(aimx, aimy, queuex[bot], queuey[bot]);
		if (queuex[bot] < aimx && queuey[bot] <= aimy)
		{
			e = atan(double(queuey[bot] - (aimy)) / double(queuex[bot] - (aimx)));
		}
		else if (queuex[bot] < aimx && queuey[bot] > aimy)
		{
			e = (3.1415926 * 2) + atan(double(queuey[bot] - (aimy)) / double(queuex[bot] - (aimx)));
		}
		else if (queuex[bot] > aimx)
		{
			e = 3.1415926 + atan(double(queuey[bot] - (aimy)) / double(queuex[bot] - (aimx)));
		}
		else if (queuey[bot] < aimy)
		{
			e = 3.1415926 / 2;
		}
		else
		{
			e = 3 * 3.1415926 / 2;
		}
		if (dep < depth[bot])
		{
			dep = depth[bot];
		}
		if (dis < 14 && judgewall2(api, map, e, dis, queuex[bot] * 1000 + 500, queuey[bot] * 1000 + 500, 1000) == 0)
		{
			flag = 1;
			sum = root[bot];
		}
		else
		{
			if (queuex[bot] + 1 < 50)
			{
				if (map[queuex[bot] + 1][queuey[bot]] != 5 && map[queuex[bot] + 1][queuey[bot]] != 6 && pass[queuex[bot] + 1][queuey[bot]] == 0)
				{
					top++;
					queuex[top] = queuex[bot] + 1;
					queuey[top] = queuey[bot];
					if (dep == 0)
					{
						root[top] = 0;
					}
					else
					{
						root[top] = root[bot];
					}
					depth[top] = depth[bot] + 1;
					pass[queuex[top]][queuey[top]] = 1;
				}
			}
			if (queuex[bot] - 1 >= 0)
			{
				if (map[queuex[bot] - 1][queuey[bot]] != 5 && map[queuex[bot] - 1][queuey[bot]] != 6 && pass[queuex[bot] - 1][queuey[bot]] == 0)
				{
					top++;
					queuex[top] = queuex[bot] - 1;
					queuey[top] = queuey[bot];
					if (dep == 0)
					{
						root[top] = 3.1415926;
					}
					else
					{
						root[top] = root[bot];
					}
					depth[top] = depth[bot] + 1;
					pass[queuex[top]][queuey[top]] = 1;
				}
			}
			if (queuey[bot] + 1 < 50)
			{
				if (map[queuex[bot]][queuey[bot] + 1] != 5 && map[queuex[bot]][queuey[bot] + 1] != 6 && pass[queuex[bot]][queuey[bot] + 1] == 0)
				{
					top++;
					queuex[top] = queuex[bot];
					queuey[top] = queuey[bot] + 1;
					if (dep == 0)
					{
						root[top] = 3.1415926 / 2;
					}
					else
					{
						root[top] = root[bot];
					}
					depth[top] = depth[bot] + 1;
					pass[queuex[top]][queuey[top]] = 1;
				}
			}
			if (queuey[bot] - 1 >= 0)
			{
				if (map[queuex[bot]][queuey[bot] - 1] != 5 && map[queuex[bot]][queuey[bot] - 1] != 6 && pass[queuex[bot]][queuey[bot] - 1] == 0)
				{
					top++;
					queuex[top] = queuex[bot];
					queuey[top] = queuey[bot] - 1;
					if (dep == 0)
					{
						root[top] = 3 * 3.1415926 / 2;
					}
					else
					{
						root[top] = root[bot];
					}
					depth[top] = depth[bot] + 1;
					pass[queuex[top]][queuey[top]] = 1;
				}
			}
		}
	}
	free(queuex);
	free(queuey);
	free(depth);
	free(root);

	if (flag == 1)
	{
		return sum;
	}
	else
	{
		return -1;
	}
}

double rush(int x, int y, int aimx, int aimy, int* map[], IAPI& api)
{
	int  top = 0, bot = 0, flag = 0, dep = 0;
	int* queuex = (int*)malloc(sizeof(int) * 30000);
	int* queuey = (int*)malloc(sizeof(int) * 30000);
	int* depth = (int*)malloc(sizeof(int) * 30000);
	double* root = (double*)malloc(sizeof(double) * 30000);
	int pass[50][50] = { 0 };
	double e = 0;
	*queuex = x;
	*queuey = y;
	*depth = 0;
	pass[x][y] = 1;
	double dis = distance(aimx, aimy, queuex[bot], queuey[bot]);
	if (queuex[bot] < aimx && queuey[bot] <= aimy)
	{
		e = atan(double(queuey[bot] - (aimy)) / double(queuex[bot] - (aimx)));
	}
	else if (queuex[bot] < aimx && queuey[bot] > aimy)
	{
		e = (3.1415926 * 2) + atan(double(queuey[bot] - (aimy)) / double(queuex[bot] - (aimx)));
	}
	else if (queuex[bot] > aimx)
	{
		e = 3.1415926 + atan(double(queuey[bot] - (aimy)) / double(queuex[bot] - (aimx)));
	}
	else if (queuey[bot] < aimy)
	{
		e = 3.1415926 / 2;
	}
	else
	{
		e = 3 * 3.1415926 / 2;
	}
	if (judgewall3(api, map, e, dis, x * 1000 + 500, y * 1000 + 500, 500) == 0
		&& judgewall3(api, map, e, dis, x * 1000 + 500, y * 1000 + 500, 1000) == 0)
	{
		free(queuex);
		free(queuey);
		free(depth);
		free(root);
		return e;
	}
	e = 0;
	for (; (flag == 0 || dep >= depth[bot]) && bot <= top; bot++)
	{
		if (dep < depth[bot])
		{
			dep = depth[bot];
		}
		if (queuex[bot] == aimx && queuey[bot] == aimy)
		{
			flag = 1;
			e = root[bot];
		}
		if (queuex[bot] + 1 < 50)
		{
			if (map[queuex[bot] + 1][queuey[bot]] != 5 && map[queuex[bot] + 1][queuey[bot]] != 6 && pass[queuex[bot] + 1][queuey[bot]] == 0)
			{
				top++;
				queuex[top] = queuex[bot] + 1;
				queuey[top] = queuey[bot];
				if (dep == 0)
				{
					root[top] = 0;
				}
				else
				{
					root[top] = root[bot];
				}
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuex[bot] - 1 >= 0)
		{
			if (map[queuex[bot] - 1][queuey[bot]] != 5 && map[queuex[bot] - 1][queuey[bot]] != 6 && pass[queuex[bot] - 1][queuey[bot]] == 0)
			{
				top++;
				queuex[top] = queuex[bot] - 1;
				queuey[top] = queuey[bot];
				if (dep == 0)
				{
					root[top] = 3.1415926;
				}
				else
				{
					root[top] = root[bot];
				}
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuey[bot] + 1 < 50)
		{
			if (map[queuex[bot]][queuey[bot] + 1] != 5 && map[queuex[bot]][queuey[bot] + 1] != 6 && pass[queuex[bot]][queuey[bot] + 1] == 0)
			{
				top++;
				queuex[top] = queuex[bot];
				queuey[top] = queuey[bot] + 1;
				if (dep == 0)
				{
					root[top] = 3.1415926 / 2;
				}
				else
				{
					root[top] = root[bot];
				}
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuey[bot] - 1 >= 0)
		{
			if (map[queuex[bot]][queuey[bot] - 1] != 5 && map[queuex[bot]][queuey[bot] - 1] != 6 && pass[queuex[bot]][queuey[bot] - 1] == 0)
			{
				top++;
				queuex[top] = queuex[bot];
				queuey[top] = queuey[bot] - 1;
				if (dep == 0)
				{
					root[top] = 3 * 3.1415926 / 2;
				}
				else
				{
					root[top] = root[bot];
				}
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
	}
	free(queuex);
	free(queuey);
	free(depth);
	free(root);

	if (flag == 1)
	{
		return e;
	}
	else
	{
		return -1;
	}
}

Vector bush(int x, int y, int* map[], IAPI& api)
{
	int  top = 0, bot = 0, flag = 0, dep = 0;
	int* queuex = (int*)malloc(sizeof(int) * 30000);
	int* queuey = (int*)malloc(sizeof(int) * 30000);
	int* depth = (int*)malloc(sizeof(int) * 30000);
	int pass[50][50] = { 0 };
	double e = 0;
	*queuex = x;
	*queuey = y;
	*depth = 0;
	pass[x][y] = 1;
	Vector aim;
	aim.x = 0, aim.y = 0;
	for (; (flag == 0 || dep >= depth[bot]) && bot <= top; bot++)
	{
		if (dep < depth[bot])
		{
			dep = depth[bot];
		}
		if (map[queuex[bot]][queuey[bot]] >= 1 && map[queuex[bot]][queuey[bot]] <= 3)
		{
			flag = 1;
			aim.x = queuex[bot];
			aim.y = queuey[bot];
		}
		if (queuex[bot] + 1 < 50)
		{
			if (map[queuex[bot] + 1][queuey[bot]] != 5 && map[queuex[bot] + 1][queuey[bot]] != 6 && pass[queuex[bot] + 1][queuey[bot]] == 0)
			{
				top++;
				queuex[top] = queuex[bot] + 1;
				queuey[top] = queuey[bot];
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuex[bot] - 1 >= 0)
		{
			if (map[queuex[bot] - 1][queuey[bot]] != 5 && map[queuex[bot] - 1][queuey[bot]] != 6 && pass[queuex[bot] - 1][queuey[bot]] == 0)
			{
				top++;
				queuex[top] = queuex[bot] - 1;
				queuey[top] = queuey[bot];
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuey[bot] + 1 < 50)
		{
			if (map[queuex[bot]][queuey[bot] + 1] != 5 && map[queuex[bot]][queuey[bot] + 1] != 6 && pass[queuex[bot]][queuey[bot] + 1] == 0)
			{
				top++;
				queuex[top] = queuex[bot];
				queuey[top] = queuey[bot] + 1;
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuey[bot] - 1 >= 0)
		{
			if (map[queuex[bot]][queuey[bot] - 1] != 5 && map[queuex[bot]][queuey[bot] - 1] != 6 && pass[queuex[bot]][queuey[bot] - 1] == 0)
			{
				top++;
				queuex[top] = queuex[bot];
				queuey[top] = queuey[bot] - 1;
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
	}
	free(queuex);
	free(queuey);
	free(depth);
	return aim;
}

int stepbush(int x, int y, int* map[], IAPI& api)
{
	int  top = 0, bot = 0, flag = 0, dep = 0;
	int* queuex = (int*)malloc(sizeof(int) * 30000);
	int* queuey = (int*)malloc(sizeof(int) * 30000);
	int* depth = (int*)malloc(sizeof(int) * 30000);
	int pass[50][50] = { 0 };
	double e = 0;
	*queuex = x;
	*queuey = y;
	*depth = 0;
	pass[x][y] = 1;
	int step;
	for (; (flag == 0 || dep >= depth[bot]) && bot <= top; bot++)
	{
		if (dep < depth[bot])
		{
			dep = depth[bot];
		}
		if (map[queuex[bot]][queuey[bot]] >= 1 && map[queuex[bot]][queuey[bot]] <= 3)
		{
			flag = 1;
			step = dep;
		}
		if (queuex[bot] + 1 < 50)
		{
			if (map[queuex[bot] + 1][queuey[bot]] != 5 && map[queuex[bot] + 1][queuey[bot]] != 6 && pass[queuex[bot] + 1][queuey[bot]] == 0)
			{
				top++;
				queuex[top] = queuex[bot] + 1;
				queuey[top] = queuey[bot];
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuex[bot] - 1 >= 0)
		{
			if (map[queuex[bot] - 1][queuey[bot]] != 5 && map[queuex[bot] - 1][queuey[bot]] != 6 && pass[queuex[bot] - 1][queuey[bot]] == 0)
			{
				top++;
				queuex[top] = queuex[bot] - 1;
				queuey[top] = queuey[bot];
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuey[bot] + 1 < 50)
		{
			if (map[queuex[bot]][queuey[bot] + 1] != 5 && map[queuex[bot]][queuey[bot] + 1] != 6 && pass[queuex[bot]][queuey[bot] + 1] == 0)
			{
				top++;
				queuex[top] = queuex[bot];
				queuey[top] = queuey[bot] + 1;
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuey[bot] - 1 >= 0)
		{
			if (map[queuex[bot]][queuey[bot] - 1] != 5 && map[queuex[bot]][queuey[bot] - 1] != 6 && pass[queuex[bot]][queuey[bot] - 1] == 0)
			{
				top++;
				queuex[top] = queuex[bot];
				queuey[top] = queuey[bot] - 1;
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
	}
	free(queuex);
	free(queuey);
	free(depth);
	return dep;
}

Vector pickitem(int x, int y, int* map[], int item, IAPI& api)
{
	int  top = 0, bot = 0, flag = 0, dep = 0;
	int* queuex = (int*)malloc(sizeof(int) * 30000);
	int* queuey = (int*)malloc(sizeof(int) * 30000);
	int* depth = (int*)malloc(sizeof(int) * 30000);
	int pass[50][50] = { 0 };
	double e = 0;
	*queuex = x;
	*queuey = y;
	*depth = 0;
	pass[x][y] = 1;
	Vector aim;
	aim.x = 0, aim.y = 0;
	for (; (flag == 0 || dep >= depth[bot]) && bot <= top; bot++)
	{
		if (dep < depth[bot])
		{
			dep = depth[bot];
		}
		if (map[queuex[bot]][queuey[bot]] == item)
		{
			flag = 1;
			aim.x = queuex[bot];
			aim.y = queuey[bot];
		}
		if (queuex[bot] + 1 < 50)
		{
			if (map[queuex[bot] + 1][queuey[bot]] != 5 && map[queuex[bot] + 1][queuey[bot]] != 6 && pass[queuex[bot] + 1][queuey[bot]] == 0)
			{
				top++;
				queuex[top] = queuex[bot] + 1;
				queuey[top] = queuey[bot];
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuex[bot] - 1 >= 0)
		{
			if (map[queuex[bot] - 1][queuey[bot]] != 5 && map[queuex[bot] - 1][queuey[bot]] != 6 && pass[queuex[bot] - 1][queuey[bot]] == 0)
			{
				top++;
				queuex[top] = queuex[bot] - 1;
				queuey[top] = queuey[bot];
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuey[bot] + 1 < 50)
		{
			if (map[queuex[bot]][queuey[bot] + 1] != 5 && map[queuex[bot]][queuey[bot] + 1] != 6 && pass[queuex[bot]][queuey[bot] + 1] == 0)
			{
				top++;
				queuex[top] = queuex[bot];
				queuey[top] = queuey[bot] + 1;
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuey[bot] - 1 >= 0)
		{
			if (map[queuex[bot]][queuey[bot] - 1] != 5 && map[queuex[bot]][queuey[bot] - 1] != 6 && pass[queuex[bot]][queuey[bot] - 1] == 0)
			{
				top++;
				queuex[top] = queuex[bot];
				queuey[top] = queuey[bot] - 1;
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
	}
	free(queuex);
	free(queuey);
	free(depth);
	return aim;
}

Vector pickdisitem(int x, int y, int* map[], IAPI& api)
{
	int  top = 0, bot = 0, flag = 0, dep = 0;
	int* queuex = (int*)malloc(sizeof(int) * 30000);
	int* queuey = (int*)malloc(sizeof(int) * 30000);
	int* depth = (int*)malloc(sizeof(int) * 30000);
	int pass[50][50] = { 0 };
	double e = 0;
	*queuex = x;
	*queuey = y;
	*depth = 0;
	pass[x][y] = 1;
	Vector aim;
	aim.x = 0, aim.y = 0;
	for (; (flag == 0 || dep >= depth[bot]) && bot <= top; bot++)
	{
		if (dep < depth[bot])
		{
			dep = depth[bot];
		}
		if (map[queuex[bot]][queuey[bot]] > 1)
		{
			flag = 1;
			aim.x = queuex[bot];
			aim.y = queuey[bot];
		}
		if (queuex[bot] + 1 < 50)
		{
			if (map[queuex[bot] + 1][queuey[bot]] != 5 && map[queuex[bot] + 1][queuey[bot]] != 6 && pass[queuex[bot] + 1][queuey[bot]] == 0)
			{
				top++;
				queuex[top] = queuex[bot] + 1;
				queuey[top] = queuey[bot];
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuex[bot] - 1 >= 0)
		{
			if (map[queuex[bot] - 1][queuey[bot]] != 5 && map[queuex[bot] - 1][queuey[bot]] != 6 && pass[queuex[bot] - 1][queuey[bot]] == 0)
			{
				top++;
				queuex[top] = queuex[bot] - 1;
				queuey[top] = queuey[bot];
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuey[bot] + 1 < 50)
		{
			if (map[queuex[bot]][queuey[bot] + 1] != 5 && map[queuex[bot]][queuey[bot] + 1] != 6 && pass[queuex[bot]][queuey[bot] + 1] == 0)
			{
				top++;
				queuex[top] = queuex[bot];
				queuey[top] = queuey[bot] + 1;
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
		if (queuey[bot] - 1 >= 0)
		{
			if (map[queuex[bot]][queuey[bot] - 1] != 5 && map[queuex[bot]][queuey[bot] - 1] != 6 && pass[queuex[bot]][queuey[bot] - 1] == 0)
			{
				top++;
				queuex[top] = queuex[bot];
				queuey[top] = queuey[bot] - 1;
				depth[top] = depth[bot] + 1;
				pass[queuex[top]][queuey[top]] = 1;
			}
		}
	}
	free(queuex);
	free(queuey);
	free(depth);
	return aim;
}

Vector distance_CPU(IAPI& api, int* map[], Vector store)
{
	int i = 0;
	int mindis = 10000;
	int dis;
	auto self = api.GetSelfInfo();
	Vector vect;
	vect.x = -1;
	vect.y = -1;
	for (i = 0; i < api.GetProps().size(); i++)
	{
		if (api.GetProps()[i]->type == THUAI5::PropType::CPU
			&& rush(api.GridToCell(self->x), api.GridToCell(self->y), api.GridToCell(api.GetProps()[i]->x), api.GridToCell(api.GetProps()[i]->y), map, api) != -1
			&& ((api.GetProps()[i]->isMoving == 0) || (api.GridToCell(api.GetProps()[i]->x) - store.x) * (api.GridToCell(api.GetProps()[i]->x) - store.x) + (api.GridToCell(api.GetProps()[i]->y) - store.y) * (api.GridToCell(api.GetProps()[i]->y) - store.y) > 225)
			&& ((api.GridToCell(api.GetProps()[i]->x) != store.x) || (api.GridToCell(api.GetProps()[i]->y) != store.y))
			&& map[api.GridToCell(api.GetProps()[i]->x)][api.GridToCell(api.GetProps()[i]->y)] != 6
			&& api.GetProps()[i]->isMoving == 0)
		{
			dis = (api.GridToCell(self->x) - api.GridToCell(api.GetProps()[i]->x)) * (api.GridToCell(self->x) - api.GridToCell(api.GetProps()[i]->x)) + (api.GridToCell(self->y) - api.GridToCell(api.GetProps()[i]->y)) * (api.GridToCell(self->y) - api.GridToCell(api.GetProps()[i]->y));
			if (dis < mindis)
			{
				mindis = dis;
				vect.x = api.GridToCell(api.GetProps()[i]->x);
				vect.y = api.GridToCell(api.GetProps()[i]->y);
			}
		}
	}
	return vect;
}

double escape(IAPI& api, int* map[], double speed)
{
	auto self = api.GetSelfInfo();
	double xself = self->x;
	double yself = self->y;
	bullet bulenemy[250];
	double step = 3000;
	double alpha = 0;
	double beta1 = 0;
	double beta2 = 0;
	double averagedistance = 0;
	double mindistance = 10000;
	double averagedistance1 = 0;
	int m = 0;
	int n = 0;
	int s = 0;
	int q = 0;
	int bulletnum = 0;
	double xt;
	double yt;
	double dis;
	trace traenemy[4];
	int r[10] = { 1,-2,3,-4,5,-6,7,-8,9,-10 };
	int i;
	for (i = bulbot; i != bultop; i = (i + 1) % 200)
	{
		if (bul[i].x != 0 && bul[i].y != 0)
		{
			if (bul[i].team != api.GetSelfInfo()->teamID && sqrt((bul[i].x - xself) * (bul[i].x - xself) + (bul[i].y - yself) * (bul[i].y - yself)) <= 4500)
			{
				bulenemy[m] = bul[i];
				m = m + 1;
			}
		}
	}
	if (m == 0)
	{
		if (api.GetSelfInfo()->teamID == 0)
		{
			for (int i = 4; i <= 7; i++)
			{
				xt = tra[i].x[0];
				yt = tra[i].y[0];
				dis = sqrt((xt - xself) * (xt - xself) + (yt - yself) * (yt - yself));
				if (dis < tra[i].range / 2)
				{
					traenemy[s] = tra[i];
					s = s + 1;
				}
			}
		}
		if (api.GetSelfInfo()->teamID == 1)
		{
			for (int i = 0; i <= 3; i++)
			{
				xt = tra[i].x[0];
				yt = tra[i].y[0];
				dis = sqrt((xt - xself) * (xt - xself) + (yt - yself) * (yt - yself));
				if (dis < tra[i].range / 2)
				{
					traenemy[s] = tra[i];
					s = s + 1;
				}
			}
		}
		for (i = 0; i <= s - 1; i++)
		{
			if (traenemy[i].bul != 0)
			{
				bulletnum = bulletnum + 1;
			}
		}
		if (bulletnum == 0)
		{
			return 0;
		}
		if (s > 0)
		{
			api.UseCommonSkill();
		}
		for (i = 0; i <= s - 1; i++)
		{
			if (xself - traenemy[i].x[0] > 0)
			{
				beta1 = atan((traenemy[i].y[0] - yself) / (traenemy[i].x[0] - xself));
			}
			if (xself - traenemy[i].x[0] < 0)
			{
				beta1 = atan((traenemy[i].y[0] - yself) / (traenemy[i].x[0] - xself)) + 3.1415926;
			}
			if (xself - traenemy[i].x[0] == 0 && yself - traenemy[i].y[0] > 0)
			{
				beta1 = 1.5707963;
			}
			if (xself - traenemy[i].x[0] == 0 && yself - traenemy[i].y[0] < 0)
			{
				beta1 = -1.5707963;
			}
			alpha = (alpha * n + beta1) / (n + 1);
			n = n + 1;
		}
		if (judgewall4(api, map, alpha, 2, 500) == 1)
		{
			for (int i = 0; i <= 20; i++)
			{
				alpha = alpha + 0.3141593;
				if (judgewall4(api, map, alpha, 2, 500) == 0)
				{
					break;
				}
			}
		}
		if (s != 0 && self->timeUntilCommonSkillAvailable <= 24000)
		{
			return alpha;
		}
		if (s == 0 || self->timeUntilCommonSkillAvailable > 24000)
			return 0;
	}
	if (self->timeUntilCommonSkillAvailable == 0)
	{
		api.UseCommonSkill();
	}
	for (i = 0; i <= m - 1; i++)
	{
		averagedistance = (averagedistance * n + sqrt((bulenemy[i].x - xself) * (bulenemy[i].x - xself) + (bulenemy[i].y - yself) * (bulenemy[i].y - yself))) / (n + 1);
		if (sqrt((bulenemy[i].x - xself) * (bulenemy[i].x - xself) + (bulenemy[i].y - yself) * (bulenemy[i].y - yself)) < mindistance)
		{
			mindistance = sqrt((bulenemy[i].x - xself) * (bulenemy[i].x - xself) + (bulenemy[i].y - yself) * (bulenemy[i].y - yself));
			q = i;
		}
		n = n + 1;
	}
	n = 0;
	if (mindistance <= 2000)
	{
		if (xself - bulenemy[q].x > 0)
		{
			beta1 = atan((bulenemy[q].y - yself) / (bulenemy[q].x - xself));
		}
		if (xself - bulenemy[q].x < 0)
		{
			beta1 = atan((bulenemy[q].y - yself) / (bulenemy[q].x - xself)) + 3.1415926;
		}
		alpha = beta1;
		if (judgewall4(api, map, alpha, 2, 500) == 1)
		{
			for (int i = 0; i <= 9; i++)
			{
				alpha = alpha + 0.3141593 * r[i];
				if (judgewall4(api, map, alpha, 2, 500) == 0)
				{
					break;
				}
			}
		}
	}
	if (mindistance > 2000)
	{
		for (i = 0; i <= m - 1; i++)
		{
			if (xself - bulenemy[i].x > 0)
			{
				beta1 = atan((bulenemy[i].y - yself) / (bulenemy[i].x - xself));
			}
			if (xself - bulenemy[i].x < 0)
			{
				beta1 = atan((bulenemy[i].y - yself) / (bulenemy[i].x - xself)) + 3.1415926;
			}
			alpha = (alpha * n + beta1) / (n + 1);
			beta2 = (beta2 * n + bulenemy[i].e) / (n + 1);
			n = n + 1;
		}
		if (judgewall4(api, map, alpha + 1.5707963, 2, 500) == 0 && abs(alpha + 1.5707963 - beta2) > 1.5707963)
		{
			alpha = alpha + 1.5707963;
		}
		if (judgewall4(api, map, alpha - 1.5707963, 2, 500) == 0 && abs(alpha - 1.5707963 - beta2) > 1.5707963)
		{
			alpha = alpha - 1.5707963;
		}
		if (judgewall4(api, map, alpha + 1.5707963, 2, 500) == 1 && abs(alpha + 1.5707963 - beta2) > 1.5707963)
		{
			alpha = alpha + 1.5707963;
			for (int i = 0; i <= 9; i++)
			{
				alpha = alpha + 0.3141593 * r[i];
				if (judgewall4(api, map, alpha, 2, 500) == 0)
				{
					break;
				}
			}
		}
		if (judgewall4(api, map, alpha - 1.5707963, 2, 500) == 1 && abs(alpha + 1.5707963 - beta2) > 1.5707963)
		{
			alpha = alpha - 1.5707963;
			for (int i = 0; i <= 9; i++)
			{
				alpha = alpha + 0.3141593 * r[i];
				if (judgewall4(api, map, alpha, 2, 500) == 0)
				{
					break;
				}
			}
		}
	}
	api.MovePlayer(step / speed, alpha);
	return 1;
}

void AI::play(IAPI& api)
{
	//获得信息
	//&&(self->playerID==0|| self->playerID == 1)
	auto self = api.GetSelfInfo();
	if ((self->teamID == 0 || self->teamID == 1))
	{
		//获取机器人信息
		auto Robots = api.GetRobots();

		//获取场上的道具信息
		auto Props = api.GetProps();
		auto cpu = THUAI5::PropType::CPU;
		auto battery = THUAI5::PropType::Battery;
		auto booster = THUAI5::PropType::Booster;
		auto shield = THUAI5::PropType::Shield;
		auto ShieldBreaker = THUAI5::PropType::ShieldBreaker;

		double movespeed = self->speed / 1000;
		int x = api.GridToCell(self->x), y = api.GridToCell(self->y);
		int aimx, aimy;
		int i, j;
		int face = 0;
		int* map[50];
		int* map1[50];
		int* itemmap[50];
		int itemnum[10] = { 0 };
		int* p, * q;
		p = (int*)malloc(sizeof(int));
		q = (int*)malloc(sizeof(int));
		int frame;
		frame = api.GetFrameCount();
		Vector area, store;
		area.x = 0, area.y = 0;
		store.x = 0, store.y = 0;
		for (i = 0; i < 50; i++)
		{
			map[i] = (int*)malloc(sizeof(int) * 50);
			map1[i] = (int*)malloc(sizeof(int) * 50);
			itemmap[i] = (int*)malloc(sizeof(int) * 50);
			for (j = 0; j < 50; j++)
			{
				itemmap[i][j] = 0;
			}
		}

		double fflag = escape(api, map, movespeed);

		//存储地图
		for (i = 0; i < 50; i++)
		{
			for (j = 0; j < 50; j++)
			{
				auto PlaceType = api.GetPlaceType(i, j);
				if (PlaceType == THUAI5::PlaceType::CPUFactory)
				{
					map[i][j] = 4;
				}
				else if (PlaceType == THUAI5::PlaceType::Wall)
				{
					map[i][j] = 5;
				}
				else if (int(PlaceType) >= 5 && int(PlaceType) <= 12 && int(PlaceType) != (5 + self->playerID + self->teamID * 4))
				{
					map[i][j] = 6;
				}
				else if (PlaceType == THUAI5::PlaceType::BlindZone1)
				{
					map[i][j] = 1;
				}
				else if (PlaceType == THUAI5::PlaceType::BlindZone2)
				{
					map[i][j] = 2;
				}
				else if (PlaceType == THUAI5::PlaceType::BlindZone3)
				{
					map[i][j] = 3;
				}
				else
				{
					map[i][j] = 0;
				}
				map1[i][j] = map[i][j];
				if (int(PlaceType) == 5 && self->teamID == 0)
				{
					store.x = i;
					store.y = j;
				}
				if (int(PlaceType) == 12 && self->teamID == 1)
				{
					store.x = i;
					store.y = j;
				}
			}
		}
		for (i = 0; i < api.GetRobots().size(); i++)
		{
			map[api.GridToCell(api.GetRobots()[i]->x)][api.GridToCell(api.GetRobots()[i]->y)] = 6;
			map1[api.GridToCell(api.GetRobots()[i]->x)][api.GridToCell(api.GetRobots()[i]->y)] = 6;
		}
		for (i = 0; i < 50; i++)
		{
			for (j = 1; j < 48; j++)
			{
				if (map[i][j - 1] == 5 && map[i][j + 1] == 5 && map[i][j] != 5)
				{
					map[i][j] = 6;
					map1[i][j] = 6;
				}
				if (map[j - 1][i] == 5 && map[j + 1][i] == 5 && map[j][i] != 5)
				{
					map[j][i] = 6;
					map1[j][i] = 6;
				}
			}
		}
		for (i = 0; i < Props.size(); i++)
		{
			if (Props[i]->type == battery)
			{
				itemmap[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] = 1;
				itemnum[1]++;
			}
			if (Props[i]->type == booster)
			{
				itemmap[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] = 2;
				itemnum[2]++;
			}
			if (Props[i]->type == shield)
			{
				itemmap[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] = 3;
				itemnum[3]++;
			}
			if (Props[i]->type == ShieldBreaker)
			{
				itemmap[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] = 4;
				itemnum[4]++;
			}
			if (Props[i]->type == cpu)
			{
				itemmap[api.GridToCell(Props[i]->x)][api.GridToCell(Props[i]->y)] = 5;
				itemnum[5]++;
			}
		}
		int ID = self->playerID + self->teamID * 4;

		//记录各单位轨迹
		record(api);
		if (stuck(ID) == 1)
		{
			adjust(map, x, y, movespeed, ID, api);
			if (api.GetSelfInfo()->x == tra[ID].x[0] && api.GetSelfInfo()->y == tra[ID].y[0])
			{
				adjust1(movespeed, api);
			}
		}

		//寻找最大CPU工厂
		for (int i = 0; i <= 10; i++)
		{
			maxarea(map1, p, q, 0, 0);
			if (step == 1)
			{
				break;
			}
			if (step > face)
			{
				face = step;
				area.x = *p;
				area.y = *q;
			}
			step = 1;
		}


		self = api.GetSelfInfo();
		for (i = 0; i < 7; i++)
		{
			if (distance(self->x, self->y, tra[i].x[0], tra[i].y[0]) <= tra[i].range && tra[i].x[0] != 0 && tra[i].y[0] != 0)
			{
				api.UseCommonSkill();
			}
		}
		Props = api.GetProps();

		if (stepbush(x, y, map, api) <= 3 && movespeed <= 5 && (map[x][y] < 1 || map[x][y]>3))
		{
			Vector aim = bush(x, y, map, api);
			if (aim.x != x || aim.y != y)
			{
				double e = rush(x, y, aim.x, aim.y, map, api);

				if (fflag != 1)
					api.MovePlayer(int(1000 / movespeed), e);
			}
		}
		else if (frame >= 10200 && int(api.GetPlaceType(store.x, store.y)) == (5 + ID) && eated == 0)
		{
			if (itemmap[store.x][store.y] == 5)
			{
				int flag = 1;
				if (x != store.x || y != store.y)
				{
					double e = rush(x, y, store.x, store.y, map, api);
					api.MovePlayer(int(1000 / movespeed), e);
				}
				else if (api.GetSelfInfo()->isResetting == 0)
				{
					for (int i = 0; i < 100; i++)
					{
						api.Pick(cpu);
					}
					api.UseCPU(100000);
					eated = 1;
				}
			}
			else
			{
				eated = 1;
			}
		}
		else if (self->cpuNum > 0 && frame < 10200)
		{
			double dis = distance(store.x * 1000 + 500, store.y * 1000 + 500, self->x, self->y);
			double e = 0;
			if (self->x < store.x * 1000 + 500 && self->y <= store.y * 1000 + 500)
			{
				e = atan(double(self->y - (store.y * 1000 + 500)) / double(self->x - (store.x * 1000 + 500)));
			}
			else if (self->x < store.x * 1000 + 500 && self->y > store.y * 1000 + 500)
			{
				e = (3.1415926 * 2) + atan(double(self->y - (store.y * 1000 + 500)) / double(self->x - (store.x * 1000 + 500)));
			}
			else if (self->x > store.x * 1000 + 500)
			{
				e = 3.1415926 + atan(double(self->y - (store.y * 1000 + 500)) / double(self->x - (store.x * 1000 + 500)));
			}
			else if (y < store.y)
			{
				e = 3.1415926 / 2;
			}
			else
			{
				e = 3 * 3.1415926 / 2;
			}

			if (dis < 15000 && judgewall(api, map, e, dis / 1000, 800) == 0)
			{
				api.ThrowCPU(dis / 3, e, self->cpuNum);
			}
			else
			{
				double e = locate(x, y, store.x, store.y, map, api);
				api.MovePlayer(int(1000 / movespeed), e);
			}
		}
		else if (Props.size() != 0)
		{
			aimx = distance_CPU(api, map, store).x;
			aimy = distance_CPU(api, map, store).y;
			if ((x != aimx || y != aimy) && (aimx != -1 && aimy != -1) && frame < 10200)
			{
				double e = rush(x, y, aimx, aimy, map, api);
				if (fflag != 1)
					api.MovePlayer(int(1000 / movespeed), e);
			}
			else if ((x - aimx) * (x - aimx) + (y - aimy) * (y - aimy) <= 2)
			{
				api.Pick(cpu);
			}
			else if (self->life < 7000 && itemnum[1]>0)
			{
				Vector aim = pickitem(x, y, itemmap, 1, api);
				if (aim.x != x || aim.y != y)
				{
					double e = rush(x, y, aim.x, aim.y, map, api);

					if (fflag != 1)
						api.MovePlayer(int(1000 / movespeed), e);
				}
				else
				{
					api.Pick(battery);
				}
			}
			else if (self->life > 7000 && self->signalJammerNum >= 3 && self->timeUntilCommonSkillAvailable == 0 && frame < 10200)
			{
				aimx = area.x;
				aimy = area.y;
				if (aimx != x || aimy != y)
				{
					double e = rush(x, y, aimx, aimy, map, api);
					if (fflag == 1)
						api.MovePlayer(int(1000 / movespeed), e);

				}
			}
			else if (itemnum[2] > 0 || itemnum[3] > 0 || itemnum[4] > 0)
			{
				Vector aim = pickdisitem(x, y, itemmap, api);
				if (aim.x != x || aim.y != y)
				{
					double e = rush(x, y, aim.x, aim.y, map, api);

					if (fflag != 1)
						api.MovePlayer(int(1000 / movespeed), e);
				}
				else
				{
					api.Pick(booster);
					api.Pick(shield);
					api.Pick(ShieldBreaker);
				}
			}
			else if (frame < 10200)
			{
				if (self->playerID == 0 || self->playerID == 1)
				{
					aimx = area.x;
					aimy = area.y;
					if (aimx != x || aimy != y)
					{
						double e = rush(x, y, aimx, aimy, map, api);
						if (fflag == 0)
							api.MovePlayer(int(1000 / movespeed), e);
					}
				}
				else if (self->playerID == 2 || self->playerID == 3)
				{
					Vector aim = bush(x, y, map, api);
					if (aim.x != x || aim.y != y)
					{
						double e = rush(x, y, aim.x, aim.y, map, api);
						if (fflag == 0)
							api.MovePlayer(int(1000 / movespeed), e);
					}
				}
			}
			else
			{
				Vector aim = bush(x, y, map, api);
				if (aim.x != x || aim.y != y)
				{
					double e = rush(x, y, aim.x, aim.y, map, api);

					if (fflag == 0)
						api.MovePlayer(int(1000 / movespeed), e);
				}
			}
		}
		else if (frame < 10200)
		{
			aimx = area.x;
			aimy = area.y;
			if (aimx != x || aimy != y)
			{
				double e = rush(x, y, aimx, aimy, map, api);
				if (fflag == 0)
					api.MovePlayer(int(1000 / movespeed), e);
			}
		}
		else
		{
			Vector aim = bush(x, y, map, api);
			if (aim.x != x || aim.y != y)
			{
				double e = rush(x, y, aim.x, aim.y, map, api);

				if (fflag == 0)
					api.MovePlayer(int(1000 / movespeed), e);
			}
		}

		api.Pick(battery);
		api.Pick(booster);
		api.Pick(shield);
		api.Pick(ShieldBreaker);
		api.UseProp();
		if (x != store.x && y != store.y)
		{
			api.Pick(cpu);
		}
		if (api.GetFrameCount() % 3 == 0)
		{
			attack(api, map);
		}
		if (fflag != 0 && fflag != 1)
		{
			api.MovePlayer(int(1000 / movespeed), fflag);
		}
		if (frame > 10200 && (int(api.GetPlaceType(store.x, store.y)) == (5 + self->playerID + self->teamID * 4) || eated == 1))
		{
			api.UseCPU(10000);
		}

		for (i = 0; i < 50; i++)
		{
			free(map[i]);
			free(map1[i]);
			free(itemmap[i]);
		}
	}

}
