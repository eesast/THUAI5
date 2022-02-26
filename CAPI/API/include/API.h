#pragma once
#ifndef API_H
#define API_H

#include <string>
#include <optional>
#include <fstream>
#include <Message2Server.pb.h>
#include <Message2Clients.pb.h>
#include <MessageType.pb.h>

#include "state.h"
#include "constants.h"


#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#define NUM_OF_GRID_PER_CELL 1000

using XYPosition = ::std::pair<::std::int32_t, ::std::int32_t>;

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
    virtual bool SendInfo(Protobuf::MessageToServer&) = 0;

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
    virtual std::optional<std::string> GetInfo() = 0;

    /// <summary>
    /// 等待
    /// </summary>
    /// <returns></returns>
    virtual bool WaitThread() = 0;

    /// <summary>
    /// 获取计数器
    /// </summary>
    /// <returns></returns>
    virtual int GetCounter() const = 0;

    /// 获取信息（因为必须保证线程安全，所以必须在Logic类的内部实现这些接口）

    /// <summary>
    /// 获取可视的人物信息
    /// </summary>
    /// <returns></returns>
    virtual std::vector<std::shared_ptr<const THUAI5::Character>> GetCharacters() const = 0;

    /// <summary>
    /// 获取道具的信息
    /// </summary>
    /// <returns></returns>
    virtual std::vector<std::shared_ptr<const THUAI5::Prop>> GetProps() const = 0;

    /// <summary>
    /// 获取子弹的信息
    /// </summary>
    /// <returns></returns>
    virtual std::vector<std::shared_ptr<const THUAI5::Bullet>> GetBullets() const = 0;

    /// <summary>
    /// 获取自身信息
    /// </summary>
    /// <returns></returns>
    virtual std::shared_ptr<const THUAI5::Character> GetSelfInfo() const = 0;

    /// <summary>
    /// 获取地图信息
    /// </summary>
    /// <returns></returns>
    virtual THUAI5::PlaceType GetPlaceType(int CellX, int CellY) const = 0;

    /// <summary>
    /// 获取队伍分数
    /// </summary>
    /// <returns></returns>
    virtual uint32_t GetTeamScore() const = 0;

    /// <summary>
    /// 获取场上玩家的GUID信息
    /// </summary>
    /// <returns></returns>
    virtual const std::vector<int64_t> GetPlayerGUIDs() const = 0;
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
    virtual bool  Attack(double angleInRadian) = 0;
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
    [[nodiscard]] virtual bool MessageAvailable() = 0;
    [[nodiscard]] virtual std::optional<std::string> TryGetMessage() = 0;
    [[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI5::Character>> GetCharacters() const = 0;
    [[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI5::Prop>> GetProps() const = 0;
    [[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI5::Bullet>> GetBullets() const = 0;
    [[nodiscard]] virtual std::shared_ptr<const THUAI5::Character> GetSelfInfo() const = 0;
    [[nodiscard]] virtual THUAI5::PlaceType GetPlaceType(int32_t CellX, int32_t CellY) const = 0;
    [[nodiscard]] virtual uint32_t GetTeamScore() const = 0;
    [[nodiscard]] virtual const std::vector<int64_t> GetPlayerGUIDs() const = 0;
    [[nodiscard]] virtual int GetFrameCount() const = 0;

    //***********选手可能用到的辅助函数***********//
    [[nodiscard]] static constexpr inline int CellToGrid(int cell) noexcept // 获取指定格子中心的坐标
    {
        return cell * NUM_OF_GRID_PER_CELL + NUM_OF_GRID_PER_CELL / 2;
    }

    [[nodiscard]] static constexpr inline int GridToCell(int grid) noexcept // 获取指定坐标点所位于的格子的 X 序号
    {
        return grid / NUM_OF_GRID_PER_CELL;
    }

    //***********构造函数************//
    IAPI(ILogic& logic) :logic(logic) {}

    //***********析构函数************//
    virtual ~IAPI() {}

protected:
    ILogic& logic;
};

/// <summary>
/// 给Logic使用的IAPI接口，至于为什么这样写会在issue中解释
/// </summary>
class IAPI_For_Logic :public IAPI
{
public:
    IAPI_For_Logic(ILogic& logic) :IAPI(logic) {}
    virtual void StartTimer() = 0;
    virtual void EndTimer() = 0;
};

/// <summary>
/// 一般API
/// </summary>
class API final :public IAPI_For_Logic
{
public:
    API(ILogic& logic) :IAPI_For_Logic(logic) {}

    //***********选手可执行的操作***********//

    // 移动
    bool MovePlayer(uint32_t timeInMilliseconds, double angleInRadian) override;
    bool MoveRight(uint32_t timeInMilliseconds) override;
    bool MoveUp(uint32_t timeInMilliseconds) override;
    bool MoveLeft(uint32_t timeInMilliseconds) override;
    bool MoveDown(uint32_t timeInMilliseconds) override;

    // 攻击
    bool Attack(double angleInRadian) override;
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
    std::optional<std::string> TryGetMessage() override;

    std::vector<std::shared_ptr<const THUAI5::Character>> GetCharacters() const override;
    std::vector<std::shared_ptr<const THUAI5::Prop>> GetProps() const override;
    std::vector<std::shared_ptr<const THUAI5::Bullet>> GetBullets() const override;
    std::shared_ptr<const THUAI5::Character> GetSelfInfo() const override;
    THUAI5::PlaceType GetPlaceType(int32_t CellX, int32_t CellY) const override;

    uint32_t GetTeamScore() const override;
    const std::vector<int64_t> GetPlayerGUIDs() const override;
    int GetFrameCount() const override;

private:
    void StartTimer() override {}
    void EndTimer() override {}
};

class DebugAPI final :public IAPI_For_Logic
{
public:
    DebugAPI(ILogic& logic, std::ostream& Out = std::cout, bool ExamineValidity = true) :IAPI_For_Logic(logic), Out(Out), ExamineValidity(ExamineValidity) {}

    //***********选手可执行的操作***********//
    // 移动
    bool MovePlayer(uint32_t timeInMilliseconds, double angleInRadian) override;
    bool MoveRight(uint32_t timeInMilliseconds) override;
    bool MoveUp(uint32_t timeInMilliseconds) override;
    bool MoveLeft(uint32_t timeInMilliseconds) override;
    bool MoveDown(uint32_t timeInMilliseconds) override;

    // 攻击
    bool Attack(double angleInRadian) override;
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
    std::optional<std::string> TryGetMessage() override;

    std::vector<std::shared_ptr<const THUAI5::Character>> GetCharacters() const override;
    std::vector<std::shared_ptr<const THUAI5::Prop>> GetProps() const override;
    std::vector<std::shared_ptr<const THUAI5::Bullet>> GetBullets() const override;
    std::shared_ptr<const THUAI5::Character> GetSelfInfo() const override;
    THUAI5::PlaceType GetPlaceType(int32_t CellX, int32_t CellY) const override;

    uint32_t GetTeamScore() const override;
    const std::vector<int64_t> GetPlayerGUIDs() const override;
    int GetFrameCount() const override;

private:
    bool CanPick(THUAI5::PropType propType, std::shared_ptr<const THUAI5::Character> &selfInfo);
    bool CanUseActiveSkill(std::shared_ptr<const THUAI5::Character> &selfInfo);

    bool ExamineValidity;
    std::ostream& Out;
    std::chrono::system_clock::time_point StartPoint; // 记录起始时间

    void StartTimer()override;
    void EndTimer() override;
};

#endif
