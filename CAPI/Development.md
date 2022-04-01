* [class] ClientCommunication

  1. 有关委托

     `CAPI(std::function<void()> onconnect, std::function<void()> onclose, std::function<void(pointer_m2c)> onreceive);`

     ``ClientCommunication(std::function<void(pointer_m2c)> OnReceive, std::function<void() > OnConnect, std::function<void() > CloseHandler = nullptr);`

     构造时CAPI是嵌套类。OnConnect为两者相同。OnClose被设置为可选，ClientCommunication中被设置为空，CAPI中为其套了一层壳。OnReceive则为两者完全不相关。
     
  2. 有关运行控制的几个参量
  
     * loop: 默认为true，若capi连接失败，capi关闭连接为false。若loop为true，则会一直控制processmessage子线程进行运行。
     * blocking：默认为false，当消息队列为空时为true。此时信号量会阻塞processmessage线程，直到队列中有信息。还有一种强制唤醒的方法（此时一般是loop为false，需要同时关掉子线程和主线程，详见UnBlock()）这里processmessage线程被阻塞并不会影响主线程中对信息的收取？？
     * counter：每调用一次send就将其加1，如果大于Limit将无法发送。除非在OnReceive中收到一条MessageToClient。
  
  3. ProcessMessage()
  
     作为子线程在loop为True时不断运行，除非队列消息为空。
  
  4. Start()
  
     每次Start必然会开启ProcessMessage子线程。如果连接失败，会先解锁子线程，等待其运行结束（此时loop已经为false，子线程应该会立刻结束）。然后结束子线程。
  
  5. Unblock()
  
     将blocking强制置为false并强制唤醒processmessage子线程（无视空队列）
  
  6. Join()
  
     按上述思路终止子线程和主线程。
  
  
  
* [class] Logic
  
  1. a
  
  2. 关于Buffer和State（THUAI4）
  
     在调用load函数时，首先要load到buffer。所有buffer需要的信息都加载完毕后，就将Updated转为true。**这时候state还没被player访问，就把buffer转到state。**
  
     在进行buffer和state的转换时，只需要将两者的指针互换，再将Updated改为false。
  
  3. 关于两个UnBlock函数

     也是在特殊情况下强制唤醒线程。
  
  4. 关于AI_loop
     
     类似于Communication中的loop变量控制通信线程那样，如果AI_loop始终为true，则运行AI函数。
  
  5. 关于AI_start
     
     决定何时启用AI线程。可能和AI_loop的功能有一定的重合，并不是一个好的设计。
     
  6. 关于cv_ai和cv_buffer
     
     cv_ai毫无疑问是控制AI线程的。~~但我在THUAI4中并没有找到与cv_buffer对应的子线程。我只好认为cv_buffer控制的是主线程了。（当然，主线程可能只是一个相对的概念）~~
     
  7. 关于counter_buffer和counter_state
  
     
  
* [class] LogicInterFace

  1. 这个类定义了API中所需要的函数对象，分为若干个类。
  
* [class] API
  1. 该类中使用template来决定是否更新参数。然后用到了if constexpr(asyn)来在编译时段决定编译哪一段代码(略似预处理器指令)，这个设计是否需要保留？
  2. 其实asyn=true就比asyn=false多了两句话
```cpp
	std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
	Members<asyn>::TryUpDate();
```

​    也许是Members这个类存在的唯一意义？

​	这两句话的含义大致是：先上锁，再调用Members中的函数对象（在Logic.h中被赋值为logic中被定义的wait），更新信息（互换现存状态和缓冲状态的指针）。

  以下介绍的不一定是具体的类，而是“模块”（几个类的组合）
简短的讲：
Communication：负责与Server（由C#写成，含有游戏的核心运行逻辑）进行通信，这个本质上是和C# 写的ClientCommunication是一致的。
API：可供用户调用的接口。会调用Communication类中的函数。注意在这个类中没有实际调用接口。
Logic：将API中的接口和Communication中的信息函数作为参数传入，并调用之，C++ Client端的总体运行逻辑就写在这里。
Main：作为命令行程序提供给用户使用，并在命令行参数准备完毕后调用Logic端的总执行函数。

为什么这部分代码十分难懂呢？
1.THUAI4中的Communication组件中用到了模板，但只要proto中的信息已经确定好了，就根本不需要这样设计了。这块我已经删掉了。
2.为了彻底实现API、Communication和Logic三者之间的完全解耦，项目中大量使用了函数对象、lambda函数等用法。比如，我前面提到API中要调用Communication中的函数，该怎么办？API不直接包含Communication类，而是继承一个LogicInterface（内含一些函数接口），用这个接口包装Communication类中的函数。这样做的一个好处是使用了函数对象以后，函数实现的自由度大大提高，可以随便传入lambda函数（解耦的意义），但随之而来的是代码可读性变差一些）
3.线程调度。主要在Logic类中，这块我不知道是不是需要重构，反正我是看不大懂。。。

  

  

  