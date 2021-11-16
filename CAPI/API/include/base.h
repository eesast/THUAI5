#pragma once
#ifndef BASE_H
#define BASE_H

/// <summary>
/// 游戏常量
/// </summary>
struct StateConstant
{
    constexpr static inline int nTeams = 2;
    constexpr static inline int nPlayers = 4;
    constexpr static inline int nCells = 50;
};

/// <summary>
/// API通用接口，可派生为一般API和DebugAPI
/// </summary>
class IAPI
{
public:
     
private:
    
};

/// <summary>
/// AI通用接口
/// </summary>
class AIBase
{
public:
    virtual void play(IAPI& api) = 0;
};

#endif // !BASE_H
