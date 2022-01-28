@echo off

start cmd /r ..\Client\bin\Debug\net5.0-windows\Client.exe 

start cmd /r ..\Server\bin\Debug\net5.0\Server.exe --port=7777 --teamCount=2 --playerCount=1  --gameTimeInSecond=30 --playBack --fileName=video

pause