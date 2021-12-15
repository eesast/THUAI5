#pragma once
#ifndef AI_H
#define AI_H

#include <memory>
#include "API.h"

/// <summary>
/// AI通用接口
/// </summary>
class IAI
{
public:
    virtual void play(IAPI& api) = 0;
};
using CreateAIFunc = std::unique_ptr<IAI>(*)();

//  此处应该是一个工厂模式——生产不同种类的AI

/// <summary>
/// 一般AI
/// </summary>
class AI :public IAI
{
public:
    AI() :IAI() {}
    virtual void play(IAPI& api) override;
};

/// <summary>
/// 记录AI的ID（模仿THUAI4，应该还有更好的写法）
/// </summary>
class ID
{
private:
    static inline int playerID = 0;
    static inline int teamID = 0;

public:
    [[nodiscard]] static int GetPlayerID() { return playerID; }
    [[nodiscard]] static int GetTeamID() { return teamID; }
    
    void SetPlayerID(int playerID)
    {
        this->playerID = playerID;
    }

    void SetTeamID(int teamID)
    {
        this->teamID = teamID;
    }
};

/// 命令行程序主函数
int thuai5_main(int argc, char** argv, CreateAIFunc AIBuilder);


#endif // !AI_H