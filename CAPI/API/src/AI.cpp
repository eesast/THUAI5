#include <random>
#include "../include/AI.h"

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;

// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::ActiveSkillType playerActiveSkill = THUAI5::ActiveSkillType::NuclearWeapon;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::PassiveSkillType playerPassiveSkill = THUAI5::PassiveSkillType::RecoverAfterBattle;

namespace
{
    [[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
    [[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

int mydirection = 0;

void AI::play(IAPI& api)
{

    auto self = api.GetSelfInfo();
    if (mydirection == 0)
    {
        if (api.GetPlaceType(self->x / 1000 - 1, self->y / 1000) == THUAI5::PlaceType(1))
        {
            mydirection = 1;
        }
        else
        {
            api.MoveUp(50);
        }
    }
    if (mydirection == 1)
    {
        if (api.GetPlaceType(self->x / 1000, self->y / 1000 - 1) == THUAI5::PlaceType(1))
        {
            mydirection = 2;
        }
        else
        {
            api.MoveLeft(50);
        }
    }
    if (mydirection == 2)
    {
        if (api.GetPlaceType(self->x / 1000 + 1, self->y / 1000) == THUAI5::PlaceType(1))
        {
            mydirection = 3;
        }
        else
        {
            api.MoveDown(50);
        }
    }
    if (mydirection == 3)
    {
        if (api.GetPlaceType(self->x / 1000, self->y / 1000 + 1) == THUAI5::PlaceType(1))
        {
            mydirection = 0;
        }
        else
        {
            api.MoveRight(50);
        }
    }
    
    api.Attack(1.0);
}
