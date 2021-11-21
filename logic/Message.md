# Message

- 下面的信息类型分别对应server中能够处理的消息类型
- 需要指定的部分为protobuf的消息中，需要赋值的部分
- 一般来说，发送的信息都应该包括TeamID, PlayerID，用来指定谁要做这件事

## AddPlayer

- 初始化时应发送的信息
- 需要指定：TeamID, PlayerID, PSkill(被动技能)，ASkill1(主动技能)

## Move

- 移动
- 需要指定：TeamID, PlayerID, TimeInMilliseconds（移动时间）, Angle（角度）

## Attack

- 攻击
- 需要指定：TeamID, PlayerID, Angle（角度）

## UseCommonSkill

- 使用技能
- 需要指定：TeamID, PlayerID

## Send

- 向队友发信息
- 需要指定：TeamID, PlayerID（这里的两个ID指的是接受者的ID，谁是发送者貌似是无法知道的）

## Pick

- 捡道具
- 需要指定：TeamID, PlayerID，PropType(要捡起的道具类型)

## UseProp

- 使用道具（不包括宝石）
- 需要指定：TeamID, PlayerID

## UseGem

- 使用宝石，换取分数
- 需要指定：TeamID, PlayerID, GemSize（使用的宝石数量）

## ThrowProp

- 扔出道具
- 需要指定：TeamID, PlayerID, TimeInMilliseconds, Angle

## ThrowGem

- 扔出宝石
- 需要指定：TeamID, PlayerID, TimeInMilliseconds, Angle, GemSize（扔出的宝石数量）

# 对选手的限制

## 视野

- 草丛外的人看不到草丛内的物体

- 具体逻辑：

  - 根据消息中的place进行判断

  - 假设有物体A和物体B，它们的place分别记为Xa，Xb，那么A能否看到B呢？

    - ~~~c#
      // A发起请求要看到周围物体，下面分析B能否被A看见
      if (Xb == PlaceType.Invisible) //先判断B是否隐身，隐身则A看不见B
          return false;
      if (Xb == PlaceType.Land) //B是否在land上，若是，则A看得见B
          return true;
      if (obj.Place == this.Place) //A和B是否在同一个草丛里，如果是，则A看的见B
          return true;
      return false;  //A看不见B
      ~~~

- 视野要在客户端进行限制
