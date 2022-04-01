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
			M_SCI::std::int32_t PowerEmission = 3000;
			M_SCI::std::int32_t Invisible = 4000;
			M_SCI::std::int32_t Amplification = 3000;
			M_SCI::std::int32_t Booster = 2500;
		};

		struct HP
		{
			M_SCI::std::int32_t PowerEmission = 9500;
			M_SCI::std::int32_t Invisible = 5000;
			M_SCI::std::int32_t Amplification = 3600;
			M_SCI::std::int32_t Booster = 6000;
		};

		struct SkillTime
		{
			M_SCI::std::int32_t PowerEmission = 10000;
			M_SCI::std::int32_t Invisible = 5000;
			M_SCI::std::int32_t Amplification = 3000;
			M_SCI::std::int32_t Booster = 2000;
		};

		M_SCI::std::int32_t AttackCD = 3000;
		M_SCI::std::int32_t SkillCD = 30000;
		M_SCI::std::int32_t MaxBulletNum = 5;
	};

	// hardware skill constants
	struct HardwareConstants
	{
		struct AttackRange
		{
			M_SCI::std::int32_t PowerBank = 4500;
			M_SCI::std::int32_t EnergyConvert = 900;
			M_SCI::std::int32_t EmissionAccessory = 90000;
		};
	};
}
 
#endif // !CONSTANTS_H
