protoc --version
protoc --csharp_out=. Message2Clients.proto
protoc --csharp_out=. Message2Server.proto
protoc --csharp_out=. MessageType.proto
mv ./Message2Clients.cs ../../communication/Proto/Message2Clients.cs
mv ./Message2Server.cs ../../communication/Proto/Message2Server.cs
mv ./MessageType.cs ../../communication/Proto/MessageType.cs
