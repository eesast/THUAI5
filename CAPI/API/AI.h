#pragma once
#ifndef AI_H
#define AI_H

#include<memory>
#include"API.h"

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


#endif // !AI_H