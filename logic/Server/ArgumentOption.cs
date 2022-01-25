using CommandLine;

namespace Server
{
    static class DefaultArgumentOptions
    {
        public static string FileName = "不用记啊";     // An impossible name of the playback file to indicate -f is not sepcified.
        public static string Token = "随手推啊";         // An impossible name of the token to indicate -f is not sepcified.
        public static string Url = "闭眼写啊";
        public static string MapResource = "多简单啊";
    }

    public class ArgumentOptions
    {
        [Option('p', "port", Required = true, HelpText = "Server listening port")]
        public ushort ServerPort { get; set; } = 10086;

        [Option('t', "teamCount", Required = false, HelpText = "The number of teams, 1 by defualt")]
        public ushort TeamCount { get; set; } = 1;

        [Option('c', "playerCount", Required = false, HelpText = "The number of players per team, 1 by default")]
        public ushort PlayerCountPerTeam { get; set; } = 1;

        [Option('g', "gameTimeInSecond", Required = false, HelpText = "The time of the game in second, 10 minutes by default")]
        public uint GameTimeInSecond { get; set; } = 10 * 60;

        [Option('f', "fileName", Required = false, HelpText = "The file to store playback file or to read file.")]
        public string FileName { get; set; } = DefaultArgumentOptions.FileName;

        [Option('b', "playBack", Required = false, HelpText = "Whether open the server in a playback mode.")]
        public bool PlayBack { get; set; } = false;

        [Option("playBackSpeed", Required = false, HelpText = "The speed of the playback, between 0.25 and 4.0")]
        public double PlayBackSpeed { get; set; } = 1.0;

        [Option("resultOnly", Required = false, HelpText = "In playback mode to get the result directly")]
        public bool ResultOnly { get; set; } = false;

        [Option('k', "token", Required = false, HelpText = "Web API Token")]
        public string Token { get; set; } = DefaultArgumentOptions.Token;

        [Option('u', "url", Required = false, HelpText = "Web Url")]
        public string Url { get; set; } = DefaultArgumentOptions.Url;

        [Option('m', "mapResource", Required = false, HelpText = "Map Resource Path")]
        public string mapResource { get; set; } = DefaultArgumentOptions.MapResource;

        [Option("requestOnly", Required = false, HelpText = "Only send web requests")]
        public bool RequestOnly { get; set; } = false;

        [Option("finalGame", Required = false, HelpText = "Whether it is the final game")]
        public bool FinalGame { get; set; } = false;

        [Option("cheatMode", Required = false, HelpText = "Whether to open the cheat code")]
        public bool CheatMode { get; set; } = false;
    }
}
