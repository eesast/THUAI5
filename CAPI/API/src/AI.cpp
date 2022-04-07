#include <random>
#include "../include/AI.h"
#pragma warning(disable:C26495)

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
const double PI = 3.1415926;

THUAI5::PlaceType GetPlaceTCL(IAPI& api, int32_t cellX, int32_t cellY)
{
	if (cellX < 0 || cellX >= 50 || cellY < 0 || cellY >= 50)
		return THUAI5::PlaceType::Land;
	else return api.GetPlaceType(cellX, cellY);
}

double getDirection(uint32_t selfPoisitionX, uint32_t selfPoisitionY, uint32_t aimPositionX, uint32_t aimPositionY)//获取角度的内联函数
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

double getDistance(uint32_t selfPoisitionX, uint32_t selfPoisitionY, uint32_t aimPositionX, uint32_t aimPositionY)
{
	return sqrt((aimPositionX - selfPoisitionX) * (aimPositionX - selfPoisitionX) + (aimPositionY - selfPoisitionY) * (aimPositionY - selfPoisitionY));
}
void dash(IAPI& api, uint32_t selfPoisitionX, uint32_t selfPoisitionY, uint32_t aimPositionX, uint32_t aimPositionY, uint32_t movespeed)
{
	double direction = getDirection(selfPoisitionX, selfPoisitionY, aimPositionX, aimPositionY);
	double distance = getDistance(selfPoisitionX, selfPoisitionY, aimPositionX, aimPositionY);
	api.MovePlayer(1000 * distance / movespeed, direction);
}
bool Attackornot(std::shared_ptr<const THUAI5::Robot> self, uint32_t selfPoisitionX, uint32_t selfPoisitionY, uint32_t aimPositionX, uint32_t aimPositionY)
{
	double ATTACK_R = self->attackRange;
	return ATTACK_R > sqrt((aimPositionX - selfPoisitionX) * (aimPositionX - selfPoisitionX) + (aimPositionY - selfPoisitionY) * (aimPositionY - selfPoisitionY));
}

void attackaround(IAPI& api, std::shared_ptr<const THUAI5::Robot> self)
{
	auto player = api.GetRobots();
	if (!player.empty() && self->signalJammerNum > 0)
	{
		for (int i = 0; i < player.size(); i++)
		{
			if (self->teamID != player[i]->teamID && !player[i]->isResetting && Attackornot(self, self->x, self->y, player[i]->x, player[i]->y))
			{
				uint32_t px = player[i]->x;
				uint32_t py = player[i]->y;
				double e = getDirection(self->x, self->y, px, py);
				api.Attack(e);
				api.Attack(e);
				api.Attack(e);
				api.Attack(e);
				api.Attack(e);
			}
		}
	}
}
uint32_t getCellPosition(uint32_t t)
{
	return t / 1000;
}

bool moveable_diagonal(IAPI& api, uint32_t selfPoisitionX, uint32_t selfPoisitionY, uint32_t aimPositionX, uint32_t aimPositionY)
{
	uint32_t maxX = (selfPoisitionX < aimPositionX ? aimPositionX : selfPoisitionX);
	uint32_t maxY = (selfPoisitionY < aimPositionY ? aimPositionY : selfPoisitionY);
	uint32_t minX = (selfPoisitionX > aimPositionX ? aimPositionX : selfPoisitionX);
	uint32_t minY = (selfPoisitionY > aimPositionY ? aimPositionY : selfPoisitionY);
	auto wall = THUAI5::PlaceType::Wall;
	for (int i = getCellPosition(minX); i <= getCellPosition(maxX); i++)
	{
		for (int j = getCellPosition(minY); j <= getCellPosition(maxY); j++)
		{
			if (GetPlaceTCL(api, i, j) == wall)
				return false;
		}
	}
	return true;
}
bool moveable_y(IAPI& api, uint32_t selfPoisitionX, uint32_t selfPoisitionY, uint32_t aimPositionY)
{
	uint32_t maxY = (selfPoisitionY < aimPositionY ? aimPositionY : selfPoisitionY);
	uint32_t minY = (selfPoisitionY > aimPositionY ? aimPositionY : selfPoisitionY);
	auto wall = THUAI5::PlaceType::Wall;
	for (int i = getCellPosition(minY); i <= getCellPosition(maxY); i++)
	{
		if (GetPlaceTCL(api, selfPoisitionX / 1000 + 1, i) == wall || GetPlaceTCL(api, selfPoisitionX, i) == wall || GetPlaceTCL(api, selfPoisitionX / 1000 - 1, i) == wall)
			return false;
	}
	return true;
}
bool moveable_x(IAPI& api, uint32_t selfPoisitionX, uint32_t selfPoisitionY, uint32_t aimPositionX)
{
	uint32_t maxX = (selfPoisitionX < aimPositionX ? aimPositionX : selfPoisitionX);
	uint32_t minX = (selfPoisitionX > aimPositionX ? aimPositionX : selfPoisitionX);
	auto wall = THUAI5::PlaceType::Wall;
	for (int i = getCellPosition(minX); i <= getCellPosition(maxX); i++)
	{
		if (GetPlaceTCL(api, i, selfPoisitionY / 1000 + 1) == wall || GetPlaceTCL(api, i, selfPoisitionY / 1000) == wall || GetPlaceTCL(api, i, selfPoisitionY / 1000 - 1) == wall)
			return false;
	}
	return true;
}
void hunt(IAPI& api, std::shared_ptr<const THUAI5::Robot> self, uint32_t x, uint32_t y)
{
	uint32_t selfx = self->x;
	uint32_t selfy = self->y;
	if (selfx != x && selfy != y)
	{
		if (moveable_diagonal(api, selfx, selfy, x, y))
		{
			dash(api, selfx, selfy, x, y, self->speed);
		}
		else if (selfx != x && moveable_x(api, selfx, selfy, x))
		{
			if (selfx < x)
			{
				api.MoveDown(100);
			}
			else
			{
				api.MoveUp(100);
			}
		}
		else if (selfy != y && moveable_y(api, selfx, selfy, y))
		{
			if (selfy < y)
			{
				api.MoveRight(100);
			}
			else
			{
				api.MoveLeft(100);
			}
		}
		else
		{
			bool moveable;
			if (selfy < y)
			{
				moveable = api.MoveLeft(100);
			}
			else
			{
				moveable = api.MoveRight(100);
			}
			if (!moveable && selfx > x)
			{
				api.MoveDown(100);
			}
			else
			{
				api.MoveUp(100);
			}
		}
	}
}
void AI::play(IAPI& api)
{
	std::ios::sync_with_stdio(false);
	auto self = api.GetSelfInfo();
	auto prop = api.GetProps();
    for (int i = prop.size() - 1; i > 0; i--)
    {
        if (prop[i]->x / 1000 == self->x / 1000 && prop[i]->y / 1000 == self->y / 1000)
            api.Pick(prop[i]->type);
        else
            hunt(api, self, prop[i]->x, prop[i]->y);
    }
	for (int i = 0; i < 3; i++)
	{
		std::string s = "000";
		api.Send(i, s);
	}
	if (api.MessageAvailable())
	{
		auto s = api.TryGetMessage();
		std::cout << s.value();
	}
	attackaround(api, self);
	api.UseCommonSkill();
	api.ThrowCPU(50, PI, 2);
	api.ThrowProp(50, PI);
}
