@echo off

start cmd /k ..\Server\bin\Debug\net5.0\Server.exe --port 7777 --teamCount 2 --playerCount 1 --gameTimeInSecond 600 --fileName video 

start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe --cl --port=7777 --teamID=1 --characterID=0 --software=4 --hardware=3

start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe --cl --port=7777 --teamID=0 --characterID=0 --software=4 --hardware=3
