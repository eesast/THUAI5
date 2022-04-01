#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define M_SCI static const constexpr inline
#define MF_SCI static constexpr inline

namespace Constants
{
    struct Map
    {
        using XYPosition = ::std::pair<::std::int32_t, ::std::int32_t>;
        M_SCI::std::uint64_t sightRadius = 5000;									
        M_SCI::std::uint64_t sightRadiusSquared = sightRadius * sightRadius;		
        M_SCI::std::int32_t numOfGridPerCell = 1000;

		[[nodiscard]] MF_SCI auto CellToGrid(int x, int y) noexcept				// 获取指定格子中心的坐标
		{
			return ::std::make_pair<::std::int32_t, ::std::int32_t>(x * numOfGridPerCell + numOfGridPerCell / 2, y * numOfGridPerCell + numOfGridPerCell / 2);
		}
		
		[[nodiscard]] MF_SCI ::std::int32_t GridToCellX(XYPosition pos) noexcept	// 获取指定坐标点所位于的格子的 X 序号
		{
			return pos.first / numOfGridPerCell;
		}
		
		[[nodiscard]] MF_SCI ::std::int32_t GridToCellY(XYPosition pos) noexcept	// 获取指定坐标点所位于的格子的 Y 序号
		{
			return pos.second / numOfGridPerCell;
		}
	};

	// software skill constants
	struct SoftwareConstants
	{
		struct Speed
		{
		private:
			M_SCI::std::int32_t BasicSpeed = Map::numOfGridPerCell;  
		public:	
			M_SCI::std::int32_t PowerEmission = 3000;    // 功率发射软件速度 3000
			M_SCI::std::int32_t Invisible = 4000;        // 干扰信号软件速度 4000
			M_SCI::std::int32_t Amplification = 2500;    // 信号放大软件速度 2500
			M_SCI::std::int32_t Booster = 3000;          // 助推器充能软件速度 3000
		};

		struct HP
		{
			M_SCI::std::int32_t PowerEmission = 9500;   // 功率发射软件最大电量  9500
			M_SCI::std::int32_t Invisible = 5000;       // 干扰信号软件最大电量  5000
			M_SCI::std::int32_t Amplification = 3600;   // 信号放大软件最大电量  3600
			M_SCI::std::int32_t Booster = 6000;         // 助推器充能软件最大电量  6000
		};

		struct SkillTime
		{
			M_SCI::std::int32_t PowerEmission = 10000;  // 功率发射软件技能持续时间  10000
			M_SCI::std::int32_t Invisible = 5000;       // 干扰信号软件技能持续时间  5000
			M_SCI::std::int32_t Amplification = 2000;   // 信号放大软件技能持续时间  2000
			M_SCI::std::int32_t Booster = 3000;         // 助推器充能软件技能持续时间  3000
		};

		M_SCI::std::int32_t AttackCD = 3000;      // 攻击冷却时间 3000
		M_SCI::std::int32_t SkillCD = 30000;      // 技能冷却时间 30000
		M_SCI::std::int32_t MaxBulletNum = 5;     // 最大子弹数  5
	};

	// hardware skill constants
	struct HardwareConstants
	{
		struct AttackRange
		{
			M_SCI::std::int32_t PowerBank = 4500;        // 自动充电装置攻击范围 4500
			M_SCI::std::int32_t EnergyConvert = 9000;    // 电能转换装置攻击范围 9000
			M_SCI::std::int32_t EmissionAccessory = 900; // 强制功率发射配件攻击范围 900
		};
	};
}
 
#endif // !CONSTANTS_H
