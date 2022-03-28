@echo off

:: start cmd /k ..\Server\bin\Release\net5.0\Server.exe --port=7777 --teamCount=2 --playerCount=1 --gameTimeInSecond=30 --mapResource=testmap1.txt --fileName=test > 1.txt

start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe -u --port=7777 --teamID=3330 --characterID=3330 --software=2 --hardware=2

