#include "../include/AI.h"

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

//选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::ActiveSkillType playerActiveSkill = THUAI5::ActiveSkillType::SuperFast;

//选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::PassiveSkillType playerPassiveSkill = THUAI5::PassiveSkillType::SpeedUpWhenLeavingGrass; 

void AI::play(IAPI& api)
{

}