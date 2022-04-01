#pragma once
#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <cstdint>
#include <array>
#include <map>

namespace THUAI5
{
    /// <summary>
    /// 道具
    /// </summary>
    enum class PropType :unsigned char
    {
        NullPropType = 0,
        addElectricity = 1,
        addAP = 2,
        Booster = 3,
        Battery = 4,
        minusCD = 5,
        CPU = 6,    
        Shield = 7,
        ShieldBreaker = 8,
        minusSpeed = 9,
        minusAP = 10,
        addCD = 11
    };

    /// <summary>
    /// 游戏实体的形状标志
    /// </summary>
    enum class ShapeType :unsigned char
    {
        NullShapeType = 0,
        Circle = 1,
        Square = 2
    };

    /// <summary>
    /// 位置标志
    /// </summary>
    enum class PlaceType :unsigned char
    {
        Land = 0, // 空地
        Wall = 1, // 墙
        BlindZone1 = 2, // 盲区1
        BlindZone2 = 3, // 盲区2
        BlindZone3 = 4, // 盲区3
        BirthPlace = 5, // 出生点
        CPUFactory = 6 // CPU工厂
    };

    /// <summary>
    /// 信号干扰器
    /// </summary>
    enum class SignalJammerType :unsigned char
    {
        NullJammerType = 0, 
        CommonJammer1 = 1, // 普通信号干扰器
        CommonJammer2 = 2, // 普通信号干扰器
        FastJammer = 3, // 快速信号干扰器
        OrdinaryJammer = 4, 
        StrongJammer = 5 // 强力信号干扰器
    };

    /// <summary>
    /// buff
    /// </summary>
    enum class BuffType :unsigned char
    {
        NullBuffType = 0,
        MoveSpeed = 1, // 加速
        AP = 2, // 增加攻击力
        CD = 3, // 减少技能冷却时间
        AddLIFE = 4, // 增加电量
        ShieldBuff = 5, // 护盾
        SpearBuff = 6 // 破盾
    };

    /// <summary>
    ///  硬件
    /// </summary>
    enum class HardwareType :unsigned char 
    { 
        NullHardwareType = 0, 
        PowerBank = 1, // 自动充电
        EnergyConvert = 2, // 电能转化
        EmissionAccessory = 3, // 强制功率发射配件
        Hardware3 = 4, // coming soon
        Hardware4 = 5,
        Hardware5 = 6
    };

    /// <summary>
    /// 软件
    /// </summary>
    enum class SoftwareType :unsigned char
    {
        NullSoftwareType = 0, 
        PowerEmission = 1, // 功率发射软件
        Invisible = 2, // 隐身
        Amplification = 3, // 信号放大软件
        Booster = 4, // 助推器充能软件
        Software4 = 5,
        Software5 = 6
    };

    /// <summary>
    /// 机器人
    /// </summary>
    struct Robot
    {
        bool canMove;                                   // 是否可以移动
        bool isResetting;                               // 是否在复活中

        uint32_t x;                                     // x坐标
        uint32_t y;                                     // y坐标
        uint32_t signalJammerNum;                       // 信号干扰器数量 
        uint32_t speed;                                 // 机器人移动速度
        uint32_t life;                                  // 电量（生命值）
        uint32_t cpuNum;                                // CPU数
        uint32_t radius;                                // 圆形物体的半径或正方形物体的内切圆半径
        uint32_t CD;                                    // 回复一个信号干扰器需要的时间
        uint32_t lifeNum;		                        // 第几次复活
        uint32_t score;                                 // 分数

        uint64_t teamID;                                // 队伍ID
        uint64_t playerID;                              // 玩家ID
        uint64_t guid;                                  // 操作方法：Client和Server互相约定guid。非负整数中，1-8这8个guid预留给8个人物，其余在子弹或道具被创造/破坏时分发和回收。Client端用向量[guid]储存物体信息和对应的控件实例。
                                                        // 0号guid存储单播模式中每人Client对应的GUID。

        double attackRange;                             // 攻击范围
        double timeUntilCommonSkillAvailable;           // 普通软件效果的冷却时间 
        double timeUntilUltimateSkillAvailable;         // 特殊软件效果的冷却时间
        double emissionAccessory;                       // 强制功率发射配件工作效率

        std::vector<BuffType> buff;                     // 所拥有的buff
        PropType prop;                                  // 所持有的道具
        PlaceType place;                                // 机器人所在位置
        SignalJammerType signalJammerType;              // 信号干扰器类型
        HardwareType hardwareType;                      // 持有的硬件属性（被动技能） 
        SoftwareType softwareType;                      // 持有的软件属性（主动技能）
    };

    /// <summary>
    /// 墙
    /// </summary>
    struct Wall
    {
        ShapeType shapeType;                            // 墙的形状（正方形）
        uint16_t radius;                                // 圆形物体的半径或正方形内切圆半径
        uint32_t x;                                     // x坐标
        uint32_t y;                                     // y坐标
        int64_t guid;                                   // guid
    };

    /// <summary>
    /// 道具
    /// </summary>
    struct Prop
    {
        uint32_t x;                                     // x坐标
        uint32_t y;                                     // y坐标
        uint32_t size;                                  // 道具尺寸
        uint64_t guid;                                  // guid

        double facingDirection;                         // 朝向

        PropType type;                                  // 种类
        PlaceType place;                                // 道具放置位置
    };

    /// <summary>
    /// 子弹
    /// </summary>
    struct SignalJammer
    {
        uint32_t x;                                     // x坐标
        uint32_t y;                                     // y坐标

        uint64_t guid;                                  // guid
        uint64_t parentTeamID;                          // 所属队伍ID

        double facingDirection;                         // 朝向

        SignalJammerType type;                          // 信号干扰器种类
        PlaceType place;                                // 放置位置
    };

    // debug方便使用。名称可以改动

    inline std::map<THUAI5::PropType, std::string> prop_dict
    {
        { PropType::NullPropType,"NullPropType"},
        { PropType::addElectricity,"addElectricity"},
        { PropType::addAP,"addAP"},
        { PropType::Booster,"Booster"},
        { PropType::Battery ,"Battery "},
        { PropType::minusCD ,"minusCD "},
        { PropType::CPU ,"CPU "},
        { PropType::Shield ,"Shield "},
        { PropType::ShieldBreaker ,"ShieldBreaker "},
        { PropType::minusSpeed ,"minusSpeed "},
        { PropType::minusAP ,"minusAP "},
        { PropType::addCD ,"addCD "}
    };

    inline std::map<THUAI5::PlaceType, std::string> place_dict
    {
        { PlaceType::Land ,"Land "},
        { PlaceType::Wall ,"Wall "},
        { PlaceType::BlindZone1 ,"BlindZone1 "},
        { PlaceType::BlindZone2 ,"BlindZone2 "},
        { PlaceType::BlindZone3 ,"BlindZone3 "},
        { PlaceType::BirthPlace ,"BirthPlace "},
        { PlaceType::CPUFactory ,"CPUFactory "}
    };

    inline std::map<THUAI5::BuffType, std::string> buff_dict
    {
        { BuffType::NullBuffType ,"NullBuffType "},
        { BuffType::MoveSpeed ,"MoveSpeed "},
        { BuffType::AP ,"AP "},
        { BuffType::CD ,"CD "},
        { BuffType::AddLIFE ,"AddLIFE "},
        { BuffType::ShieldBuff ,"ShieldBuff "},
        { BuffType::SpearBuff ,"SpearBuff "},
    };

    inline std::map<THUAI5::SignalJammerType, std::string> jammer_dict
    {
        { SignalJammerType::NullJammerType ,"NullJammerType "},
        { SignalJammerType::CommonJammer1 ,"CommonJammer1 "},
        { SignalJammerType::CommonJammer2 ,"CommonJammer2 "},
        { SignalJammerType::FastJammer ,"FastJammer "},
        { SignalJammerType::OrdinaryJammer ,"OrdinaryJammer "},
        { SignalJammerType::StrongJammer ,"StrongJammer "}
    };

    inline std::map<THUAI5::SoftwareType, std::string> software_dict
    {
        { SoftwareType::NullSoftwareType ,"NullSoftwareType "},
        { SoftwareType::PowerEmission,"PowerEmission "},
        { SoftwareType::Invisible ,"Invisible "},
        { SoftwareType::Amplification ,"Amplification "},
        { SoftwareType::Booster ,"Booster "},
        { SoftwareType::Software4 ,"Software4 "},
        { SoftwareType::Software5 ,"Software5 "}
    };

    inline std::map<THUAI5::HardwareType, std::string> hardware_dict
    {
        { HardwareType::NullHardwareType ,"NullHardwareType "},
        { HardwareType::PowerBank ,"PowerBank "},
        { HardwareType::EnergyConvert ,"EnergyConvert "},
        { HardwareType::EmissionAccessory ,"EmissionAccessory "},
        { HardwareType::Hardware3 ,"Hardware3 "},
        { HardwareType::Hardware4 ,"Hardware4 "},
        { HardwareType::Hardware5 ,"Hardware5 "}
    };
}

#endif // !STRUCTURES_H
