# playback

## 简介

回放组件

## 目标

### 基本目标

生成供产生回放文件和读取回放文件的基本组件，生成 .NET 类库。  

## 统一约定

+ 目标平台：`.NET Standard` 与 `.NET 5`（是否改用 `.NET 6` 视进度而定）类库，`.NET Standard` 版本供给 Unity 客户端使用，`.NET 5/6` 版本供给其他客户端与服务器使用。  

## 特别说明

+ 本项目几乎可完全复用~~（照抄）~~去年的 THUAI4    
+ 在去年的 THUAI4 中，`.NET Core 3.1`  版本位于 `playback` 项目内，而 `.NET Standard` 版本位于 `playbackForUnity` 目录内  
+ 在去年的 THUAI4 中，回放文件播放器 `PlayBackPlayer`、`PlayBackPlayerDll`、`PlayBackPlayerResources` 的出现纯属意外，THUAI5 非必要则请不要编写类似的播放器，回放文件的读取与播放应该由 Unity 客户端与 logic 内的调试客户端两者完成   

## 开发人员

+ ……（自己加）  
