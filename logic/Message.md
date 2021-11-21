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



