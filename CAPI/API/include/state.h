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
    std::shared_ptr<THUAI5::Robot> self;

    // 全局游戏信息
    std::vector<std::shared_ptr<THUAI5::Robot>> robots;
    std::vector<std::shared_ptr<THUAI5::Prop>> props;
    std::vector<std::shared_ptr<THUAI5::SignalJammer>> jammers;
    THUAI5::PlaceType gamemap[51][51];

    // GUID信息
    std::vector<int64_t> guids;

};

#endif // !STATE_H

