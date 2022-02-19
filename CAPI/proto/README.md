# 使用须知
每次生成新的`.cc`和`.h`文件时，必须在`.h`文件中添加宏`#define PROTOBUF_USE_DLLS`，否则在windows平台下编译时会报错！