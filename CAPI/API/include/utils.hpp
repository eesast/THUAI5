// 供API和Logic使用的杂项函数
#pragma once
#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <memory>
#include <Message2Clients.pb.h>
#include <Message2Server.pb.h>

#include "../include/structures.h"

/// <summary>
/// 辅助函数：将Proto类转换为THUAI类
/// </summary>
namespace Proto2THUAI
{
    /// <summary>
    /// 将protobuf类转换为THUAI5命名空间的结构体（人物）
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    std::shared_ptr<THUAI5::Character> Protobuf2THUAI5_C(const Protobuf::MessageOfCharacter& c)
    {
        std::shared_ptr<THUAI5::Character> character = std::make_shared<THUAI5::Character>();
        character->activeSkillType = (THUAI5::ActiveSkillType)c.activeskilltype();
        character->attackRange = c.attackrange();
        character->buff = (THUAI5::BuffType)c.buff();
        character->bulletNum = c.bulletnum();
        character->bulletType = (THUAI5::BulletType)c.bullettype();
        character->canMove = c.canmove();
        character->CD = c.cd();
        character->gemNum = c.gemnum();
        character->guid = c.guid();
        character->isResetting = c.isresetting();
        character->life = c.life();
        character->lifeNum = c.lifenum();
        character->passiveSkillType = (THUAI5::PassiveSkillType)c.passiveskilltype();
        character->place = (THUAI5::PlaceType)c.place();
        character->playerID = c.playerid();
        character->prop = (THUAI5::PropType)c.prop();
        character->radius = c.radius();
        character->score = c.score();
        character->speed = c.speed();
        character->teamID = c.teamid();
        character->timeUntilCommonSkillAvailable = c.timeuntilcommonskillavailable();
        character->timeUntilUltimateSkillAvailable = c.timeuntilultimateskillavailable();
        character->vampire = c.vampire();
        character->x = c.x();
        character->y = c.y();

        return character;
    }

    /// <summary>
    /// 将protobuf类转换为THUAI5命名空间的结构体（子弹）
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    std::shared_ptr<THUAI5::Bullet> Protobuf2THUAI5_B(const Protobuf::MessageOfBullet& b)
    {
        std::shared_ptr<THUAI5::Bullet> bullet = std::make_shared<THUAI5::Bullet>();
        bullet->facingDirection = b.facingdirection();
        bullet->guid = b.guid();
        bullet->parentTeamID = b.parentteamid();
        bullet->place = (THUAI5::PlaceType)b.place();
        bullet->type = (THUAI5::BulletType)b.type();
        bullet->x = b.x();
        bullet->y = b.y();

        return bullet;
    }

    /// <summary>
    /// 将protobuf类转换为THUAI5命名空间的结构体（道具）
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    std::shared_ptr<THUAI5::Prop> Protobuf2THUAI5_P(const Protobuf::MessageOfProp& p)
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
}

/// <summary>
/// 辅助函数：管理游戏时间信息
/// </summary>
namespace Time
{
    double TimeSinceStart(const std::chrono::system_clock::time_point& sp)
    {
        std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> time_span = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(tp - sp);
        return time_span.count();
    }
}

/// <summary>
/// 辅助函数：管理游戏空间信息
/// </summary>
namespace Space
{
    inline bool InSameCell(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2)
    {
        return (x1 / Constants::Map::numOfGridPerCell == x2 / Constants::Map::numOfGridPerCell) ||
               (y1 / Constants::Map::numOfGridPerCell == y2 / Constants::Map::numOfGridPerCell);
    }
}

/// <summary>
/// 辅助函数：判断是否可见
/// </summary>
namespace Vision
{
    /*
    * 人物是否可见的判定机制如下：
    * 1.若人物不在草丛里，则看不到技能隐身和在草丛里的玩家
    * 2.若人物在草丛里，则可以看得到与自己位于同一草丛的玩家，但是看不到技能隐身的玩家
    */

    static bool visible(std::shared_ptr<THUAI5::Character> self, const Protobuf::MessageOfCharacter& c)
    {
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

    static bool visible(std::shared_ptr<THUAI5::Character> self, const Protobuf::MessageOfBullet& b)
    {
        int64_t dx = self->x - b.x();
        int64_t dy = self->y - b.y();
        uint64_t distanceSquared = dx * dx + dy * dy;
        return distanceSquared <= Constants::Map::sightRadiusSquared;
    }

    static bool visible(std::shared_ptr<THUAI5::Character> self, const Protobuf::MessageOfProp& p)
    {
        int64_t dx = self->x - p.x();
        int64_t dy = self->y - p.y();
        uint64_t distanceSquared = dx * dx + dy * dy;
        return distanceSquared <= Constants::Map::sightRadiusSquared;
    }
}

#endif // UTILS_HPP_