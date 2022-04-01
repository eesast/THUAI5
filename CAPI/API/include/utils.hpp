// 供API和Logic使用的杂项函数
#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <memory>
#include <Message2Clients.pb.h>
#include <Message2Server.pb.h>
#include <MessageType.pb.h>

#include "structures.h"

#define PROTO2THUAI_NAMESPACE_BEGIN namespace Proto2THUAI {
#define PROTO2THUAI_NAMESPACE_END }

#define TIME_NAMESPACE_BEGIN namespace Time {
#define TIME_NAMESPACE_END }

#define SPACE_NAMESPACE_BEGIN namespace Space {
#define SPACE_NAMESPACE_END }

#define VISION_NAMESPACE_BEGIN namespace Vision {
#define VISION_NAMESPACE_END }

/// <summary>
/// 辅助函数：将Proto类转换为THUAI类
/// </summary>
PROTO2THUAI_NAMESPACE_BEGIN
    /// <summary>
    /// 将protobuf类转换为THUAI5命名空间的结构体（机器人）
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    inline std::shared_ptr<THUAI5::Robot> Protobuf2THUAI5_C(const Protobuf::MessageOfCharacter& c)
    {
        std::shared_ptr<THUAI5::Robot> robot = std::make_shared<THUAI5::Robot>();
        robot->softwareType = (THUAI5::SoftwareType)c.activeskilltype();
        robot->attackRange = c.attackrange();
        for (auto it = c.buff().begin(); it != c.buff().end(); it++)
        {
            robot->buff.push_back((THUAI5::BuffType)(*it));
        }
        robot->signalJammerNum = c.bulletnum();
        robot->signalJammerType = (THUAI5::SignalJammerType)c.bullettype();
        robot->canMove = c.canmove();
        robot->CD = c.cd();
        robot->cpuNum = c.gemnum();
        robot->guid = c.guid();
        robot->isResetting = c.isresetting();
        robot->life = c.life();
        robot->lifeNum = c.lifenum();
        robot->hardwareType = (THUAI5::HardwareType)c.passiveskilltype();
        robot->place = (THUAI5::PlaceType)c.place();
        robot->playerID = c.playerid();
        robot->prop = (THUAI5::PropType)c.prop();
        robot->radius = c.radius();
        robot->score = c.score();
        robot->speed = c.speed();
        robot->teamID = c.teamid();
        robot->timeUntilCommonSkillAvailable = c.timeuntilcommonskillavailable();
        robot->timeUntilUltimateSkillAvailable = c.timeuntilultimateskillavailable();
        robot->emissionAccessory = c.vampire();
        robot->x = c.x();
        robot->y = c.y();

        return robot;
    }

    /// <summary>
    /// 将protobuf类转换为THUAI5命名空间的结构体（信号干扰器）
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    inline std::shared_ptr<THUAI5::SignalJammer> Protobuf2THUAI5_B(const Protobuf::MessageOfBullet& b)
    {
        std::shared_ptr<THUAI5::SignalJammer> jammer = std::make_shared<THUAI5::SignalJammer>();
        jammer->facingDirection = b.facingdirection();
        jammer->guid = b.guid();
        jammer->parentTeamID = b.parentteamid();
        jammer->place = (THUAI5::PlaceType)b.place();
        jammer->type = (THUAI5::SignalJammerType)b.type();
        jammer->x = b.x();
        jammer->y = b.y();

        return jammer;
    }

    /// <summary>
    /// 将protobuf类转换为THUAI5命名空间的结构体（道具）
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    inline std::shared_ptr<THUAI5::Prop> Protobuf2THUAI5_P(const Protobuf::MessageOfProp& p)
    {
        std::shared_ptr<THUAI5::Prop> prop = std::make_shared<THUAI5::Prop>();
        prop->facingDirection = p.facingdirection();
        prop->guid = p.guid();
        prop->place = (THUAI5::PlaceType)p.place();
        prop->size = p.size();
        prop->type = (THUAI5::PropType)p.type();
        prop->x = p.x();
        prop->y = p.y();

        return prop;
    }

    /// <summary>
    /// 将server端的地图信息映射为地点种类
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    inline THUAI5::PlaceType Protobuf2THUAI5_M(int32_t data)
    {
        THUAI5::PlaceType placetype = THUAI5::PlaceType::Land;
        switch (data)
        {
        case 1:
            placetype = THUAI5::PlaceType::Wall;
            break;
        
        case 2:
            placetype = THUAI5::PlaceType::BlindZone1;
            break;

        case 3:
            placetype = THUAI5::PlaceType::BlindZone2;
            break;

        case 4:
            placetype = THUAI5::PlaceType::BlindZone3;
            break;

        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            placetype = THUAI5::PlaceType::BirthPlace;
            break;

        case 13:
            placetype = THUAI5::PlaceType::CPUFactory;
        
        default:
            placetype = THUAI5::PlaceType::Land;
        }

        return placetype;
    }


PROTO2THUAI_NAMESPACE_END

/// <summary>
/// 辅助函数：管理游戏时间信息
/// </summary>
TIME_NAMESPACE_BEGIN
    inline double TimeSinceStart(const std::chrono::system_clock::time_point& sp)
    {
        std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> time_span = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(tp - sp);
        return time_span.count();
    }
TIME_NAMESPACE_END

/// <summary>
/// 辅助函数：管理游戏空间信息
/// </summary>
SPACE_NAMESPACE_BEGIN
    inline bool InSameCell(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2)
    {
        return (x1 / Constants::Map::numOfGridPerCell == x2 / Constants::Map::numOfGridPerCell) ||
               (y1 / Constants::Map::numOfGridPerCell == y2 / Constants::Map::numOfGridPerCell);
    }
SPACE_NAMESPACE_END

/// <summary>
/// 辅助函数：判断是否可见
/// </summary>
VISION_NAMESPACE_BEGIN
    /*
    * 机器人是否可见的判定机制如下：
    * 1.若机器人不在电磁屏蔽区里，则看不到软件为干扰信号软件和在电磁屏蔽区里的机器人
    * 2.若机器人在电磁屏蔽去里，则可以看得到与自己位于同一电磁屏蔽区的玩家，但是看不到软件为干扰信号软件的玩家
    */

    inline bool visible(std::shared_ptr<THUAI5::Robot> self, const Protobuf::MessageOfCharacter& c)
    {
        if(!self)
        {
            return false;
        }
        int64_t dx = self->x - c.x();
        int64_t dy = self->y - c.y();
        uint64_t distanceSquared = dx * dx + dy * dy;
        if (!(distanceSquared <= Constants::Map::sightRadiusSquared))
        {
            return false;
        }
        if (c.isinvisible())
        {
            return false;
        }
        if (c.place() == Protobuf::PlaceType::Grass1 ||c.place() == Protobuf::PlaceType::Grass2 || c.place() == Protobuf::PlaceType::Grass3) // 人物在草丛中
        {
            if (self->place == (THUAI5::PlaceType)c.place())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    inline bool visible(std::shared_ptr<THUAI5::Robot> self, const Protobuf::MessageOfBullet& b)
    {
        if(!self)
        {
            return false;
        }
        int64_t dx = self->x - b.x();
        int64_t dy = self->y - b.y();
        uint64_t distanceSquared = dx * dx + dy * dy;
        return distanceSquared <= Constants::Map::sightRadiusSquared;
    }

    inline bool visible(std::shared_ptr<THUAI5::Robot> self, const Protobuf::MessageOfProp& p)
    {
        if(!self)
        {
            return false;
        }
        int64_t dx = self->x - p.x();
        int64_t dy = self->y - p.y();
        uint64_t distanceSquared = dx * dx + dy * dy;
        return distanceSquared <= Constants::Map::sightRadiusSquared;
    }
VISION_NAMESPACE_END

#undef PROTO2THUAI_NAMESPACE_BEGIN 
#undef PROTO2THUAI_NAMESPACE_END 

#undef TIME_NAMESPACE_BEGIN 
#undef TIME_NAMESPACE_END 

#undef SPACE_NAMESPACE_BEGIN 
#undef SPACE_NAMESPACE_END 

#undef VISION_NAMESPACE_BEGIN 
#undef VISION_NAMESPACE_END 

#endif // UTILS_HPP