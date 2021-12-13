#pragma once
#ifndef API_H
#define API_H

#include <string>
#include "Message2Server.pb.h"
#include "state.h"



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
    //***********选手可执行的操作***********//
    
    // 移动
    virtual bool MovePlayer(uint32_t timeInMilliseconds, double angleInRadian) = 0;
    virtual bool MoveRight(uint32_t timeInMilliseconds) = 0;
    virtual bool MoveUp(uint32_t timeInMilliseconds) = 0;
    virtual bool MoveLeft(uint32_t timeInMilliseconds) = 0;
    virtual bool MoveDown(uint32_t timeInMilliseconds) = 0;

    // 攻击
    virtual bool Attack(uint32_t timeInMilliseconds, double angleInRadian) = 0;
    virtual bool UseCommonSkill() = 0;

    // 通信
    virtual bool Send(int toPlayerID,std::string) = 0;

    // 道具
    virtual bool Pick(THUAI5::PropType) = 0; // 需要指定道具属性
    virtual bool ThrowProp(uint32_t timeInMilliseconds, double angleInRadian) = 0;
    virtual bool UseProp() = 0;
    virtual bool ThrowGem(uint32_t timeInMilliseconds, double angleInRadian,uint32_t gemNum) = 0;
    virtual bool UseGem(uint32_t gemNum) = 0;

    // 其它
    virtual bool Wait() = 0;

    //***********选手可获取的信息***********//
    // 待补充，此处只写了和THUAI4相同的内容
    [[nodiscard]] virtual bool MessageAvailable() = 0;
    [[nodiscard]] virtual bool TryGetMessage(std::string&) = 0;

    [[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI5::Character>> GetCharacters() const = 0;
    [[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI5::Wall>> GetWalls() const = 0;
    [[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI5::Prop>> GetProps() const = 0;
    [[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI5::Bullet>> GetBullets() const = 0;
    [[nodiscard]] virtual std::shared_ptr<const THUAI5::Character> GetSelfInfo() const = 0;

    [[nodiscard]] virtual uint32_t GetTeamScore() const = 0;
    [[nodiscard]] virtual const std::vector<std::vector<int64_t>> GetPlayerGUIDs() const = 0;

protected:
    // **********需要在logic中指定的，辅助API进行的成员和函数***************//
    std::function<bool(Protobuf::MessageToServer&)> SendInfo;           // 发送信息
    std::function<bool()> Empty;                                        // 判断队列是否为空
    std::function<bool(std::string&)> GetInfo;                          // 获取信息
    std::function<int()> GetCounter;
    std::function<void()> WaitThread;                                   // 等待
    State*& pState;                                                     // 当前状态
    friend class APIBuilder;                                            // 十分糟糕的设计
};


class API final :public IAPI
{
public:
    API(); // 待定

    //***********选手可执行的操作***********//

    // 移动
    bool MovePlayer(uint32_t timeInMilliseconds, double angleInRadian) override;
    bool MoveRight(uint32_t timeInMilliseconds) override;
    bool MoveUp(uint32_t timeInMilliseconds) override;
    bool MoveLeft(uint32_t timeInMilliseconds) override;
    bool MoveDown(uint32_t timeInMilliseconds) override;

    // 攻击
    bool Attack(uint32_t timeInMilliseconds, double angleInRadian) override;
    bool UseCommonSkill() override;

    // 通信
    bool Send(int toPlayerID, std::string) override;

    // 道具
    bool Pick(THUAI5::PropType) override; // 需要指定道具属性
    bool ThrowProp(uint32_t timeInMilliseconds, double angleInRadian) override;
    bool UseProp() override;
    bool ThrowGem(uint32_t timeInMilliseconds, double angleInRadian, uint32_t gemNum) override;
    bool UseGem(uint32_t gemNum) override;

    // 其它
    bool Wait() override;

    //***********选手可获取的信息***********//
    // 待补充，此处只写了和THUAI4相同的内容
    bool MessageAvailable() override;
    bool TryGetMessage(std::string&) override;

    std::vector<std::shared_ptr<const THUAI5::Character>> GetCharacters() const override;
    std::vector<std::shared_ptr<const THUAI5::Wall>> GetWalls() const override;
    std::vector<std::shared_ptr<const THUAI5::Prop>> GetProps() const override;
    std::vector<std::shared_ptr<const THUAI5::Bullet>> GetBullets() const override;
    std::shared_ptr<const THUAI5::Character> GetSelfInfo() const override;

    uint32_t GetTeamScore() const override;
    const std::vector<std::vector<int64_t>> GetPlayerGUIDs() const override;
};

class DebugAPI final :public IAPI
{
public:
    DebugAPI(); // 待定

};

class APIBuilder
{
public:
    APIBuilder(bool type);
    virtual void set_SendInfo() = 0;
    virtual void set_Empty() = 0;
    virtual void set_GetInfo() = 0;
    virtual void set_getCounter() = 0;
    virtual void set_WaitThread() = 0;
    
    std::shared_ptr<IAPI> get_api();
    virtual ~APIBuilder() {};

private:
    std::shared_ptr<IAPI> api;
};

class APIBuilder_1 : public APIBuilder
{
public:
    APIBuilder_1(bool type);
    void set_SendInfo() override;
    void set_Empty() override;
    void set_GetInfo() override;
    void set_getCounter() override;
    void set_WaitThread() override;

};

#endif
