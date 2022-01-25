using System;
using System.IO;
using Google.Protobuf;

namespace Communication.Proto
{
    /// <summary>
    /// 游戏信息类型。此处的类型完全根据proto文件的信息决定，实际上可以自行删减——把所有的"MessageToxx"类的名称都列举在这里就可以了
    /// </summary>
    public enum PacketType
    {
        MessageToServer = 0,
        MessageToOneClient = 1, // 单人信息
        // 更新游戏中的可变属性。采用每帧发送所有对象信息的方式，因此储存的是一个数组
        MessageToClient = 2,
        MessageToInitialize = 3
    }
    /// <summary>
    /// 信息的通用接口，包含信息类型和内容
    /// </summary>
    public interface IGameMessage
    {
        PacketType PacketType { get; set; }
        IMessage Content { get; set; }
    }

    /// <summary>
    /// 信息类，定义了信息的两个重要操作方法：序列化和反序列化
    /// </summary>
    public class Message : IGameMessage
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
            // 由于枚举值增多，这里将“枚举转类型名称”改写成一种更加简洁的方式
            typename = "Communication.Proto." + Enum.GetName(typeof(PacketType), type);

            try
            {
                CodedInputStream codedInputStream = new CodedInputStream(istream);
                content = (IMessage)Activator.CreateInstance(Type.GetType(typename)); // 这里按照原来的写法写会报错，难道和版本有关?
                content.MergeFrom(codedInputStream); // 这一步才是真正的反序列化过程
            }
            catch (Exception e)
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
