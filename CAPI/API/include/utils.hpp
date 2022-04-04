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

// 用于将protobuf类型转换为THUAI5类型的字典
// C++的map有一个很好的地方：当找不到匹配的key时，会默认返回0（此处就是Nullxxxtype）
inline std::map<Protobuf::ActiveSkillType,THUAI5::SoftwareType> _softwaredict
{
    {Protobuf::ActiveSkillType::BecomeAssassin, THUAI5::SoftwareType::Invisible},
    {Protobuf::ActiveSkillType::BecomeVampire, THUAI5::SoftwareType::PowerEmission},
    {Protobuf::ActiveSkillType::SuperFast, THUAI5::SoftwareType::Booster},
    {Protobuf::ActiveSkillType::NuclearWeapon, THUAI5::SoftwareType::Amplification},
};

inline std::map<Protobuf::PassiveSkillType, THUAI5::HardwareType> _hardwaredict
{
    {Protobuf::PassiveSkillType::RecoverAfterBattle,THUAI5::HardwareType::PowerBank},
    {Protobuf::PassiveSkillType::SpeedUpWhenLeavingGrass,THUAI5::HardwareType::EnergyConvert},
    {Protobuf::PassiveSkillType::Vampire,THUAI5::HardwareType::EmissionAccessory}
};

inline std::map<Protobuf::PropType, THUAI5::PropType> _propdict
{
    {Protobuf::PropType::Gem,THUAI5::PropType::CPU},
    {Protobuf::PropType::addSpeed,THUAI5::PropType::Booster},
    {Protobuf::PropType::addLIFE,THUAI5::PropType::Battery},
    {Protobuf::PropType::Shield,THUAI5::PropType::Shield},
    {Protobuf::PropType::Spear,THUAI5::PropType::ShieldBreaker}
};

inline std::map<Protobuf::BulletType, THUAI5::SignalJammerType> _jammerdict
{
    {Protobuf::BulletType::LineBullet,THUAI5::SignalJammerType::LineJammer},
    {Protobuf::BulletType::OrdinaryBullet,THUAI5::SignalJammerType::CommonJammer},
    {Protobuf::BulletType::FastBullet,THUAI5::SignalJammerType::FastJammer},
    {Protobuf::BulletType::AtomBomb,THUAI5::SignalJammerType::StrongJammer},
};

inline std::map<Protobuf::BuffType, THUAI5::BuffType> _buffdict
{
    {Protobuf::BuffType::MoveSpeed,THUAI5::BuffType::MoveSpeed},
    {Protobuf::BuffType::AddLIFE,THUAI5::BuffType::AddLIFE},
    {Protobuf::BuffType::ShieldBuff,THUAI5::BuffType::ShieldBuff},
    {Protobuf::BuffType::SpearBuff,THUAI5::BuffType::SpearBuff}
};

inline std::map<Protobuf::PlaceType, THUAI5::PlaceType> _placedict
{
    {Protobuf::PlaceType::Land,THUAI5::PlaceType::Land},
    {Protobuf::PlaceType::Grass1,THUAI5::PlaceType::BlindZone1},
    {Protobuf::PlaceType::Grass2,THUAI5::PlaceType::BlindZone2},
    {Protobuf::PlaceType::Grass3,THUAI5::PlaceType::BlindZone3},
};

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
        robot->signalJammerType = _jammerdict[c.bullettype()];
        robot->canMove = c.canmove();
        robot->CD = c.cd();
        robot->cpuNum = c.gemnum();
        robot->guid = c.guid();
        robot->isResetting = c.isresetting();
        robot->life = c.life();
        robot->lifeNum = c.lifenum();
        robot->hardwareType = _hardwaredict[c.passiveskilltype()];
        robot->place = _placedict[c.place()];
        robot->playerID = c.playerid();
        robot->prop = _propdict[c.prop()];
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
        jammer->place = _placedict[b.place()];
        jammer->type = _jammerdict[b.type()];
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
        prop->place = _placedict[p.place()];
        prop->size = p.size();
        prop->type = _propdict[p.type()];
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
            placetype = THUAI5::PlaceType::BirthPlace1;
            break;

        case 6:
            placetype = THUAI5::PlaceType::BirthPlace2;
            break;

        case 7:
            placetype = THUAI5::PlaceType::BirthPlace3;
            break;

        case 8:
            placetype = THUAI5::PlaceType::BirthPlace4;
            break;

        case 9:
            placetype = THUAI5::PlaceType::BirthPlace5;
            break;

        case 10:
            placetype = THUAI5::PlaceType::BirthPlace6;
            break;

        case 11:
            placetype = THUAI5::PlaceType::BirthPlace7;
            break;

        case 12:
            placetype = THUAI5::PlaceType::BirthPlace8;
            break;

        case 13:
            placetype = THUAI5::PlaceType::CPUFactory;
            break;
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
        if(!self) // 防止在第一帧出现空指针
        {
            return false;
        }
        if (c.isinvisible()) // 如果人物本身是隐身状态，将不可见
        {
            return false;
        }
        if (c.place() == Protobuf::PlaceType::Grass1 ||c.place() == Protobuf::PlaceType::Grass2 || c.place() == Protobuf::PlaceType::Grass3) // 人物在草丛中
        {
            if (self->place == _placedict[c.place()]) // 在同一片草丛中则可视
            {
                return true;
            }
            else // 不在同一片草丛中，不可视
            {
                return false;
            }
        }
        return true;
    }

    inline bool visible(std::shared_ptr<THUAI5::Robot> self, const Protobuf::MessageOfBullet& b)
    {
        if (b.place() == Protobuf::PlaceType::Grass1 || b.place() == Protobuf::PlaceType::Grass2 || b.place() == Protobuf::PlaceType::Grass3) // 子弹在草丛中
        {
            if (self->place == _placedict[b.place()]) // 在同一片草丛中则可视
            {
                return true;
            }
            else // 不在同一片草丛中，不可视
            {
                return false;
            }
        }
        return true;
    }

    inline bool visible(std::shared_ptr<THUAI5::Robot> self, const Protobuf::MessageOfProp& p)
    {
        if (p.place() == Protobuf::PlaceType::Grass1 || p.place() == Protobuf::PlaceType::Grass2 || p.place() == Protobuf::PlaceType::Grass3) // 道具在草丛中
        {
            if (self->place == _placedict[p.place()]) // 在同一片草丛中则可视
            {
                return true;
            }
            else // 不在同一片草丛中，不可视
            {
                return false;
            }
        }
        return true;
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