using System;
using System.IO;
using Google.Protobuf;

namespace Communication.Proto
{ 
    public enum PacketType
    {
        MessageToClient = 0,    // 全局信息
        MessageToOneClient = 1, // 单人信息
        MessageToServer = 2,
    }

    public interface IGameMessage
    {
        PacketType PacketType { get; set; }
        IMessage Content { get; set; } // Protobuf的通用接口
    }

    /// <summary>
    /// 信息类，定义了信息的两个重要操作方法：序列化和反序列化
    /// </summary>
    public class Message:IGameMessage
    {
        private PacketType type;
        public PacketType PacketType { get => type; set => type = value; }

        private IMessage content; 
        public IMessage Content { get => content; set => content = value; }

        /// <summary>
        /// 反序列化字节流
        /// </summary>
        /// <param name="bytes"></param>
        public void Deserialize(byte[] bytes)
        {
            MemoryStream istream = new MemoryStream(bytes);
            BinaryReader br = new BinaryReader(istream);

            type = (PacketType)br.ReadInt32();
            string typename = null;
            switch (type)
            {
                case PacketType.MessageToClient:
                    typename = "Communication.Proto.MessageToClient";
                    break;
                case PacketType.MessageToOneClient:
                    typename = "Communication.Proto.MessageToOneClient";
                    break;
                case PacketType.MessageToServer:
                    typename = "Communication.Proto.MessageToServer";
                    break;
            }

            try
            {
                CodedInputStream codedInputStream = new CodedInputStream(istream);
                content = (IMessage)Activator.CreateInstance(Type.GetType(typename)); // 这里按照原来的写法写会报错，难道和版本有关?
                content.MergeFrom(codedInputStream); // 这一步才是真正的反序列化过程
            }
            catch(Exception e)
            {
                Console.WriteLine($"Unhandled exception while trying to deserialize packet: {e}");
            }
        }

        /// <summary>
        /// 序列化对象，转为字节流
        /// </summary>
        /// <param name="bytes"></param>
        public void Serialize(out byte[] bytes)
        {
            MemoryStream ostream = new MemoryStream();
            BinaryWriter bw = new BinaryWriter(ostream);
            bw.Write((int)type);
            bw.Flush();

            using (CodedOutputStream output = new CodedOutputStream(ostream, true))
            {
                Content.WriteTo(output);
                // 使用此方法将所有信息从基础缓冲区移动到其目标或清除缓冲区
                output.Flush();
            }
            bytes = ostream.ToArray();
        }
    }
}
