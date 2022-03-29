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

const constexpr int num_of_grid_per_cell = 1000;

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
    /// 获取可视的机器人信息
    /// </summary>
    /// <returns></returns>
    virtual std::vector<std::shared_ptr<const THUAI5::Robot>> GetRobots() const = 0;

    /// <summary>
    /// 获取道具的信息
    /// </summary>
    /// <returns></returns>
    virtual std::vector<std::shared_ptr<const THUAI5::Prop>> GetProps() const = 0;

    /// <summary>
    /// 获取信号干扰器的信息
    /// </summary>
    /// <returns></returns>
    virtual std::vector<std::shared_ptr<const THUAI5::SignalJammer>> GetSignalJammers() const = 0;

    /// <summary>
    /// 获取自身信息
    /// </summary>
    /// <returns></returns>
    virtual std::shared_ptr<const THUAI5::Robot> GetSelfInfo() const = 0;

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

    // 指挥本角色进行移动，`timeInMilliseconds` 为移动时间，单位为毫秒；`angleInRadian` 表示移动的方向，单位是弧度，使用极坐标——竖直向下方向为 x 轴，水平向右方向为 y 轴。
    virtual bool MovePlayer(uint32_t timeInMilliseconds, double angleInRadian) = 0;

    // 向特定方向移动。
    virtual bool MoveRight(uint32_t timeInMilliseconds) = 0;
    virtual bool MoveUp(uint32_t timeInMilliseconds) = 0;
    virtual bool MoveLeft(uint32_t timeInMilliseconds) = 0;
    virtual bool MoveDown(uint32_t timeInMilliseconds) = 0;

    // 发射信号干扰器进行攻击。`angleInRadian`给出了信号干扰器的发射方向。
    virtual bool Attack(double angleInRadian) = 0;

    // 使用主动技能。
    virtual bool UseCommonSkill() = 0;

    // 给同队的队友发送消息。`toPlayerID` 指定发送的对象，`message` 指定发送的内容。
    virtual bool Send(int toPlayerID,std::string) = 0;

    // 捡起与自己处于同一个格子（cell）的道具。需要指定道具种类
    virtual bool Pick(THUAI5::PropType) = 0;

    // 扔出手中的道具（如果有的话），使其变为未捡起状态，可以供他人捡起。
    virtual bool ThrowProp(uint32_t timeInMilliseconds, double angleInRadian) = 0;

    // 使用道具
    virtual bool UseProp() = 0;

    // 扔出手中的CPU
    virtual bool ThrowCPU(uint32_t timeInMilliseconds, double angleInRadian,uint32_t cpuNum) = 0;

    // 使用手中的CPU
    virtual bool UseCPU(uint32_t cpuNum) = 0;

    // 在`asynchronous` 为 `true` 的情况下，选手可以调用此函数，阻塞当前线程，直到下一次消息更新时继续运行。
    virtual bool Wait() = 0;

    //***********选手可获取的信息***********//
    // 查看目前是否有队友发来的尚未接收的信息。
    [[nodiscard]] virtual bool MessageAvailable() = 0;

    // 获取队友发来的信息，注意当信息队列为空时，返回`nullptr`。
    [[nodiscard]] virtual std::optional<std::string> TryGetMessage() = 0;

    // 获取场地内所有可视玩家的信息。
    [[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI5::Robot>> GetRobots() const = 0;

    // 获取当前场地内的所有尚未被捡起的的可视道具的信息。
    [[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI5::Prop>> GetProps() const = 0;

    // 获取当前场地内的所有可视信号干扰器的信息。
    [[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI5::SignalJammer>> GetSignalJammers() const = 0;

    // 获取自身信息
    [[nodiscard]] virtual std::shared_ptr<const THUAI5::Robot> GetSelfInfo() const = 0;

    // 返回某一位置场地种类信息。注意此处的`CellX`和`CellY`指的是地图格数，而不是绝对坐标。
    [[nodiscard]] virtual THUAI5::PlaceType GetPlaceType(int32_t CellX, int32_t CellY) const = 0;

    // 获取队伍分数。
    [[nodiscard]] virtual uint32_t GetTeamScore() const = 0;

    // 获取所有玩家的GUID（全局唯一标识符）
    [[nodiscard]] virtual const std::vector<int64_t> GetPlayerGUIDs() const = 0;

    // 获取游戏目前所进行的帧数
    [[nodiscard]] virtual int GetFrameCount() const = 0;

    //***********此暂时仅供debug使用，不过后续也可考虑将其加入选手接口中***********//

    // 打印信息（但除非特殊情况，我们不建议使用该函数，这会增加系统开销，影响游戏效果）
    virtual void PrintRobots() const = 0;
    virtual void PrintProps() const = 0;
    virtual void PrintSignalJammers() const = 0;
    virtual void PrintSelfInfo() const = 0;

    //***********选手可能用到的辅助函数***********//

     // 获取指定格子中心的坐标
    [[nodiscard]] static inline int CellToGrid(int cell) noexcept
    {
        return cell * num_of_grid_per_cell + num_of_grid_per_cell / 2;
    }

    // 获取指定坐标点所位于的格子的 X 序号
    [[nodiscard]] static inline int GridToCell(int grid) noexcept
    {
        return grid / num_of_grid_per_cell;
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
    bool ThrowCPU(uint32_t timeInMilliseconds, double angleInRadian, uint32_t cpuNum) override;
    bool UseCPU(uint32_t cpuNum) override;

    // 其它
    bool Wait() override;

    //***********选手可获取的信息***********//
    // 待补充，此处只写了和THUAI4相同的内容
    bool MessageAvailable() override;
    std::optional<std::string> TryGetMessage() override;

    std::vector<std::shared_ptr<const THUAI5::Robot>> GetRobots() const override;
    std::vector<std::shared_ptr<const THUAI5::Prop>> GetProps() const override;
    std::vector<std::shared_ptr<const THUAI5::SignalJammer>> GetSignalJammers() const override;
    std::shared_ptr<const THUAI5::Robot> GetSelfInfo() const override;
    THUAI5::PlaceType GetPlaceType(int32_t CellX, int32_t CellY) const override;

    void PrintRobots() const override;
    void PrintProps() const override;
    void PrintSignalJammers() const override;
    void PrintSelfInfo() const override;

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
    bool ThrowCPU(uint32_t timeInMilliseconds, double angleInRadian, uint32_t cpuNum) override;
    bool UseCPU(uint32_t cpuNum) override;

    // 其它
    bool Wait() override;

    //***********选手可获取的信息***********//
    // 待补充，此处只写了和THUAI4相同的内容
    bool MessageAvailable() override;
    std::optional<std::string> TryGetMessage() override;

    std::vector<std::shared_ptr<const THUAI5::Robot>> GetRobots() const override;
    std::vector<std::shared_ptr<const THUAI5::Prop>> GetProps() const override;
    std::vector<std::shared_ptr<const THUAI5::SignalJammer>> GetSignalJammers() const override;
    std::shared_ptr<const THUAI5::Robot> GetSelfInfo() const override;
    THUAI5::PlaceType GetPlaceType(int32_t CellX, int32_t CellY) const override;

    uint32_t GetTeamScore() const override;
    const std::vector<int64_t> GetPlayerGUIDs() const override;
    int GetFrameCount() const override;

    void PrintRobots() const override;
    void PrintProps() const override;
    void PrintSignalJammers() const override;
    void PrintSelfInfo() const override;

private:
    bool CanPick(THUAI5::PropType propType, std::shared_ptr<const THUAI5::Robot> &selfInfo);
    bool CanUseSoftware(std::shared_ptr<const THUAI5::Robot> &selfInfo);

    bool ExamineValidity;
    std::ostream& Out;
    std::chrono::system_clock::time_point StartPoint; // 记录起始时间

    void StartTimer()override;
    void EndTimer() override;
};

#endif
