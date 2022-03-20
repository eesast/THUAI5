param([string]$Dir = $(throw "Parameter missing: -Dir Directory to output"))
dotnet publish "./logic/Server/Server.csproj" -c Release -r win-x64 --self-contained false -o $Dir
dotnet publish "./logic/Client/Client.csproj" -c Release -r win-x64 --self-contained false -o $Dir