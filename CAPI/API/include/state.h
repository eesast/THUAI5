#pragma once
#ifndef STATE_H
#define STATE_H

#include <vector>
#include <cstdint>
#include <memory>

#include "structures.h"

/// <summary>
/// 存储玩家状态 
/// </summary>
struct State
{
    // 队伍分数信息
    uint32_t teamScore;

    // 自身信息
    std::shared_ptr<THUAI5::Character> self;

    // 全局游戏信息
    std::vector<std::shared_ptr<THUAI5::Character>> characters;
    std::vector<std::shared_ptr<THUAI5::Wall>> walls;
    std::vector<std::shared_ptr<THUAI5::Prop>> props;
    std::vector<std::shared_ptr<THUAI5::Bullet>> bullets;
};

#endif // !STATE_H

