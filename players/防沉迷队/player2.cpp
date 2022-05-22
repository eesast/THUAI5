#include <random>
#include "../include/AI.h"
#include<thread>
#pragma warning(disable:C26495)
using namespace std;

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = true;


// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Invisible;	

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::PowerBank;

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}
auto Land = THUAI5::PlaceType::Land; // 空地
auto Wall = THUAI5::PlaceType::Wall; // 墙
auto BlindZone1 = THUAI5::PlaceType::BlindZone1; // 盲区1
auto BlindZone2 = THUAI5::PlaceType::BlindZone2; // 盲区2
auto BlindZone3 = THUAI5::PlaceType::BlindZone3; // 盲区3
auto BirthPlace1 = THUAI5::PlaceType::BirthPlace1; // 出生点1-8
auto BirthPlace2 = THUAI5::PlaceType::BirthPlace2;
auto BirthPlace3 = THUAI5::PlaceType::BirthPlace3;
auto BirthPlace4 = THUAI5::PlaceType::BirthPlace4;
auto BirthPlace5 = THUAI5::PlaceType::BirthPlace5;
auto BirthPlace6 = THUAI5::PlaceType::BirthPlace6;
auto BirthPlace7 = THUAI5::PlaceType::BirthPlace7;
auto BirthPlace8 = THUAI5::PlaceType::BirthPlace8;
auto cpufactory = THUAI5::PlaceType::CPUFactory; // CPU工厂
auto NullPlaceType = THUAI5::PlaceType::NullPlaceType; // 非法PlaceType
auto cpu = THUAI5::PropType::CPU;
double getdirection(int x1, int y1, int x2, int y2, IAPI& api)
{
	double x = x1 - x2;
	double y = y1 - y2;
	if (x <= 0 && y > 0)
		return 3.1415926 * 0.5 + atan((-x) / y);
	else if (x > 0 && y >= 0)
		return atan(y / x);
	else if (x < 0 && y <= 0)
		return atan(y / x) + 3.1415926;
	else if (x >= 0 && y < 0)
		return atan(x / (-y)) + 3.1415926 * 1.5;
	else
		return 0;
}
class Point
{
public:
	Point(const IAPI& api, int xa, int ya, int Ha, int Ga) :api(api), x(xa), y(ya), H(Ha), G(Ga)
	{
		this->Placetype = api.GetPlaceType(x, y);
		this->father = this;
		this->fatherplace = 5;
	}
	Point(const Point& a) :api(a.api), x(a.x), y(a.y), H(a.H), G(a.G), Placetype(a.Placetype)
	{
		if (a.father == &a)
			this->father = this, this->fatherplace = 5;
		else
			this->father = a.father, this->fatherplace = a.fatherplace;
	}
	//假如a的father指向自己，则this的father也指向自己。
	//假如a的father指向某个Point，这this的father也指向该Point
	~Point() {};
	Point Up(Point& a);
	Point Down(Point& a);
	Point Left(Point& a);
	Point Right(Point& a);
	Point UpLeft(Point& a);
	Point UpRight(Point& a);
	Point DownLeft(Point& a);
	Point DownRight(Point& a);
	Point operator=(const Point& a);
	int GetX(const Point& a);
	int GetY(const Point& a);//获取点的X与Y
	friend int operator==(const Point& a, const Point& b);
	friend int operator!=(const Point& a, const Point& b);
	int ifcanmove(const Point& a);
	const IAPI& api;
	THUAI5::PlaceType Getplacetype(const Point& a);
	int GetH(const Point& a, const Point& b);//计算H
	int H;//为此点到终点的预计耗费
	int G;//为起点到此点的耗费
	Point* father;//它的父点
	int fatherplace;
private:
	int x;
	int y;
	THUAI5::PlaceType Placetype;
};
THUAI5::PlaceType Point::Getplacetype(const Point& a)
{
	return a.Placetype;
}
int Point::GetH(const Point& a, const Point& b)
{
	return 10 * (abs(a.x - b.x) + abs(a.y - b.y));
}
Point Point::operator=(const Point& a)//调用=赋值时，左值与右值的father指向同一个Point
{
	this->x = a.x, this->y = a.y, this->G = a.G, this->H = a.H, this->Placetype = a.Placetype;
	this->father = a.father;
	this->fatherplace = a.fatherplace;
	return *this;
}
int operator==(const Point& a, const Point& b)
{
	if (b.x == (a.x) && b.y == a.y && b.Placetype == a.Placetype)
		return 1;
	else
		return 0;
}
int operator!=(const Point& a, const Point& b)
{
	if (b.x == (a.x) && b.y == a.y && b.Placetype == a.Placetype)
		return 0;
	else
		return 1;
}
int Point::GetX(const Point& a)
{
	return a.x;
}
int Point::GetY(const Point& a)
{
	return a.y;
}
Point Point::Up(Point& a)
{
	if (a.x != 0)
		return Point(a.api, (a.x - 1), a.y, 0, 0);
	else
		return Point(a.api, 0, 0, 0, 0);
}
Point Point::Down(Point& a)
{
	if (a.x != 50)
		return Point(a.api, (a.x + 1), a.y, 0, 0);
	else
		return Point(a.api, 0, 0, 0, 0);
}
Point Point::Left(Point& a)
{
	if (a.y != 0)
		return Point(a.api, a.x, a.y - 1, 0, 0);
	else
		return Point(a.api, 0, 0, 0, 0);
}
Point Point::Right(Point& a)
{
	if (a.y != 50)
		return Point(a.api, a.x, a.y + 1, 0, 0);
	else
		return Point(a.api, 0, 0, 0, 0);
}
Point Point::UpLeft(Point& a)
{
	if (a.x != 0 && a.y != 0)
		return Point(a.api, a.x - 1, a.y - 1, 0, 0);
	else
		return Point(a.api, 0, 0, 0, 0);
}
Point Point::UpRight(Point& a)
{
	if (a.x != 0 && a.y != 50)
		return Point(a.api, a.x - 1, a.y + 1, 0, 0);
	else
		return Point(a.api, 0, 0, 0, 0);
}
Point Point::DownLeft(Point& a)
{
	if (a.x != 50 && a.y != 0)
		return Point(a.api, a.x + 1, a.y - 1, 0, 0);
	else
		return Point(a.api, 0, 0, 0, 0);
}
Point Point::DownRight(Point& a)
{
	if (a.x != 50 && a.y != 50)
		return Point(a.api, a.x + 1, a.y + 1, 0, 0);
	else
		return Point(a.api, 0, 0, 0, 0);
}
int Point::ifcanmove(const Point& a)
{
	if (a.Placetype == Wall || a.Placetype == BirthPlace1 || a.Placetype == BirthPlace2 || a.Placetype == BirthPlace3 
		|| a.Placetype == BirthPlace5 || a.Placetype == BirthPlace6 || a.Placetype == BirthPlace7 )
		return 0;
	else
		return 1;
}
int* GetWalk(int* distance, int* Step, Point& start, Point& finall, Point* map)//寻路函数
{
	int breaks = 0;//调试
	int* walk1 = (int*)malloc(sizeof(int) * 1);
	//if (start.ifcanmove(start) == 0)
		//return NULL;
	if (finall.ifcanmove(finall) == 0)
		return NULL;
	Point* open = (Point*)malloc(sizeof(Point) * 110000);
	new (open) Point(start);
	if (open == NULL)
		return NULL;
	(open)->G = 0;
	(open)->H = (open)->GetH(*open, finall);
	(open)->father = open;//设置开启列表的第一个点
	int s1 = 1;//s1为开启列表的元素个数
	Point* close = (Point*)malloc(sizeof(Point) * 110000);
	if (close == NULL)
		return NULL;
	int s2 = 0;//s2为关闭列表的元素个数
	int i = 0;
	while (i == 0)
	{
		breaks++;
		if (s1 == 0)
			return NULL;//返回错误值
		int k, k1 = 0;
		Point C(*open);
		for (k = 0; k < s1; k++)//找开启列表中离终点最近的点
		{
			if (*(open + k) == finall)
			{
				int step = 0;
				*(distance) = (open + k)->G;
				Point C(*(open + k));
				while (*(C.father) != C)
				{
					step += 1;
					C = *(C.father);
				}
				if (step != 0)
				{
					*(Step) = step;
					free(walk1);
					walk1 = (int*)malloc(sizeof(int) * step);//重置walk数组
					if (walk1 == NULL)
						return NULL;
					int o;
					Point C1(*(open + k));
					for (o = (step - 1); o >= 0; o--)
					{
						*(walk1 + o) = 10 - C1.fatherplace;
						C1 = *(C1.father);
					}
				}
				else
				{
					*(Step) = 0;
					*(walk1) = 5;
				}
				free(open);
				free(close);
				//std::cout << breaks << std::endl;
				return walk1;
			}
			if ((C.G + C.H) > ((open + k)->G + (open + k)->H))
			{
				C = *(open + k), k1 = k;//k1记录C在开启列表中的位置
			}
		}
		int u = 0, d = 0, l = 0, r = 0, ul = 0, ur = 0, dl = 0, dr = 0;
		for (k = 0; k < s2; k++)//检测C点的周围是否在关闭列表内
		{
			if (C.Up(C) == *(close + k) || C.Up(C) == *(map))
				u = 1;
			if (C.Down(C) == *(close + k) || C.Down(C) == *(map))
				d = 1;
			if (C.Left(C) == *(close + k) || C.Left(C) == *(map))
				l = 1;
			if (C.Right(C) == *(close + k) || C.Right(C) == *(map))
				r = 1;
			if (C.UpLeft(C) == *(close + k) || C.UpLeft(C) == *(map))
				ul = 1;
			if (C.UpRight(C) == *(close + k) || C.UpRight(C) == *(map))
				ur = 1;
			if (C.DownLeft(C) == *(close + k) || C.DownLeft(C) == *(map))
				dl = 1;
			if (C.DownRight(C) == *(close + k) || C.DownRight(C) == *(map))
				dr = 1;
		}
		*(open + k1) = *(open + s1 - 1);
		s1 -= 1;//将C移除开启列表
		new(close + s2) Point(C);
		s2 += 1;//将C放入关闭列表
		if (u == 0 && C.Up(C).ifcanmove(C.Up(C)))
		{
			int g1 = 0;
			int g2 = -1;
			for (g1 = 0; g1 < s1; g1++)
			{
				if (*(open + g1) == C.Up(C))
					g2 = g1;
			}
			if (g2 == -1)
			{
				new(open + s1) Point(C.Up(C));
				(open + s1)->H = (open + s1)->GetH(*(open + s1), finall);
				(open + s1)->G = C.G + 10;
				(open + s1)->father = (close + s2 - 1);
				(open + s1)->fatherplace = 2;
				s1 += 1;
			}
			else if ((open + g2)->G > C.G + 10)
			{
				(open + g2)->H = C.Up(C).GetH(C.Up(C), finall);
				(open + g2)->G = C.G + 10;
				(open + g2)->father = (close + s2 - 1);
				(open + g2)->fatherplace = 2;
			}
		}
		if (d == 0 && C.Down(C).ifcanmove(C.Down(C)))
		{
			int g1 = 0;
			int g2 = -1;
			for (g1 = 0; g1 < s1; g1++)
			{
				if (*(open + g1) == C.Down(C))
					g2 = g1;
			}
			if (g2 == -1)
			{
				new(open + s1) Point(C.Down(C));
				(open + s1)->H = (open + s1)->GetH(*(open + s1), finall);
				(open + s1)->G = C.G + 10;
				(open + s1)->father = (close + s2 - 1);
				(open + s1)->fatherplace = 8;
				s1 += 1;
			}
			else if ((open + g2)->G > C.G + 10)
			{
				(open + g2)->H = C.Down(C).GetH(C.Down(C), finall);
				(open + g2)->G = C.G + 10;
				(open + g2)->father = (close + s2 - 1);
				(open + g2)->fatherplace = 8;
			}
		}
		if (l == 0 && C.Left(C).ifcanmove(C.Left(C)))
		{
			int g1 = 0;
			int g2 = -1;
			for (g1 = 0; g1 < s1; g1++)
			{
				if (*(open + g1) == C.Left(C))
					g2 = g1;
			}
			if (g2 == -1)
			{
				new(open + s1) Point(C.Left(C));
				(open + s1)->H = (open + s1)->GetH(*(open + s1), finall);
				(open + s1)->G = C.G + 10;
				(open + s1)->father = (close + s2 - 1);
				(open + s1)->fatherplace = 6;
				s1 += 1;
			}
			else if ((open + g2)->G > C.G + 10)
			{
				(open + g2)->H = C.Left(C).GetH(C.Left(C), finall);
				(open + g2)->G = C.G + 10;
				(open + g2)->father = (close + s2 - 1);
				(open + g2)->fatherplace = 6;
			}
		}
		if (r == 0 && C.Right(C).ifcanmove(C.Right(C)))
		{
			int g1 = 0;
			int g2 = -1;
			for (g1 = 0; g1 < s1; g1++)
			{
				if (*(open + g1) == C.Right(C))
					g2 = g1;
			}
			if (g2 == -1)
			{
				new(open + s1) Point(C.Right(C));
				(open + s1)->H = (open + s1)->GetH(*(open + s1), finall);
				(open + s1)->G = C.G + 10;
				(open + s1)->father = (close + s2 - 1);
				(open + s1)->fatherplace = 4;
				s1 += 1;
			}
			else if ((open + g2)->G > C.G + 10)
			{
				(open + g2)->H = C.Right(C).GetH(C.Right(C), finall);
				(open + g2)->G = C.G + 10;
				(open + g2)->father = (close + s2 - 1);
				(open + g2)->fatherplace = 4;
			}
		}
		if (u == 0 && C.UpLeft(C).ifcanmove(C.UpLeft(C)) && C.Up(C).ifcanmove(C.Up(C)) && C.Left(C).ifcanmove(C.Left(C)))
		{
			int g1 = 0;
			int g2 = -1;
			for (g1 = 0; g1 < s1; g1++)
			{
				if (*(open + g1) == C.UpLeft(C))
					g2 = g1;
			}
			if (g2 == -1)
			{
				new(open + s1) Point(C.UpLeft(C));
				(open + s1)->H = (open + s1)->GetH(*(open + s1), finall);
				(open + s1)->G = C.G + 14;
				(open + s1)->father = (close + s2 - 1);
				(open + s1)->fatherplace = 3;
				s1 += 1;
			}
			else if ((open + g2)->G > C.G + 14)
			{
				(open + g2)->H = C.UpLeft(C).GetH(C.UpLeft(C), finall);
				(open + g2)->G = C.G + 14;
				(open + g2)->father = (close + s2 - 1);
				(open + g2)->fatherplace = 3;
			}
		}
		if (u == 0 && C.UpRight(C).ifcanmove(C.UpRight(C)) && C.Up(C).ifcanmove(C.Up(C)) && C.Right(C).ifcanmove(C.Right(C)))
		{
			int g1 = 0;
			int g2 = -1;
			for (g1 = 0; g1 < s1; g1++)
			{
				if (*(open + g1) == C.UpRight(C))
					g2 = g1;
			}
			if (g2 == -1)
			{
				new(open + s1) Point(C.UpRight(C));
				(open + s1)->H = (open + s1)->GetH(*(open + s1), finall);
				(open + s1)->G = C.G + 14;
				(open + s1)->father = (close + s2 - 1);
				(open + s1)->fatherplace = 1;
				s1 += 1;
			}
			else if ((open + g2)->G > C.G + 14)
			{
				(open + g2)->H = C.UpRight(C).GetH(C.UpRight(C), finall);
				(open + g2)->G = C.G + 14;
				(open + g2)->father = (close + s2 - 1);
				(open + g2)->fatherplace = 1;
			}
		}
		if (u == 0 && C.DownLeft(C).ifcanmove(C.DownLeft(C)) && C.Down(C).ifcanmove(C.Down(C)) && C.Left(C).ifcanmove(C.Left(C)))
		{
			int g1 = 0;
			int g2 = -1;
			for (g1 = 0; g1 < s1; g1++)
			{
				if (*(open + g1) == C.DownLeft(C))
					g2 = g1;
			}
			if (g2 == -1)
			{
				new(open + s1) Point(C.DownLeft(C));
				(open + s1)->H = (open + s1)->GetH(*(open + s1), finall);
				(open + s1)->G = C.G + 14;
				(open + s1)->father = (close + s2 - 1);
				(open + s1)->fatherplace = 9;
				s1 += 1;
			}
			else if ((open + g2)->G > C.G + 14)
			{
				(open + g2)->H = C.DownLeft(C).GetH(C.DownLeft(C), finall);
				(open + g2)->G = C.G + 14;
				(open + g2)->father = (close + s2 - 1);
				(open + g2)->fatherplace = 9;
			}
		}
		if (u == 0 && C.DownRight(C).ifcanmove(C.DownRight(C)) && C.Down(C).ifcanmove(C.Down(C)) && C.Right(C).ifcanmove(C.Right(C)))
		{
			int g1 = 0;
			int g2 = -1;
			for (g1 = 0; g1 < s1; g1++)
			{
				if (*(open + g1) == C.DownRight(C))
					g2 = g1;
			}
			if (g2 == -1)
			{
				new(open + s1) Point(C.DownRight(C));
				(open + s1)->H = (open + s1)->GetH(*(open + s1), finall);
				(open + s1)->G = C.G + 14;
				(open + s1)->father = (close + s2 - 1);
				(open + s1)->fatherplace = 7;
				s1 += 1;
			}
			else if ((open + g2)->G > C.G + 14)
			{
				(open + g2)->H = C.DownRight(C).GetH(C.DownRight(C), finall);
				(open + g2)->G = C.G + 14;
				(open + g2)->father = (close + s2 - 1);
				(open + g2)->fatherplace = 7;
			}
		}
		//将C的上下左右放入开启列表中
	}

}
void move(int* Walk, std::shared_ptr<const THUAI5::Robot> self, IAPI& api)
{
	/*if (((self->x) + 500) % 1000 != 0 || ((self->y) + 500) % 1000 != 0)
	{
		int xx = api.CellToGrid(api.GridToCell(self->x));
		int yy = api.CellToGrid(api.GridToCell(self->y));
		double di = sqrt((xx - self->x) * (xx - self->x) + (yy - self->y) * (yy - self->y));
		double time = 1000 * di / (double)self->speed;
		double e = getdirection(xx, yy, self->x, self->y, api);
		api.MovePlayer((int)time, e);
	}
	else*/
		
	switch (*Walk)
	{
	case 1:
	{
		double time = 1000 * 1414 / (double)self->speed;
		time = 50;
		api.MovePlayer((int)time, 1.75 * 3.1415926);
		break;
	}
	case 2:
	{
		double time = 1000 * 1000 / (double)self->speed;
		time = 50;
		api.MovePlayer((int)time, 0);
		break;
	}
	case 3:
	{
		double time = 1000 * 1414 / (double)self->speed;
		time = 50;
		api.MovePlayer((int)time, 0.25 * 3.1415926);
		break;
	}
	case 4:
	{
		double time = 1000 * 1000 / (double)self->speed;
		time = 50;
		api.MovePlayer((int)time, 1.5 * 3.1415926);
		break;
	}
	case 6:
	{
		double time = 1000 * 1000 / (double)self->speed;
		time = 50;
		api.MovePlayer((int)time, 0.5 * 3.1415926);
		break;
	}
	case 7:
	{
		double time = 1000 * 1414 / (double)self->speed;
		time = 50;
		api.MovePlayer((int)time, 1.25 * 3.1415926);
		break;
	}
	case 8:
	{
		double time = 1000 * 1000 / (double)self->speed;
		time = 50;
		api.MovePlayer((int)time, 1. * 3.1415926);
		break;
	}
	case 9:
	{
		double time = 1000 * 1414 / (double)self->speed;
		time = 50;
		api.MovePlayer((int)time, 0.75 * 3.1415926);
		break;
	}
	default:
		break;
	}
}
void move(int* Walk, std::shared_ptr<const THUAI5::Robot> self, int step, IAPI& api)
{
	int i;
	int k = 0;
	for (i = 0; i < step; i++)
	{
		if (*(Walk + i) >= 1 && *(Walk + i) <= 9)
			k++;
		else//退出循环
			i = step;
	}
	for (i = 0; i <= k; i++)
	{
		switch (*(Walk + i))
		{
		case 1:
		{
			double time = 1000 * 1414 / (double)self->speed;
			time = 80;
			api.MovePlayer((int)time, 1.75 * 3.1415926);
			break;
		}
		case 2:
		{
			double time = 1000 * 1000 / (double)self->speed;
			time = 80;
			api.MovePlayer((int)time, 0);
			break;
		}
		case 3:
		{
			double time = 1000 * 1414 / (double)self->speed;
			time = 80;
			api.MovePlayer((int)time, 0.25 * 3.1415926);
			break;
		}
		case 4:
		{
			double time = 1000 * 1000 / (double)self->speed;
			time = 80;
			api.MovePlayer((int)time, 1.5 * 3.1415926);
			break;
		}
		case 6:
		{
			double time = 1000 * 1000 / (double)self->speed;
			time = 80;
			api.MovePlayer((int)time, 0.5 * 3.1415926);
			break;
		}
		case 7:
		{
			double time = 1000 * 1414 / (double)self->speed;
			time = 80;
			api.MovePlayer((int)time, 1.25 * 3.1415926);
			break;
		}
		case 8:
		{
			double time = 1000 * 1000 / (double)self->speed;
			time = 80;
			api.MovePlayer((int)time, 1. * 3.1415926);
			break;
		}
		case 9:
		{
			double time = 1000 * 1414 / (double)self->speed;
			time = 80;
			api.MovePlayer((int)time, 0.75 * 3.1415926);
			break;
		}
		default:
			break;
		}

	}
}
double getdirection(double x1, double y1, double x2, double y2)
{
	double x = x2 - x1;
	double y = y2 - y1;
	if (x <= 0 && y > 0)
		return 3.1415926 * 0.5 + atan((-x) / y);
	else if (x > 0 && y >= 0)
		return atan(y / x);
	else if (x < 0 && y <= 0)
		return atan(y / x) + 3.1415926;
	else if (x >= 0 && y < 0)
		return atan(x / (-y)) + 3.1415926 * 1.5;
	else
		return 0;
}
bool fun(double startx, double starty, double finalx, double finaly, Point* map, IAPI& api)   //判断两个点是否联通,参数为坐标
{
	double A, B, C, C1, C2;     //直线方程为Ax+By+C=0
	int x1 = api.GridToCell(startx);
	int y1 = api.GridToCell(starty);
	int x2 = api.GridToCell(finalx);
	int y2 = api.GridToCell(finaly);
	Point finall(*(map + x2 * 50 + y2));//finall是终点
	Point start(*(map + x1 * 50 + y1));//start是起点
	double x = finalx - startx;
	double y = finaly - starty;
	if (x == 0.0 && y == 0.0) return false;
	if (x == 0.0)
	{
		A = 1;
		B = 0;
		C = (0 - finalx);
		C1 = C - 500 * 1.4143;
		C2 = C + 500 * 1.4143;
	}
	else
	{
		B = 1;
		A = (0 - y / x);
		C = (0 - A * finalx - B * finaly);
		double C3 = 500 * 1.4143 * sqrt(A * A + B * B);
		C1 = C - C3;
		C2 = C + C3;
	}
	if (B == 0)  //如果是x=a形式
	{
		for (double i = min(starty, finaly); i <= max(starty, finaly); i += 10.0)
		{
			int x31 = api.GridToCell(finalx), x32 = api.GridToCell(finalx - 500 * 1.4143 - 1), x33 = api.GridToCell(finalx + 500 * 1.4143 + 1);
			int y3 = api.GridToCell(i);
			Point D1(*(map + x31 * 50 + y3)), D2(*(map + x32 * 50 + y3)), D3(*(map + x33 * 50 + y3));
			if (D1.Getplacetype(D1) == Wall)
				return false;
			if (D2.Getplacetype(D2) == Wall)
				return false;
			if (D3.Getplacetype(D3) == Wall)
				return false;
		}
	}
	else
	{
		for (double i = min(startx, finalx); i <= max(startx, finalx); i += 10.0)
		{
			int x3 = api.GridToCell(i);
			int y31 = api.GridToCell(0 - A * i / B - C / B), y32 = api.GridToCell(0 - A * i / B - C1 / B), y33 = api.GridToCell(0 - A * i / B - C2 / B);
			Point D1(*(map + x3 * 50 + y31)), D2(*(map + x3 * 50 + y32)), D3(*(map + x3 * 50 + y33));
			if (D1.Getplacetype(D1) == Wall)
				return false;
			if (D2.Getplacetype(D2) == Wall)
				return false;
			if (D3.Getplacetype(D3) == Wall)
				return false;
		}
	}
	return true;
}
auto LineJammer = THUAI5::SignalJammerType::LineJammer;
auto CommonJammer = THUAI5::SignalJammerType::CommonJammer;
auto FastJammer = THUAI5::SignalJammerType::FastJammer;
auto StrongJammer = THUAI5::SignalJammerType::StrongJammer;
double PointdistancePoint(int ax, int ay, int bx, int by)
{
	double x = abs(ax - bx);
	double y = abs(ay - by);
	return sqrt(x * x + y * y);
}
double PointdistanceLine(int ax, int ay, int tx, int ty, double e)
{
	double k = tan(e);
	double b = ty - tx * k;
	return (cos(e) * (ay - k * ax - b));
}
double PointdistancePoint(Point& a1, Point& a2)
{
	double x = abs(a1.GetX(a1) - a2.GetX(a2));
	double y = abs(a1.GetY(a1) - a2.GetY(a2));
	return sqrt(x * x + y * y);
}
int ifdanger(std::shared_ptr<const THUAI5::Robot> self, std::vector<std::shared_ptr<const THUAI5::SignalJammer>> singaljammers, IAPI& api)
{
	if (singaljammers.size() == 0)
	{
		return 0;
	}
	else
	{
		int i = 0;
		for (i = 0; i < singaljammers.size(); i++)
		{
			if (singaljammers[i]->type == LineJammer
				&& (PointdistanceLine(self->x, self->y, singaljammers[i]->x, singaljammers[i]->y, singaljammers[i]->facingDirection) <= 1500
				&& PointdistancePoint(self->x, self->y, singaljammers[i]->x, singaljammers[i]->y) <= 4900 || PointdistancePoint(self->x, self->y, singaljammers[i]->x, singaljammers[i]->y) <= 1500)&&singaljammers[i]->parentTeamID!=self->teamID)
			{
				return i;
			}
			if (singaljammers[i]->type == StrongJammer
				&& (PointdistanceLine(self->x, self->y, singaljammers[i]->x, singaljammers[i]->y, singaljammers[i]->facingDirection) <= 9500
				&& PointdistancePoint(self->x, self->y, singaljammers[i]->x, singaljammers[i]->y) <= 11000 || PointdistancePoint(self->x, self->y, singaljammers[i]->x, singaljammers[i]->y) <= 9500) && singaljammers[i]->parentTeamID != self->teamID)
			{
				return i;
			}
			if (singaljammers[i]->type == CommonJammer
				&& (PointdistanceLine(self->x, self->y, singaljammers[i]->x, singaljammers[i]->y, singaljammers[i]->facingDirection) <= 3500
				&& PointdistancePoint(self->x, self->y, singaljammers[i]->x, singaljammers[i]->y) <= 7500 || PointdistancePoint(self->x, self->y, singaljammers[i]->x, singaljammers[i]->y) <= 3500)&& singaljammers[i]->parentTeamID != self->teamID)
			{
				return i;
			}
			if (singaljammers[i]->type == FastJammer
				&& (PointdistanceLine(self->x, self->y, singaljammers[i]->x, singaljammers[i]->y, singaljammers[i]->facingDirection) <= 2500
				&& PointdistancePoint(self->x, self->y, singaljammers[i]->x, singaljammers[i]->y) <= 7500 || PointdistancePoint(self->x, self->y, singaljammers[i]->x, singaljammers[i]->y) <= 2500)&& singaljammers[i]->parentTeamID != self->teamID)
			{
				return i;
			}
		}
		return 0;
	}
}
/*int dodge(std::shared_ptr<const THUAI5::Robot> self, std::shared_ptr<const THUAI5::SignalJammer> jam, IAPI& api)
{
	auto wall = THUAI5::PlaceType::Wall;

	if (jam->type == LineJammer)
	{
		double e = jam->facingDirection;
		double time = 1100 * 1000 / (double)self->speed + 100;
		if (api.GetPlaceType(api.GridToCell(self->x+1000), api.GridToCell(self->y + 1000)) == wall)//右下方向左走
			api.MoveLeft(200);
		else if (api.GetPlaceType(api.GridToCell(self->x+1000), api.GridToCell(self->y - 1000)) == wall)//左下方向右
			api.MoveRight(200);
		else if (api.GetPlaceType(api.GridToCell(self->x-1000), api.GridToCell(self->y - 1000)) == wall)//左上方向右
			api.MoveRight(200);
		else if (api.GetPlaceType(api.GridToCell(self->x-1000), api.GridToCell(self->y + 1000)) == wall)//右上方向左
			api.MoveLeft(200);
		else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y)) == wall)
			api.MoveLeft(200);
		else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y)) == wall)
			api.MoveLeft(200);
		else if (api.GetPlaceType(api.GridToCell(self->x ), api.GridToCell(self->y+1000)) == wall)
			api.MoveUp(200);
		else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y-1000)) == wall)
			api.MoveUp(200);
		else	api.MovePlayer((int)time + 1, e);
		return 1;
	}
	//激光弹只需横向躲避
	if (jam->type == StrongJammer)
	{
		double e = jam->facingDirection;
		double time = 1000 * 7000 / (double)self->speed + 100;
		if (time < 1500)
		{
			if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y + 1000)) == wall)//右下方向左走
				api.MoveLeft(200);
			else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y - 1000)) == wall)//左下方向右
				api.MoveRight(200);
			else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y - 1000)) == wall)//左上方向右
				api.MoveRight(200);
			else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y + 1000)) == wall)//右上方向左
				api.MoveLeft(200);
			else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y)) == wall)
				api.MoveLeft(200);
			else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y)) == wall)
				api.MoveLeft(200);
			else if (api.GetPlaceType(api.GridToCell(self->x), api.GridToCell(self->y + 1000)) == wall)
				api.MoveUp(200);
			else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y - 1000)) == wall)
				api.MoveUp(200);
			else api.MovePlayer((int)time, e);
			std::this_thread::sleep_for(std::chrono::milliseconds(1500 - (int)time));
			return 1;
		}
		else
		{
			api.MovePlayer(1500, e);
		}
	}
	//强力干扰弹躲避后等待爆炸
	if (jam->type == FastJammer || jam->type == CommonJammer)
	{
		double ejam = jam->facingDirection;
		double dis = PointdistanceLine(self->x, self->y, jam->x, jam->y, ejam);
		if (dis <= 2000)
		{
			if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y + 1000)) == wall)//右下方向左走
				api.MoveLeft(200);
			else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y - 1000)) == wall)//左下方向右
				api.MoveRight(200);
			else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y - 1000)) == wall)//左上方向右
				api.MoveRight(200);
			else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y + 1000)) == wall)//右上方向左
				api.MoveLeft(200);
			else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y)) == wall)
				api.MoveLeft(200);
			else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y)) == wall)
				api.MoveLeft(200);
			else if (api.GetPlaceType(api.GridToCell(self->x), api.GridToCell(self->y + 1000)) == wall)
				api.MoveUp(200);
			else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y - 1000)) == wall)
				api.MoveUp(200);
			api.MovePlayer((int)time + 1, ejam);
		}
		return 1;
	}
	//快速弹与普通弹擦弹躲避
	else
	{
		return 0;
	}
}*/
int back_to_centre(std::shared_ptr<const THUAI5::Robot> self, IAPI& api)
{
	if (((self->x) + 500) % 1000 > 0 || ((self->y) + 500) % 1000 > 0)
	{
		int xx = api.CellToGrid(api.GridToCell(self->x));
		int yy = api.CellToGrid(api.GridToCell(self->y));
		double di = sqrt((xx - self->x) * (xx - self->x) + (yy - self->y) * (yy - self->y));
		double time = 500 * di / (double)self->speed;
		double e = getdirection(xx, yy, self->x, self->y, api);
		api.MovePlayer((int)time, e);
	}
	return 0;
}
void AI::play(IAPI& api)
{
	std::ios::sync_with_stdio(false);
	float distance;
	auto self = api.GetSelfInfo();//自身信息
	auto robots = api.GetRobots();//可见机器人信息
	auto Props = api.GetProps();//道具信息
	auto guid = api.GetPlayerGUIDs();//获取guid信息
	static Point* map = (Point*) operator new(sizeof(Point) * 2500);//扫描地图
	//格子是x,y
	//对应的Point是Point A1(api,x,y,0,0)
	int* path = new int;
	int* DISTANCE = new int;
	int* STEP = new int;
	int i = 0;
	double ef = 0.0;
	static int game = 0;//用来判定第一次扫描地图
	auto place = api.GetPlaceType(0, 0);
	THUAI5::PlaceType place1, place2, place3, place4;
	static int hide[50][50] = { 0 };
	static int beforex = 0, beforey = 0;
	double speed = self->speed;
	static int Frame = 0;
	static int store_x = 0, store_y = 0;
	static long long int savingtime = 0;
	static long int start_time = 0;
	if (start_time == 0)
		start_time = time(NULL);
	long int now_time = time(NULL);
	int x = api.GridToCell(self->x), y = api.GridToCell(self->y);
	Point ME(api, x, y, 0, 0);
	static int target_x = 0, target_y = 0;
	static int target[50][50] = { 0 };
	if (game == 0)
	{
		int i1 = 0;
		int i2 = 0;
		for (i1 = 0; i1 < 50; i1++)
			for (i2 = 0; i2 < 50; i2++)
			{
				place = api.GetPlaceType(i1, i2);
				if (place == BlindZone1 || place == BlindZone2 || place == BlindZone3)
					hide[i1][i2] = 1;
				if (self->teamID == 0 && place == BirthPlace4)
				{
					store_x = i1;
					store_y = i2;
				}
				/*else if (self->teamID == 0 && place == BirthPlace4)
				{
					target_x = i1;
					target_y = i2;
				}*/
				/*else if (self->teamID == 1 && place == BirthPlace8)
				{
					target_x = i1;
					target_y = i2;
				}*/
				else if (self->teamID == 1 && place == BirthPlace8)
				{
					store_x = i1;
					store_y = i2;
				}
				new(map + i1 * 50 + i2) Point(api, i1, i2, 0, 0);
			}
		game = 1;
		std::cout << "success swap the map" << std::endl;
		std::cout << std::endl;

	}
	if (target_x == 0)
	{
		int i1 = 0;
		int i2 = 0;
		for (i1 = 0; i1 < 50; i1++)
			for (i2 = 0; i2 < 50; i2++)
			{
				place = api.GetPlaceType(i1, i2);
				place1 = api.GetPlaceType(i1 + 1, i2);
				place2 = api.GetPlaceType(i1, i2 + 1);
				place3 = api.GetPlaceType(i1-1, i2);
				place4 = api.GetPlaceType(i1 , i2-1);
				distance = sqrt((i1 - store_x) * (i1 - store_x) + (i2 - store_y) * (i2 - store_y));
				if ((place == Land || place == BlindZone1 || place == BlindZone2 || place == BlindZone3 || place == cpufactory)
					&&(place1 == Land || place1 == BlindZone1 || place1 == BlindZone2 || place1 == BlindZone3 || place1 == cpufactory)
					&&(place2 == Land || place2 == BlindZone1 || place2 == BlindZone2 || place2 == BlindZone3 || place2 == cpufactory)
					&&(place3 == Land || place3 == BlindZone1 || place3 == BlindZone2 || place3 == BlindZone3 || place3 == cpufactory)
					&&(place4 == Land || place4 == BlindZone1 || place4 == BlindZone2 || place4 == BlindZone3 || place4 == cpufactory)
					&& distance <= 12)
				{
					if (fun(api.CellToGrid(i1), api.CellToGrid(i2), api.CellToGrid(store_x), api.CellToGrid(store_y), map, api)
						&& fun(api.CellToGrid(i1+1), api.CellToGrid(i2), api.CellToGrid(store_x), api.CellToGrid(store_y), map, api)
						&& fun(api.CellToGrid(i1-1), api.CellToGrid(i2), api.CellToGrid(store_x), api.CellToGrid(store_y), map, api)
						&& fun(api.CellToGrid(i1), api.CellToGrid(i2+1), api.CellToGrid(store_x), api.CellToGrid(store_y), map, api)
						&& fun(api.CellToGrid(i1), api.CellToGrid(i2-1), api.CellToGrid(store_x), api.CellToGrid(store_y), map, api))
					{
						target[i1][i2] = 1;
						//cout << "(" << i1 << "," << i2 << ") is target!" << endl;
					}
				}
			}
		target_x = 1;
	}
	int saver = 0;//确定是否是最后捡CPU	的人
	if ((self->teamID == 0 && self->playerID == 3) || self->teamID == 1 && self->playerID == 3)
		saver = 1;
	Point STORE(api, store_x, store_y, 0, 0);
	auto cpu = THUAI5::PropType::CPU;
	auto shield = THUAI5::PropType::Shield;
	auto battery = THUAI5::PropType::Battery;
	auto booster = THUAI5::PropType::Booster;
	auto shieldbreaker = THUAI5::PropType::ShieldBreaker;
	auto props = api.GetProps();
	auto robotss = api.GetRobots();
	int i0;
	float distance_i = 9999;
	int i1, i2, hidex, hidey;
	int k = 0;
	int backing = 0;//是否回家扔CPU
	static int picking = 0;//是否回家捡cpu,0为未前往，1为前往但未捡起，2为已捡起未使用，3为已使用
	static int move_saving = 0;
	if (now_time - start_time >= 540 && !picking)
	{
		picking = 1;
	}
	static int escaping = 0;
	srand(savingtime);
	if (self->canMove)
	{
		//cout << "saving is:" <<move_saving<< endl;
		auto jammer = api.GetSignalJammers();
		if (jammer.size() != 0)
		{
			int i = 0;
			if ((i = ifdanger(self, jammer, api)) != 0)
			{
				if (self->life < 2600)
					api.UseCPU(self->cpuNum);
				//dodge(self, jammer[i], api);
				ef = getdirection(self->x, self->y, jammer[i]->x, jammer[i]->y);
				if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y + 1000)) == Wall)//右下方向左走
					api.MoveLeft(150);
				else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y - 1000)) == Wall)//左下方向右
					api.MoveRight(150);
				else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y - 1000)) == Wall)//左上方向右
					api.MoveRight(150);
				else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y + 1000)) == Wall)//右上方向左
					api.MoveLeft(150);
				else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y)) == Wall)
					api.MoveLeft(150);
				else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y)) == Wall)
					api.MoveRight(150);
				else if (api.GetPlaceType(api.GridToCell(self->x), api.GridToCell(self->y + 1000)) == Wall)
					api.MoveUp(150);
				else if (api.GetPlaceType(api.GridToCell(self->x ), api.GridToCell(self->y - 1000)) == Wall)
					api.MoveDown(150);
				else api.MovePlayer(150, ef + 0.9 * 3.1415926 + (rand() % 2) *0.2* 3.1415926);
				if (escaping == 0)
				{
					escaping = 30;
				}
				//cout << "escaping!: " << escaping << endl;
			}
			else if (escaping > 0)
			{
				distance_i = 999;
				i0 = 0;
				for (i = 0; i < jammer.size(); i++)
				{
					distance = sqrt((jammer[i]->x - self->x) * (jammer[i]->x - self->x) + (jammer[i]->y - self->y) * (jammer[i]->y - self->y));
					if (distance_i > distance && jammer[i]->parentTeamID != self->teamID)
					{
						distance_i = distance;
						i0 = i;
					}
				}
				ef = getdirection(self->x, self->y, jammer[i0]->x, jammer[i0]->y);
				if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y + 1000)) == Wall)//右下方向左走
					api.MoveLeft(150);
				else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y - 1000)) == Wall)//左下方向右
					api.MoveRight(150);
				else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y - 1000)) == Wall)//左上方向右
					api.MoveRight(150);
				else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y + 1000)) == Wall)//右上方向左
					api.MoveLeft(150);
				else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y)) == Wall)
					api.MoveLeft(150);
				else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y)) == Wall)
					api.MoveLeft(150);
				else if (api.GetPlaceType(api.GridToCell(self->x), api.GridToCell(self->y + 1000)) == Wall)
					api.MoveUp(150);
				else if (api.GetPlaceType(api.GridToCell(self->x), api.GridToCell(self->y - 1000)) == Wall)
					api.MoveUp(150);
				api.MovePlayer(150, ef +0.9*3.1415926+(rand()%2)*0.2*3.1415926);
				cout << "escaping!: " << escaping << endl;
			}
		}
		else escaping = 0;
		i0 = 0;
		//cout << "escaping is:" << escaping << endl;
		if (escaping == 0)
		{
			/*if (jammer.size() != 0)
			{
				distance_i = 999;
				i0 = 0;
				for (i = 0; i < jammer.size(); i++)
				{
					distance = sqrt((jammer[i]->x - self->x) * (jammer[i]->x - self->x) + (jammer[i]->y - self->y) * (jammer[i]->y - self->y));
					if (distance_i > distance && jammer[i]->parentTeamID != self->teamID)
					{
						distance_i = distance;
						i0 = i;
					}
				}
				ef = getdirection(self->x, self->y, jammer[i0]->x, jammer[i0]->y);
				if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y + 1000)) == Wall)//右下方向左走
					api.MoveLeft(150);
				else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y - 1000)) == Wall)//左下方向右
					api.MoveRight(150);
				else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y - 1000)) == Wall)//左上方向右
					api.MoveRight(150);
				else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y + 1000)) == Wall)//右上方向左
					api.MoveLeft(150);
				else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y)) == Wall)
					api.MoveLeft(150);
				else if (api.GetPlaceType(api.GridToCell(self->x - 1000), api.GridToCell(self->y)) == Wall)
					api.MoveLeft(150);
				else if (api.GetPlaceType(api.GridToCell(self->x), api.GridToCell(self->y + 1000)) == Wall)
					api.MoveUp(150);
				else if (api.GetPlaceType(api.GridToCell(self->x + 1000), api.GridToCell(self->y - 1000)) == Wall)
					api.MoveUp(150);
				else api.MovePlayer(150, ef + 0.9 * 3.1415926 + (rand() % 2) * 0.2 * 3.1415926);
			}*/
			if (robots.size() > 0)
			{
				for (int i = 0; i < robots.size(); i++)
				{
					if ((sqrt(pow(robots[i]->x - self->x, 2) + pow(robots[i]->y - self->y, 2)) < 4000 && !robots[i]->isResetting && robots[i]->teamID != self->teamID) )
					{
						double angle = getdirection(self->x, self->y, robots[i]->x, robots[i]->y);
						api.MovePlayer(100, angle + 1.25 * 3.1415926 - 0.5 * 3.1415926 * (rand() % 2));
						break;
					}
					else if((sqrt(pow(robots[i]->x - self->x, 2) + pow(robots[i]->y - self->y, 2)) < 1250 && !robots[i]->isResetting && robots[i]->teamID == self->teamID))
					{
						double angle = getdirection(self->x, self->y, robots[i]->x, robots[i]->y);
						api.MovePlayer(200, angle + 1.25 * 3.1415926 - 0.5 * 3.1415926 * (rand() % 2));
						break;
					}
				}
			}
			if ((Frame >= 2 && beforex == self->x && beforey == self->y) || move_saving)
			{
				if (move_saving == 0)
					move_saving = 4;
				int angle = 0;//判断卡角方向，1为右下，2为右上，3为左下，4为左上
				int type = 0;//卡墙类型，当等于0时原因不是卡墙，当等于1时表示在一个角卡住，当大于1时表示多个角卡住
				auto barrier1 = api.GetPlaceType(api.GridToCell(self->x + 750), api.GridToCell(self->y + 750));//右下角
				auto barrier2 = api.GetPlaceType(api.GridToCell(self->x - 750), api.GridToCell(self->y + 750));//右上角
				auto barrier3 = api.GetPlaceType(api.GridToCell(self->x + 750), api.GridToCell(self->y - 750));//左下角
				auto barrier4 = api.GetPlaceType(api.GridToCell(self->x - 750), api.GridToCell(self->y - 750));//左上角
				if (barrier1 == Wall || barrier1 == BirthPlace1 || barrier1 == BirthPlace2 || barrier1 == BirthPlace3 || barrier1 == BirthPlace4
					|| barrier1 == BirthPlace5 || barrier1 == BirthPlace6 || barrier1 == BirthPlace7 || barrier1 == BirthPlace8)
				{
					type++;
					angle = 1;
				}
				if (barrier2 == Wall || barrier2 == BirthPlace1 || barrier2 == BirthPlace2 || barrier2 == BirthPlace3 || barrier2 == BirthPlace4
					|| barrier2 == BirthPlace5 || barrier2 == BirthPlace6 || barrier2 == BirthPlace7 || barrier2 == BirthPlace8)
				{
					type++;
					angle = 2;
				}
				if (barrier3 == Wall || barrier3 == BirthPlace1 || barrier3 == BirthPlace2 || barrier3 == BirthPlace3 || barrier3 == BirthPlace4
					|| barrier3 == BirthPlace5 || barrier3 == BirthPlace6 || barrier3 == BirthPlace7 || barrier3 == BirthPlace8)
				{
					type++;
					angle = 3;
				}
				if (barrier4 == Wall || barrier4 == BirthPlace1 || barrier4 == BirthPlace2 || barrier4 == BirthPlace3 || barrier4 == BirthPlace4
					|| barrier4 == BirthPlace5 || barrier4 == BirthPlace6 || barrier4 == BirthPlace7 || barrier4 == BirthPlace8)
				{
					type++;
					angle = 4;
				}
				if (type>=2)
				{
					back_to_centre(self,api);
				}
				else if (type==1)
				{
					switch (angle)
					{
					case 1:
						api.MovePlayer(500 * 1000 / speed, 1.35 * 3.1415926-0.2*3.1415926*(rand()%2));
						break;
					case 2:
						api.MovePlayer(500 * 1000 / speed, 1.85 * 3.1415926 - 0.2 * 3.1415926 * (rand() % 2));
						break;
					case 3:
						api.MovePlayer(500 * 1000 / speed, 0.85 * 3.1415926 - 0.2 * 3.1415926 * (rand() % 2));
						break;
					case 4:
						api.MovePlayer(500 * 1000 / speed, 0.35 * 3.1415926 - 0.2 * 3.1415926 * (rand() % 2));
						break;
					default:
						break;
					}
				}
				ef = (double)(rand() % 128) / 64;
				api.MovePlayer(25 , ef * 3.1415);
				//cout << "saving!" << endl;
			}
			if (move_saving == 0)
			{
				distance = sqrt((self->x - api.CellToGrid(store_x)) * (self->x - api.CellToGrid(store_x)) + (self->y - api.CellToGrid(store_y)) * (self->y - api.CellToGrid(store_y)));
				if (self->cpuNum >= 1 && picking == 0)
				{
					backing = 1;
					if (distance <= 14500 && fun(self->x, self->y, api.CellToGrid(store_x), api.CellToGrid(store_y), map, api))
						api.ThrowCPU(distance / 3, getdirection(self->x, self->y, api.CellToGrid(store_x), api.CellToGrid(store_y)), self->cpuNum);
					else
					{
						distance_i = 999;
						for (i1 = 0; i1 < 50; i1++)
						{
							for (i2 = 0; i2 < 50; i2++)
							{
								if (target[i1][i2])
								{
									distance = sqrt((x - i1) * (x - i1) + (y - i2) * (y - i2));
									if (distance_i > distance)
									{
										distance_i = distance;
										target_x = i1;
										target_y = i2;
									}
								}
							}
						}
						Point TARGET(api, target_x, target_y, 0, 0);
						path = GetWalk(DISTANCE, STEP, ME, TARGET, map);
						move(path, self, 1, api);
					}
					//cout << "backing!" << endl;
				}
				else
					backing = 0;
				distance = 0;
				ef = 0;
				i0 = 0;
				if (picking == 2)
				{
					cout << "using cpuNum is:" << self->cpuNum << endl;
					api.UseCPU(self->cpuNum);
					picking++;
				}
				if (picking == 1 && saver == 1)
				{
					if (x != store_x || y != store_y)
					{
						path = GetWalk(DISTANCE, STEP, ME, STORE, map);
						move(path, self, 1, api);
					}
					else
					{
						for (i0 = 0; i0 < 60; i0++)
							api.Pick(cpu);
						picking++;
					}
				}
				if (!backing && (picking != 1 || saver != 1))
				{
					if (props.size() != 0)
					{
						i0 = 0;
						distance_i = 999;
						for (i = 0; i < props.size(); i++)
						{
							i1 = api.GridToCell(props[i]->x);
							i2 = api.GridToCell(props[i]->y);
							place = api.GetPlaceType(i1, i2);
							if (props[i]->type == cpu && (place == Land || place == BlindZone1 || place == BlindZone2 || place == BlindZone3 || place == cpufactory) && !props[i]->isMoving && place != THUAI5::PlaceType::NullPlaceType)
							{
								k++;
								i1 = api.GridToCell(props[i]->x);
								i2 = api.GridToCell(props[i]->y);
								distance = sqrt((x - i1) * (x - i1) + (y - i2) * (y - i2));
								if (distance_i > distance)
								{
									distance_i = distance; i0 = i;
								}
							}
						}
						if (k > 0)
						{
							Point TARGET(api, api.GridToCell(props[i0]->x), api.GridToCell(props[i0]->y), 0, 0);
							path = GetWalk(DISTANCE, STEP, ME, TARGET, map);
							move(path, self, 1, api);
							//cout << "going for cpu!" << endl;
						}
						else
						{
							distance_i = 999;
							for (i = 0; i < props.size(); i++)
							{
								if (props[i]->type != cpu)
								{
									i1 = api.GridToCell(props[i]->x);
									i2 = api.GridToCell(props[i]->y);
									distance = sqrt((x - i1) * (x - i1) + (y - i2) * (y - i2));
									
									if (distance_i > distance)
									{
										distance_i = distance; i0 = i;
									}
								}
							}
							if (distance_i <= 15)
							{
								Point TARGET(api, api.GridToCell(props[i0]->x), api.GridToCell(props[i0]->y), 0, 0);
								path = GetWalk(DISTANCE, STEP, ME, TARGET, map);
								move(path, self, 1, api);
								//cout << "going for props!" << endl;
							}
							else
							{
								distance_i = 999;
								for (i1 = 0; i1 < 50; i1++)
								{
									for (i2 = 0; i2 < 50; i2++)
									{
										if (hide[i1][i2])
										{
											distance = sqrt((x - i1) * (x - i1) + (y - i2) * (y - i2)) + 3 * sqrt((25 - i1) * (25 - i1) + (25 - i2) * (25 - i2));
											
											if (distance_i > distance)
											{
												distance_i = distance;
												hidex = i1;
												hidey = i2;
											}
										}
									}
								}
								Point TARGET(api, hidex, hidey, 0, 0);
								path = GetWalk(DISTANCE, STEP, ME, TARGET, map);
								move(path, self, 1, api);
							}
						}
						api.Pick(cpu);
						api.Pick(shield); api.Pick(booster); api.Pick(battery);  api.Pick(shieldbreaker);
						api.UseProp();
					}
					else
					{
					distance_i = 999;
						for (i1 = 0; i1 < 50; i1++)
						{
							for (i2 = 0; i2 < 50; i2++)
							{
								if (hide[i1][i2])
								{
									distance = sqrt((x - i1) * (x - i1) + (y - i2) * (y - i2)) + 3 * sqrt((25 - i1) * (25 - i1) + (25 - i2) * (25 - i2));
									
									if (distance_i > distance)
									{
										distance_i = distance;
										hidex = i1;
										hidey = i2;
									}
								}
							}
						}
						Point TARGET(api, hidex, hidey, 0, 0);
						path = GetWalk(DISTANCE, STEP, ME, TARGET, map);
						move(path, self, 1, api);
					}
				}
				else
					backing = 0;
			}
			else
				move_saving--;
		}
		else
		{
			move_saving = 0;
			escaping--;
		}
		int len = robots.size();
		for (i = 0; i < len; i++)
		{
			if (robots[i]->teamID != self->teamID && (abs((robots[i]->x) - (self->x)) + abs((robots[i]->y) - (self->y))) <= 9000 && !robots[i]->isResetting)
				api.UseCommonSkill();
			if (robots[i]->teamID != self->teamID && (abs((robots[i]->x) - (self->x)) + abs((robots[i]->y) - (self->y))) <= 5000 && !robots[i]->isResetting
				&& fun(self->x, self->y, robots[i]->x, robots[i]->y, map, api))
			{
				self = api.GetSelfInfo();
				ef = getdirection(self->x, self->y, robots[i]->x, robots[i]->y);
				if (self->signalJammerNum >= 3)
				{
					api.Attack(ef + 0.39);
					api.Attack(ef - 0.39);
					api.Attack(ef);
				}
				else if (self->signalJammerNum == 2)
				{
					api.Attack(ef + 0.2984);
					api.Attack(ef - 0.2984);
				}
			}
		}
	}
	else//重置开关
	{
		escaping = 0;
		move_saving = 0;
	}
	if (Frame >= 2)
	{
		Frame = 0;
		beforex = self->x;
		beforey = self->y;
	}
	if (self->canMove)
		Frame++;
	if (move_saving < 0)
		move_saving = 0;
	if (escaping < 0)
		escaping = 0;
	savingtime++;
	delete path;
	delete DISTANCE;
	delete STEP;
	//cout << "Frame is: " << savingtime << endl;
}
