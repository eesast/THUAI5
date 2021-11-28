#pragma once
#ifndef API_H
#define API_H

#include <string>

#include "Message2Server.pb.h"


/// <summary>
/// API通用接口，可派生为一般API和DebugAPI
/// </summary>
class IAPI
{
public:
    // 指

    // 移动
    virtual bool MovePlayer(uint32_t timeInMilliseconds, double angleInRadian) = 0;
    virtual bool MoveRight(uint32_t timeInMilliseconds) = 0;
    virtual bool MoveUp(uint32_t timeInMilliseconds) = 0;
    virtual bool MoveLeft(uint32_t timeInMilliseconds) = 0;
    virtual bool MoveDown(uint32_t timeInMilliseconds) = 0;

    // 攻击
    virtual void Attack(uint32_t timeInMilliseconds, double angleInRadian) = 0;
    virtual void UseCommonSkill() = 0;

    // 通信
    virtual void Send(int toPlayerID,std::string) = 0;

    // 道具
    virtual void Pick() = 0; // 需要指定道具属性
    virtual void ThrowProp(uint32_t timeInMilliseconds, double angleInRadian) = 0;
    virtual void UseProp() = 0;
    virtual void ThrowGem(uint32_t timeInMilliseconds, double angleInRadian,uint32_t gemNum) = 0;
    virtual void UseGem(uint32_t gemNum) = 0;



private:

};


#endif
