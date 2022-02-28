@echo off

start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe

start cmd /k ..\Server\bin\Debug\net5.0\Server.exe --port=7777 --teamCount=2 --playerCount=1  --gameTimeInSecond=30 --fileName=video --playBack  

pause