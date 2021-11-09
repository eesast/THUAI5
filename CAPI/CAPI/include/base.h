#pragma once
#ifndef BASE_H
#define BASE_H

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
