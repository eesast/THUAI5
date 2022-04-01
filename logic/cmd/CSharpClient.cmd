@echo off

start cmd /k ..\CSharpInterface\bin\Debug\net5.0\CSharpInterface.exe 0 0

start cmd /k ..\CSharpInterface\bin\Debug\net5.0\CSharpInterface.exe 1 0

start cmd /k ..\CSharpInterface\bin\Debug\net5.0\CSharpInterface.exe 0 1

start cmd /k ..\CSharpInterface\bin\Debug\net5.0\CSharpInterface.exe 1 1

start cmd /k ..\CSharpInterface\bin\Debug\net5.0\CSharpInterface.exe 0 2

start cmd /k ..\CSharpInterface\bin\Debug\net5.0\CSharpInterface.exe 1 2

start cmd /k ..\CSharpInterface\bin\Debug\net5.0\CSharpInterface.exe 0 3

start cmd /k ..\CSharpInterface\bin\Debug\net5.0\CSharpInterface.exe 1 3

# start cmd /k ..\Client\bin\Debug\net5.0-windows\Client.exe --cl --port=7777 --teamID=5000 --characterID=5000 --software=3 --hardware=3