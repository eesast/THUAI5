protoc --csharp_out=. Message2Clients.proto
protoc --csharp_out=. Message2Server.proto
protoc --csharp_out=. MessageType.proto
copy .\Message2Clients.cs ..\..\communication\Proto\Message2Clients.cs
copy .\Message2Server.cs ..\..\communication\Proto\Message2Server.cs
copy .\MessageType.cs ..\..\communication\Proto\MessageType.cs
del Message2Clients.cs
del Message2Server.cs
del MessageType.cs