#pragma once
#ifndef API_H
#define API_H

#include <string>
#include "Message2Server.pb.h"
#include "state.h"


/// <summary>
/// API中依赖Logic的部分
/// </summary>
class ILogic
{
public:
    /// <summary>
    /// 向Server端发送信息
    /// </summary>
    /// <returns></returns>
    virtual bool SendInfo(Protobuf::MessageToServer) = 0;

    /// <summary>
    /// Logic中的队友消息队列是否为空
    /// </summary>
    /// <returns></returns>
    virtual bool Empty() = 0;

    /// <summary>
    /// 获取消息队列中的信息
    /// </summary>
    /// <param name="s"></param>
    /// <returns></returns>
    virtual bool GetInfo(std::string& s) = 0;

    /// <summary>
    /// 等待
    /// </summary>
    /// <returns></returns>
    virtual bool WaitThread() = 0;

    /// <summary>
    /// 获取计数器
    /// </summary>
    /// <returns></returns>
    virtual int GetCounter() = 0;

    /// <summary>
    /// 获取当前的状态指针
    /// </summary>
    /// <returns></returns>
    virtual State* GetpState() = 0;
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
    [[nodiscard]] virtual const std::vector<int64_t> GetPlayerGUIDs() const = 0;
    
    //***********构造函数************//
    IAPI(ILogic& logic) :logic(logic) {}

    //***********析构函数************//
    virtual ~IAPI() {}

protected:
    ILogic& logic;
};

/// <summary>
/// 一般API
/// </summary>
class API final :public IAPI
{
public:
    API(ILogic& logic) :IAPI(logic) {}

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
    const std::vector<int64_t> GetPlayerGUIDs() const override;
};

class DebugAPI final :public IAPI
{
public:
    DebugAPI(ILogic& logic) :IAPI(logic) {}
};

#endif
