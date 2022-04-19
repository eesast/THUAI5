@echo off

start cmd /k ..\Server\bin\Debug\net5.0\Server.exe --port 7777 --teamCount 2 --playerCount 1 --gameTimeInSecond 600 --fileName video 

start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe --cl --port=7777 --teamID=5000 --characterID=5000 --software=4 --hardware=3

