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

* 既然决意要去掉Agent，暂时应该做以下调整：

  1. Agent的绝大多数功能都要迁移到Server中。在THUAI4中，endpoint和port都需要在Agent中指定，而现在需要将这一功能迁移到Server中。因此需要在Server中添加对命令行参数的处理。

     但这就又涉及到了一个问题：需不需要做Commandline Tool。在THUAI4的Agent中，由于Agent.cs是一个Program，可以直接用`CommandLineUtils`做一个命令行工具，但THUAI5的ServerCommunication是一个dll，不能直接按Commandline Tool的思路去做。

     但其实也不怎么影响使用，只不过用户体验感会差一些。
     
  2. 使用Server时，命令行参数（包括端口号和IP）应该如何指定？难道在Server test里加吗？
  
  3. 如何实现所有client全部关闭时server也自动关闭？
  
* 再次梳理以下THUAI4和THUAI5中的端口分配

  1. THUAI4:Client不需要手动指定端口，直连`"127.0.0.1", 7777`即可。因为Client直连Agent，Agent一定在本机上，而Agent默认的侦听端口是7777。虽然THUAI4中可以在命令行中设置侦听端口，但改成7777以外的值会出现“Agent start unsuccessfully”。Agent需要指定`-s -p`，`-s`除了指定IP地址外，还需要指定目标机的侦听端口（默认是8888）。而Server也是默认侦听8888。`-p`则是Agent的侦听端口。
  2. THUAI5:从THUAI4的设计上看，似乎没有解决端口占用的问题??(也可能是我太菜没看出来)，因此THUAI5暂时..放过这个问题。但远程IP地址的指定还是必须的。所以client的命令行参数还要再多一个IP地址...

## 开发人员

+ ……（自己加）  

