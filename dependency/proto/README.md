# proto

Protobuf 源代码文件  

## 简要说明

我不清楚这是否是逻辑组原本的意思，但为了我能在开发时不会被绕晕，我还是简单总结一下改动吧。
1. Message2Clients中原本有一些关于GameObj的枚举，现在全部移到了MessageType中。
2. MessageToClient类被取消，取而代之的是更加具体的，代表各种GameObj的Message。