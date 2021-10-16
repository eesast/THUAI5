# communication

## 简介

C# 通信组件  

## 目标

### 基本目标

+ 基于 Protobuf 与 HPSocket，为服务器和客户端提供 C# 通信组件  
  + 为服务器提供 ServerCommunication 通信组件，目标平台为 .NET 5 类库（可能根据开发进度改为 .NET 6）  
  + 为客户端提供 ClientCommunication 通信组件，目标平台为 .NET 5 类库（可能根据开发进度改为 .NET 6）以及 .NET Standard 2.1（供 Unity 使用，可能根据开发进度升级版本）  

## 重要目标

+ 确定好通信的方式，保证通信的高效  
+ 修正 THUAI4 信息重复发送的问题  
+ 将通信逻辑与游戏逻辑彻底分开，以供日后复用  
+ 支持多终端游戏、观战等，尽量不对客户端人数、观战人数等做出任何假定，游戏人数完全由服务器决定  
+ 去除往年遗留下来的以 Agent 作为通信中介的方式，改为客户端直连服务器  

### 提高目标（饼）  

+ 提供其他语言（如 Python、C#、Java 等）的选手接口  
+ ……（未完待画）  

## 注意事项

+ 客户端用于通信的类库需要生成两个目标平台，一个供 C# 服务器和客户端使用，目标平台采用 .NET 5（或 .NET 6）；另一个供 Unity 使用，目标平台为 .NET Standard 2.1（可能根据开发进度升级版本）。可以先开发 `.NET 5/6`  版本，待 Unity 客户端编写完毕后再生成 `.NET Standard` 版本  
+ 与其他组共同商定通信协议  
+ 避免忙等待，注意线程安全，做好线程同步 

## 一些想法

1. 命令行参数的问题基本已经解决，即使有，到时候只要与逻辑组稍加协商后就可改动，问题应该不大。

2. 关于ServerCommunication：现在可以发送的类有：MessageToInitialize，MessageToOperate，MessageToRefresh，MessageToOneClient。MessageToAddInstance和MessageToDestroyInstance已经嵌套在了MessageToOperate中，故不需要单独发送（而这两个类又嵌套了MessageOfProp和MessageOfBullet，更不需要单独发送了）。

3. 关于ServerTest：本来我想写得更细化一些，模拟一次完整的游戏过程，但考虑到我不太懂游戏逻辑，部分细节不明确，只好作罢（

4. 关于“如何降低proto文件和server的耦合关系”，我想做以下几点说明：

   * Message.cs中的枚举值PacketType是必须要根据protobuf文件改动的，这个确实没有更好的解决办法。
   * ServerCommunication.cs中的若干个重载函数，理论上可以都可以用一个IMessage接口实现，但考虑到发送的信息本身与逻辑息息相关，需要在终端输出不同的success、warning、error等提示信息，所以只好多一个类就多写一个重载。

   proto和server不能完全解耦，但改动思路还是比较简单的。

## 开发人员

+ ……（自己加）  

