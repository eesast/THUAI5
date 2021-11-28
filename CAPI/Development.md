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

  8. 关于
     
  
     
  
     
  
  