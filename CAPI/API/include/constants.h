#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H

struct StateConstant
{
	constexpr static inline int nTeams = 2; // 更加强调常量
	constexpr static inline int nPlayers = 4; // static inline 这样的常量可以被多个.cpp文件包含
	constexpr static inline int nCells = 50;
};

#endif // !CONSTANTS_H
