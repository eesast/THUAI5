@echo off

start cmd /k ..\Server\bin\Debug\net5.0\Server.exe --port=7777 --teamCount=2 --playerCount=4  --gameTimeInSecond=300

start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe --cl --port=7777 --teamID=5000 --characterID=5000 --software=3 --hardware=3

<<<<<<< HEAD
=======
::start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe --cl --port=7777 --teamID=5000 --characterID=5000 --software=3 --hardware=3

>>>>>>> 9577156720ae45332eb20a4d9f810a7e05f9ba75
