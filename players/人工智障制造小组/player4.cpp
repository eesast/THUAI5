//#include "../include/util.h"

//util.h
#include <random>
#include <utility>
#include <cassert>
#include <chrono>
#include <queue>
#include <cstdarg>
#include "../include/AI.h"

//版本号：051701
//修改时间：05/17 15:30
//1.添加了存储自身buff的bool数组util.my_buff
//2.添加全局常量BUFF_COUNT，表示buff的种类数(5)
//3.添加了用于寻找某方向上最近墙的util.check_nearest_wall
//4.对util.scan_dist进行了性能优化
//5.对util.navigate的默认参数进行了优化
//*.util.evading和util.hitting数组的问题暂时没有解决【我没有权限控制play()去发送这些消息】

//调试相关变量
//log开关，打开则输出log
#define LOG_SWITCH false
//在LOG_SWITCH打开的前提下，打开LOG_STDOUT则将log输出到命令行
#define LOG_STDOUT false
//能够输出的log的最低等级（其实一般不用更改）
#define LOG_LEVEL 0
//是否在每次log之后立刻清空缓冲区
//在调试不明原因崩溃的时候适合打开
#define LOG_ALWAYS_FLUSH false


//结构体声明区

struct Cell;
//坐标结构体
//支持加减、数乘、点乘等运算，可以当做二维向量用
struct Coord {
	//坐标
	int x, y;

	//向量判等、加减、数乘、点乘等运算符
	bool operator==(const Coord& b) const { return (x == b.x) && (y == b.y); }
	bool operator!=(const Coord& b) const { return (x != b.x) || (y != b.y); }
	Coord operator+(const Coord& b) const { return Coord({ x + b.x, y + b.y }); }
	Coord operator-(const Coord& b) const { return Coord({ x - b.x, y - b.y }); }
	Coord operator-() const { return Coord({ -x, -y }); }
	Coord operator*(int k) const { return Coord({ k * x, k * y }); }
	Coord operator*(double k) const { return Coord({ (int)std::round(k * x),(int)std::round(k * y) }); }
	long long operator*(const Coord& b) const { return (long long)x * b.x + (long long)y * b.y; }

	//获取该向量的其中一个垂直向量，其模长与原向量相等
	Coord get_verti() const { return Coord({ -y, x }); }
	//获取该向量的“方块长度”，即|x|+|y|
	int get_block_leng() const { return std::abs(x) + std::abs(y); }
	//获取该向量的长度
	double get_leng() const { return std::sqrt(((long long)x * x) + ((long long)y * y)); }
	//返回该向量与向量b的夹角
	double angle_between(const Coord& b) const;
	//返回从该坐标沿直线走向坐标b应使用的极角
	double angle_to(const Coord& b) const { return (b - *this).to_rad(); }
	//获取该向量对应的极角
	double to_rad() const;
	//获取该坐标所在的格子
	Cell to_cell() const;

	//返回该向量的可读string形式（可用于输出等）
	std::string to_string() const;
};
//格子结构体
//同样可以当做二维向量用
struct Cell {
	//坐标
	int x, y;

	//向量判等、加减、数乘、点乘等运算符
	bool operator==(const Cell& b) const { return (x == b.x) && (y == b.y); }
	bool operator!=(const Cell& b) const { return (x != b.x) || (y != b.y); }
	Cell operator+(const Cell& b) const { return Cell({ x + b.x, y + b.y }); }
	Cell operator-(const Cell& b) const { return Cell({ x - b.x, y - b.y }); }
	Cell operator-() const { return Cell({ -x, -y }); }
	Cell operator*(int k) const { return Cell({ k * x, k * y }); }
	Cell operator*(double k) const { return Cell({ (int)std::round(k * x),(int)std::round(k * y) }); }
	int operator*(const Cell& b) const { return x * b.x + y * b.y; }

	//获取该向量的其中一个垂直向量，其模长与原向量相等
	Cell get_verti() const { return Cell({ -y,x }); }
	//获取该向量的“方块长度”，即|x|+|y|
	int get_block_leng() const { return std::abs(x) + std::abs(y); }
	//获取该向量的长度
	double get_leng() const { return std::sqrt(x * x + y * y); }
	//返回该向量与向量b的夹角
	double angle_between(const Cell& b) const;
	//返回从该坐标沿直线走向坐标b应使用的极角
	double angle_to(const Cell& b) const { return (b - *this).to_rad(); }
	//获取该向量对应的极角
	double to_rad() const;
	//获取该坐标所在的格子
	Coord to_grid() const { return Coord({ IAPI::CellToGrid(x), IAPI::CellToGrid(y) }); }

	//返回该向量的可读string形式（可用于输出等）
	std::string to_string() const;
};
//秒表结构体
//可看距离上次掐表过了多久，或看现在是否到设定的时间了
struct Timer {
	//内部变量
	long long _time;
	long long _tgt;

	//获取当前时刻（半内部函数）
	static long long get_now() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }

	//重置秒表的“开始计时点”为当前时刻
	void reset() { _time = get_now(); }
	//设置秒表的“目标时刻”为当前时间+milli毫秒
	void set_target(int milli) { _tgt = get_now() + milli; }
	//返回从“开始计时点”到现在经过的毫秒数
	int look() const { return get_now() - _time; }
	//返回距离“目标时刻”还有多少毫秒（若目标时刻已过，将返回负值）
	int time_remain() const { return _tgt - get_now(); }
	//检查“目标时刻”是否已到
	bool time_up() const { return get_now() >= _tgt; }
	//Timer在构造时会自动调用reset()和set_target(0)
	Timer() { _time = _tgt = get_now(); }
};
//信息结构体
//保存着一次通讯的信息
struct Message {
	//信息类型
	int type;
	//发送时间戳
	int frame;
	//发送者的playerId(0~3)
	int sender;
	//具体讯息
	std::string msg;

	//从msg中读入数据
	//offset是偏移量（从下标offset开始读取）
	//format和...的参数格式同scanf，返回值也同scanf
	int scan_msg(int offset, const char* format, ...) const;
};


//宏定义及全局常量区

//地图边长
const int MAP_LENGTH = 50;
//场上机器人的数量
const int BOT_COUNT = 8;
//
const int BUFF_COUNT = 5;
//常数PI
const double PI = 3.1415926535897932;

//8向行走有8向...
const int ACT_CNT = 8;
//8向行走的“移动向量”
const Cell ACT_CELL[8] = { {1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1} };
//8向行走对应的极角
const double ACT_RAD[8] = { 0,PI,0.5 * PI,-0.5 * PI,0.25 * PI,-0.25 * PI,0.75 * PI,-0.75 * PI };

//子弹信息表
//NULL、激光、普通、快速、强力
const double BULLET_DMG[5] = { 0, 2000, 2500, 1500, 7000 };
const double BULLET_SPD[5] = { 0, 1000, 2500, 5000, 2000 };
const double BULLET_FLYRNG[5] = { 0, 900, 4500, 9000, 2000 };
const double BULLET_EXPRNG[5] = { 0, 4000, 2500, 1500, 7000 };

//调试相关logger类
class Logger {
private:
	int botid;
	int frame;
	char buffer[505];

	std::FILE* file_log;
	std::FILE* file_table;
public:
	//以等级level记录一条log
	//format和...的参数格式同printf
	void log(int level, const char* format, ...);
	//手动清空缓冲区
	void flush();

	Logger() {}
	~Logger();
	//【内部成员】初始化logger
	void init(int botid);
	//【内部成员】设定log头
	void config(int frame) { this->frame = frame; }
};


//【大锅】实用工具类
class Util {
private:
	//init相关工作
	int _init = 0;

	static const int _MAP_CNT = 2;
	static const int _MAP_CK_CNT = 3;
	static constexpr Cell _MAP_CK_POS[_MAP_CK_CNT][_MAP_CK_CNT] = {
		{{4,18},{25,25},{9,43}}
		,{{24,19},{25,25},{25,30}}
	};
	static constexpr int _MAP_CK_NUM[_MAP_CK_CNT][_MAP_CK_CNT] = { {6,13,1},{2,1,2} };

	//update_data相关工作

	//将被robots指向的内部变量
	THUAI5::Robot _virtual_bot[BOT_COUNT];
	//处理信息
	//格式：<type> <frame> <sender> <msg>
	//type1000：队友信息或敌人信息。格式：顺序输出，除teamID、playerID和buff，buff格式为<n> <bf1> <bf2> ...
	void _get_message();
	//发送自身信息
	void _send_message();

	//get_dist相关准备工作
	Coord _lst_pos[BOT_COUNT];
	struct _BFS_CMP { bool operator()(std::pair<double, Cell> p1, std::pair<double, Cell> p2) { return p1.first > p2.first; } };
	std::priority_queue<std::pair<double, Cell>, std::vector<std::pair<double, Cell>>, _BFS_CMP> _BFS_P_QUEUE;
	void _scan_dist(int botid);


	//【保留名字】
	int enemy_score;

public:
	//公有变量区

	//当前地图类型
	//-1:其它地图 0:初赛地图 1:新地图
	int map_type;
	//各机器人的出生点，下标为机器人编号
	Cell born_point[BOT_COUNT];
	//“丢CPU目的地”
	Cell cpu_home;

	//游戏进行的帧数
	int frame;
	//我方总分
	int team_score;

	//对应playerID的队友是否在躲避子弹？
	//【暂不可用】
	bool evading[4];
	//对应playerID的队友是否在攻击？
	//【暂不可用】
	bool hitting[4];

	//本机器人的各类编号
	int botid, playerId, teamId;
	//本机器人的指针
	const THUAI5::Robot* self;
	//本机器人具有的buff
	bool my_buff[BUFF_COUNT];
	//指向各机器人的指针，下标为机器人编号
	//现在可以保证不含空指针
	const THUAI5::Robot* robots[BOT_COUNT];
	//各机器人信息最后一次更新的frame，下标为机器人编号
	//如果未曾收到该机器人的信息，则值为-100000
	int bot_update[BOT_COUNT];

	//指向各道具及CPU的指针
	std::vector<const THUAI5::Prop*> props;
	//指向各干扰弹的指针
	std::vector<const THUAI5::SignalJammer*> jammers;
	//接收到的信息，下标为Message的type
	std::vector<Message> msgs[10];

	//【半内部】地图各格子的类型
	int map_data[MAP_LENGTH][MAP_LENGTH];
	//【半内部】get_dist的底层数据区
	double dist_map[BOT_COUNT][MAP_LENGTH][MAP_LENGTH];


	//内部成员区

	Util() {}
	~Util() {}
	//【内部函数】初始化
	void pre_init();
	void post_init();
	//【内部函数】每次play()的数据更新
	void update_data();


	//公有函数区

	//检查_cell是否在地图范围内
	bool in_map(const Cell& _cell) const { return _cell.x >= 0 && _cell.y >= 0 && _cell.x < MAP_LENGTH&& _cell.y < MAP_LENGTH; }
	//获取_cell对应格子的类型
	int get_map(const Cell& _cell) const;
	//检查编号为`botid`的机器人是否可见
	//【已移除】
	// bool is_bot_visible(int botid) const { return (bool)robots[botid]; }

	//获取编号为botid的机器人对象
	//botid默认是当前机器人的编号
	const THUAI5::Robot& get_robot(int botid = -1) const;
	//获取_bot的编号
	int get_botid(const THUAI5::Robot& _bot) const { return _bot.teamID * 4 + _bot.playerID; }

	//获取当前机器人的坐标
	Coord get_self_pos() const { return Coord({ (int)self->x, (int)self->y }); }
	//获取_bot的坐标
	Coord get_pos(const THUAI5::Robot& _bot) const { return Coord({ (int)_bot.x, (int)_bot.y }); }
	//获取_prop的坐标
	Coord get_pos(const THUAI5::Prop& _prop) const { return Coord({ (int)_prop.x, (int)_prop.y }); }
	//获取编号为botid机器人的坐标
	//botid的默认值是自身id，注意【目标机器人必须可见！】
	Coord get_pos(int botid = -1) const { return get_pos(get_robot(botid)); }
	//获取编号为botid的机器人到target的距离（考虑墙壁，单位是Cell）
	//botid的默认值是自身id，注意【目标机器人必须可见！】
	double get_dist(const Cell& target, int botid = -1) const;

	//检查编号为botid的机器人是否可以通过_cell代表的格子
	//会考虑墙壁以及其它机器人，botid的默认值是自身id
	bool is_walkable(const Cell& _cell, int botid = -1) const;
	//检查子弹是否能飞过_cell代表的格子
	bool is_flyable(const Cell& _cell) const;
	//使用is_flyable()检查从from到to的线段上是否有墙
	//若walk为真，则使用is_walkable()进行检查
	bool raytrace_wall(const Coord& from, const Coord& to, bool walk = false) const;
	//考虑物体半径obj_r的raytrace_wall()变种
	bool raytrace_wall_r(const Coord& from, const Coord& to, double obj_r, bool walk = false) const;
	//返回从src出发，向direction方向行进，在碰到墙前能走的最大距离，精度略差于100Coord单位
	//obj_r及walk的含义同上
	int check_nearest_wall(const Coord& src, double direction, double obj_r = 0, bool walk = false) const;

	//向playerId为target的队友发送消息msg
	//【target的取值范围是0~3】
	void send(int target, int msg_type, const std::string& msg);
	//send函数的printf格式版本
	void send_f(int target, int msg_type, const char* format, ...);
	//向全队广播msg
	void broadcast(int msg_type, const std::string& msg) { for (int i = 0; i < 4; i++) send(i, msg_type, msg); }
	//broadcast函数的printf格式版本
	void broadcast_f(int msg_type, const char* format, ...);
	//向除了自己的其它人广播
	void broadcast_other_f(int msg_type, const char* format, ...);

	//【半内部】寻路函数，输入起点及终点坐标，输出下一步的移动方向
	//【由关毅烜接管】
	double find_path(const Coord& from, const Coord& to);
	//让当前机器人向to移动max_dist的距离（调用find_path来寻路），max_dist默认是50ms能走的距离
	//【由关毅烜接管】
	void navigate(const Coord& to, int max_dist = -1);
};


//util.cpp
//版本号：051701
//修改时间：05/17 15:30


//引入外部变量
extern IAPI* api;
extern Logger logger;


//坐标结构体
double Coord::angle_between(const Coord& b) const {
	assert(this->get_block_leng() * b.get_block_leng());
	return std::acos(((*this) * b) / this->get_leng() / b.get_leng());
}
double Coord::to_rad() const {
	if (this->y < 0) return 2 * PI - Coord({ 1,0 }).angle_between(*this);
	else return Coord({ 1,0 }).angle_between(*this);
}
Cell Coord::to_cell() const { return Cell({ IAPI::GridToCell(x), IAPI::GridToCell(y) }); }
std::string Coord::to_string() const {
	static char buf[20];
	sprintf(buf, "(%5d,%5d)", x, y);
	return std::string(buf);
}
//格子结构体
double Cell::angle_between(const Cell& b) const {
	assert(this->get_block_leng() * b.get_block_leng());
	return std::acos(((*this) * b) / this->get_leng() / b.get_leng());
}
double Cell::to_rad() const {
	if (this->y < 0) return 2 * PI - Cell({ 1,0 }).angle_between(*this);
	else return Cell({ 1,0 }).angle_between(*this);
}
std::string Cell::to_string() const {
	static char buf[10];
	sprintf(buf, "(%2d,%2d)", x, y);
	return std::string(buf);
}
//信息结构体
int Message::scan_msg(int offset, const char* format, ...) const {
	va_list args;
	va_start(args, format);
	int ret = vsscanf(msg.c_str() + offset, format, args);
	va_end(args);
	return ret;
}


//Logger类
Logger::~Logger() {
	flush();
	fclose(file_log);
	fclose(file_table);
}
void Logger::log(int level, const char* format, ...) {
	if (!LOG_SWITCH) return;
	if (level >= LOG_LEVEL) {
		va_list args;
		va_start(args, format);
		vsprintf(buffer, format, args);
		va_end(args);
		fprintf(file_log, "%05d: %s\n", frame, buffer);
	}
	fprintf(file_table, "%d\t%d\t%s\n", frame, botid, buffer);
	if (LOG_ALWAYS_FLUSH) flush();
}
void Logger::flush() {
	fflush(file_log);
	fflush(file_table);
}
void Logger::init(int _botid) {
	botid = _botid;
	char log_name[20];
	char table_name[20];

	sprintf(log_name, "player%d.log", botid);
	sprintf(table_name, "table%d.log", botid);
	if (LOG_STDOUT) file_log = stdout;
	else file_log = fopen(log_name, "w");
	file_table = fopen(table_name, "w");
}


//实用工具类
void Util::_scan_dist(int botid) {
	const Cell& from = get_pos(botid).to_cell();

	while (_BFS_P_QUEUE.size()) _BFS_P_QUEUE.pop();
	for (int i = 0; i < MAP_LENGTH; i++) for (int j = 0; j < MAP_LENGTH; j++) dist_map[botid][i][j] = -1;
	dist_map[botid][from.x][from.y] = 0;

	_BFS_P_QUEUE.push(std::make_pair(0, from));
	while (_BFS_P_QUEUE.size()) {
		std::pair<double, Cell> temp = _BFS_P_QUEUE.top();
		_BFS_P_QUEUE.pop();
		for (int i = 0; i < ACT_CNT; i++) {
			Cell next = temp.second + ACT_CELL[i];
			if (dist_map[botid][next.x][next.y] != -1 || !in_map(next)) continue;
			if (i >= 4) if (!is_walkable(temp.second + ACT_CELL[i >= 6]) || !is_walkable(temp.second + ACT_CELL[2 + (i % 2)])) continue;

			if (is_walkable(next)) {
				dist_map[botid][next.x][next.y] = dist_map[botid][temp.second.x][temp.second.y] + ((i < 4) ? 1 : 1.41421);
				_BFS_P_QUEUE.push(std::make_pair(dist_map[botid][next.x][next.y], next));
			}
		}
	}
}
void Util::pre_init() {
	if (_init) return;
	_init = 1;
	//logger初始化
	logger.init(get_botid(*api->GetSelfInfo()));
	//地图相关初始化
	for (int x = 0; x < MAP_LENGTH; x++) {
		for (int y = 0; y < MAP_LENGTH; y++) {
			map_data[x][y] = (int)api->GetPlaceType(x, y);
			if (map_data[x][y] >= 5 && map_data[x][y] <= 12) born_point[map_data[x][y] - 5] = Cell({ x,y });
		}
	}
	//地图类型检查
	map_type = -1;
	for (int i = 0; i < _MAP_CNT; i++) {
		bool passed = true;
		for (int j = 0; j < _MAP_CK_CNT && passed; j++) if (get_map(_MAP_CK_POS[i][j]) != _MAP_CK_NUM[i][j]) passed = false;
		if (passed) {
			map_type = i;
			break;
		}
	}
	printf("\nThis is map %d\n", map_type);
	logger.log(1, "This is map %d", map_type);
	//Robot初始化
	for (int i = 0; i < BOT_COUNT; i++) {
		_lst_pos[i] = Coord({ -1,-1 });
		robots[i] = &_virtual_bot[i];
		bot_update[i] = -100000;
	}
}
void Util::post_init() {
	if (_init != 1) return;
	_init = 2;

	//选择cpu_home
	int rnd = Timer::get_now() / 3600000;//小时数
	if (map_type == -1) cpu_home = born_point[teamId * 4 + rnd % 4];
	else if (map_type == 0) {
		if (teamId) cpu_home = born_point[teamId * 4 + rnd % 3 + 1];
		else cpu_home = born_point[teamId * 4 + (rnd % 2) * 3];
	}
	else if (map_type == 1) cpu_home = born_point[teamId * 4 + rnd % 2 + 1];
}
void Util::update_data() {
	pre_init();

	//开始维护
	frame = api->GetFrameCount();
	printf("\nplay() at frame %d\n", frame);
	team_score = api->GetTeamScore();

	//设定logger头
	logger.config(frame);

	//Robots的初步维护
	self = &*api->GetSelfInfo();
	botid = get_botid(*self), playerId = self->playerID, teamId = self->teamID;
	for (const auto& _bot : api->GetRobots()) {
		int _botid = get_botid(*_bot);
		_virtual_bot[_botid] = *_bot;
		bot_update[_botid] = frame;
	}
	_virtual_bot[botid] = *self;
	bot_update[botid] = frame;

	//信息处理并第二次维护Robots
	_send_message();
	_get_message();

	//dist的维护
	int cnt = 0;
	for (int i = 0; i < BOT_COUNT; i++) {
		if (bot_update[i] >= 0 && _lst_pos[i].to_cell() != get_pos(i).to_cell()) cnt++, _scan_dist(i);
		_lst_pos[i] = get_pos(i);
	}
	//my_buff的维护
	for (int i = 0; i < BUFF_COUNT; i++) my_buff[i] = false;
	for (const auto& _buff : self->buff) my_buff[(int)_buff] = true;
	//props, jammers的维护
	props.clear(), jammers.clear();
	for (const auto& _prop : api->GetProps()) props.push_back(&*_prop);
	for (const auto& _jam : api->GetSignalJammers()) jammers.push_back(&*_jam);


	post_init();
}
void Util::_get_message() {
	while (true) {
		const auto& tmp = api->TryGetMessage();
		if (!tmp.has_value()) break;
		std::string content = tmp.value();
		size_t next = 0;

		int type = std::stoi(content, &next);
		content = content.substr(next);
		int frame = std::stoi(content, &next);
		content = content.substr(next);
		int sender = std::stoi(content, &next);
		content = content.substr(next);

		Message msg_obj({ type,frame,sender,content });
		//直接截流特殊消息
		if (type == 1000) {//队友自身信息或敌人信息
			int offset = 0, delta;
			int desc_id = 0;
			msg_obj.scan_msg(offset, "%d%n", &desc_id, &offset);//获取id
			if (bot_update[desc_id] == this->frame) continue;

			int buf_cnt;
			THUAI5::Robot& tg = _virtual_bot[desc_id];
			msg_obj.scan_msg(offset, "%d %d %d %d %d %d %d %d %d %d %d %d %d %lf %lf %lf %lf %d %d %d %d %d %d%n"
				, &tg.canMove, &tg.isResetting, &tg.x, &tg.y, &tg.signalJammerNum, &tg.speed, &tg.life, &tg.cpuNum, &tg.radius, &tg.CD, &tg.lifeNum, &tg.score, &tg.guid //2个bool+11个int
				, &tg.attackRange, &tg.timeUntilCommonSkillAvailable, &tg.timeUntilUltimateSkillAvailable, &tg.emissionAccessory //4个double
				, &tg.prop, &tg.place, &tg.signalJammerType, &tg.hardwareType, &tg.softwareType //5个type(int)
				, &buf_cnt, &delta); // buff的数量，以及读取字符数
			offset += delta;

			tg.buff.clear();
			for (int i = 0; i < buf_cnt; i++, offset += delta) {
				int tmp;
				msg_obj.scan_msg(offset, "%d%n", &tmp, &delta);
				tg.buff.push_back((THUAI5::BuffType)tmp);
			}

			bot_update[desc_id] = frame;
			robots[desc_id] = &_virtual_bot[desc_id];
			logger.log(0, "update robot %d complete (data from frame %d)", desc_id, frame);
			continue;
		}
		else if (type == 1001) {//evading及hitting信息
			msg_obj.scan_msg(0, "%d%d", &evading[sender], &hitting[sender]);
			continue;
		}

		assert(type >= 0 && type < 10);
		msgs[type].push_back(msg_obj);
		logger.log(0, "received msg from bot %d frame %d : %s", sender, frame, content.c_str());
	}
}
void Util::_send_message() {
	//发敌人信息
	for (int i = 0; i < BOT_COUNT; i++) {
		if (!robots[i] || robots[i]->teamID == teamId) continue;//自己人或不可见
		const THUAI5::Robot& a = *robots[i];
		if ((int)a.place < 2 || (int)a.place > 4) continue;//不在盲区中

		std::string buffvec = std::to_string(a.buff.size()) + " ";
		for (const auto& _buff : a.buff) buffvec += std::to_string((int)_buff) + " ";
		broadcast_other_f(1000, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %.6f %.6f %.6f %.6f %d %d %d %d %d %s"
			, i, a.canMove, a.isResetting, a.x, a.y, a.signalJammerNum, a.speed, a.life, a.cpuNum, a.radius, a.CD, a.lifeNum, a.score, a.guid //1个id+2个bool+11个int
			, a.attackRange, a.timeUntilCommonSkillAvailable, a.timeUntilUltimateSkillAvailable, a.emissionAccessory //4个.6f
			, a.prop, a.place, a.signalJammerType, a.hardwareType, a.softwareType //5个type(int)
			, buffvec.c_str());//buff_vector(string)
		logger.log(0, "broadcast message of %d", i);
	}

	//发自己的信息
	if ((int)self->place < 2 || (int)self->place > 4) return;//自己不在盲区中

	const THUAI5::Robot& a = *self;
	std::string buffvec = std::to_string(a.buff.size()) + " ";
	for (const auto& _buff : a.buff) buffvec += std::to_string((int)_buff) + " ";
	broadcast_other_f(1000, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %.6f %.6f %.6f %.6f %d %d %d %d %d %s"
		, botid, a.canMove, a.isResetting, a.x, a.y, a.signalJammerNum, a.speed, a.life, a.cpuNum, a.radius, a.CD, a.lifeNum, a.score, a.guid //1个id+2个bool+11个int
		, a.attackRange, a.timeUntilCommonSkillAvailable, a.timeUntilUltimateSkillAvailable, a.emissionAccessory //4个.6f
		, a.prop, a.place, a.signalJammerType, a.hardwareType, a.softwareType //5个type(int)
		, buffvec.c_str());//buff_vector(string)
	logger.log(0, "broadcast message of myself");
}

int Util::get_map(const Cell& _cell) const {
	assert(in_map(_cell));
	return map_data[_cell.x][_cell.y];
}
const THUAI5::Robot& Util::get_robot(int botid) const {
	if (botid == -1) botid = this->botid;
	assert(robots[botid] && botid >= 0 && botid < BOT_COUNT);
	return *robots[botid];
}
double Util::get_dist(const Cell& target, int botid) const {
	if (botid == -1) botid = this->botid;
	assert(botid >= 0 && botid < BOT_COUNT);
	assert(in_map(target));
	return dist_map[botid][target.x][target.y];
}

bool Util::is_walkable(const Cell& _cell, int botid) const {
	if (botid == -1) botid = this->botid;
	int val = get_map(_cell);
	assert(val != 14);
	if (val == 1 || (val >= 5 && val <= 12 && botid + 5 != val)) return false;
	for (int i = 0; i < BOT_COUNT; i++) if (i != botid && bot_update[i] + 10 >= frame && get_pos(i).to_cell() == _cell) return false;
	return true;
}
bool Util::is_flyable(const Cell& _cell) const {
	int val = get_map(_cell);
	assert(val != 14);
	if (val == 1) return false;
	return true;
}
bool Util::raytrace_wall(const Coord& from, const Coord& to, bool walk) const {
	assert(in_map(from.to_cell()));
	assert(in_map(to.to_cell()));
	int check_num = 10 + int((from - to).get_leng() / 800);
	const Coord delta({ (to.x - from.x) / check_num, (to.y - from.y) / check_num });
	for (Coord now = from; check_num > 0; check_num--, now = now + delta) {
		if (!walk && !is_flyable(now.to_cell())) return true;
		else if (walk && !is_walkable(now.to_cell())) return true;
	}
	return false;
}
bool Util::raytrace_wall_r(const Coord& from, const Coord& to, double obj_r, bool walk) const {
	assert(in_map(from.to_cell()));
	assert(in_map(to.to_cell()));
	if (raytrace_wall(from, to, walk)) return true;

	const Coord& verti = (to - from).get_verti() * (obj_r / (to - from).get_leng());
	return raytrace_wall(from + verti, to + verti, walk) || raytrace_wall(from - verti, to - verti, walk);
}
int Util::check_nearest_wall(const Coord& src, double direction, double obj_r, bool walk) const {
	assert(in_map(src.to_cell()));

	static const double PACE = 100;
	const Coord verti({ (int)round(obj_r * sin(direction)), (int)round(-obj_r * cos(direction)) });

	bool ok = true;
	const double step_x = PACE * cos(direction);
	const double step_y = PACE * sin(direction);
	for (double nx = src.x, ny = src.y, dis = 0; ; dis += PACE, nx += step_x, ny += step_y) { //写得真臭啊...
		if (!walk && !is_flyable(Coord({ (int)nx,(int)ny }).to_cell())) ok = false;
		else if (walk && !is_walkable(Coord({ (int)nx,(int)ny }).to_cell())) ok = false;
		if (obj_r) {
			if (!walk && !is_flyable((Coord({ (int)nx,(int)ny }) + verti).to_cell())) ok = false;
			else if (walk && !is_walkable((Coord({ (int)nx,(int)ny }) + verti).to_cell())) ok = false;
			if (!walk && !is_flyable((Coord({ (int)nx,(int)ny }) - verti).to_cell())) ok = false;
			else if (walk && !is_walkable((Coord({ (int)nx,(int)ny }) - verti).to_cell())) ok = false;
		}
		if (!ok || !in_map(Coord({ (int)nx,(int)ny }).to_cell())) return dis;
	}
	assert(false);
}

void Util::send(int target, int msg_type, const std::string& msg) {
	std::string sending = std::to_string(msg_type) + ' ' + std::to_string(frame) + ' ' + std::to_string(playerId) + ' ' + msg;
	api->Send(target, sending);
}
void Util::send_f(int target, int msg_type, const char* format, ...) {
	static char buf[270];
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	send(target, msg_type, std::string(buf));
}
void Util::broadcast_f(int msg_type, const char* format, ...) {
	static char buf[270];
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	broadcast(msg_type, std::string(buf));
}
void Util::broadcast_other_f(int msg_type, const char* format, ...) {
	static char buf[270];
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	std::string str_buf(buf);
	for (int i = 0; i < (BOT_COUNT >> 1); i++) if (i != playerId) send(i, msg_type, str_buf);
}

double Util::find_path(const Coord& from, const Coord& to) {
	static double dist[MAP_LENGTH][MAP_LENGTH];
	const Cell _from = from.to_cell(), _to = to.to_cell();//把目标点和初始点写成cell类型
	assert(_from.x != 0 && _from.x != MAP_LENGTH - 1 && _from.y != 0 && _from.y != MAP_LENGTH - 1);

	while (_BFS_P_QUEUE.size()) _BFS_P_QUEUE.pop();
	for (int i = 0; i < MAP_LENGTH; i++) for (int j = 0; j < MAP_LENGTH; j++) dist[i][j] = 1e9;

	dist[_to.x][_to.y] = 0;//置目的地距离为0，同时入队
	_BFS_P_QUEUE.push(std::make_pair(0, _to));
	while (_BFS_P_QUEUE.size()) {
		std::pair<double, Cell> temp = _BFS_P_QUEUE.top();
		_BFS_P_QUEUE.pop();

		for (int i = 0; i < ACT_CNT; i++) {
			Cell next = temp.second + ACT_CELL[i];//下一步要走到的节点
			if (dist[next.x][next.y] != 1e9 || !in_map(next)) continue;
			if (i >= 4)//对“斜着走时会经过的另外两个对角线格点”额外判定
				if (!is_walkable(temp.second + ACT_CELL[i >= 6]) || !is_walkable(temp.second + ACT_CELL[2 + (i % 2)])) continue;

			if (is_walkable(next)) {
				dist[next.x][next.y] = dist[temp.second.x][temp.second.y] + ((i < 4) ? 1 : 1.41421);
				_BFS_P_QUEUE.push(std::make_pair(dist[next.x][next.y], next));
			}
		}
	}
	//4.20以上部分是算出一张距离地图，把每个点都标上距离，这部分应该没有问题（有问题也应该被下面一段强烈的筛查发现），所以主要看下面的判断

	//到此为止能实现规划出一条正确的路径且完成了距离地图，下一步要实现输出具体的路。需要比较一个点八向行走后生成的八个点的距离，最小者输出。
	double nearest_dst = 1e9;
	int direction = 0;
	bool compound_dir = true;//可否斜向行走
	for (int i = 0; i < ACT_CNT && compound_dir; i++) if (!is_walkable(_from + ACT_CELL[i])) compound_dir = false;
	for (int i = 0; i < ACT_CNT; i++) {
		Cell next = _from + ACT_CELL[i];

		//4.20  基本判断，包括要求坐标合法，迭代后的坐标距离更小，下一步可走；（不管是斜着走还是直着走都应该满足基本判断）
		if (!in_map(next) || dist[next.x][next.y] >= nearest_dst || !is_walkable(next)) continue;

		//4.20  针对45°行走的判断，要求8方向无墙；（要求整个九宫格内都没有障碍物，因为奇怪的情况太多了，这里直接把约束加到最强）
		if (i >= 4 && !compound_dir) break;//8方向无墙只需判断一次
		nearest_dst = dist[next.x][next.y], direction = i;
	}
	//4.20已经做好第一步路径规划，这一步规划保证了要么是直着走；要么是保证不会撞墙的斜着走
	//4.20接下来做的事情是避免因为PI值的不精确导致 与墙平行直着走也会撞到墙上
	static const int WALL_DST = 30;
	if (direction == 0 || direction == 1) {
		Cell left = _from;
		left.y--;
		Cell right = _from;
		right.y++;

		//4.20  判断机器人靠不靠墙，如果靠墙太近就以远离的方向取代原来做出的路径规划
		//（使用傻瓜循环对每一个四向行走的方向都检查一遍，剩下三个同理）
		if (!(is_walkable(left + ACT_CELL[direction]) && is_walkable(left)))
			if (from.y - num_of_grid_per_cell * _from.y < 500 + WALL_DST) return PI * 0.5;
		if (!(is_walkable(right + ACT_CELL[direction]) && is_walkable(right)))
			if (from.y - num_of_grid_per_cell * _from.y > 500 - WALL_DST) return -PI * 0.5;
	}
	else if (direction == 2 || direction == 3) {
		Cell up = _from;
		up.x--;
		Cell down = _from;
		down.x++;

		if (!(is_walkable(up + ACT_CELL[direction]) && is_walkable(up)))
			if (from.x - num_of_grid_per_cell * _from.x < 500 + WALL_DST) return 0;
		if (!(is_walkable(down + ACT_CELL[direction]) && is_walkable(down)))
			if (from.x - num_of_grid_per_cell * _from.x > 500 - WALL_DST) return PI;
	}
	// printf("find path selected dir (%d,%d)\n",ACT_CELL[direction].x,ACT_CELL[direction].y);
	return ACT_RAD[direction];
}
void Util::navigate(const Coord& to, int max_dist) {
	const Coord& from = get_self_pos();
	if (max_dist < 0) max_dist = self->speed / 20;
	if (!raytrace_wall_r(from, to, 550, true)) {
		// printf("trace success, walk %d ms to %.2f\n",max_dist * num_of_grid_per_cell / self->speed,from.angle_to(to));
		api->MovePlayer(max_dist * num_of_grid_per_cell / self->speed, from.angle_to(to));
		return;
	}

	double b = find_path(from, to);
	(*api).MovePlayer(max_dist * num_of_grid_per_cell / self->speed, b);
}

//AI.cpp

//AI常量区

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous = false;
// 选手主动技能，选手 !!必须!! 定义此变量来选择主动技能
extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Invisible;
// 选手被动技能，选手 !!必须!! 定义此变量来选择被动技能
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::PowerBank;


//全局变量区

//指向IAPI的指针，每次调用play时将自动更新
IAPI* api;
//杂七杂八类Util的实例
Util util;
//Logger实例
Logger logger;


//“机器人状态空间”
//版本号：050101
//修改时间：05/01 20:30
namespace Status {
	Timer play_timer;

	//当前阶段
	int stage = 1;

	//是否正在躲避子弹
	bool evading;
	//是否正在攻击
	bool hitting;
	//是否正在surprise
	bool surprising;

	int carrier_botid = -1;
	int team_score;

	//接CPU附加信息
	namespace catch_st {
		int catching_flag = 0;
		int Tgt_x = 0;
		int Tgt_y = 0;
		int Fly_t = 0;
		Timer tmr;
	}
	//扔cpu和扔回家模式附加信息
	namespace throw_st {
		double throw_direction = -1;
		bool throw_home_flag = true;
		Cell home;
		int moving_flag = 0;
		Coord moving_target{-1,-1};
		bool search_again = false;
		int search_time = 0;
	}
	//捡cpu附加信息
	namespace pick_st {
		int pick_home_flag = 0;
		int home_picked_flag = 0;
		int home_cpu_num = 0;
		int home_id = -1;
	}
	//占据草丛附加信息
	namespace stand_st {
		Cell stand_pos[6];
		bool stand_flag = true;
		int standing = 0;
		Cell stand_target = { -1,-1 };
	}

	//更新Status内Actions相关信息
	bool action_info_update();

	//计算可能受到的伤害
	int pro_damage();

	//计算最近的强力弹敌人到自己的距离
	int min_strong_emy_dist();

}


//“动作空间”
namespace Actions {
	//判断是否需要帮队友接cpu，若是则返回true并直接进行对应操作，否则返回false
	//**注意原有的通讯方式已经不可用了**
	//【由崔祎璐负责】
	//安排捡CPU的行为
	bool try_catch();
	//【由崔祎璐负责】
	//尝试向队友传递CPU，如果能传就立刻传并返回true
	bool try_throw(bool to_home,bool mov, int _botid = -1);
	//【由崔祎璐负责】
	//尝试捡cpu或道具
	bool try_pick(bool home);
	//【由张蕊负责】
	//自身隐身好了时，模式1：Boring时找一个最近的敌人去surprise 模式2：范围内有人则surprise
	bool try_surprise(int sur_mod);
	//【由关毅烜负责】
	//检查攻击范围内是否有敌人，若是则返回true并直接进攻，否则返回false
	bool try_hit_mobile();
	bool try_hit_laser();
	bool try_hit_strong();
	bool try_hit_evade();
	//【由张蕊负责】
	//检查范围内是否出现“有威胁的子弹”与“有威胁的敌人”，若是则返回true并进行躲避，否则返回false
	bool evade_bullet();
	//广播自身目标信息
	void broadcast_pickinfo(Coord& tgt);
	//试图占据cpu工厂附近的草丛
	bool try_stand();
	//试图聚众（用来打人）
	bool try_gather();
	//广播聚众信息
	void broadcast_gatherinfo(int _botid);
	//清除聚众信息
	void clear_gatherinfo();
}

//版本号：050301
//修改时间：0502
//规范了通信方式
bool Actions::try_catch() {//返回值表示需不需要为了捡cpu阻止对其他函数的调用
	const int throw_command_type = 1;
	if (Status::catch_st::catching_flag == 0) {
		if (util.msgs[throw_command_type].size() == 0) return false;//没有未接收的消息，返回false
		//std::vector<int> command;//提取消息队列的请求类型，位置信息,仅保存command=1的位置信息
		std::vector<int> target_x;
		std::vector<int> target_y;
		std::vector<int> cpu_num;
		std::vector<int> fly_time;
		int catch_called_flag = 0;//收到的接cpu请求的数目
		int max_cpu_num = 0;//请求接的最多的cpu
		int try_catch_number = 0;//应该接受的请求编号
		for (int index = 0; index < util.msgs[throw_command_type].size(); index++) {
			if (util.msgs[throw_command_type][index].frame < util.frame - 20) continue;
			std::string Msg = util.msgs[throw_command_type][index].msg;
			char* p;
			const char* d = " ";
			p = strtok((char*)(Msg).c_str(), d);
			target_x.push_back(atoi(p));
			p = strtok(NULL, d);
			target_y.push_back(atoi(p));
			p = strtok(NULL, d);
			cpu_num.push_back(atoi(p));
			p = strtok(NULL, d);
			fly_time.push_back(atoi(p));
		}
		util.msgs[throw_command_type].clear();

		if (cpu_num.size() == 0) return false;
		else {
			for (int i = 0; i < cpu_num.size(); i++) {
				if ((i == 0) || (cpu_num[i] >= max_cpu_num)) {
					max_cpu_num = cpu_num[i];
					try_catch_number = i;
				}
			}//找出哪个请求的cpu数目最多
		}
		Status::catch_st::Tgt_x = target_x[try_catch_number];
		Status::catch_st::Tgt_y = target_y[try_catch_number];
		Status::catch_st::Fly_t = fly_time[try_catch_number];
		Status::catch_st::tmr.reset();
		Status::catch_st::catching_flag = 1;
		Coord catch_target{ Status::catch_st::Tgt_x,Status::catch_st::Tgt_y };//构造目标位置
		util.navigate(catch_target);//前往目标位置
		return true;
	}
	if (Status::catch_st::catching_flag == 1) {
		Coord catch_target{ Status::catch_st::Tgt_x,Status::catch_st::Tgt_y };
		if (util.get_pos().to_cell() == catch_target.to_cell()) {
			api->Pick((THUAI5::PropType)3);
			Status::catch_st::catching_flag = 0;
			Status::catch_st::Tgt_x = 0;
			Status::catch_st::Tgt_y = 0;
			Status::catch_st::Fly_t = 0;
			return false;

		}
		else if (Status::catch_st::tmr.look() <= 20000) {
			if (api->Pick((THUAI5::PropType)3)) return false;
			util.navigate(catch_target);
			return true;
		}
		else {
			api->Pick((THUAI5::PropType)3);
			Status::catch_st::catching_flag = 0;
			Status::catch_st::Tgt_x = 0;
			Status::catch_st::Tgt_y = 0;
			Status::catch_st::Fly_t = 0;
			return false;
		}
	}
	else return false;
}
//版本号：051601
//修改时间：0516 19：20
//增加了最多3次的重新搜索机制，防止敌方拦截
bool Actions::try_throw(bool to_home,bool mov, int _botid) {
	if (to_home) {//扔回家模式
		if (mov) {//主动扔模式
			if ((!Status::throw_st::moving_flag)) {
				const Coord& my_pos = util.get_self_pos();
				const Coord& home = Status::throw_st::home.to_grid();
				double min_enemy_straight_time = -1;
				const double max_straight_range = 0.5;//这个半径内有敌人的时候不选这个方向，防拦截
				int test_length;//长度搜索变量
				double test_angle;//角度搜索变量
				const int min_length = 2900;//搜索范围1160-14500（暂定）
				const int max_length = 14500;
				double(*mov_dist)[20] = new double[41][20]();
				double min_dist = 80;
				printf("1");
				//暴力遍历所有可能的扔cpu地点
				for (test_length = min_length; test_length <= max_length; test_length += 290)
					for (test_angle = 0; test_angle < 2 * PI; test_angle += 0.1 * PI) {
						int i = ((test_length-2610 )/ 290) - 1;
						int j = test_angle / (0.1 * PI);
						assert(mov_dist[i][j] == 0);
						double test_x =home.x + test_length * cos(test_angle);//计算目标位置的坐标
						double test_y = home.y + test_length * sin(test_angle);
						if ((test_x < 2000) || (test_y < 2000) || (test_x > 48000) || (test_y > 48000)) continue;
						Coord test_target{ (int)test_x,(int)test_y };//构造目标位置
						Cell target_cell = test_target.to_cell();
						printf("2");
						//防止拦截（主要防止就地扔时候的拦截）
						int enemy_flag = 0;
						for (int id = 0; id < 8; id++) {
							if ((util.robots[id] == NULL) || (util.bot_update[id] < util.frame - 10)) continue;
							if (util.robots[id]->teamID == util.self->teamID) continue;
							Coord enemy_pos = util.get_pos(id);
							double straight_dist = (enemy_pos - home).get_leng();
							double straight_time = straight_dist / util.robots[id]->speed;
							if ((enemy_flag == 0) || (straight_time < min_enemy_straight_time)) {
								min_enemy_straight_time = straight_time;
								enemy_flag = 1;
							}
						}
						printf("3");
						//一些不可取的地点
						if (!util.is_walkable(target_cell)) continue;
						if (util.raytrace_wall_r(test_target, home,600)) continue;
						if ((enemy_flag != 0) && (min_enemy_straight_time < max_straight_range * test_length / 3000)&&(min_enemy_straight_time!=-1)) continue;
						mov_dist[i][j] = util.get_dist(target_cell);
					}
				printf("4");
				//选最近的可取的地点
				int max_i = 0;
				int max_j = 0;
				for (int i = 0; i < 41; i++)
					for (int j = 0; j < 20; j++) {
						if (mov_dist[i][j] < min_dist&&mov_dist[i][j]!=0) {
							min_dist = mov_dist[i][j];
							max_i = i;
							max_j = j;
						}
					}
				delete []mov_dist;

				printf("5");
				if (min_dist == 80||util.self->cpuNum==0) {//如果都不可取或者根本没得扔就不扔了，直接初始化
					Status::throw_st::moving_target = { -1,-1 };
					Status::throw_st::moving_flag = 0;
					return false;
				}
				else {//构造好目标地点等信息存下来，如果直接扔了就初始化
					printf("6");
					double tgt_x = home.x + (290 * (max_i + 1)+2610) * cos(0.1 * PI * max_j);
					double tgt_y = home.y + (290 * (max_i + 1)+2610) * sin(0.1 * PI * max_j);
					Coord tgt{ (int)tgt_x,(int)tgt_y };
					Status::throw_st::moving_target = tgt;
					Status::throw_st::moving_flag = 1;
					if (((my_pos-tgt).x>=-300)&& ((my_pos - tgt).x <= 300)&& ((my_pos - tgt).y >= -300)&& ((my_pos - tgt).y <= 300)) {
						double throw_angle;
						if (max_j * 0.1 * PI >= PI) throw_angle = max_j * 0.1 * PI - PI;
						else throw_angle = PI + max_j * 0.1 * PI;
						int throw_time = ((max_i + 1) * 290+2610) / 3;
						api->ThrowCPU(throw_time, throw_angle, util.self->cpuNum);
						Status::throw_st::moving_target = { -1,-1 };
						Status::throw_st::moving_flag = 0;
						Status::throw_st::search_again = false;
						Status::throw_st::search_time = 0;
						return false;
					}
					else {//不能直接扔就导航去目的地
						printf("7");
						printf("tgt:%d,%d\n", tgt.x, tgt.y);
						util.navigate(tgt);
						return true;
					}
				}
			}
			else if ((Status::throw_st::moving_target.x == -1) || (Status::throw_st::moving_target.y == -1)) {//错误调用的情况，直接初始化
				printf("8");
				Status::throw_st::moving_flag = 0;
				Status::throw_st::moving_target = { -1,-1 };
				Status::throw_st::search_again = false;
				Status::throw_st::search_time = 0;
				return false;
			}
			else {//已经搜索好了，判断自己是不是到了扔cpu的地点
			printf("9");
				Coord my_pos = util.get_self_pos();
				if (((my_pos - Status::throw_st::moving_target).x >= -300) && ((my_pos - Status::throw_st::moving_target).x <= 300) && ((my_pos - Status::throw_st::moving_target).y >= -300) && ((my_pos - Status::throw_st::moving_target).y <= 300)) {
					const Coord& home = Status::throw_st::home.to_grid();
					auto self = api->GetSelfInfo();
					const Coord& self_pos = { self->x,self->y };
					int throw_time = (Status::throw_st::home.to_grid() - self_pos).get_leng() / 3;
					double throw_angle = (Status::throw_st::home.to_grid() - self_pos).to_rad();
					api->ThrowCPU(throw_time, throw_angle, util.self->cpuNum);
					Status::throw_st::moving_target = { -1,-1 };
					Status::throw_st::moving_flag = 0;
					Status::throw_st::search_again = false;
					Status::throw_st::search_time = 0;
					return false;

				}
				else {
					printf("12");
					//如果没到目标地点就导航去
					util.navigate(Status::throw_st::moving_target);
					return true;
				}
			}
		}
		else {
			const Coord& my_pos = util.get_self_pos();
			const Coord& home = Status::throw_st::home.to_grid();
			if ((!util.raytrace_wall_r(my_pos, home, 600)) && ((my_pos - home).get_leng() <= 14500)) {
				int throw_time = (my_pos - home).get_leng() / 3;
				double throw_angle = (home - my_pos).to_rad();
				api->ThrowCPU(throw_time, throw_angle, util.self->cpuNum);
				return true;
			}
			else return false;
		}
	}
	else {
		int test_length;//长度搜索变量
		double test_angle;//角度搜索变量
		const int min_length = 290;//搜索范围290-14500（暂定）
		const int max_length = 14500;
		int movable_partner_num = 0;//能动的队友数量，为0则没法扔出
		int visible_enemy_num = 0;//能看到的敌人数量
		double min_enemy_dist_time = 0;//敌人到达扔出目标的最小的时间
		double min_partner_dist_time = 0;//队友到达目标的最小的时间
		double min_enemy_straight_time = 0;
		const double max_straight_range = 0.5;//这个半径内有敌人的时候不选这个方向，防拦截
		double(*score)[20] = new double[50][20]();//储存所有搜索位置对应的分数
		int(*test_catch_id)[20] = new int[50][20]();//搜索位置对应的最适合接cpu的队友id，是实际id+10，保证id=0无意义
		int catch_id;//最终决定去接cpu的队友id
		const Coord&& my_pos = util.get_self_pos();//自己当前的位置
		const double zero_enemy_weight = 3;//下面四行是四个参数，用于构成score
		const double dist_diff_weight = 2;
		const double partner_life_weight = 0;
		const double throwable_min_score = 1;//分数的阈值，最高分小于此值则无法扔出cpu
		const int throw_command_type = 1;//扔cpu的消息类型


		if ((_botid != -1) && ((_botid < 0) || (_botid > 7))) return false;//输入id错误
		if (_botid != -1) {
			if ((util.robots[_botid] == NULL)||(util.bot_update[_botid]<util.frame-10)) return false;
			if (!util.robots[_botid]->canMove) return false;
			if (util.robots[_botid]->isResetting) return false;
			if (_botid == util.botid) return false;
		}
		for (int id = 0; id < 8; id++) {
			if ((util.robots[id] != NULL)&&(util.bot_update[id]>=util.frame-10)) {
				if (util.robots[id]->teamID != util.self->teamID) visible_enemy_num++;
				if ((util.robots[id]->teamID == util.self->teamID) && util.robots[id]->canMove) movable_partner_num++;
			}
		}
		if (movable_partner_num == 0) return false;//看不到能动的队友则不扔
		for (test_length = min_length; test_length <= max_length; test_length += 290)//搜索间隔290（暂定）
			for (test_angle = 0; test_angle < 2 * PI; test_angle += 0.1 * PI) {//搜索间隔0.2*PI（暂定）
				int i = (test_length / 290) - 1;
				int j = test_angle / (0.1 * PI);
				double test_x = my_pos.x + test_length * cos(test_angle);//计算目标位置的坐标
				double test_y = my_pos.y + test_length * sin(test_angle);
				assert(score[i][j] == 0);
				if ((test_x < 1000) || (test_y < 1000) || (test_x > 49000) || (test_y > 49000)) continue;
				Coord test_target{ (int)test_x,(int)test_y };//构造目标位置
				Cell target_cell = test_target.to_cell();
				if (util.raytrace_wall_r(my_pos, test_target,600) == true) continue;//如果扔出路线上有墙则不考虑这个位置，score=0
				if (_botid == -1) {
					int enemy_flag = 0;//两个flag辅助找到各种最短时间
					int partner_flag = 0;
					for (int id = 0; id < 8; id++) {
						if (util.robots[id] == NULL) continue;//选中的机器人不可见则跳过
						if (util.bot_update[id] < util.frame - 10) continue;//更新时间太早的不可信
						if (util.robots[id]->guid == util.self->guid) continue;//选中的机器人是自己则跳过
						if (util.robots[id]->isResetting) continue;//选中的机器人在复活则跳过
						if ((util.robots[id]->teamID == util.self->teamID) && (util.robots[id]->life <= 3000)) continue;//不传给生命太低的队友，这个阈值可以改动，也可以不设置这个机制
						if ((util.robots[id]->teamID != util.self->teamID) && util.robots[id]->canMove) {
							Coord enemy_pos = util.get_pos(*util.robots[id]);
							double straight_time = (enemy_pos - test_target).get_leng() / util.robots[id]->speed;
							double dist_time = util.get_dist(target_cell, id) * 1000 / util.robots[id]->speed;
							if ((enemy_flag == 0) || (dist_time <= min_enemy_dist_time)) min_enemy_dist_time = dist_time;
							if ((enemy_flag == 0) || (straight_time <= min_enemy_straight_time)) min_enemy_straight_time = straight_time;
							enemy_flag = 1;
						}
						if ((util.robots[id] != NULL)&&(util.bot_update[id]>=util.frame-10)) {
							if ((util.robots[id]->teamID == util.self->teamID) && (util.robots[id]->canMove)) {
								Coord partner_pos = util.get_pos(*util.robots[id]);
								double dist_time = util.get_dist(target_cell, id) * 1000 / util.robots[id]->speed;
								if ((partner_flag == 0) || (dist_time <= min_partner_dist_time)) {
									min_partner_dist_time = dist_time;
									test_catch_id[i][j] = util.robots[id]->playerID + 10;//为了确保0不是任何有意义的id，所以加10
								}
								partner_flag = 1;
							}
						}
					}
					if (partner_flag == 0) continue;
					if ((enemy_flag == 0) && visible_enemy_num == 4) score[i][j] += zero_enemy_weight;
					if ((enemy_flag != 0) && (min_enemy_straight_time < max_straight_range * test_length/3000)) continue;
					double partner_life = 0;
					for (int id = 1; id < 8; id++) {
						if ((util.robots[id] != NULL)&&(util.bot_update[id]>=util.frame-10)) {
							if ((util.robots[id]->teamID == util.self->teamID) && (util.robots[id]->playerID == test_catch_id[i][j] - 10)) {
								partner_life = util.robots[id]->life;//获取最适合接cpu的机器人的生命值，可能不需要这个
								break;
							}
						}
					}
					score[i][j] = score[i][j] + dist_diff_weight * (min_enemy_dist_time - min_partner_dist_time) + partner_life_weight * partner_life;
					//分数总共由三部分组成，生命值那部分可能要删掉
				}
				else {
					int enemy_flag = 0;
					for (int id = 0; id < 8; id++) {
						if (util.robots[id] == NULL) continue;//选中的机器人不可见则跳过
						if (util.bot_update[id] < util.frame - 10) continue;//更新时间太早的不可信 
						if (util.robots[id]->guid == util.self->guid) continue;//选中的机器人是自己则跳过
						if (util.robots[id]->isResetting) continue;//选中的机器人在复活则跳过
						if ((util.robots[id]->teamID != util.self->teamID) && util.robots[id]->canMove) {
							Coord enemy_pos = util.get_pos(*util.robots[id]);
							double straight_time = (enemy_pos - test_target).get_leng() / util.robots[id]->speed;
							double dist_time = util.get_dist(target_cell, id) * 1000 / util.robots[id]->speed;
							if ((enemy_flag == 0) || (dist_time <= min_enemy_dist_time)) min_enemy_dist_time = dist_time;
							if ((enemy_flag == 0) || (straight_time <= min_enemy_straight_time)) min_enemy_straight_time = straight_time;
							enemy_flag = 1;
						}
					}
					min_partner_dist_time = util.get_dist(target_cell, _botid) * 1000 / util.robots[_botid]->speed;
					if ((enemy_flag != 0) && (min_enemy_straight_time < max_straight_range * test_length / 3000)) continue;
					if ((enemy_flag == 0) && visible_enemy_num == 4) score[i][j] += zero_enemy_weight;
					score[i][j] = score[i][j] + dist_diff_weight * (min_enemy_dist_time - min_partner_dist_time);
				}
			}
		int max_i = 0;
		int max_j = 0;
		double max_score = 0;
		for (int i = 0; i < 50; i++)
			for (int j = 0; j < 20; j++) {
				if ((i == 0 && j == 0) || score[i][j] > max_score) {
					max_score = score[i][j];
					max_i = i;
					max_j = j;
				}
			}//找到最大分数以及对应的目标位置

		if (max_score <= throwable_min_score) return false;//最大分数达不到阈值，不扔
		if (_botid == -1) {
			catch_id = test_catch_id[max_i][max_j] - 10;//找到最适合接cpu的队友
		}
		else {
			if (_botid < 4) catch_id = _botid;
			else catch_id = _botid - 4;
		}
		delete[]score;
		delete[]test_catch_id;
		int throw_time = (max_i + 1) * 290 / 3;
		double throw_angle = max_j * 0.1 * PI;//确定throwcpu的参数
		int target_x = (int)(my_pos.x + (max_i + 1) * 290 * cos(max_j * 0.1 * PI));
		int target_y = (int)(my_pos.y + (max_i + 1) * 290 * sin(max_j * 0.1 * PI));
		Coord throwable_target{ target_x,target_y };
		std::string x = std::to_string(target_x);
		std::string y = std::to_string(target_y);
		std::string cpunum = std::to_string(util.self->cpuNum);
		std::string fly_time = std::to_string(throw_time);
		std::string message = x + ' ' + y + ' ' + cpunum + ' ' + fly_time;//依次发送两个目标位置的坐标，扔出的cpu数量，cpu飞行的时间
		util.send(catch_id, throw_command_type, message);
		api->ThrowCPU(throw_time, throw_angle, util.self->cpuNum);//这里是否一定扔出全部cpu？
		if (_botid != -1) Status::throw_st::throw_direction = throw_angle;
		else Status::throw_st::throw_direction = -1;
		return true;
	}
}
//版本号：050301
//修改时间：0503
//考虑了队友的目标，避免目标重复（效果可能不好）
bool Actions::try_pick(bool home) {

	logger.log(1, "picked called\n");
	if (!home) {
			//信息处理
			static int call_num = 0;
			const int pickmsg_type = 2;
			const int min_frame = util.frame - 10;
			int pickmsg_size = util.msgs[pickmsg_type].size();
			Message pick_msg[4] = { {0,-1,-1," "},{0,-1,-1," "}, {0,-1,-1," "}, {0,-1,-1," "} };
			std::string pickmsg[4] = { " "," "," "," " };
			int partner_x[4] = { -1,-1,-1,-1 };
			int partner_y[4] = { -1,-1,-1,-1 };
			int ptn_tgt_x[4] = { -1,-1,-1,-1 };
			int ptn_tgt_y[4] = { -1,-1,-1,-1 };
			double ptn_dst[4] = { -1,-1,-1,-1 };
			if (pickmsg_size > 0) {
				for (int index = 0; index < pickmsg_size; index++) {
					if ((util.msgs[pickmsg_type][index].sender == 0) && (util.msgs[pickmsg_type][index].frame > min_frame) && (util.msgs[pickmsg_type][index].frame >= pick_msg[0].frame)) {
						pick_msg[0] = util.msgs[pickmsg_type][index];
						continue;
					}
					if ((util.msgs[pickmsg_type][index].sender == 1) && (util.msgs[pickmsg_type][index].frame > min_frame) && (util.msgs[pickmsg_type][index].frame >= pick_msg[1].frame)) {
						pick_msg[1] = util.msgs[pickmsg_type][index];
						continue;
					}
					if ((util.msgs[pickmsg_type][index].sender == 2) && (util.msgs[pickmsg_type][index].frame > min_frame) && (util.msgs[pickmsg_type][index].frame >= pick_msg[2].frame)) {
						pick_msg[2] = util.msgs[pickmsg_type][index];
						continue;
					}
					if ((util.msgs[pickmsg_type][index].sender == 3) && (util.msgs[pickmsg_type][index].frame > min_frame) && (util.msgs[pickmsg_type][index].frame >= pick_msg[3].frame)) {
						pick_msg[3] = util.msgs[pickmsg_type][index];
						continue;
					}
				}
				for (int index = 0; index < 4; index++) {
					if (pick_msg[index].sender != -1) pickmsg[index] = pick_msg[index].msg;
				}
				for (int index = 0; index < 4; index++) {
					if (pickmsg[index] == " ") continue;
					char* p;
					const char* d = " ";
					p = strtok((char*)(pickmsg[index]).c_str(), d);
					partner_x[index] = atoi(p);
					p = strtok(NULL, d);
					partner_y[index] = atoi(p);
					p = strtok(NULL, d);
					ptn_tgt_x[index] = atoi(p);
					p = strtok(NULL, d);
					ptn_tgt_y[index] = atoi(p);
					p = strtok(NULL, d);
					ptn_dst[index] = atof(p);
				}
				call_num++;
				if (call_num % 10 == 0) util.msgs[pickmsg_type].clear();
			}

		//权重计算
		int prop_weight;
		int prop_num = util.props.size();//获得场上的道具数量
		if (prop_num == 0) return false;
		if ((int)util.self->prop != 0)//如果手上有道具，那么道具的权重就会变成0，不会捡大于1个道具
			prop_weight = 0;
		else
			prop_weight = 1;
		

		double weight[200] = { 0 };//用于设置优先级
		double dist[200];
		Coord position_coord[200];
		Cell position_cell[200];
		static const Cell forbiden[18] = { {42, 10}, {41, 10}, {40, 10}, {41, 9}, {41, 11}, {9, 48},
									{31, 1},  {32, 1},{33, 1},  {34, 1}, {35, 1},  {36, 1},
									{37, 1},{10, 48}, {11, 48}, {12, 48}, {13,48},{43,7} }; //钓鱼区域
		assert(prop_num < 200);
		int throw_derc_flag = 0;
		int home_cpu_num = 0;

		//遍历所有场上的道具
		for (int i = 0; i < prop_num; i++) {
			//避坑
			bool forbidden = true;
			position_coord[i] = util.get_pos(*util.props[i]);
			position_cell[i] = position_coord[i].to_cell();
			if (util.map_type == 0) {
				for (int n = 0; n < 18; n++) {
					if (position_cell[i] == forbiden[n])
						forbidden = false;
				}
			}
			if (forbidden != true)
				continue;
			assert(util.props[i]);

			//避开移动的cpu
			if (util.props[i]->isMoving == true) continue;

			//避开家里的cpu
			if ((util.get_pos(*util.props[i]).to_cell() == Status::throw_st::home)) {
				home_cpu_num += util.props[i]->size;
				continue;
			}

			//根据距离加第一个权重
			dist[i] = util.get_dist(position_cell[i]);
			if (util.get_pos(*util.props[i]).to_cell() == util.get_pos().to_cell()) {
				api->Pick(util.props[i]->type);
				continue;
			}
			if ((int)util.props[i]->type == 3) {
				weight[i] = (10 * (util.props[i]->size)) / dist[i];
			}
			else if (((int)util.props[i]->type == 1) || ((int)util.props[i]->type == 5))
				weight[i] = 3 * prop_weight / dist[i];
			else
				weight[i] = prop_weight / dist[i];

			//新地图上根据位置加第二个权重
			if (util.map_type == 1) {
				if ((position_cell[i].y >= 23 && position_cell[i].y <= 26) && ((position_cell[i].x >= 41 && position_cell[i].x <= 44) || (position_cell[i].x >= 5 && position_cell[i].x <= 8))) {
					if (((int)util.self->prop == 2 || (int)util.self->prop == 4||(util.self->timeUntilCommonSkillAvailable==0&&(int)util.self->softwareType==1))&&(int)util.props[i]->type==3) {
						weight[i] += 0.5;
					}
					else if((int)util.props[i]->type==3) {
						weight[i] -= 0.5;
					}
				}
			}
		}

		//记录家里的cpu数目
		Status::pick_st::home_cpu_num = home_cpu_num;


		//道具使用
		if ((int)util.self->prop == 1) api->UseProp();

		//寻找目标
		double max_weight = 0;
		int best_srl = -1;
		for (int index = 0; index < prop_num; index++) {
			if (weight[index] > max_weight) {
				int skip_flag = 0;
				for (int i = 0; i < 4; i++) {
						if ((abs(position_coord[index].x - ptn_tgt_x[i]) < 100) && (abs(position_coord[index].y - ptn_tgt_y[i]) < 100) && (ptn_tgt_x[i] != -1) && (ptn_tgt_y[i] != -1)) {
							if (1000 * dist[index] / util.self->speed - ptn_dst[i] > 0.1) skip_flag = 1;
						}
				}
				if (skip_flag == 1) continue;
				else {
					best_srl = index;
					max_weight = weight[index];
				}
			}
		}
		if ((best_srl == -1) || (max_weight <= 0)) return false;


		//广播目标
		Coord target = position_coord[best_srl];
		broadcast_pickinfo(target);

		//判断使用隐身
		if (util.map_type == 1) {
			if ((position_cell[best_srl].y >= 23 && position_cell[best_srl].y <= 26) && ((position_cell[best_srl].x >= 41 && position_cell[best_srl].x <= 44) || (position_cell[best_srl].x >= 5 && position_cell[best_srl].x <= 8))) {
				if (util.get_dist(position_cell[best_srl]) <= 8 && (util.self->timeUntilCommonSkillAvailable == 0 && (int)util.self->softwareType == 2)) {
					api->UseCommonSkill();
				}
			}
		}

		//前往目标
		util.navigate(target);
		return true;
	}
	else {//捡家里模式
		Cell self_pos = util.get_self_pos().to_cell();
		if (self_pos == Status::throw_st::home) {
			api->Pick((THUAI5::PropType)3);
			Status::pick_st::home_cpu_num--;
			if (Status::pick_st::home_cpu_num == 0) Status::pick_st::home_picked_flag = 1;
			return true;
		}
		else {
			Coord tgt = Status::throw_st::home.to_grid();
			util.navigate(tgt);
			return true;
		}
	}
}
//版本号：052001_mobile_beta1
//修改时间：05/20 8:30
bool Actions::try_hit_mobile() {
	static const int SHIELD_HEALTH = 3000; //血量严格小于此值时，将使用护盾
	static const int UPD_TIME_LIMIT = 3;
	static const double delta_rad = PI * 13 / 180;
	static const double R1_MULT = 1.2;
	static const double SHORT_HIT_DIST = 2000;

	static Timer atk_timer;
	static int lst_enemy = -1;
	static Coord lst_pos;

	//预检查
	logger.log(0, "try_hit working, bullet num:%d", util.self->signalJammerNum);
	assert(util.self->signalJammerType == THUAI5::SignalJammerType::CommonJammer);
	if (!util.self->signalJammerNum) return false;

	//目标搜索
	int tgt_id = -1;
	double min_dist = 1e8;
	const Coord& my_pos = util.get_self_pos();
	for (int i = 0; i < BOT_COUNT; i++) {
		const THUAI5::Robot& bot = *util.robots[i];
		const Coord& enemy_pos = util.get_pos(bot);
		double leng = (enemy_pos - my_pos).get_leng();
		if (bot.teamID == util.teamId || util.bot_update[i] + UPD_TIME_LIMIT < util.frame || bot.isResetting) continue;
		if (util.raytrace_wall_r(my_pos, enemy_pos, 400)) continue;//直接trace
		if (util.self->timeUntilCommonSkillAvailable >= 24500 && leng >= SHORT_HIT_DIST) continue;
		if (leng >= util.self->attackRange + 1500 || leng >= min_dist) continue;//暂时写死半径

		tgt_id = i;
		min_dist = leng;
	}
	if (tgt_id == -1) return false;
	logger.log(0, "atk tgt:%d dist:%.2f", tgt_id, min_dist);
	//buff检查
	static bool enemy_buff[BUFF_COUNT];
	const THUAI5::Robot& enemy = *util.robots[tgt_id];
	for (int i = 0; i < BUFF_COUNT; i++) enemy_buff[i] = false;
	for (const auto& _buff : enemy.buff) enemy_buff[(int)_buff] = true;

	//准备攻击
	const Coord& enemy_pos = util.get_pos(tgt_id);
	double dist = (enemy_pos - my_pos).get_leng();
	double tgt_rad = util.get_self_pos().angle_to(enemy_pos);

	if (enemy_buff[3] && !util.my_buff[4] && util.self->prop != THUAI5::PropType::ShieldBreaker) return false; //有盾且无攻击道具
	if ((int)util.self->prop) { //使用道具
		if (enemy_buff[3] && !util.my_buff[4] && util.self->prop == THUAI5::PropType::ShieldBreaker) api->UseProp();
		else if ((int)util.self->prop == 4 && util.self->life < SHIELD_HEALTH && !util.my_buff[3]) api->UseProp(); //濒死
		else if (tgt_id != lst_enemy && (int)util.self->prop != 4 && (int)util.self->prop != 5) api->UseProp();
	}
	//多人计算
	int near_cnt = 0;
	for (int i = 0; i < BOT_COUNT; i++) {
		const THUAI5::Robot& bot = *util.robots[i];
		const Coord& _bot_pos = util.get_pos(bot);
		if (bot.teamID == util.teamId || util.bot_update[i] + UPD_TIME_LIMIT < util.frame || bot.isResetting || i == tgt_id) continue;
		if ((enemy_pos - _bot_pos).get_leng() <= 4000) near_cnt++;
	}
	logger.log(0, "atk tgt:%d dist:%.2f near:%d", tgt_id, min_dist, near_cnt);

	double tan_vmvb = enemy.speed / BULLET_SPD[(int)enemy.signalJammerType];
	double r1 = 500 + 900 * sqrt(tan_vmvb * tan_vmvb + 1) / tan_vmvb * R1_MULT;
	if (dist <= r1 || dist <= SHORT_HIT_DIST && atk_timer.look() >= 150) api->Attack(tgt_rad);
	else if (near_cnt && atk_timer.look() >= 1500) api->Attack(tgt_rad);
	else if (util.self->signalJammerNum >= 2 && dist <= util.self->attackRange + 500 && atk_timer.look() >= 500) {
		api->Attack(tgt_rad + delta_rad);
		api->Attack(tgt_rad - delta_rad);
	}
	else return false;

	lst_enemy = tgt_id;
	lst_pos = enemy_pos;
	atk_timer.reset();
	return true;
}

//版本号：051702_strong
//修改时间：05/17 18:30
bool Actions::try_hit_strong() {
	static const int SHIELD_HEALTH = 3000; //血量严格小于此值时，将使用护盾
	static const double delta_rad = PI * 20 / 180;
	static const double R1_MULT = 1.2;

	static Timer atk_timer;
	static int lst_enemy = -1;
	static Coord lst_pos;

	//预检查
	const Coord& my_pos = util.get_self_pos();
	//assert(util.self->signalJammerType == THUAI5::SignalJammerType::CommonJammer || util.self->signalJammerType == THUAI5::SignalJammerType::StrongJammer);
	if (!util.self->signalJammerNum || atk_timer.look() < 200) return false;

	if (util.self->timeUntilCommonSkillAvailable == 0 && util.self->signalJammerNum == 2) { //搜索强力弹发射角度
		static const int SCAN_CNT = 36;
		static const int SCAN_STEP = 360 / SCAN_CNT;
		static int enemy_hp[BOT_COUNT >> 1];
		static int can_dmg[SCAN_CNT + 1][BOT_COUNT >> 1];

		bool has_spear = util.my_buff[4] || util.self->prop == THUAI5::PropType::ShieldBreaker;
		//计算各敌人需要几发强力弹
		for (int i = 0; i < (BOT_COUNT >> 1); i++) enemy_hp[i] = 100000;
		for (int i = 0; i < BOT_COUNT; i++) {
			const THUAI5::Robot& bot = *util.robots[i];
			if (util.bot_update[i] + 5 < util.frame || bot.teamID == util.teamId || bot.isResetting) continue; //认为打不到了

			bool can_hit = true;
			if (!has_spear) for (const auto& _buff : bot.buff) if (_buff == THUAI5::BuffType::ShieldBuff) can_hit = false;
			if (!can_hit) continue;
			enemy_hp[bot.playerID] = bot.life;
		}
		//上手搜角度
		for (int i = 0; i < SCAN_CNT; i++) for (int j = 0; j < (BOT_COUNT >> 1); j++) can_dmg[i][j] = 0;
		for (int i = 0, deg = 0; i < SCAN_CNT; i++, deg += SCAN_STEP) {
			double rad = PI * deg / 180;
			int hit_dis = util.check_nearest_wall(my_pos, rad, 400);
			if (hit_dis > util.self->attackRange) hit_dis = util.self->attackRange;

			double hit_mult = 1;
			if (hit_dis >= 500) hit_mult = pow(0.5, ((double)hit_dis - 500) / 2000);
			const Coord& hit_point = my_pos + Coord({ (int)round(hit_dis * cos(rad)),(int)round(hit_dis * sin(rad)) });

			for (int j = 0; j < BOT_COUNT; j++) {
				const THUAI5::Robot& bot = *util.robots[j];
				if (bot.teamID == util.teamId || util.bot_update[j] + 5 < util.frame || bot.isResetting) continue;
				if ((util.get_pos(bot) - hit_point).get_leng() < BULLET_EXPRNG[4]) can_dmg[i][bot.playerID] = BULLET_DMG[4] * hit_mult;
			}
		}
		for (int j = 0; j < BOT_COUNT; j++) { //特殊的自爆方向
			const THUAI5::Robot& bot = *util.robots[j];
			if (bot.teamID == util.teamId || util.bot_update[j] + 5 < util.frame || bot.isResetting) continue;
			if ((util.get_pos(bot) - my_pos).get_leng() < BULLET_EXPRNG[4] - 500) can_dmg[SCAN_CNT][bot.playerID] = BULLET_DMG[4];
		}
		//继续循环搜角度
		int max_kill = 0, max_dmg = 0;
		Coord best{ 0,0 };
		for (int a1 = 0; a1 < SCAN_CNT; a1++) { //暂时关闭自爆模式
			for (int a2 = 0; a2 <= a1; a2++) {
				int kill = 0, dmg = 0;
				for (int i = 0; i < (BOT_COUNT >> 1); i++) {
					if (can_dmg[a1][i] + can_dmg[a2][i] >= enemy_hp[i]) {
						kill++;
						dmg += enemy_hp[i];
					}
					else if (enemy_hp[i] != 100000) dmg += can_dmg[a1][i] + can_dmg[a2][i];
				}
				if (kill > max_kill || (kill == max_kill && dmg > max_dmg)) {
					max_kill = kill, max_dmg = dmg;
					best = { a1,a2 };
				}
			}
		}
		// printf("ck: best %d,%d kill:%d dmg:%d\n",best.x * SCAN_STEP,best.y * SCAN_STEP,max_kill,max_dmg);
		//最终决定
		if (max_kill >= 2) {
			if (util.self->prop == THUAI5::PropType::ShieldBreaker) api->UseProp();
			api->UseCommonSkill();

			bool self_trigger = (best.x == SCAN_CNT) || (best.y == SCAN_CNT);
			api->Attack(PI * (best.x * SCAN_STEP) / 180);
			api->Attack(PI * (best.y * SCAN_STEP) / 180);
			if (self_trigger) api->MovePlayer(50, PI * (SCAN_CNT * SCAN_STEP) / 180);

			logger.log(1, "Attack -> %d %d expected kill:%d dmg:%d", best.x * SCAN_STEP, best.y * SCAN_STEP, max_kill, max_dmg);
			// printf("Attack -> %d %d expected kill %d dmg %d\n",best.x * SCAN_STEP,best.y * SCAN_STEP,max_kill,max_dmg);
			return true;
		}
	}
	//强力模式未触发则进入普通模式
	//目标搜索
	int tgt_id = -1;
	double min_dist = 1e8;
	for (int i = 0; i < BOT_COUNT; i++) {
		const THUAI5::Robot& bot = *util.robots[i];
		const Coord& enemy_pos = util.get_pos(bot);
		if (bot.teamID == util.teamId || util.bot_update[i] + 5 < util.frame || bot.isResetting) continue;
		if ((enemy_pos - my_pos).get_leng() >= util.self->attackRange || (enemy_pos - my_pos).get_leng() >= min_dist) continue;//暂时写死半径
		if (util.raytrace_wall_r(my_pos, enemy_pos, 400)) continue;//直接trace

		tgt_id = i;
		min_dist = (enemy_pos - my_pos).get_leng();
	}
	if (tgt_id == -1) return false;
	//buff检查
	static bool enemy_buff[BUFF_COUNT];
	const THUAI5::Robot& enemy = *util.robots[tgt_id];
	for (int i = 0; i < BUFF_COUNT; i++) enemy_buff[i] = false;
	for (const auto& _buff : enemy.buff) enemy_buff[(int)_buff] = true;

	//准备攻击
	const Coord& enemy_pos = util.get_pos(tgt_id);
	double dist = (enemy_pos - my_pos).get_leng();
	double tgt_rad = util.get_self_pos().angle_to(enemy_pos);

	if (enemy_buff[3] && !util.my_buff[4] && util.self->prop != THUAI5::PropType::ShieldBreaker) return false; //有盾且无攻击道具
	if ((int)util.self->prop) { //使用道具
		if (enemy_buff[3] && !util.my_buff[4] && util.self->prop == THUAI5::PropType::ShieldBreaker) api->UseProp();
		else if ((int)util.self->prop == 4 && util.self->life < SHIELD_HEALTH && !util.my_buff[3]) api->UseProp(); //濒死
		else if (tgt_id != lst_enemy && (int)util.self->prop != 4 && (int)util.self->prop != 5) api->UseProp();
	}

	double tan_vmvb = enemy.speed / BULLET_SPD[(int)enemy.signalJammerType];
	double r1 = 500 + 900 * sqrt(tan_vmvb * tan_vmvb + 1) / tan_vmvb * R1_MULT;
	if (dist <= r1) api->Attack(tgt_rad);
	else if (util.self->signalJammerNum >= 2) {
		api->Attack(tgt_rad + delta_rad);
		api->Attack(tgt_rad - delta_rad);
	}
	else return false;

	lst_enemy = tgt_id;
	lst_pos = enemy_pos;
	atk_timer.reset();
	return true;
}

//temp
bool Actions::try_hit_evade() {
	static const double delta_rad = PI * 15 / 180;

	static Timer atk_timer;
	static int lst_enemy = -1;
	static Coord lst_pos;
	if (atk_timer.look() < 500) return false;

	const THUAI5::Robot* tgt = NULL;
	const Coord&& my_pos = util.get_self_pos();
	for (const auto& robot : api->GetRobots()) {
		const Coord&& enemy_pos = util.get_pos(*robot);
		if (robot->guid == util.self->guid || robot->teamID == util.self->teamID) continue;
		if ((enemy_pos - my_pos).get_leng() >= util.self->attackRange + 3000 / 4) continue;//暂时写死半径
		if (util.raytrace_wall_r(my_pos, enemy_pos, 400)) continue;//直接trace

		tgt = &*robot;
		break;
	}

	if (util.self->signalJammerNum && tgt) {
		int enemy_id = util.get_botid(*tgt);
		const Coord&& enemy_pos = util.get_pos(*tgt);

		if (enemy_id != lst_enemy && (int)util.self->prop) api->UseProp();
		else if (enemy_id != lst_enemy && util.self->timeUntilCommonSkillAvailable == 0) api->UseCommonSkill();

		double tgt_rad = util.get_self_pos().angle_to(enemy_pos);
		if (enemy_id != lst_enemy || atk_timer.look() > 3000) {//第一次发射
			api->Attack(tgt_rad + delta_rad);
			api->Attack(tgt_rad - delta_rad);
		}
		else {//尝试预测路径的发射
			api->Attack(util.get_self_pos().angle_to(enemy_pos * 2 - lst_pos));
		}

		if (!Status::evading) api->MovePlayer(200 * 1000 / util.self->speed, tgt_rad);//跟Timer的配合有点问题

		lst_enemy = enemy_id;
		lst_pos = util.get_pos(enemy_id);
		atk_timer.reset();
		return true;
	}
	return false;
}


//版本号：051301
//修改时间：05/15 15:15
//贴脸打人
bool Actions::try_surprise(int sur_mod) {//模式1：boring的时候主动找一个距离最近的机器人去surprise 模式2：如果检测范围内存在人，则surprise
	auto self = util.self;
	auto robot = util.robots;
	double attack_range = 9000;                                           //敌方机器人的攻击范围
	double jammer_range = 7000;                                           //敌方机器人子弹爆炸半径
	double walkdis_stt = 50000;                                         //我到目标的距离
	double distance_min = 50000;                                        //用于模式1中判断最近的机器人的量
	Coord enemy_coord[8];
	Cell enemy_cell[8];
	Cell delta[4] = { {1,0},{-1,0},{0,1},{0,-1} };
	int i = 0;
	const THUAI5::Robot* tgt = NULL;
	const Coord&& my_pos = util.get_self_pos();
	double value[8], value_max = 0;
	//定义一堆变量

	for (i = 0; i < 8; i++) {
		enemy_coord[i] = { 0,0 };
		enemy_cell[i] = enemy_coord[i].to_cell();
		value[i] = 0;
	}
	//变量的初始化


	if (sur_mod == 1) {
		for (i = 0; i < 8; i++) {
			if (robot[i] == NULL)continue;
			if (util.frame - util.bot_update[robot[i]->playerID] > 3)continue;  //如果该机器人的信息已经是三帧以前的了，则视为无效
			if (robot[i]->isResetting)continue;                                 //死了的机器人不要管它
			if (robot[i]->guid == self->guid || robot[i]->teamID == self->teamID) continue;//友方不要管它
			enemy_coord[i] = util.get_pos(*robot[i]);                           //敌方机器人坐标
			enemy_cell[i] = enemy_coord[i].to_cell();                           //敌方机器人所在格子
			if (((int)robot[i]->softwareType == 3) &&
				((robot[i]->timeUntilCommonSkillAvailable <= 5000) || (robot[i]->timeUntilCommonSkillAvailable >= 69000)))
				jammer_range = 7000;//敌方带的强力弹并且软件好了
			else if ((int)robot[i]->hardwareType == 1) jammer_range = 2500;
			else if ((int)robot[i]->hardwareType == 2) jammer_range = 1500;
			else if ((int)robot[i]->hardwareType == 3) jammer_range = 4000;
			attack_range = robot[i]->attackRange + jammer_range;                //敌方攻击范围=攻击距离+子弹爆炸半径
			for (int index = 0; index < 4; index++) {
				if (util.get_dist(enemy_cell[i] + delta[index]) > 0) {
					walkdis_stt = 1000 * (util.get_dist(enemy_cell[i] + delta[index]) + 1);
					break;
				}
			}
			if (walkdis_stt < 0)continue;
			else value[i] += 50000 / walkdis_stt;//距离：最大为50000,最小为1
			value[i] += 9500 / robot[i]->life;//血量：最大为9500，最小为1
			value[i] += 3200 * robot[i]->cpuNum;//cpu数量：一个+3200
			if (value[i] >= value_max) {
				value_max = value[i];
				tgt = &*robot[i];
			}
		}
	}//模式1：选择目标-根据估价函数

	else if (sur_mod == 2) {
		for (i = 0; i < 8; i++) {
			if (robot[i] == NULL)continue;
			if (util.frame - util.bot_update[robot[i]->playerID] > 3)continue;
			if (robot[i]->isResetting)continue;
			if (robot[i]->guid == self->guid || robot[i]->teamID == self->teamID) continue;
			enemy_coord[i] = util.get_pos(*robot[i]);
			enemy_cell[i] = enemy_coord[i].to_cell();
			if (((int)robot[i]->softwareType == 3) &&
				((robot[i]->timeUntilCommonSkillAvailable <= 5000) || (robot[i]->timeUntilCommonSkillAvailable >= 69000)))
				jammer_range = 7000;//敌方带的强力弹并且软件好了
			else if ((int)robot[i]->hardwareType == 1) jammer_range = 2500;
			else if ((int)robot[i]->hardwareType == 2) jammer_range = 1500;
			else if ((int)robot[i]->hardwareType == 3) jammer_range = 4000;
			attack_range = robot[i]->attackRange + jammer_range;        //敌方攻击范围=攻击距离+子弹爆炸半径
			for (int index = 0; index < 4; index++) {
				if (util.get_dist(enemy_cell[i] + delta[index]) > 0) {
					walkdis_stt = 1000 * (util.get_dist(enemy_cell[i] + delta[index]) + 1);
					break;
				}
			}
			if (walkdis_stt < 0)continue;
			//以上和模式1一模一样
			if (walkdis_stt > 9000)continue;                                 //走过去太久则不考虑
			else tgt = &*robot[i];                                           //满足“在范围内”这个条件，则设为目标
			break;
		}
	}//选定受害人
	//模式2：选择目标-范围内的

	if ((util.self->timeUntilCommonSkillAvailable == 0 || util.self->timeUntilCommonSkillAvailable > 24000) && (tgt != NULL)) {
		const Coord&& enemy_pos = util.get_pos(*tgt);                      //确定敌方坐标
		double tgt_rad = util.get_self_pos().angle_to(enemy_pos);          //确定攻击角度

		if ((self->timeUntilCommonSkillAvailable < 24500) && (self->timeUntilCommonSkillAvailable != 0)) {
			if (sur_mod == 1)Status::surprising = 1;
			if (sur_mod == 2)Status::surprising = 2;
		}
		//改变status

		if ((int)self->prop == 1)api->UseProp();                           //有加速就用加速
		if ((self->timeUntilCommonSkillAvailable == 0) && (walkdis_stt <= 1.1 * attack_range)) api->UseSoftware();//快进敌方攻击范围了就开隐身

		if ((enemy_pos - my_pos).get_leng() >= 1400) {
			util.navigate(enemy_pos, 250);                                     //向敌方前进
		}

		if ((enemy_pos - my_pos).get_leng() <= 1500 || ((self->timeUntilCommonSkillAvailable < 24500) && (self->timeUntilCommonSkillAvailable != 0))) {
			if (((int)self->prop == 5) && ((int)tgt->prop == 4))api->UseProp();//我有破盾，敌方有护盾则用破盾
			api->Attack(tgt_rad);
			api->Attack(tgt_rad + PI / 6);
			api->Attack(tgt_rad - PI / 6);
			Status::surprising = 0;
		}
		//攻击

		return true;
	}

	Status::surprising = 0;
	return false;
}

//版本号：051701
//修改时间：05/17 16:15
//修了逻辑错误 搜躲避方向时考虑了子弹种类
bool Actions::evade_bullet() {
	const THUAI5::SignalJammer* tgtj[32];                //32个可能的子弹
	const THUAI5::Robot* tgtr[8];                        //8个可能的敌人
	auto self = util.self;                               //我
	auto robot = util.robots;                            //敌人
	int i = 0, j = 0, flag = 0, flag_min = 40000;        //flag:我往某个方向躲了以后会损失的血量
	double mtj_angle[32], distance[32], inc_angle[32];   //mtj：我指向子弹的向量角度 distance：向量模长 inc:我与子弹的连线 与 子弹方向 的锐角夹角
	int v[32], r[32], l[32], a[32];                       //子弹的速度、半径、飞行距离参数
	auto jammer = util.jammers;
	int jammer_num = jammer.size(), dangerj = 0, dangerr = 0;//jn:场上子弹的数量 dj:有威胁的子弹数量 dr：有威胁的敌人数量
	const Coord& my_pos = util.get_self_pos();           //我的位置
	Coord enemy_pos[8];                                  //敌人的位置
	double try_angle = 0;                                //尝试躲避的方向
	double wd_angle = 0, wdt_angle = 0;                  //wd:最终确定的躲避方向 wdt：尝试的默认躲避方向
	Coord fut_pos_s[360], fut_pos_j[32], jammer_pos[32]; //尝试躲避0.1秒后到达的坐标、子弹走0.1秒后到达的坐标、子弹目前的坐标
	double distance_f[360], distance_max = 0;            //躲避后与之后子弹的距离
	double wdd_try1 = 0, wdd_try2 = 0;                   //沿垂直方向躲避后与原子弹的距离
	int x = 0, y = 0;
	int self_v = 0;
	//定义一堆变量

	for (i = 0; i < 8; i++) {
		enemy_pos[i] = { 1500,1500 };
		tgtr[i] = NULL;
	}
	for (i = 0; i < 32; i++) {
		tgtj[i] = NULL;
		mtj_angle[i] = 0;
		distance[i] = 50000;
		v[i] = 0;
		r[i] = 0;
		l[i] = 0;
		a[i] = 0;
		fut_pos_j[i] = { 0,0 };
		jammer_pos[i] = { 0,0 };
	}
	for (i = 0; i < 360; i++) {
		distance_f[i] = 0;
		fut_pos_s[i] = { 0,0 };
	}
	self_v = self->speed;
	//以上变量的初始化

	for (i = 0; i < jammer_num; i++) {
		if (jammer[i] == NULL) continue;                           //判空
		if (jammer[i]->parentTeamID == self->teamID) continue;     //自家的不管

		jammer_pos[i] = { jammer[i]->x,jammer[i]->y };
		distance[i] = abs((jammer_pos[i] - my_pos).get_leng());
		mtj_angle[i] = util.get_self_pos().angle_to(jammer_pos[i]);
		inc_angle[i] = mtj_angle[i] - jammer[i]->facingDirection;
		if ((int)jammer[i]->type == 1) v[i] = 3000, l[i] = 900, r[i] = 2000, a[i] = 2000;
		else if ((int)jammer[i]->type == 2) v[i] = 2500, r[i] = 2500, l[i] = 4500, a[i] = 2500;
		else if ((int)jammer[i]->type == 3) v[i] = 5000, r[i] = 1500, l[i] = 9000, a[i] = 1500;
		else if ((int)jammer[i]->type == 4) v[i] = 2000, r[i] = 7000, l[i] = 7000, a[i] = 7000;
		//给子弹赋各种参数

		if ((jammer[i]->x + (int)(v[i] * 0.2 * cos(jammer[i]->facingDirection))) >= 49000) {
			x = 49000;
		}
		else if ((jammer[i]->x + (int)(v[i] * 0.2 * cos(jammer[i]->facingDirection))) <= 1000) {
			x = 1000;
		}
		else x = jammer[i]->x + (int)(v[i] * 0.2 * cos(jammer[i]->facingDirection));
		if ((jammer[i]->y + (int)(v[i] * 0.2 * sin(jammer[i]->facingDirection))) >= 49000) {
			y = 49000;
		}
		else if ((jammer[i]->y + (int)(v[i] * 0.2 * sin(jammer[i]->facingDirection))) <= 1000) {
			y = 1000;
		}
		else y = jammer[i]->y + (int)(v[i] * 0.2 * sin(jammer[i]->facingDirection));
		fut_pos_j[i] = { x,y };
		//存下所有子弹0.2秒后的位置，已经判断了是否会飞出地图

		if ((int)jammer[i]->type == 1) {
			if (distance[i] * abs(cos(mtj_angle[i])) > 0.1 * self_v + 2900) continue;
			if (distance[i] * abs(sin(mtj_angle[i])) > 0.1 * self_v + 1000) continue;
		}
		else {
			if (distance[i] * abs(cos(inc_angle[i])) > r[i] + l[i] + 0.1 * self_v) continue;
			if (distance[i] * abs(sin(inc_angle[i])) > 500 + r[i] + 0.1 * self_v) continue;
		}
		//确定一个子弹是否有威胁

		tgtj[i] = &*jammer[i];
		dangerj += 1;
	}
	//把有威胁的子弹存在tgtj里用于确定默认躲避方向，暂时没有威胁的子弹也存下它们的预计位置

	/*for (i = 0; i < 8; i++) {
		if (robot[i] == NULL)continue;
		if (util.frame - util.bot_update[robot[i]->playerID] > 3)continue;
		if (robot[i]->isResetting)continue;
		if (robot[i]->guid == self->guid || robot[i]->teamID == self->teamID) continue;
		enemy_pos[i] = util.get_pos(*robot[i]);
		if (abs((enemy_pos[i] - my_pos).get_leng()) <= 1200) {
			tgtr[i] = &*robot[i];
			dangerr += 1;
		}
	}*/
	//把有威胁的敌人存在tgtr里

	if (dangerj > 0) {
		for (i = 0; i < jammer_num; i++) {
			if (tgtj[i] == NULL)continue;
			else {
				wdt_angle = my_pos.angle_to(jammer_pos[i]) + PI;
			}
			
		}
		//确定默认躲避方向：第一颗有威胁的子弹的垂直离开方向

		for (j = 0; j < 360; j++) {
			try_angle = j * PI / 180 + wdt_angle;

			if ((util.self->x + (int)(0.2 * self_v * cos(try_angle))) >= 48500) continue;
			else if ((util.self->x + (int)(0.2 * self_v * cos(try_angle))) <= 1500) continue;
			else x = util.self->x + (int)(0.2 * self_v * cos(try_angle));
			if ((util.self->y + (int)(0.2 * self_v * sin(try_angle))) >= 48500) continue;
			else if ((util.self->y + (int)(0.2 * self_v * sin(try_angle))) <= 1500) continue;
			else y = util.self->y + (int)(0.2 * self_v * sin(try_angle));
			fut_pos_s[j] = { x,y };
			//假设往某个方向走了0.2秒，已经判断了会不会跑出地图

			if (util.raytrace_wall_r(util.get_self_pos(), fut_pos_s[j], 600, true) == true) continue;//看看会不会撞墙

			for (i = 0; i < jammer_num; i++) {
				if (jammer[i] == NULL)continue;
				if (abs((fut_pos_s[j] - fut_pos_j[i]).get_leng()) <= r[i])
					flag += a[i];
			}
			//看看这时会因为撞子弹损失多少血

			if (flag == 0) {
				wd_angle = try_angle;
				break;
			}                                       //如果能找到一个不会撞上子弹的方向就退出循环
			else if (flag < flag_min) {
				flag_min = flag;
				wd_angle = try_angle;
			}                                                              //如果一定要撞上子弹，则撞最少的子弹
		}
		//确定躲避方向：不会撞墙、躲避后撞上的子弹最少

		if (((int)(self->prop) == 1) || ((int)(self->prop) == 3) || ((int)(self->prop) == 4)) api->UseProp();

		api->Pick(THUAI5::PropType(3));
		api->MovePlayer(250000 / self_v, wd_angle);
		Status::evading = 1;
		return true;
	}
	//有子弹：返回true并进行躲避

	Status::evading = 0;
	return false;
}


//版本号：050301
//修改时间：0503
//广播自身位置和目标信息
void Actions::broadcast_pickinfo(Coord& tgt) {
	const int pickmsg_type = 2;
	int self_pos_x = util.self->x;
	int self_pos_y = util.self->y;
	int tgt_x = tgt.x;
	int tgt_y = tgt.y;
	double dist = util.get_dist(tgt.to_cell());
	double dist_time = 1000 * dist / (util.self->speed);
	std::string sp_x = std::to_string(self_pos_x);
	std::string sp_y = std::to_string(self_pos_y);
	std::string tp_x = std::to_string(tgt_x);
	std::string tp_y = std::to_string(tgt_y);
	std::string dst = std::to_string(dist_time);
	std::string msg = sp_x + ' ' + sp_y + ' ' + tp_x + ' ' + tp_y + ' ' + dst;
	logger.log(1, "broadcast pick info\n");
	util.broadcast(pickmsg_type, msg);
}
//版本号：051301
//修改时间：0513 21：00
//蹲草丛函数（测试）
bool Actions::try_stand() {
	Coord self_pos = util.get_self_pos();
	if (Status::stand_st::standing == 0) {
		int ran = rand();
		int tgt_num = ran % 6;
		if (util.is_walkable(Status::stand_st::stand_pos[tgt_num])) {
			Status::stand_st::stand_target = Status::stand_st::stand_pos[tgt_num];
			Status::stand_st::standing = 1;
			util.navigate(Status::stand_st::stand_target.to_grid());
		}
		else {
			return false;
		}
		if (self_pos.to_cell() == Status::stand_st::stand_target) {
			Status::stand_st::standing = 0;
			Status::stand_st::stand_target = { -1,-1 };
		}
	}
	else if (Status::stand_st::stand_target.x > 0 && Status::stand_st::stand_target.y > 0) {
		util.navigate(Status::stand_st::stand_target.to_grid());
		if (self_pos.to_cell() == Status::stand_st::stand_target) {
			Status::stand_st::standing = 0;
			Status::stand_st::stand_target = { -1,-1 };
		}
	}
	else { 
		Status::stand_st::standing = 0;
		Status::stand_st::stand_target = { -1,-1 };
		return false;
	}
	return true;

}

//版本号：051301
//修改时间：0513 19：00
//主函数内调用，更新Status内的home和stand_pos
bool Status::action_info_update() {
	int map_type = util.map_type;
	int team_id = util.self->teamID;
	int player_id = util.self->playerID;
	if (map_type == 0) {
		if (team_id == 0) {
			throw_st::home = util.born_point[3];
			pick_st::home_id = 3;
			switch (player_id)
			{
			case 0:
				stand_st::stand_pos[0] = { 13,10 };
				stand_st::stand_pos[1] = { 14,12 };
				stand_st::stand_pos[2] = { 14,15 };
				stand_st::stand_pos[3] = { 14,18 };
				stand_st::stand_pos[4] = { 15,16 };
				stand_st::stand_pos[5] = { 15,19 };
				break;
			case 1:
				stand_st::stand_pos[0] = { 22,12 };
				stand_st::stand_pos[1] = { 25,12 };
				stand_st::stand_pos[2] = { 29,12 };
				stand_st::stand_pos[3] = { 22,15 };
				stand_st::stand_pos[4] = { 26,10 };
				stand_st::stand_pos[5] = { 29,10 };
				break;
			case 2:
				stand_st::stand_pos[0] = { 17,26 };
				stand_st::stand_pos[1] = { 16,29 };
				stand_st::stand_pos[2] = { 21,28 };
				stand_st::stand_pos[3] = { 26,28 };
				stand_st::stand_pos[4] = { 14,30 };
				stand_st::stand_pos[5] = { 28,24 };
				break;
			case 3:
				stand_st::stand_pos[0] = { 40,45 };
				stand_st::stand_pos[1] = { 30,45 };
				stand_st::stand_pos[2] = { 40,43 };
				stand_st::stand_pos[3] = { 30,43 };
				stand_st::stand_pos[4] = { 40,41 };
				stand_st::stand_pos[5] = { 30,41 };
				break;
			default:
				return false;
			}
		}
		else if (team_id == 1) {
			throw_st::home = util.born_point[7];
			pick_st::home_id = 3;
			switch (player_id)
			{
			case 0:
				stand_st::stand_pos[0] = { 13,10 };
				stand_st::stand_pos[1] = { 14,12 };
				stand_st::stand_pos[2] = { 14,15 };
				stand_st::stand_pos[3] = { 14,18 };
				stand_st::stand_pos[4] = { 15,16 };
				stand_st::stand_pos[5] = { 15,19 };
				break;
			case 1:
				stand_st::stand_pos[0] = { 22,12 };
				stand_st::stand_pos[1] = { 25,12 };
				stand_st::stand_pos[2] = { 29,12 };
				stand_st::stand_pos[3] = { 22,15 };
				stand_st::stand_pos[4] = { 26,10 };
				stand_st::stand_pos[5] = { 29,10 };
				break;
			case 2:
				stand_st::stand_pos[0] = { 17,26 };
				stand_st::stand_pos[1] = { 16,29 };
				stand_st::stand_pos[2] = { 21,28 };
				stand_st::stand_pos[3] = { 26,28 };
				stand_st::stand_pos[4] = { 14,30 };
				stand_st::stand_pos[5] = { 28,24 };
				break;
			case 3:
				stand_st::stand_pos[0] = { 40,45 };
				stand_st::stand_pos[1] = { 30,45 };
				stand_st::stand_pos[2] = { 40,43 };
				stand_st::stand_pos[3] = { 30,43 };
				stand_st::stand_pos[4] = { 40,41 };
				stand_st::stand_pos[5] = { 30,41 };
				break;
			default:
				return false;
			}
		}
	}
	else if (map_type == 1) {
		if (team_id == 0) {
			Status::throw_st::home = util.born_point[3];
			Status::pick_st::home_id = 3;
			switch (player_id)
			{
			case 0:
				stand_st::stand_pos[0] = { 14,6 };
				stand_st::stand_pos[1] = { 19,9 };
				stand_st::stand_pos[2] = { 25,11 };
				stand_st::stand_pos[3] = { 30,9 };
				stand_st::stand_pos[4] = { 37,12 };
				stand_st::stand_pos[5] = { 10,13 };
				break;
			case 1:
				stand_st::stand_pos[0] = { 17,22 };
				stand_st::stand_pos[1] = { 19,19 };
				stand_st::stand_pos[2] = { 23,19 };
				stand_st::stand_pos[3] = { 26,19 };
				stand_st::stand_pos[4] = { 30,19 };
				stand_st::stand_pos[5] = { 32,22 };
				break;
			case 2:
				stand_st::stand_pos[0] = { 17,27 };
				stand_st::stand_pos[1] = { 19,30 };
				stand_st::stand_pos[2] = { 23,30 };
				stand_st::stand_pos[3] = { 26,30 };
				stand_st::stand_pos[4] = { 30,30 };
				stand_st::stand_pos[5] = { 32,27 };
				break;
			case 3:
				stand_st::stand_pos[0] = { 14,43 };
				stand_st::stand_pos[1] = { 19,40 };
				stand_st::stand_pos[2] = { 25,38 };
				stand_st::stand_pos[3] = { 30,40 };
				stand_st::stand_pos[4] = { 37,37 };
				stand_st::stand_pos[5] = { 10,36 };
				break;
			default:
				return false;
			}
		}
		else if (team_id == 1) {
			Status::throw_st::home = util.born_point[7];
			Status::pick_st::home_id = 3;
			switch (player_id)
			{
			case 0:
				stand_st::stand_pos[0] = { 35,6 };
				stand_st::stand_pos[1] = { 30,9 };
				stand_st::stand_pos[2] = { 24,11 };
				stand_st::stand_pos[3] = { 19,9 };
				stand_st::stand_pos[4] = { 12,12 };
				stand_st::stand_pos[5] = { 39,13 };
				break;
			case 1:
				stand_st::stand_pos[0] = { 32,22 };
				stand_st::stand_pos[1] = { 30,19 };
				stand_st::stand_pos[2] = { 26,19 };
				stand_st::stand_pos[3] = { 23,19 };
				stand_st::stand_pos[4] = { 29,19 };
				stand_st::stand_pos[5] = { 17,22 };
				break;
			case 2:
				stand_st::stand_pos[0] = { 32,27 };
				stand_st::stand_pos[1] = { 30,30 };
				stand_st::stand_pos[2] = { 26,30 };
				stand_st::stand_pos[3] = { 23,30 };
				stand_st::stand_pos[4] = { 19,30 };
				stand_st::stand_pos[5] = { 17,27 };
				break;
			case 3:
				stand_st::stand_pos[0] = { 35,43 };
				stand_st::stand_pos[1] = { 30,40 };
				stand_st::stand_pos[2] = { 24,38 };
				stand_st::stand_pos[3] = { 19,40 };
				stand_st::stand_pos[4] = { 12,37 };
				stand_st::stand_pos[5] = { 39,36 };
				break;
			default:
				return false;
			}
		}
	}
	else {
		stand_st::stand_flag = false;
		if (team_id == 0) {
			int home_score[4] = { 0 };
			int min_score = 5;
			int home_num = -1;
			for (int id = 0; id < 4; id++) {
				Cell test_home = util.born_point[id];
				Coord test_coord = test_home.to_grid();
				for (double test_angle = 0; test_angle < 2 * PI; test_angle += 0.1 * PI) {
					int throw_pos_x = test_coord.x + 10000 * cos(test_angle);
					int throw_pos_y = test_coord.y + 10000 * sin(test_angle);
					Coord throw_pos = { throw_pos_x,throw_pos_y };
					if (!util.raytrace_wall_r(test_coord, throw_pos, 600)) home_score[id]++;
				}
			}
			for (int id = 0; id < 4; id++) {
				if (home_score[id] >= min_score) {
					home_num = id;
					min_score = home_score[id];
				}
			}
			if (home_num != -1) {
				throw_st::home = util.born_point[home_num];
				pick_st::home_id = home_num;
			}
			else {
				throw_st::home = { 0,0 };
				throw_st::throw_home_flag = false;
			}
		}
		if (team_id == 1) {
			int home_score[4] = { 0 };
			int min_score = 5;
			int home_num = -1;
			for (int id = 4; id < 7; id++) {
				Cell test_home = util.born_point[id];
				Coord test_coord = test_home.to_grid();
				for (double test_angle = 0; test_angle < 2 * PI; test_angle += 0.1 * PI) {
					int throw_pos_x = test_coord.x + 10000 * cos(test_angle);
					int throw_pos_y = test_coord.y + 10000 * sin(test_angle);
					Coord throw_pos = { throw_pos_x,throw_pos_y };
					if (!util.raytrace_wall_r(test_coord, throw_pos, 600)) home_score[id - 4]++;
				}
			}
			for (int id = 4; id < 7; id++) {
				if (home_score[id - 4] >= min_score) {
					home_num = id;
					min_score = home_score[id - 4];
				}
			}
			if (home_num != -1) {
				throw_st::home = util.born_point[home_num];
				pick_st::home_id = home_num - 4;
			}
			else {
				throw_st::home = { 0,0 };
				throw_st::throw_home_flag = false;
			}
		}
	}
	return true;
}
//如果呆在原地不动，可能会被子弹打到从而损失的电量
int Status::pro_damage() {
	const THUAI5::SignalJammer* tgt[32];                 //32个可能的子弹
	auto self = util.self;
	int i = 0, j = 0, flag_min = 16;                     //flag:我往某个方向躲了以后会撞上的子弹数量
	double mtj_angle[32], distance[32], inc_angle[32];   //mtj：我指向子弹的向量角度 distance：向量模长 inc:我与子弹的连线 与 子弹方向 的锐角夹角
	int v[32], r[32], l[32], a[32];                      //子弹的速度、半径、飞行距离参数
	auto jammer = util.jammers;
	int jammer_num = jammer.size();                      //场上子弹的数量
	const Coord& my_pos = util.get_self_pos();           //我的位置
	Coord jammer_pos[32];
	int x = 0, y = 0;
	int powerlose = 0;
	int self_v = 4000;
	//定义一堆变量

	for (i = 0; i < 32; i++) {
		tgt[i] = NULL;
		mtj_angle[i] = 0;
		distance[i] = 50000;
		v[i] = 0;
		r[i] = 0;
		l[i] = 0;
		a[i] = 0;
		jammer_pos[i] = { 0,0 };
	}
	if ((int)self->softwareType == 1)self_v = 3000;
	else if ((int)self->softwareType == 2)self_v = 5000;
	else self_v = 4000;
	//以上变量的初始化

	for (i = 0; i < jammer_num; i++) {
		if (jammer[i] == NULL) continue;                           //判空
		if (jammer[i]->parentTeamID == self->teamID) continue;     //自家的不管
		jammer_pos[i] = { jammer[i]->x,jammer[i]->y };
		distance[i] = (jammer_pos[i] - my_pos).get_leng();
		mtj_angle[i] = util.get_self_pos().angle_to(jammer_pos[i]);
		inc_angle[i] = mtj_angle[i] - jammer[i]->facingDirection;

		if ((int)jammer[i]->type == 1) {
			v[i] = 3000, l[i] = 900, r[i] = 2000, a[i] = 2000;//矩形弹姑且假设半径为2000吧
			if (distance[i] * abs(cos(mtj_angle[i])) > 2900) continue;
			if (distance[i] * abs(sin(mtj_angle[i])) > 1000) continue;
		}
		else {
			if ((int)jammer[i]->type == 2) {
				v[i] = 2500, r[i] = 2500, l[i] = 4500, a[i] = 2500;
			}
			else if ((int)jammer[i]->type == 3) {
				v[i] = 5000, r[i] = 1500, l[i] = 9000, a[i] = 1500;
			}
			else if ((int)jammer[i]->type == 4) {
				v[i] = 2000, r[i] = 7000, l[i] = 2000, a[i] = 7000;
			}
			if (distance[i] * abs(cos(inc_angle[i])) > r[i] + l[i]) continue;
			if (distance[i] * abs(sin(inc_angle[i])) > 500 + r[i]) continue;
		}
		//这些确定一个子弹是否有威胁的判断条件应该没有问题

		powerlose += a[i];
	}
	//计算血量

	return powerlose;
}

//版本号：
//修改时间：
//距离自己最近的敌人的距离
int Status::min_strong_emy_dist() {
	int min_emy_dist = -1;
	Coord self_pos = util.get_self_pos();
	for (int id = 0; id < 8; id++) {
		if (util.robots[id] == NULL) continue;
		if (util.bot_update[id] < util.frame - 10) continue;
		if (util.robots[id]->teamID == util.self->teamID) continue;
		if ((int)util.robots[id]->softwareType != 3) continue;
		Coord enemy_pos = { util.robots[id]->x,util.robots[id]->y };
		if ((min_emy_dist == -1) || (self_pos+ -enemy_pos).get_leng() <= min_emy_dist) min_emy_dist = (int)(self_pos - enemy_pos).get_leng();
	}
	return min_emy_dist;
}

//版本号：051901
//修改时间：
//试图聚众
bool Actions::try_gather() {
	//对场上的队友人数进行判断
	int alive_ptn_num = 0;
	for (int id = 0; id < 8; id++) {
		if (util.robots[id]->teamID != util.self->teamID) continue;
		if (util.robots[id]->isResetting) continue;
		if (util.robots[id] == NULL) continue;
		alive_ptn_num++;
	}
	if (alive_ptn_num < 3) return false;

	//自身位置预处理
	const Coord& self_pos = util.get_self_pos();
	const Cell _self_cell = self_pos.to_cell();
	static const Cell delta[4] = { {1,0},{-1,0},{0,1},{0,-1} };
	Cell self_cell = _self_cell;
	for (int index = 0; index < 4; index++) {
		Cell test = _self_cell + delta[index];
		if (util.is_walkable(test) && (util.map_data[test.x][test.y] < 5) && (util.map_data[test.x][test.y] > 12)) {
			self_cell = test;
			break;
		}
	}
	double min_straight_dist = 1200;
	if ((util.self->timeUntilCommonSkillAvailable > 24000) && (util.self->timeUntilCommonSkillAvailable < 30000)) min_straight_dist = 1000;

	//敌人类型预处理
	static const int jammer_range[4] = { 4000,2500,1500,7000 };

	//信息处理
	static const int gathermsg_type = 3;
	const int min_frame = util.frame - 10;
	int gathermsg_size = util.msgs[gathermsg_type].size();
	Message gather_msg[4] = { {0,-1,-1," "},{0,-1,-1," "}, {0,-1,-1," "}, {0,-1,-1," "} };
	int ptn_gather_id[4] = { -1,-1,-1,-1 };
	int ptn_call_num = 0;
	int target_botid = -1;
	if (gathermsg_size > 0) {
		for (int index = 0; index < gathermsg_size; index++) {
			if ((util.msgs[gathermsg_type][index].sender == 0) && (util.msgs[gathermsg_type][index].frame > min_frame) && (util.msgs[gathermsg_type][index].frame >= gather_msg[0].frame)) {
				gather_msg[0] = util.msgs[gathermsg_type][index];
				continue;
			}
			if ((util.msgs[gathermsg_type][index].sender == 1) && (util.msgs[gathermsg_type][index].frame > min_frame) && (util.msgs[gathermsg_type][index].frame >= gather_msg[1].frame)) {
				gather_msg[1] = util.msgs[gathermsg_type][index];
				continue;
			}
			if ((util.msgs[gathermsg_type][index].sender == 2) && (util.msgs[gathermsg_type][index].frame > min_frame) && (util.msgs[gathermsg_type][index].frame >= gather_msg[2].frame)) {
				gather_msg[2] = util.msgs[gathermsg_type][index];
				continue;
			}
			if ((util.msgs[gathermsg_type][index].sender == 3) && (util.msgs[gathermsg_type][index].frame > min_frame) && (util.msgs[gathermsg_type][index].frame >= gather_msg[3].frame)) {
				gather_msg[3] = util.msgs[gathermsg_type][index];
				continue;
			}
		}
		for (int id = 0; id < 4; id++) {
			if (id == util.self->playerID) continue;
			if (gather_msg[id].msg != " ") {
				ptn_gather_id[id] = atoi(gather_msg[id].msg.c_str());
				ptn_call_num++;
			}
		}
	}

	//如果有队友呼叫，在队友给出的目标中选择
	if (ptn_call_num != 0) {
		double mov_dist[4] = { -1,-1,-1,-1 };
		double score[4] = { -1,-1,-1,-1 };
		double min_dist = -1;
		double max_score = -1;
		int _target_botid = -1;
		for (int id = 0; id < 4; id++) {
			if (ptn_gather_id[id] == -1) continue;
			if (util.bot_update[ptn_gather_id[id]] < util.frame - 10) continue;
			if (util.robots[ptn_gather_id[id]] == NULL) continue;
			if (util.robots[ptn_gather_id[id]]->isResetting) continue;
			Coord enemy_pos = { util.robots[ptn_gather_id[id]]->x,util.robots[ptn_gather_id[id]]->y };
			Cell enemy_cell = enemy_pos.to_cell();
			mov_dist[id] = util.get_dist(self_cell,ptn_gather_id[id]);
			if (mov_dist[id] == -1) continue;
			else if ((util.robots[ptn_gather_id[id]]->life != 0) && (util.get_dist(self_cell, ptn_gather_id[id]) != 0)) {
				score[id] = 9500 / (util.robots[ptn_gather_id[id]]->life) + 3200 * util.robots[ptn_gather_id[id]]->cpuNum;
			}
		}
		for (int id = 0; id < 4; id++) {
			if ((score[id] != -1) && ((max_score == -1) || (score[id] >= max_score))) {
				max_score = score[id];
				_target_botid = ptn_gather_id[id];
			}
		}
		if (_target_botid != -1) {
			target_botid = _target_botid;
		}
	}

	//如果没有队友呼叫,自己选择目标
	if (ptn_call_num == 0) {
		int enemy_teamid = 0;
		double mov_dist[4] = {-1,-1,-1,-1};
		double score[4] = { -1,-1,-1,-1 };
		double max_score = -1;
		int _target_botid = -1;
		if (util.self->teamID == 0) enemy_teamid = 1;
		for (int id =0; id < 4; id++) {
			int botid = enemy_teamid * 4 + id;
			if (util.bot_update[botid] < util.frame - 10) continue;
			if (util.robots[botid] == NULL) continue;
			if (util.robots[botid]->isResetting) continue;
			Coord enemy_pos = { util.robots[id]->x,util.robots[id]->y };
			Cell enemy_cell = enemy_pos.to_cell();
			mov_dist[id] = util.get_dist(self_cell,botid);
			if (mov_dist[id] == -1) continue;
			else if((util.robots[botid]->life!=0)&&(util.get_dist(self_cell,botid)!=0)) {
				score[id] =  9500 / (util.robots[botid]->life) + 3200 * util.robots[botid]->cpuNum;
			}
		}
		for (int id = 0; id < 4; id++) {
			if ((score[id] != -1) && ((max_score == -1) || (score[id] >= max_score))) {
				max_score = score[id];
				_target_botid = enemy_teamid * 4 + id;
			}
		}
		if (_target_botid != -1) {
			target_botid = _target_botid;
		}
	}

	//判断是否有目标以及自己是否离目标足够近
	//有目标则广播目标
	//足够近则返回
	if (target_botid == -1) return false;
	else {
		broadcast_gatherinfo(target_botid); //广播信息
		Actions::clear_gatherinfo();//自动清除不需要的信息
		int enemy_range = util.robots[target_botid]->attackRange + jammer_range[(int)util.robots[target_botid]->signalJammerType + 1];
		Coord target_pos = { util.robots[target_botid]->x,util.robots[target_botid]->y };
		double straight_dist=(target_pos-self_pos).get_leng();
		if ((straight_dist <= min_straight_dist)&&(!util.raytrace_wall(self_pos, target_pos))) return true;
		else{
			if (((int)util.self->softwareType == 2) && (util.self->timeUntilCommonSkillAvailable == 0) && (straight_dist <= 1.1 * enemy_range)) {
				api->UseCommonSkill();
			}
			util.navigate(target_pos);
			return true;
		}
	}
}

//版本号：051901
//修改时间：
//广播聚众信息
void Actions::broadcast_gatherinfo(int _botid) {
	std::string botid_msg = std::to_string(_botid);
	static const int gathermsg_type = 3;
	util.broadcast(gathermsg_type, botid_msg);
}

//版本号：051901
//修改时间：
//清除聚众信息
void Actions::clear_gatherinfo() {
	static const int gathermsg_type = 3;
	if (util.frame % 200 == 0) util.msgs[gathermsg_type].clear();
}



//主函数：Tank
//版本号：0423_final
//修改时间：*
void AI::play(IAPI& _api) {
	//初始化
	api = &_api;
	util.update_data();
	Cell self_pos = util.get_self_pos().to_cell();
	if(util.frame%2000==1||util.frame<=20) Status::action_info_update();
	
	//时间控制
	if (Status::play_timer.look() < 50 || _api.GetSelfInfo()->isResetting) return;
	Status::play_timer.reset();
	if ((util.frame >= 10800)||(Status::pick_st::home_cpu_num>=35)) Status::pick_st::pick_home_flag = 1;

	//捡家里之前
	bool evading = false;
	bool hitting = false;
	bool picking = false;
	bool surprising = false;
	bool gathering = false;
	if ((Status::pick_st::pick_home_flag == 0)||(util.self->playerID!=Status::pick_st::home_id)) {
		evading = Actions::evade_bullet();
		logger.log(1, "evading:%d\n", (int)evading);
		if (!evading) {
			if (Actions::try_catch()) {
				logger.log(1, "catching\n");
				return;
			}
			if (!Status::throw_st::moving_flag) picking = Actions::try_pick(false);
			logger.log(1, "picking:%d\n", (int)picking);
				if (util.self->playerID == Status::pick_st::home_id) Actions::try_hit_mobile();
				else Actions::try_hit_mobile();

			logger.log(1, "cpu num:%d\n", util.self->cpuNum);
			if ((!picking)||util.self->cpuNum>=5) {
				if (util.self->cpuNum > 0||Status::throw_st::moving_flag) {
					Actions::try_throw(true, true);
					logger.log(1, "throwing home mov\n");
					logger.log(1, "moving:%d", Status::throw_st::moving_flag);
				}
			}
			if (!picking && !Status::throw_st::moving_flag) {
				gathering = Actions::try_gather();
				if (util.self->playerID == Status::pick_st::home_id) Actions::try_hit_mobile();
				else Actions::try_hit_mobile();

				if(!gathering){
						logger.log(1, "standing\n");
						Actions::try_stand();
				}
			}
			if (!gathering) Actions::clear_gatherinfo();
		}
		else {
			if (util.self->playerID == Status::pick_st::home_id) Actions::try_hit_mobile();
			else Actions::try_hit_mobile();
			//Actions::try_hit_evade();
			if (util.self->cpuNum > 0) {
				if (!Actions::try_throw(true, false)) {
					if (util.self->cpuNum > 2 && util.self->life <= 3000)
						Actions::try_throw(false, false);
				}
			}
		}
	}
	else {
		if ((util.get_self_pos().to_cell()!=Status::throw_st::home) && util.self->cpuNum >= 10) api->UseCPU(util.self->cpuNum);
		Actions::try_pick(true);
		int pro_damage = Status::pro_damage();
		logger.log(1, "pro damage:%d\n", pro_damage);
		logger.log(1, "min enemy dist:%d\n", Status::min_strong_emy_dist());
		if (Status::pick_st::home_picked_flag) {
			Actions::try_hit_mobile();
			if ((pro_damage >= util.self->life) || ((Status::min_strong_emy_dist() <= 1600) && (Status::min_strong_emy_dist() != -1)) || (util.self->life <= 2500)) {
				Actions::evade_bullet();
				api->UseCPU(util.self->cpuNum);
			}
		}
	}
	
}