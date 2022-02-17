#include <random>
#include "../include/AI.h"

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;

// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::ActiveSkillType playerActiveSkill = THUAI5::ActiveSkillType::SuperFast;

// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::PassiveSkillType playerPassiveSkill = THUAI5::PassiveSkillType::SpeedUpWhenLeavingGrass; 

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

void AI::play(IAPI& api)
{
	api.MovePlayer(10, 10);
	api.MovePlayer(10, 100);
	api.MovePlayer(10, 190);
	api.MovePlayer(10, 280);
	std::cout << api.GetSelfInfo()->x << std::endl;
	std::cout << api.GetSelfInfo()->y << std::endl;
	api.Attack(10,90);
}