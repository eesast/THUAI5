
#include "../include/AI.h"

#include <random>
#include <chrono>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <iterator>

using namespace THUAI5;

#define AI_IMPL_BEGIN namespace ai_impl {
#define AI_IMPL_END } // namespace ai_impl

AI_IMPL_BEGIN

namespace constants {
	static constexpr double PI = 3.1415926535897932;

	// 每秒的帧数
	static constexpr int ticks_per_sec = 20;
	// 每帧的毫秒数
	static constexpr int tick_elapse = 1000 / ticks_per_sec;
	// 游戏时长
	static constexpr int total_ticks = 10 * 60 * ticks_per_sec; // TODO!!!
	// 地图大小
	static constexpr int map_size = 50;
	// 格子大小
	static constexpr int cell_size = 1000;

	static constexpr double inf = 1e30;

	// 主动技信息
	struct SoftwareAttr {
		int cooldown;
		int duration;
		int speed;

		int inEffect(int value) const {
			return std::max(0, value - (duration - cooldown));
		}

		static SoftwareAttr of(SoftwareType type) {
			switch (type) {
				case SoftwareType::Invisible:
					return { 30000,  6000, 5000 };
				case SoftwareType::PowerEmission:
					return { 40000, 10000, 3000 };
				case SoftwareType::Booster:
					return { 30000,  4000, 4000 };
				case SoftwareType::Amplification:
					return { 70000,  1000, 4000 };
				default:
					return { 0, 0, 0 };
			}
		}
	};

	// 子弹信息
	struct BulletAttr {
		int power;
		int speed;
		int attack_range;
		int explosion_radius;

		int life() const {
			return std::round(attack_range / (speed * (tick_elapse / 1000.0)));
		}

		static BulletAttr of(SignalJammerType type) {
			switch (type) {
				case SignalJammerType::StrongJammer:
					return { 7000, 2000, 2000, 7000 };
				case SignalJammerType::CommonJammer:
					return { 2500, 2500, 4500, 2500 };
				case SignalJammerType::FastJammer:
					return { 1500, 5000, 9000, 1500 };
				case SignalJammerType::LineJammer:
					return { 2000, 1000,  900, 4000 };
				default:
					return { 0, 0, 0, 0 };
			}
		}
	};
}

using namespace constants;

namespace util {
	template <typename T>
	constexpr int sgn(T a, T b = T()) {
		return (a > b) - (a < b);
	}

	template <typename T>
	inline void setMin(T &a, const T &b) {
		a = std::min(a, b);
	}

	template <typename T>
	inline void setMax(T &a, const T &b) {
		a = std::max(a, b);
	}

	template <typename V, typename T>
	inline void setMinV(std::pair<V, T> &res, V value, const T &item) {
		if (value < res.first) {
			res.first = value;
			res.second = item;
		}
	}

	template <typename V, typename T>
	inline void setMaxV(std::pair<V, T> & res, V value, const T &item) {
		if (value > res.first) {
			res.first = value;
			res.second = item;
		}
	}

	// 线性插值：x = a => 0, x = b => 1
	inline double linear(double a, double b, double x) {
		return (x - a) / (b - a);
	}

	// 判断是否有指定 buff
	inline bool hasBuff(const Robot &robot, BuffType type) {
		return std::find(robot.buff.begin(), robot.buff.end(), type) != robot.buff.end();
	}
}

#pragma region 辅助类

using guid_t = uint64_t;

template <typename T>
using Entity = std::shared_ptr<const T>;

template <typename T>
using Collection = std::vector<std::shared_ptr<const T>>;

template <typename Item, typename Score = double>
struct ScoredItem {
	Item item;
	Score score;

	ScoredItem()
		: item(), score(-inf) {}
	ScoredItem(const Item &item, const Score &score)
		: item(item), score(score) {}

	bool operator<(const ScoredItem &other) const {
		return score < other.score;
	}

	bool operator>(const ScoredItem &other) const {
		return other.score < score;
	}

	void update(const Item &item, const Score &score) {
		if (this->score < score) {
			this->item = item;
			this->score = score;
		}
	}
};

template <typename T, typename Score = double>
struct ScoredEntity : public ScoredItem<Entity<T>, Score> {
	using Base = ScoredItem<Entity<T>, Score>;
	using Base::Base;

	operator bool() const { return !!Base::item; }
	Entity<T> entity() const { return Base::item; }
	const T &ref() const { return *Base::item; }
};

struct Vector2 {
	int x, y;

	constexpr Vector2() : x(0), y(0) {}
	constexpr Vector2(int x, int y) : x(x), y(y) {}

	static Vector2 polar(double len, double arg) {
		int x = len * std::cos(arg);
		int y = len * std::sin(arg);
		return Vector2(x, y);
	}

	constexpr int64_t dot(int x2, int y2) const {
		return (int64_t)x * x2 + (int64_t)y * y2;
	}

	constexpr int64_t dot(const Vector2 &other) const {
		return (int64_t)x * other.x + (int64_t)y * other.y;
	}

	constexpr int64_t cross(int x2, int y2) const {
		return (int64_t)x * y2 - (int64_t)y * x2;
	}

	constexpr int64_t cross(const Vector2 &other) const {
		return (int64_t)x * other.y - (int64_t)y * other.x;
	}

	double len() const {
		return std::sqrt(dot(*this));
	}

	double arg() const {
		return std::atan2(y, x);
	}

	Vector2 rotate(double theta) const {
		double c = std::cos(theta);
		double s = std::sin(theta);
		return Vector2(c * x - s * y, c * y + s * x);
	}

	double cos(const Vector2 &other) const {
		return dot(other) / (len() * other.len());
	}
};

template <typename T>
constexpr Vector2 operator*(T factor, const Vector2 &vec) {
	return Vector2(factor * vec.x, factor * vec.y);
}

namespace {
	static constexpr std::array dir_wasd = {
		Vector2{1, 0}, Vector2{-1, 0}, Vector2{0, 1}, Vector2{0, -1}
	};

	static constexpr std::array dir_cross = {
		Vector2{0, 0}, Vector2{1, 0}, Vector2{-1, 0}, Vector2{0, 1}, Vector2{0, -1}
	};

	static constexpr std::array dir_diagonal = {
		Vector2{1, 1}, Vector2{1, -1}, Vector2{-1, 1}, Vector2{-1, -1}
	};
}

// 坐标基类
template <typename Derived>
struct GameCoord {
	int x, y;

	GameCoord() = default;
	GameCoord(int x, int y) : x(x), y(y) {}

	// 返回 this 到 other 的距离
	double distance(const Derived &other) const {
		int dx = x - other.x;
		int dy = y - other.y;
		return std::sqrt(dx * dx + dy * dy);
	}

	// 返回 this 到 other 的向量的辐角
	double direction(const Derived &other) const {
		return std::atan2(other.y - y, other.x - x);
	}

	Derived operator+(const Vector2 &vec) const {
		return { x + vec.x, y + vec.y };
	}

	Derived operator-(const Vector2 &vec) const {
		return { x - vec.x, y - vec.y };
	}

	Vector2 operator-(const Derived &other) const {
		return { x - other.x, y - other.y };
	}

	bool operator==(const Derived &other) const {
		return x == other.x && y == other.y;
	}

	bool operator!=(const Derived &other) const {
		return x != other.x || y != other.y;
	}

	// 判断点在矩形内
	bool inRect(int x0, int y0, int x1, int y1) const {
		return x >= x0 && x <= x1 && y >= y0 && y <= y1;
	}
};

struct GridPos;
struct CellPos;

// 绝对坐标（范围为 [0, 5e4]）
struct GridPos : GameCoord<GridPos> {
	GridPos() = default;
	GridPos(int x, int y) : GameCoord(x, y) {}

	// 由实体得到绝对坐标
	template <typename Object>
	static GridPos of(const Object &obj) {
		return { (int)obj.x, (int)obj.y };
	}

	// 转换为格子坐标
	CellPos toCell() const;
};

// 格子坐标（范围为 [0, 50]）
struct CellPos : GameCoord<CellPos> {
	CellPos() = default;
	CellPos(int x, int y) : GameCoord(x, y) {}

	// 由实体得到格子坐标（注意这里默认原坐标是绝对坐标，会自动转换）
	template <typename Object>
	static CellPos of(const Object &obj) {
		return GridPos::of(obj).toCell();
	}

	// 得到格子中心坐标
	GridPos toGrid() const;
};

inline CellPos GridPos::toCell() const {
	return { IAPI::GridToCell(x), IAPI::GridToCell(y) };
}

inline GridPos CellPos::toGrid() const {
	return { IAPI::CellToGrid(x), IAPI::CellToGrid(y) };
}

// 调试输出
inline std::ostream &operator<<(std::ostream &stream, Vector2 vec) {
	return stream << '(' << vec.x << ", " << vec.y << ')';
}

template <typename T>
inline std::ostream &operator<<(std::ostream &stream, GameCoord<T> coord) {
	return stream << '(' << coord.x << ", " << coord.y << ')';
}

#pragma endregion

AI_IMPL_END




AI_IMPL_BEGIN

struct RobotOnGrass {
    int32_t x;
    int32_t y;
    uint32_t signalJammerNum;
    uint32_t speed;
    uint32_t life;
    uint32_t cpuNum;

    uint32_t teamID;
    uint32_t playerID;
    uint64_t guid;     // 全局唯一ID

    double attackRange;                     // 攻击范围
    double timeUntilCommonSkillAvailable;   // 普通软件效果的冷却时间
    double emissionAccessory;               // 强制功率发射配件工作效率

    uint32_t buff_mask;
    PropType prop;
    SignalJammerType signalJammerType;
    HardwareType hardwareType;
    SoftwareType softwareType;
};

struct BulletOnGrass : SignalJammer {
	BulletOnGrass() = default;
	BulletOnGrass(const SignalJammer &bullet) : SignalJammer(bullet) {}
};

struct TeamFightRequest {
	guid_t target_guid;
	uint8_t playerId;
};

struct TeamFightResponse;

struct CPUThrown;

struct PropClaim {
	guid_t prop_guid;
	uint32_t playerId;
	bool claimed;
};

AI_IMPL_END




AI_IMPL_BEGIN

class AIBase;
class AIFighter;

#pragma region 主动目标

class ActiveGoal {
public:
	virtual ~ActiveGoal() = default;

	// 获得目标名
	virtual const char *name() = 0;

	// 判断是否执行
	virtual double shouldExecute(IAPI &) = 0;

	// 执行目标
	virtual bool execute(IAPI &) = 0;
};


// 空闲时占据有利位置
class PlaceGoal : public ActiveGoal {
public:
	AIFighter &ai;
	std::optional<GridPos> target_pos;

	PlaceGoal(AIFighter &ai)
		: ActiveGoal(), ai(ai) {}

	const char *name() { return "PlaceGoal"; }

	double shouldExecute(IAPI &api);
	bool execute(IAPI &api);
};


// 寻找敌人并攻击
class AttackGoal : public ActiveGoal {
public:
	AIFighter &ai;
	std::optional<guid_t> target_guid;
	std::shared_ptr<const Robot> target;
	std::vector<TeamFightRequest> team_fights;

	AttackGoal(AIFighter &ai)
		: ActiveGoal(), ai(ai) {}

	const char *name() { return "AttackGoal"; }

	double shouldExecute(IAPI &api);
	bool execute(IAPI &api);

private:
	double evaluateTarget(const Robot &enemy);
	GridPos getTargetPos(const Robot &enemy);
};


// 寻找道具并收集
class PropGoal : public ActiveGoal {
public:
	AIFighter &ai;
	std::optional<guid_t> target_guid;
	std::shared_ptr<const Prop> target;
	std::unordered_map<guid_t, uint32_t> claimed_props;

	PropGoal(AIFighter &ai)
		: ActiveGoal(), ai(ai) {}

	const char *name() { return "PropGoal"; }

	double shouldExecute(IAPI &api);
	bool execute(IAPI &api);

private:
	double evaluateTarget(const Prop &prop);
};


// TODO: 扔 CPU 回家
class ThrowCPUGoal : public ActiveGoal {
public:
	AIFighter &ai;
	GridPos dest_pos;
	GridPos walk_pos;

	ThrowCPUGoal(AIFighter &ai);

	const char *name() { return "ThrowCPUGoal"; }

	double shouldExecute(IAPI &api);
	bool execute(IAPI &api);
};

#pragma endregion

#pragma region 被动目标

class PassiveGoal {
public:
	AIBase &ai;

	PassiveGoal(AIBase &ai) : ai(ai) {}

	virtual ~PassiveGoal() = default;

	virtual void execute(IAPI &api) = 0;
};


// 躲避敌人
class AvoidEnemyGoal : public PassiveGoal {
public:
	struct Ambusher {
		Entity<Robot> robot;
		Vector2 speed;
		double weight;
	};
	
	std::vector<Ambusher> ambushers;
	std::vector<Ambusher> next_ambushers;

	using PassiveGoal::PassiveGoal;

	void execute(IAPI &api);

private:
	void detectAmbushers();
};


// 躲避子弹
class AvoidBulletGoal : public PassiveGoal {
public:
	using PassiveGoal::PassiveGoal;

	void execute(IAPI &api);
};


// 广播草丛敌人和子弹
class BroadcastGrassGoal : public PassiveGoal {
public:
	using PassiveGoal::PassiveGoal;

	void execute(IAPI &api);

private:
	RobotOnGrass packRobot(const Robot &robot);
	Robot unpackRobot(const RobotOnGrass &robot);
};


// 发射子弹
class ShootGoal : public PassiveGoal {
public:
	using PassiveGoal::PassiveGoal;

	void execute(IAPI &api);
};


// 捡起道具
class PickPropGoal : public PassiveGoal {
public:
	using PassiveGoal::PassiveGoal;

	void execute(IAPI &api);
};


// 使用技能
class UseSkillGoal : public PassiveGoal {
public:
	using PassiveGoal::PassiveGoal;

	void execute(IAPI &api);

private:
	bool shouldUseSkill(IAPI &api);
};

#pragma endregion

AI_IMPL_END




AI_IMPL_BEGIN

class AIBase;

// Floyd 最短路
struct FloydGraph {
	struct Path {
		double dis;
		int next;

		Path() : dis(inf), next(-1) {}
		Path(double dis, int next) : dis(dis), next(next) {}

		// 判断路径是否合法
		bool valid() const {
			return dis < inf &&next != -1;
		}

		// 用 (first, second) 更新当前路径
		void update(const Path &first, const Path &second) {
			double d = first.dis + second.dis;
			if (d < dis) {
				dis = d;
				next = first.next;
			}
		}
	};

	int size = 0;
	std::vector<std::vector<Path>> paths;

	FloydGraph() = default;

	void init(int size);
	void floyd();
};


// 寻路算法
class PathFinder {
private:
	// corner 容错边距
	static constexpr int margin = 50;

	AIBase &ai;

	bool can_walk[map_size][map_size] = {};
	bool is_corner[map_size][map_size] = {};
	std::vector<CellPos> obstacles;
	std::vector<GridPos> corners;
	FloydGraph corner_graph;

public:
	struct GlobalPath {
		std::vector<GridPos> points;
		double distance;

		bool valid() const { return !points.empty(); }
	};

	PathFinder(AIBase &ai) : ai(ai) {}

	void init();
	GlobalPath findGlobalPath(const GridPos &start, const GridPos &end);
	bool canWalkThrough(const GridPos &start, const GridPos &end) const;

private:
	void initMap();
	void initCorners();
	void buildCornerGraph();
};

AI_IMPL_END




AI_IMPL_BEGIN

// 规划路径
class MovePlanner {
private:
    static constexpr int future_ticks = 25;
    static constexpr int future_time = future_ticks * tick_elapse;

    AIBase &ai;

public:
    struct MoveTarget {
        GridPos pos;
        int priority = -1;
    };

    struct EnemyTarget {
        std::shared_ptr<const Robot> robot;
        double threat; // 威胁大小
        double near; // 5.0 * threat
        double far; // 1.0 * threat
    };

    struct BulletTarget {
        std::shared_ptr<const SignalJammer> bullet;
    };

    MoveTarget target;
    PathFinder::GlobalPath target_path;
    std::vector<EnemyTarget> enemies;
    std::vector<BulletTarget> bullets;

    std::optional<GridPos> last_target;
    std::optional<GridPos> last_pos;
    int stop_ticks = 0;
    
    MovePlanner(AIBase &ai);

    void reset();
    void planMove(IAPI &api);

    bool setTarget(const GridPos &pos, int priority);

private:
    class MovingBullet {
    public:
        AIBase &ai;
        BulletAttr attr;
        guid_t guid;
        GridPos emit_pos;
        GridPos pos;
        double facing;
        SignalJammerType type;
        int life;

        MovingBullet(AIBase &ai, const SignalJammer &bullet);

        bool move();
        double explosionPowerAt(const GridPos &robot_pos) const;
    };

    struct Evaluation {
        double score = 0.0;  // 分数
        double threat = 0.0; // 威胁
        double damage = 0.0; // 受到伤害

        void update(double weight, const Evaluation &other) {
            score += weight * other.score;
            threat += weight * other.threat;
            damage += weight * other.damage;
        }

        bool operator<(const Evaluation &other) const {
            if (damage != other.damage) return damage > other.damage;
            double a = score - threat;
            double b = other.score - other.threat;
            return a < b;
        }
    };

    // 评判备选方案
    Evaluation evaluateChoice(const Vector2 &vec, bool debug = false);
};

AI_IMPL_END




AI_IMPL_BEGIN

enum class MessageType : uint8_t {
    null,
    hello,
    robot_on_grass,      // 草丛里的机器人
    bullet_on_grass,     // 草丛里的子弹
    team_fight_request,  // 请求团战
    team_fight_response, // 回应团战
    prop_claim           // 声明要捡道具
};

// 编码的信息
struct Message {
    MessageType type;
    std::vector<uint8_t> body;

    Message() : type(MessageType::null), body() {}

    template <typename T>
    Message(MessageType type, const T &data) : type(type) { pack(data); }

    // 打包 data
    template <typename T>
    void pack(const T &data) {
        body.resize(sizeof(data));
        memcpy(body.data(), reinterpret_cast<const uint8_t *>(&data), sizeof(data));
    }

    // 解包 data
    template <typename T>
    bool unpack(T &data) const {
        if (body.size() != sizeof(data)) {
#ifdef _DEBUG
            std::cout << "failed to unpack: body size = " << body.size() << ", data size = " << sizeof(data) << std::endl;
#endif
            return false;
        }

        memcpy(reinterpret_cast<uint8_t *>(&data), body.data(), sizeof(data));
        return true;
    }

    // 编码为字符串
    std::string encode() const {
        std::string result;
        result += char('A' + static_cast<uint8_t>(type));
        for (auto byte : body) {
            result += char('a' + (byte & 15));
            result += char('a' + (byte >> 4 & 15));
        }
        return result;
    }

    // 从字符串解码
    bool decode(const std::string &str) {
        size_t len = str.length();
        if (len % 2 != 1) {
#ifdef _DEBUG
            std::cout << "failed to decode: str(" << len << ") = " << str << std::endl;
#endif
            return false;
        }

        type = static_cast<MessageType>(str[0] - 'A');

        size_t size = (len - 1) / 2;
        body.resize(size);
        for (size_t i = 0; i < size; i++) {
            uint8_t low = str[2 * i + 1] - 'a';
            uint8_t high = str[2 * i + 2] - 'a';
            body[i] = low | high << 4;
        }
        return true;
    }
};

AI_IMPL_END




AI_IMPL_BEGIN

// AI 基类
class AIBase {
public: // 懒得封装了

	// 随机数生成器
	std::mt19937_64 rng{ std::chrono::steady_clock::now().time_since_epoch().count() };

	// 地图
	std::vector<std::vector<PlaceType>> map;
	int map_id;

	// 玩家信息
	int self_teamId;
	int self_playerId;

	// 游戏状态
	std::shared_ptr<const Robot> self;
	int tick_count = -1;
	GridPos self_pos;

	// 游戏实体
	Collection<Robot> robots;
	Collection<Robot> teammates;
	Collection<Robot> enemies;
	Collection<SignalJammer> bullets;
	Collection<Prop> props;

	// 通信
	std::vector<Message> messages;

	// 目标
	std::vector<ActiveGoal *> active_goals;
	std::vector<PassiveGoal *> passive_goals;
	std::vector<PassiveGoal *> prioritized_goals;

	// 寻路
	PathFinder path_finder;
	MovePlanner move_planner;

public:
	AIBase(IAPI &api);
	AIBase(const AIBase &) = delete;

	virtual ~AIBase();

	virtual void play(IAPI &api) = 0;

	// 获取格子类型
	PlaceType getCell(const CellPos &cell) const {
		if (cell.x < 0 || cell.y < 0 || cell.x >= map_size || cell.y >= map_size)
			return PlaceType::Wall;
		return map[cell.x][cell.y];
	}

	// 判断格子类型是否能通行
	bool canWalkOn(PlaceType type) const {
		// 判断出生点，假定出生点的类型编号连续
		int temp = (int)type - (int)PlaceType::BirthPlace1;
		if (temp >= 0 && temp < 8)
			return temp == 4 * self_teamId + self_playerId;
		// 除出生点外只有墙不能通行
		return type != PlaceType::Wall;
	}

	// 判断指定位置是否能通行
	bool canWalkOn(int x, int y) const {
		return canWalkOn(getCell({ x, y }));
	}

	// 判断是否为草地
	static bool isGrass(PlaceType type) {
		return type == PlaceType::BlindZone1 || type == PlaceType::BlindZone2 || type == PlaceType::BlindZone3;
	}

	// 判断是否为出生地
	static bool isBirthplace(PlaceType type) {
		int temp = (int)type - (int)PlaceType::BirthPlace1;
		return temp >= 0 && temp < 8;
	}

	bool isBirthplace(const CellPos &pos) {
		return isBirthplace(getCell(pos));
	}

	struct BulletData {
		guid_t guid;
		SignalJammerType type;
		int emit_time;
		GridPos emit_pos;
	};

	// 子弹信息
	std::unordered_map<guid_t, BulletData> bullet_data;

	// 寻找指定 guid 的实体
	template <typename T>
	std::shared_ptr<const T> findEntity(const Collection<T> &entities, guid_t guid) {
		for (auto &entity : entities) {
			if (entity->guid == guid) return entity;
		}
		return {};
	}

	// 广播消息
	void broadcast(IAPI &api, const Message &message) {
		std::string str = message.encode();
		for (int i = 0; i < 4; i++)
			if (i != self_playerId) api.Send(i, str);
	}

protected:
	// 获取游戏状态
	void loadState(IAPI &api);

	void addGoal(ActiveGoal *goal) {
		active_goals.push_back(goal);
	}

	void addGoal(PassiveGoal *goal) {
		passive_goals.push_back(goal);
	}

	void addPrioritizedGoal(PassiveGoal *goal) {
		prioritized_goals.push_back(goal);
	}

	// 每帧重置行为
	void resetGoals();
	// 执行全部目标
	void executeGoals(IAPI &api);
};

AI_IMPL_END



AI_IMPL_BEGIN

AIBase::AIBase(IAPI &api)
	: map(map_size, std::vector<PlaceType>(map_size)),
	  path_finder(*this), move_planner(*this) {
	std::ios::sync_with_stdio(false);

	// 获取自身信息
	auto self = api.GetSelfInfo();
	self_teamId = self->teamID;
	self_playerId = self->playerID;

	// 获取地图
	auto full_map = api.GetFullMap();
	for (int x = 0; x < map_size; x++) {
	 	map[x].assign(full_map[x].begin(), full_map[x].end());
	}
	
	// 判断地图是哪一张
	if (getCell(CellPos{ 4, 40 }) == PlaceType::BirthPlace4) {
		map_id = 1;
	} else {
		map_id = 2;
	}

	//for (int x = 0; x < map_size; x++) {
	//	for (int y = 0; y < map_size; y++) {
	//		map[x][y] = api.GetPlaceType(x, y);
	//	}
	//}

#ifdef _DEBUG
	std::cout << "Map id = " << map_id << '\n';
	for (int x = 0; x < map_size; x++) {
		for (int y = 0; y < map_size; y++) {
			std::cout << char('A' + int(map[x][y])) << ' ';
		}
		std::cout << '\n';
	}
	std::cout << "======================\n";
#endif

	path_finder.init();
}

AIBase::~AIBase() {
	for (auto goal : active_goals) delete goal;
	for (auto goal : passive_goals) delete goal;
	for (auto goal : prioritized_goals) delete goal;
}

void AIBase::loadState(IAPI &api) {
	self = api.GetSelfInfo();
	robots = api.GetRobots();
	bullets = api.GetSignalJammers();
	props = api.GetProps();
	tick_count = api.GetFrameCount();

	self_pos = GridPos::of(*self);

	teammates.clear();
	enemies.clear();
	for (auto &robot : robots) {
		if (robot->isResetting || robot->guid == self->guid) continue;
		if (robot->teamID == self_teamId) {
			teammates.push_back(robot);
		} else {
			enemies.push_back(robot);
		}
	}

	messages.clear();
	while (auto str = api.TryGetMessage()) {
		Message message;
		if (message.decode(*str)) {
			messages.emplace_back(std::move(message));
		} else {
			std::cout << "error: cannot decode message (" << *str << ")" << std::endl;
		}
	}

#ifdef _DEBUG
	{
		int remains = tick_count % 20 * 5;
		int seconds = tick_count / 20;
		int minutes = seconds / 60;
		seconds = seconds % 60;

		char buf[256];
		sprintf(buf, "%d (%02d:%02d.%02d)", tick_count, minutes, seconds, remains);
		std::cout
			<< "Tick: " << buf << ' '
			<< "Pos: " << self_pos << ' '
			<< "Health: " << self->life << ' '
			<< "CPU: " << self->cpuNum << '\n';
	}
#endif
}

void AIBase::resetGoals() {
	move_planner.reset();
}

void AIBase::executeGoals(IAPI &api) {
	if (self->isResetting) return;

	for (auto goal : prioritized_goals) {
		goal->execute(api);
	}

	// 在这里统计才能把队友发来的信息算进去
	// TODO: 发子弹时发统计信息
	for (auto &bullet : bullets) {
		if (bullet->parentTeamID != self_teamId) {
			BulletData data{ bullet->guid, bullet->type, tick_count, GridPos::of(*bullet) };
			bullet_data.insert({ bullet->guid, data });
		}
	}

	using goal_t = std::pair<ActiveGoal *, double>;

	std::vector<goal_t> goals;
	for (auto goal : active_goals) {
		goals.emplace_back(goal, goal->shouldExecute(api));
	}

	std::sort(goals.begin(), goals.end(), [](const goal_t &a, const goal_t &b) {
		return a.second > b.second;
	});

	for (auto [goal, status] : goals) {
		bool ret = goal->execute(api);
#ifdef _DEBUG
		std::cout << "execute " << goal->name() << ": " << (ret ? "ok" : "failed") << '\n';
#endif
		if (ret) break;
	}

	for (auto goal : passive_goals) {
		goal->execute(api);
	}

	move_planner.planMove(api);
}

AI_IMPL_END




AI_IMPL_BEGIN

// 测试 AI
class AIChaser : public AIBase {
public:
	static constexpr SoftwareType software = SoftwareType::Invisible;
	static constexpr HardwareType hardware = HardwareType::PowerBank;

	AvoidEnemyGoal *avoidEnemyGoal;
	AvoidBulletGoal *avoidBulletGoal;

	AIChaser(IAPI &api);

	void play(IAPI &api);
};

AI_IMPL_END



AI_IMPL_BEGIN

AIChaser::AIChaser(IAPI &api) : AIBase(api) {
	addGoal(avoidEnemyGoal = new AvoidEnemyGoal(*this));
	addGoal(avoidBulletGoal = new AvoidBulletGoal(*this));
}

void AIChaser::play(IAPI &api) {
	using util::setMin;

	loadState(api);
	resetGoals();

	// 找到最近的敌方玩家
	ScoredEntity<Robot> nearest_enemy{ {}, -inf };
	for (auto &&robot : robots) {
		if (robot->teamID == self->teamID || robot->isResetting)
			continue;
		nearest_enemy.update(robot, -self_pos.distance(GridPos::of(*robot)));
	}

	// 攻击最近的敌方玩家（若存在）
	if (nearest_enemy) {
		const Robot &enemy = nearest_enemy.ref();
		auto enemy_pos = GridPos::of(enemy);

		move_planner.setTarget(enemy_pos, 1);

		double angle = self_pos.direction(enemy_pos);
		api.Attack(angle);
	}

	// 捡起道具
	for (auto &&prop : props) {
		double dis = GridPos::of(*prop).distance(self_pos);
		// 若相距一个格子以内，则尝试捡起道具
		if (dis < cell_size) {
			api.Pick(prop->type);
		}
	}

	// 使用主动技
	if (self->timeUntilCommonSkillAvailable == 0) {
		api.UseCommonSkill();
	}

	executeGoals(api);
}

AI_IMPL_END



AI_IMPL_BEGIN

// 测试 AI
class AIDemo : public AIBase {
public:
	static constexpr SoftwareType software = SoftwareType::Booster;
	static constexpr HardwareType hardware = HardwareType::EnergyConvert;

	AIDemo(IAPI &api);

	void play(IAPI &api);
};

AI_IMPL_END



AI_IMPL_BEGIN

AIDemo::AIDemo(IAPI &api) : AIBase(api) {
	// TODO
}

void AIDemo::play(IAPI &api) {
	using util::setMin;

	loadState(api);

	// 找到最近的敌方玩家
	std::pair nearest_enemy = { inf, (const Robot *)nullptr };
	for (auto &&robot : robots) {
		if (robot->teamID == self->teamID)
			continue;
		setMin(nearest_enemy, { self_pos.distance(GridPos::of(*robot)), robot.get() });
	}

	// 向 (27, 25) 移动（随便找的一个点）
	GridPos target_pos = CellPos(27, 25).toGrid();
	auto path = path_finder.findGlobalPath(self_pos, target_pos);
	if (path.valid()) {
		auto &first = path.points.front();
		Vector2 vec = first - self_pos;
		int time = std::min(50.0, vec.len() / self->speed * 1000);
		api.MovePlayer(time, vec.arg());
	} else {
		std::cout << "cannot find path to " << target_pos << '\n';
	}

	// 攻击最近的敌方玩家（若存在）
	if (nearest_enemy.second != nullptr) {
		double angle = self_pos.direction(GridPos::of(*nearest_enemy.second));
		api.Attack(angle);
	} else {
		// 否则，随机方向攻击
		api.Attack(std::uniform_real_distribution<>(0, 2 * PI)(rng));
	}

	// 捡起道具
	for (auto &&prop : props) {
		double dis = GridPos::of(*prop).distance(self_pos);
		// 若相距一个格子以内，则尝试捡起道具
		if (dis < cell_size) {
			api.Pick(prop->type);
		}
	}

	// 使用主动技
	if (self->timeUntilCommonSkillAvailable == 0) {
		api.UseCommonSkill();
	}
}

AI_IMPL_END



AI_IMPL_BEGIN

// 战士（吸血 + 吸血）
class AIFighter : public AIBase {
public:
	static constexpr SoftwareType software = SoftwareType::Invisible;
	static constexpr HardwareType hardware = HardwareType::PowerBank;

	PlaceGoal *placeGoal;
	AttackGoal *attackGoal;
	PropGoal *propGoal;
	AvoidBulletGoal *avoidBulletGoal;
	AvoidEnemyGoal *avoidEnemyGoal;

public:
	AIFighter(IAPI &api);

	void play(IAPI &api);

	// 判断当前帧是否可以发竞争类消息
	bool isLeadingTick() const {
		return tick_count % 12 == 3 * self_playerId;
	}
};

AI_IMPL_END



AI_IMPL_BEGIN

AIFighter::AIFighter(IAPI &api) : AIBase(api) {
	addPrioritizedGoal(new BroadcastGrassGoal(*this));

	addGoal(placeGoal = new PlaceGoal(*this));
	addGoal(attackGoal = new AttackGoal(*this));
	addGoal(propGoal = new PropGoal(*this));
	addGoal(new ThrowCPUGoal(*this));

	addGoal(avoidBulletGoal = new AvoidBulletGoal(*this));
	addGoal(avoidEnemyGoal = new AvoidEnemyGoal(*this));
	addGoal(new PickPropGoal(*this));
	addGoal(new ShootGoal(*this));
	addGoal(new UseSkillGoal(*this));
}

void AIFighter::play(IAPI &api) {
	loadState(api);
	resetGoals();

	executeGoals(api);
}

AI_IMPL_END



AI_IMPL_BEGIN

#pragma region 主动目标

double PlaceGoal::shouldExecute(IAPI &api) {
	constexpr double base_score = 1.0;

	if (ai.self->life > 2500) {
		target_pos = CellPos(27, 25).toGrid();
	} else {
		target_pos = CellPos(36, 44).toGrid();
	}

	return base_score;
}

bool PlaceGoal::execute(IAPI &api) {
	return target_pos && ai.move_planner.setTarget(*target_pos, 0);
}


double AttackGoal::shouldExecute(IAPI &api) {
	constexpr double base_score = 10000.0;   // 基准分数
	constexpr double inertia_bonus = 3000.0; // 维持目标的奖励
	constexpr double min_score = 500.0;        // 进行攻击至少需要的分数

	target.reset();

	// 收取团战消息
	team_fights.clear();
	for (auto &message : ai.messages) {
		if (message.type == MessageType::team_fight_request) {
			if (TeamFightRequest data; message.unpack(data)) {
				team_fights.push_back(data);
			}
		}
	}

	// 血量低时不主动打人
	// TODO: 被动防御？
	if (ai.self->life <= 2500) {
		target_guid.reset();
		return -inf;
	}

	// 判断之前的目标
	if (target_guid) {
		target = ai.findEntity(ai.robots, *target_guid);

		if (!target || target->isResetting) {
			target.reset();
		}
	}

	// 寻找最佳目标
	ScoredEntity<Robot> best;
	for (auto &enemy : ai.enemies) {
		double score = evaluateTarget(*enemy);
		if (enemy == target) score += inertia_bonus;
		best.update(enemy, score);
	}

	if (best.score > min_score) {
		target = best.entity();
		target_guid = target->guid;

		if (ai.isLeadingTick()) {
			// 发送团战消息
			TeamFightRequest data{ target->guid, ai.self_playerId };
			ai.broadcast(api, Message{ MessageType::team_fight_request, data });
#ifdef _DEBUG
			std::cout << "team fight request " << data.target_guid << '\n';
#endif
		}

		return base_score + best.score;
	} else {
		target.reset();
		target_guid.reset();
		return -inf;
	}
}

GridPos AttackGoal::getTargetPos(const Robot &enemy) {
	auto enemy_pos = GridPos::of(enemy);
	if (ai.isBirthplace(enemy.place)) {
		Vector2 vec = ai.self_pos - enemy_pos;
		return enemy_pos + std::min(1.0, 1600.0 / vec.len()) * vec;
	} else {
		return enemy_pos;
	}
}

double AttackGoal::evaluateTarget(const Robot &enemy) {
	double score = 2500.0;

	// 距离
	auto target_pos = getTargetPos(enemy);
	auto path = ai.path_finder.findGlobalPath(ai.self_pos, target_pos);
	if (!path.valid()) return -inf;
	double time = path.distance / ai.self->speed; // 过去用时，秒
	score -= 3.0 * std::pow(time, 3.0);

	// 判断需要几颗子弹
	auto bulletAttr = BulletAttr::of(ai.self->signalJammerType);
	int cnt_bullets = std::ceil((double)enemy.life / bulletAttr.power);
	score -= 1000.0 * cnt_bullets;

	// TODO: 其它类型
	if (enemy.softwareType == SoftwareType::Amplification) {
		auto attr = SoftwareAttr::of(SoftwareType::Amplification);
		int cd = enemy.timeUntilCommonSkillAvailable;

		if (attr.inEffect(cd) || cd < 1000 * time) {
			return -inf;
		}
	}

	// TODO: 敌人数量

	// 团战请求
	for (auto &data : team_fights) {
		if (data.target_guid == enemy.guid)
			score += 800.0;
	}

	// CPU 数量
	if (enemy.cpuNum > 0)
		score += 1000.0 + 100.0 * std::pow(enemy.cpuNum, 2.0);
	
	// 蹲出生点狙击
	if (enemy.cpuNum > 10 && ai.isBirthplace(enemy.place) && ai.tick_count < total_ticks - 30 * ticks_per_sec)
		return -inf;

	// TODO: buff
	using util::hasBuff;
	if (hasBuff(enemy, BuffType::MoveSpeed))
		score -= 1000.0;
	if (hasBuff(enemy, BuffType::AddLIFE))
		score -= 10000.0;
	if (hasBuff(enemy, BuffType::ShieldBuff) && !hasBuff(*ai.self, BuffType::SpearBuff))
		score -= 10000.0;

	return score;
}

bool AttackGoal::execute(IAPI &api) {
	if (!target) return false;

#ifdef _DEBUG
	std::cout << "attack target " << GridPos::of(*target) << '\n';
#endif

	auto bulletAttr = BulletAttr::of(SignalJammerType::CommonJammer);
	double far = 0.9 * (bulletAttr.attack_range + bulletAttr.explosion_radius) - 500;
	double near = std::min(0.9 * far, 0.6 * target->attackRange);

	GridPos enemy_pos = GridPos::of(*target);
	GridPos target_pos = getTargetPos(*target);

	double distance = ai.self_pos.distance(enemy_pos);

	if (distance > near) { // 靠近敌人
#ifdef _DEBUG
		std::cout << "near target " << enemy_pos << '\n';
#endif
		ai.move_planner.setTarget(target_pos, 1);
		ai.move_planner.enemies.push_back({ target, -500, 0, near });
		ai.move_planner.enemies.push_back({ target, -100, near, far });
	}

	return true;
}


double PropGoal::shouldExecute(IAPI &api) {
	constexpr double base_score = 10000.0;
	constexpr double inertia_bonus = 2500.0;
	constexpr double min_score = 500.0;

	// 判断之前的目标
	Entity<Prop> old_target;
	if (target_guid) {
		old_target = ai.findEntity(ai.props, *target_guid);
	}

	// 收取消息
	for (auto &message : ai.messages) {
		if (message.type == MessageType::prop_claim) {
			PropClaim data;
			if (!message.unpack(data)) continue;
			if (data.playerId != ai.self_playerId) {
#ifdef _DEBUG
				std::cout << (data.claimed ? "PropClaim" : "PropUnclaim")
					<< " " << data.prop_guid << " from " << data.playerId << '\n';
#endif
				if (data.claimed) {
					claimed_props[data.prop_guid] |= uint32_t(1) << data.playerId;
				} else {
					auto iter = claimed_props.find(data.prop_guid);
					if (iter != claimed_props.end()) {
						iter->second &= ~(uint32_t(1) << data.playerId);
						if (!iter->second) claimed_props.erase(iter);
					}
				}
			}
		}
	}

	// 寻找最佳目标
	ScoredEntity<Prop> best{ {}, min_score };
	for (auto &prop : ai.props) {
		double score = evaluateTarget(*prop);
		if (prop == old_target) score += inertia_bonus;
		best.update(prop, score);
	}
	target = best.entity();

	// 判断是否放弃了之前的目标
	if (target_guid && target != old_target) {
		PropClaim payload{ *target_guid, ai.self_playerId, false };
		ai.broadcast(api, { MessageType::prop_claim, payload });
#ifdef _DEBUG
		std::cout << "Gave up prop " << *target_guid;
		if (old_target) {
			double score = evaluateTarget(*old_target) + inertia_bonus;
			std::cout << ", score = " << score;
		}
		std::cout << '\n';
#endif
	}

	if (target) {
		target_guid = target->guid;
#ifdef _DEBUG
		std::cout << "PropTarget = " << *target_guid << ", score = " << best.score;
		if (ai.isLeadingTick())
			std::cout << " (claimed)";
		std::cout << '\n';
#endif
		// 广播当前目标
		if (ai.isLeadingTick()) {
			PropClaim payload{ target->guid, ai.self_playerId, true };
			ai.broadcast(api, { MessageType::prop_claim, payload });
		}
		return base_score + best.score;
	} else {
		target_guid.reset();
		return -inf;
	}
}

double PropGoal::evaluateTarget(const Prop &prop) {
	double score = 0.0;

	// 判断是否在移动
	if (prop.isMoving) return -inf;

	// 判断是否已经被队友设为目标
	if (claimed_props.count(prop.guid)) {
		if (prop.type == PropType::CPU) {
			score += 1000.0; // CPU 不嫌人多
		} else {
			return -inf;
		}
	}

	if (prop.type == PropType::CPU) {
		bool near_end = ai.tick_count > total_ticks - 40 * ticks_per_sec;
		bool is_birthplace = ai.isBirthplace(CellPos::of(prop));
		if (near_end) { // 比赛即将结束，回收 CPU
			if (is_birthplace && ai.canWalkOn(prop.place)) score += 10000000.0;
		} else { // 比赛进行中，判断是否在出生地
			if (is_birthplace) return -inf;
		}
		score += 7000.0; // 鼓励 CPU
	}

	// 判断能否到达
	GridPos prop_pos = GridPos::of(prop);
	auto path = ai.path_finder.findGlobalPath(ai.self_pos, prop_pos);
	if (!path.valid()) return -inf;

	double time = path.distance / ai.self->speed; // 过去用时，秒

	if (ai.attackGoal->target) {
		if (time < 2.0) {
			score += 10000.0 + 10000.0 / (2.0 + time);
		}
	} else {
		score += 10000.0 / (1.0 + time);
	}

	// 附近敌人
	// for (auto &enemy : ai.enemies) {
	// 	GridPos enemy_pos = GridPos::of(*enemy);
	// 	auto path2 = ai.path_finder.findGlobalPath(enemy_pos, prop_pos);
	// 	if (!path2.valid()) continue;
	// 
	// 	double enemy_time = path2.distance / enemy->speed;
	// 
	// 	if (enemy_time < 5.0 && enemy_time < time) {
	// 		score -= 5000.0 / (1.0 + enemy_time);
	// 	}
	// }

	return score;
}

bool PropGoal::execute(IAPI &api) {
	if (!target) return false;

	GridPos target_pos = GridPos::of(*target);

	ai.move_planner.setTarget(target_pos, 2);

#ifdef _DEBUG
	std::cout << "target prop = " << target_pos << ", place = " << (int)target->place << '\n';
#endif
	return true;
}


ThrowCPUGoal::ThrowCPUGoal(AIFighter &ai) : ai(ai) {
	CellPos cell;
	if (ai.map_id == 1) {
		cell = ai.self_teamId == 0 ? CellPos(4, 40) : CellPos(44, 40);
	} else {
		cell = ai.self_teamId == 0 ? CellPos(6, 46) : CellPos(43, 46);
	}
	dest_pos = cell.toGrid();
	walk_pos = dest_pos;
}

double ThrowCPUGoal::shouldExecute(IAPI &api) {
	double score = 20000.0;

	if (ai.self->cpuNum == 0) return -inf;
	if (ai.tick_count > total_ticks - 35 * ticks_per_sec) return -inf;

	for (int i = -1; i < 16; i++) {
		if (i >= 0) walk_pos = dest_pos + Vector2::polar(1600.0, PI / 8 * i);
		auto path = ai.path_finder.findGlobalPath(ai.self_pos, walk_pos);
		if (path.valid()) return score;
	}

#ifdef _DEBUG
	std::cout << "failed to throw from " << ai.self_pos << " to " << dest_pos << '\n';
#endif
	return -inf;
}

bool ThrowCPUGoal::execute(IAPI &api) {
	ai.move_planner.setTarget(walk_pos, 2);

	int time = ai.self_pos.distance(dest_pos) / 3.0;
	if (time < 5000) {
		Vector2 vec = ai.self_pos - dest_pos;
		GridPos temp_pos = dest_pos + std::min(1.0, 1600.0 / vec.len()) * vec;
		if (ai.path_finder.canWalkThrough(ai.self_pos, temp_pos)) {
#ifdef _DEBUG
			std::cout << "throw cpu towards " << dest_pos << ", time = " << time << '\n';
#endif
			api.ThrowCPU(time, ai.self_pos.direction(dest_pos), (ai.self->cpuNum + 1) / 2);
		} else {
			std::cout << "cannot walk from " << ai.self_pos << " to " << temp_pos << '\n';
		}
	}
	
	return true;
}

#pragma endregion

#pragma region 被动目标

void AvoidEnemyGoal::execute(IAPI &api) {
	using util::linear;

	// detectAmbushers();

	// 是否应尽量保全自己
	bool safe_mode = false;
	{
		if (ai.self->life <= 2500)
			safe_mode = true;
		if (auto ai = dynamic_cast<AIFighter *>(&this->ai)) {
			if (auto prop = ai->propGoal->target) {
				if (prop->type == PropType::CPU && ai->isBirthplace(ai->getCell(CellPos::of(*prop))))
					safe_mode = true;
			}
		}
	}

	auto process = [&](Entity<Robot> robot, double weight) {
		if (robot->isResetting) return;

		if (robot->teamID == ai.self_teamId) {
			ai.move_planner.enemies.push_back({ robot, 5000, 1200, 1500 });
		} else {
			auto attr = BulletAttr::of(robot->signalJammerType);
			double near = 0.75 * attr.speed;
			double far = std::max(1.1 * near, 0.6 * (attr.attack_range + attr.explosion_radius));
			double threat = 0.005 * attr.power;

			auto skill = SoftwareAttr::of(robot->softwareType);

			if (robot->softwareType == SoftwareType::Amplification) {
				int cd = robot->timeUntilCommonSkillAvailable;
				if (skill.inEffect(cd) || cd < 5000) {
					threat *= 15.0;
					near = 12000;
					far = 15000;
				}
			} else {
				if (robot->signalJammerNum == 0) {
					threat *= 0.2;
					near *= 0.7;
					far *= 0.7;
				}

				if (ai.self->signalJammerNum == 0) {
					threat *= 1.2;
					near *= 1.1;
					far *= 1.1;
				} else if (ai.self->signalJammerNum == 1) {
					threat *= 1.0;
					near *= 0.9;
					far *= 0.95;
				} else {
					threat *= 0.8;
					near *= 0.8;
					far *= 0.8;
				}

				auto self_skill = SoftwareAttr::of(ai.self->softwareType);
				int skill_time = self_skill.inEffect(ai.self->timeUntilCommonSkillAvailable);
				if (skill_time > 3000) {
					threat *= 0.3;
					near *= 0.3;
					far *= 0.4;
				} else if (skill_time > 500) {
					threat *= 0.7;
					near *= 0.8;
					far *= 0.8;
				}
			}

			if (safe_mode) {
				threat *= 5.0;
				near *= 1.5;
				far *= 1.6;
			}

			ai.move_planner.enemies.push_back({ robot, weight * threat, near, far });
		}
	};

	for (auto &robot : ai.robots)
		process(robot, 1.0);

	for (auto &ambusher : ambushers)
		process(ambusher.robot, ambusher.weight);
}

void AvoidEnemyGoal::detectAmbushers() {
	std::vector<Ambusher> old_ambushers;
	old_ambushers.swap(next_ambushers);

	std::set<guid_t> guids;

	ambushers.clear();
	for (auto &ambusher : old_ambushers) {
		auto previous = ambusher.robot;
		GridPos prev_pos = GridPos::of(*previous);
		if (auto current = ai.findEntity(ai.robots, ambusher.robot->guid)) {
			if (ambusher.weight == 0.0 && !current->isResetting) { // 找到人，放到下一帧
				Vector2 speed = GridPos::of(*current) - prev_pos;
				next_ambushers.push_back(Ambusher{ current, speed, 0.0 });
				guids.insert(current->guid);
			}
		} else { // 找不到人，可能隐身
			// 判断速度夹角
			Vector2 vec = ai.self_pos - prev_pos;
			if (ambusher.speed.cos(vec) < 0.6) continue;

			double weight = 0.8;
			// 判断技能冷却
			if (previous->softwareType != SoftwareType::Invisible || previous->timeUntilCommonSkillAvailable > 500) {
				weight = 0.3;
			}

			// 计算新坐标
			double walk_dis = std::min(vec.len() / 2.0, previous->speed * (tick_elapse / 1000.0));
			Vector2 speed = Vector2::polar(walk_dis, vec.arg());
			GridPos cur_pos = prev_pos + speed;

			// 模拟机器人
			Robot robot = *previous;
			robot.x = cur_pos.x;
			robot.y = cur_pos.y;
			robot.place = ai.getCell(cur_pos.toCell());
			robot.signalJammerNum = 2;

			Ambusher temp{ std::make_shared<const Robot>(robot), speed, weight};
			ambushers.push_back(temp);
			next_ambushers.push_back(temp);
			guids.insert(robot.guid);

#ifdef _DEBUG
			std::cout << "Ambusher " << robot.playerID << " at " << cur_pos << ", weight = " << weight << '\n';
#endif
		}
	}

	// 根据子弹猜测机器人位置
	for (auto &bullet : ai.bullets) {
		auto iter = ai.bullet_data.find(bullet->guid);
		if (iter != ai.bullet_data.end() && iter->second.emit_time == ai.tick_count) {
			auto emit_pos = GridPos::of(*bullet) - Vector2::polar(700.0, bullet->facingDirection);
			ScoredEntity<Robot> nearest;

			for (auto &enemy : ai.enemies)
				nearest.update(enemy, -emit_pos.distance(GridPos::of(*enemy)));
			for (auto &ambusher : ambushers) {
				nearest.update(ambusher.robot, -emit_pos.distance(GridPos::of(*ambusher.robot)));
			}

			for (auto &ambusher : ambushers) {
				if (ambusher.robot == nearest.entity()) {
					auto &robot = const_cast<Robot &>(*ambusher.robot);
					robot.x = emit_pos.x;
					robot.y = emit_pos.y;
					break;
				}
			}
		}
	}

	// 判断剩余机器人
	for (auto &enemy : ai.enemies) {
		if (!guids.count(enemy->guid)) {
			next_ambushers.push_back({ enemy, Vector2{}, 0.0 });
			guids.insert(enemy->guid);
		}
	}
}


void AvoidBulletGoal::execute(IAPI &api) {
	for (auto &bullet : ai.bullets) {
		// 判断友军伤害
		if (bullet->parentTeamID == ai.self_teamId) continue;

		double distance = GridPos::of(*bullet).distance(ai.self_pos);
		auto attr = BulletAttr::of(bullet->type);
		// 粗略剪枝，只在可能打到时考虑躲避
		double max_distance = attr.attack_range + attr.explosion_radius
			+ ai.self->speed * attr.life() * tick_elapse + 600.0;
		if (distance < 1.1 * max_distance) {
			ai.move_planner.bullets.push_back({ bullet });
		}
	}
}


void BroadcastGrassGoal::execute(IAPI &api) {
	std::set<guid_t> bullet_guids;
	std::set<guid_t> robot_guids{ ai.self->guid };

	// 子弹
	for (auto &bullet : ai.bullets) {
		bullet_guids.insert(bullet->guid);
		if (ai.isGrass(bullet->place)) {
			BulletOnGrass data{ *bullet };
			ai.broadcast(api, Message{ MessageType::bullet_on_grass, data });
		}
	}

	// 其它机器人
	for (auto &robot : ai.robots) {
		robot_guids.insert(robot->guid);
		if (ai.isGrass(robot->place)) {
			RobotOnGrass data = packRobot(*robot);
			ai.broadcast(api, Message{ MessageType::robot_on_grass, data });
		}
	}

	// 自己
	auto attr = SoftwareAttr::of(ai.self->softwareType);
	if (ai.isGrass(ai.self->place) || attr.inEffect(ai.self->timeUntilCommonSkillAvailable)) {
		RobotOnGrass data = packRobot(*ai.self);
		ai.broadcast(api, Message{ MessageType::robot_on_grass, data });
	}

	// 收取
	for (auto &message : ai.messages) {
		if (message.type == MessageType::bullet_on_grass) {
			if (BulletOnGrass data; message.unpack(data)) {
				if (!bullet_guids.count(data.guid)) {
					bullet_guids.insert(data.guid);
					ai.bullets.emplace_back(std::make_shared<const SignalJammer>(data));
				}
			}
		} else if (message.type == MessageType::robot_on_grass) {
			if (RobotOnGrass data; message.unpack(data)) {
				if (!robot_guids.count(data.guid)) {
					robot_guids.insert(data.guid);
					ai.robots.emplace_back(std::make_shared<const Robot>(unpackRobot(data)));
					if (data.teamID == ai.self_teamId)
						ai.teammates.push_back(ai.robots.back());
					else
						ai.enemies.push_back(ai.robots.back());
				}
			}
		}
	}
}

RobotOnGrass BroadcastGrassGoal::packRobot(const Robot &robot) {
	RobotOnGrass data{
		robot.x, robot.y,
		robot.signalJammerNum,
		robot.speed,
		robot.life,
		robot.cpuNum,
		robot.teamID, robot.playerID, robot.guid,
		robot.attackRange,
		robot.timeUntilCommonSkillAvailable,
		robot.emissionAccessory,
		0,
		robot.prop,
		robot.signalJammerType,
		robot.hardwareType,
		robot.softwareType
	};

	for (auto buff : robot.buff)
		data.buff_mask |= 1 << (int)buff;

	return data;
}

Robot BroadcastGrassGoal::unpackRobot(const RobotOnGrass &data) {
	Robot robot{
		true, false,
		data.x, data.y,
		data.signalJammerNum,
		data.speed,
		data.life,
		data.cpuNum,
		500, // radius
		3000, // CD
		0, // lifeNum
		0, // score
		data.teamID, data.playerID, data.guid,
		data.attackRange,
		data.timeUntilCommonSkillAvailable,
		0.0, // timeUntilUltimateSkillAvailable
		data.emissionAccessory,
		{}, // buff
		data.prop,
		PlaceType::NullPlaceType, // place
		data.signalJammerType,
		data.hardwareType,
		data.softwareType
	};

	for (int i = 1; i <= 4; i++) {
		if (data.buff_mask >> i & 1)
			robot.buff.push_back(static_cast<BuffType>(i));
	}

	CellPos cell = CellPos::of(robot);
	robot.place = ai.getCell(cell);
	
	return robot;
}


void ShootGoal::execute(IAPI &api) {
	if (ai.self->signalJammerNum == 0)
		return;

	ScoredEntity<Robot> target;
	auto bulletAttr = BulletAttr::of(ai.self->signalJammerType);

	Entity<Prop> target_prop;
	{
		if (auto ai = dynamic_cast<AIFighter *>(&this->ai)) {
			if (auto &prop = ai->propGoal->target) {
				if (ai->self_pos.distance(GridPos::of(*prop)) < 9000.0)
					target_prop = prop;
			}
		}
	}

	for (auto &enemy : ai.enemies) {
		double score = 0.0;

		double far;
		if (target_prop) {
			far = 1.4 * (bulletAttr.attack_range + bulletAttr.explosion_radius);
		} else {
			far = 0.6 * (bulletAttr.attack_range + bulletAttr.explosion_radius);
		}

		GridPos target_pos = GridPos::of(*enemy);
		double distance = target_pos.distance(ai.self_pos);

		if (distance > far) continue;

		score -= distance / cell_size;
		if (target_prop)
			score += 500.0 / (0.5 + target_pos.distance(GridPos::of(*target_prop)));

		if (auto ai = dynamic_cast<AIFighter *>(&this->ai)) {
			if (enemy == ai->attackGoal->target)
				score += 50.0;
		}

		target.update(enemy, score);
	}

	if (target) {
		GridPos target_pos = GridPos::of(target.ref());
#ifdef _DEBUG
		std::cout << "shoot target " << target_pos << '\n';
#endif

		double angle = ai.self_pos.direction(target_pos);
		double disturb = std::uniform_real_distribution<>(-1.0, 1.0)(ai.rng);
		int number = target_prop ? 1 : ai.self->signalJammerNum;

		switch (number) {
			case 2:
				angle += 0.05 * disturb;
				api.Attack(angle - 0.33);
				api.Attack(angle + 0.33);
				break;
			case 3:
				angle += 0.03 * disturb;
				api.Attack(angle - 0.43);
				api.Attack(angle);
				api.Attack(angle + 0.43);
				break;
			default:
				api.Attack(angle + 0.35 * disturb);
				break;
		}
	}
}


void PickPropGoal::execute(IAPI &api) {
	bool useProp = false;

	switch (ai.self->prop) {
		case PropType::Booster:
			useProp = true;
			break;
		case PropType::Battery:
			if (ai.self->life <= 2600)
				useProp = true;
			break;
		case PropType::Shield:
			if (ai.self->life <= 2600) {
				useProp = true;
				break;
			}
			if (auto ai = dynamic_cast<AIFighter *>(&this->ai)) {
				auto bulletAttr = BulletAttr::of(ai->self->signalJammerType);
				if (auto target = ai->attackGoal->target) {
					GridPos target_pos = GridPos::of(*target);
					double far = 0.9 * (bulletAttr.attack_range + bulletAttr.explosion_radius);

					if (ai->self_pos.distance(target_pos) < far) {
						useProp = true;
						break;
					}
				}
			}
			break;
		case PropType::ShieldBreaker:
			if (auto ai = dynamic_cast<AIFighter *>(&this->ai)) {
				auto bulletAttr = BulletAttr::of(ai->self->signalJammerType);
				if (auto target = ai->attackGoal->target) {
					bool has_shield = util::hasBuff(*target, BuffType::ShieldBuff);

					GridPos target_pos = GridPos::of(*target);
					double far = 1.1 * (bulletAttr.attack_range + bulletAttr.explosion_radius);

					if (has_shield && ai->self_pos.distance(target_pos) < far) {
						useProp = true;
						break;
					}
				}
			}
			break;
	}

	if (useProp) api.UseProp();

	// 血量低时使用 CPU
	if (ai.self->cpuNum >= 5 && ai.self->life <= 6000)
		api.UseCPU(ai.self->cpuNum);

	bool near_end = ai.tick_count > total_ticks - 45 * ticks_per_sec;

	for (auto &prop : ai.props) {
		if (!near_end && prop->type == PropType::CPU) {
			if (ai.isBirthplace(CellPos::of(*prop))) continue;
		}
		GridPos prop_pos = GridPos::of(*prop);
		if (ai.self_pos.toCell() == prop_pos.toCell()) {
			// 若已经持有道具则先使用
			if (!useProp && ai.self->prop != PropType::NullPropType) {
				useProp = true;
				api.UseProp();
			}
			api.Pick(prop->type);
		}
	}
}


void UseSkillGoal::execute(IAPI &api) {
	if (shouldUseSkill(api)) {
		api.UseCommonSkill();
	}
}

bool UseSkillGoal::shouldUseSkill(IAPI &api) {
	if (auto ai = dynamic_cast<AIFighter *>(&this->ai)) {
		// 还没冷却
		if (ai->self->timeUntilCommonSkillAvailable != 0)
			return false;

		// 电量低
		if (ai->self->life <= 2500)
			return true;

		// 敌人距离较近
		auto bulletAttr = BulletAttr::of(ai->self->signalJammerType);
		double far = 1.45 * (bulletAttr.attack_range + bulletAttr.explosion_radius);
		for (auto &enemy : ai->enemies) {
			GridPos enemy_pos = GridPos::of(*enemy);
			if (ai->self_pos.distance(enemy_pos) < far)
				return true;
		}

		return false;
	}
	
	return false;
}

#pragma endregion

AI_IMPL_END



AI_IMPL_BEGIN

MovePlanner::MovePlanner(AIBase &ai) : ai(ai) {

}

void MovePlanner::reset() {
	target = { ai.self_pos, -1 };
	target_path.points.assign({ ai.self_pos });
    enemies.clear();
    bullets.clear();
}

// 设置目标
bool MovePlanner::setTarget(const GridPos &pos, int priority) {
#ifdef _DEBUG
	std::cout << "setTarget " << pos << " priority = " << priority << '\n';
#endif

	if (priority <= target.priority)
		return false;

	auto path = ai.path_finder.findGlobalPath(ai.self_pos, pos);
	if (!path.valid()) {
#ifdef _DEBUG
		std::cout << "failed: no path" << '\n';
#endif
		return false;
	}

	target = { pos, priority };
	target_path = std::move(path);
	return true;
}

// 根据目标规划并执行移动操作
void MovePlanner::planMove(IAPI &api) {
	ScoredItem choice = { Vector2{}, Evaluation{ -inf, inf, inf } };

	auto update = [this, &choice](const Vector2 &vec, double delta = 0.0) {
		Evaluation value = evaluateChoice(vec);
		value.score += delta;
		choice.update(vec, value);
	};

	double max_walk = ai.self->speed * (future_time / 1000.0);

	auto farthest = [this, max_walk](const GridPos &pos) {
		Vector2 vec = pos - ai.self_pos;
		double len = vec.len();
		if (len > max_walk)
			return (max_walk / len) * vec;
		return vec;
	};

	// 优先向目标
	update(farthest(target_path.points.front()), 10.0);

	// 继续上次的方案
	if (last_target)
		update(farthest(*last_target), 8.0);

	// 随机一些方案
	std::uniform_real_distribution<> len_gen(0.5 * max_walk, max_walk);
	std::uniform_real_distribution<> arg_gen(0, 2 * PI);
	for (int i = 0; i < 200; i++) {
		double len = len_gen(ai.rng);
		double arg = arg_gen(ai.rng);
		update(Vector2::polar(len, arg));
	}

#ifdef _DEBUG
	// evaluateChoice(choice.item, true); // 调试
#endif

	if (last_pos) {
		if (ai.self_pos.distance(*last_pos) < 5.0) {
			stop_ticks++;
			if (stop_ticks >= 3) { // 一段时间没有移动，强制随机游走
#ifdef _DEBUG
				std::cout << "force walk!" << '\n';
#endif
				choice.item = Vector2::polar(max_walk, arg_gen(ai.rng));
			}
		} else {
			stop_ticks = 0;
		}
	}
	last_pos = ai.self_pos;

	// 执行移动
	int time = std::min(50.0, choice.item.len() / ai.self->speed * 1000);
	api.MovePlayer(time, choice.item.arg());
	last_target = ai.self_pos + choice.item;

	// 血条危险时吃掉 CPU
	if (choice.score.damage >= ai.self->life)
		api.UseCPU(ai.self->cpuNum);

#ifdef _DEBUG
	std::cout << "target = " << target.pos << ", priority = " << target.priority << '\n';
	std::cout << "choice = " << choice.item << ", score = " << choice.score.score << '\n';
	if (choice.score.damage > 0) {
		std::cout << "<DAMAGE> " << choice.score.damage << '\n';
	}
#endif
}

MovePlanner::Evaluation MovePlanner::evaluateChoice(const Vector2 &vec, bool debug) {
	using util::linear;

	Evaluation result{};

	// 移动终点
	GridPos end_pos = ai.self_pos + vec;

	// 判断是否被墙阻挡
	if (!ai.path_finder.canWalkThrough(ai.self_pos, end_pos))
		result.score = -inf;

	// 到目标的时间
	result.score -= 1.0 * end_pos.distance(target_path.points.front()) / ai.self->speed;

	// 移动中的子弹
	std::vector<MovingBullet> moving_bullets;
	for (auto [bullet] : bullets)
		moving_bullets.emplace_back(ai, *bullet);

	if (debug) {
		std::cout << "@choice = " << vec << '\n';
		for (auto &[robot, threat, near, far] : enemies) {
			std::cout << "robot " << robot->teamID << ":" << robot->playerID
				<< " threat = " << threat
				<< " far = " << far
				<< " near = " << near << '\n';
		}
		for (auto &bullet : moving_bullets) {
			std::cout << jammer_dict[bullet.type]
				<< " at " << bullet.pos
				<< " facing " << bullet.facing / PI * 180
				<< " life = " << bullet.life
				<< " guid = " << bullet.guid << '\n';
		}
	}

	for (int t = 1; t <= future_ticks; t++) {
		// 经过时间
		double time_elapsed = t * (tick_elapse / 1000.0);
		// 当前位置
		GridPos cur_pos = ai.self_pos + std::min(1.0, ai.self->speed * time_elapsed / vec.len()) * vec;
		// tick 权重
		double weight = 1.0 - 0.02 * t;
		// tick 内估值
		Evaluation current{};

		if (debug) {
			std::cout << "==== Tick #" << t << " ====" << '\n';
			std::cout << "cur_pos = " << cur_pos << '\n';
		}

		// 躲避敌人
		for (auto &enemy : enemies) {
			GridPos enemy_pos = GridPos::of(*enemy.robot);
			double dis = cur_pos.distance(enemy_pos);
			double temp = 0.0;
			if (dis < enemy.near)
				temp += 5.0 * linear(enemy.near, 0.0, dis);
			if (dis < enemy.far)
				temp += 1.0 * linear(enemy.far, enemy.near, dis);
			current.threat += enemy.threat * temp;
		}

		// 躲避子弹
		// TODO: 考虑 buff
		for (auto &bullet : moving_bullets) {
			if (!bullet.move()) continue;

			double dis = cur_pos.distance(bullet.pos);
			double chance = 0.0;

			if (bullet.life > 0) {
				double radius = 600.0;
				double near = radius + bullet.attr.explosion_radius;
				if (dis <= radius) {
					chance += 1.0;
				} else if (dis <= near) {
					// chance += 0.25;
					current.threat += 10.0 * bullet.attr.power * linear(near, radius, dis);
				}
			} else {
				chance += bullet.explosionPowerAt(cur_pos);
				if (debug && bullet.life == 0) {
					std::cout << "bullet " << bullet.guid << " bomb at " << bullet.pos << '\n';
				}
			}

			current.damage += chance * bullet.attr.power;
		}

		if (debug) {
			std::cout
				<< "$score = " << current.score << '\n'
				<< "$threat = " << current.threat << '\n'
				<< "$damage = " << current.damage << '\n';
		}

		result.update(weight, current);
	}

	return result;
}


MovePlanner::MovingBullet::MovingBullet(AIBase &ai, const SignalJammer &bullet) :
	ai(ai),
	attr(BulletAttr::of(bullet.type)),
	guid(bullet.guid),
	pos(GridPos::of(bullet)),
	facing(bullet.facingDirection),
	type(bullet.type) {
	auto it = ai.bullet_data.find(guid);
	if (it != ai.bullet_data.end()) {
		emit_pos = it->second.emit_pos;
		life = attr.life() - (ai.tick_count - it->second.emit_time);
	} else {
		emit_pos = pos;
		life = attr.life();
	}
}

bool MovePlanner::MovingBullet::move() {
	if (life <= -8) return false;

	pos = pos + Vector2::polar(tick_elapse / 1000.0 * attr.speed, facing);

	// 测试撞墙
	CellPos cell = pos.toCell();
	if (ai.getCell(cell) == PlaceType::Wall) {
		life = -6;
	}
	
	return true;
}

double MovePlanner::MovingBullet::explosionPowerAt(const GridPos &robot_pos) const {
	if (type == SignalJammerType::LineJammer) {
		Vector2 vec = (robot_pos - pos).rotate(-facing); // TODO: emit_pos or pos?
		int dx = std::max(-500 - vec.x, vec.x - 4500);
		int dy = std::max(-1000 - vec.y, vec.y - 1000);
		int dis = std::min(dx, dy);

		if (dis <= 0)
			return 1.0;
		if (dis <= 100)
			return 0.5 * util::linear(100, 0, dis);
	} else {
		double dis = pos.distance(robot_pos);
		double near = attr.explosion_radius + 525.0;
		double far = attr.explosion_radius + 625.0;
		
		if (dis < near)
			return 1.0;
		//if (dis < far)
		//	return 1.0 * util::linear(far, near, dis);
	}
	return 0.0;
}

AI_IMPL_END



AI_IMPL_BEGIN

void FloydGraph::init(int size) {
	this->size = size;
	paths.resize(size);
	for (int u = 0; u < size; u++) {
		paths[u].resize(size);
		paths[u][u] = { 0, u };
	}
}

void FloydGraph::floyd() {
	for (int w = 0; w < size; w++) {
		for (int u = 0; u < size; u++) {
			for (int v = 0; v < size; v++) {
				paths[u][v].update(paths[u][w], paths[w][v]);
			}
		}
	}
}


void PathFinder::init() {
	initMap();
	initCorners();
	buildCornerGraph();
}

void PathFinder::initMap() {
	for (int x = 0; x < map_size; x++) {
		for (int y = 0; y < map_size; y++) {
			can_walk[x][y] = ai.canWalkOn(x, y);
			if (!can_walk[x][y])
				obstacles.emplace_back(x, y);
		}
	}
}

void PathFinder::initCorners() {
	// # . .
	// . . .
	// . . .

	auto checkCorner = [this](int x, int y) {
		for (auto &[dx, dy] : dir_cross)
			if (!can_walk[x + dx][y + dy]) return;

		GridPos pos;
		bool found = false;
		for (auto &vec : dir_diagonal) {
			if (!can_walk[x + vec.x][y + vec.y]) {
				if (found) return;
				found = true;
				pos = CellPos(x, y).toGrid() - margin * vec;
			}
		}
		if (!found) return;

		is_corner[x][y] = true;
		corners.push_back(pos);
	};

	for (int x = 1; x < map_size - 1; x++)
		for (int y = 1; y < map_size - 1; y++)
			checkCorner(x, y);

#ifdef _DEBUG
	std::cout << "Map" << '\n';
	for (int x = 0; x < map_size; x++) {
		for (int y = 0; y < map_size; y++) {
			std::cout << (can_walk[x][y] ? is_corner[x][y] ? 'C' : '.' : '#') << ' ';
		}
		std::cout << '\n';
	}
#endif
}

// 建图
void PathFinder::buildCornerGraph() {
	int corner_cnt = corners.size();
	auto &G = corner_graph;
	G.init(corner_cnt);

	for (int u = 1; u < corner_cnt; u++) {
		for (int v = 0; v < u; v++) {
			if (canWalkThrough(corners[u], corners[v])) {
				double dis = corners[u].distance(corners[v]);
				G.paths[u][v] = { dis, v };
				G.paths[v][u] = { dis, u };
			}
		}
	}

	G.floyd();
}

// 判断 start 到 end 能否直线直达
bool PathFinder::canWalkThrough(const GridPos &start, const GridPos &end) const {
	using util::sgn;

	Vector2 vec = end - start;
	int64_t C = vec.cross(start.x, start.y);

	for (auto &cell : obstacles) {
		GridPos grid = cell.toGrid();
		int x_0 = grid.x - cell_size;
		int y_0 = grid.y - cell_size;
		int x_1 = grid.x + cell_size;
		int y_1 = grid.y + cell_size;

		if (start.x <= x_0 && end.x <= x_0) continue;
		if (start.x >= x_1 && end.x >= x_1) continue;
		if (start.y <= y_0 && end.y <= y_0) continue;
		if (start.y >= y_1 && end.y >= y_1) continue;

		bool vis[3] = { 0 };
		vis[1 + sgn(vec.cross(x_0, y_0), C)] = true;
		vis[1 + sgn(vec.cross(x_1, y_0), C)] = true;
		vis[1 + sgn(vec.cross(x_0, y_1), C)] = true;
		vis[1 + sgn(vec.cross(x_1, y_1), C)] = true;

		if (vis[0] && vis[2]) return false;
	}

	return true;
}

// 找到全局路径
PathFinder::GlobalPath PathFinder::findGlobalPath(const GridPos &start, const GridPos &end) {
	using Path = FloydGraph::Path;

	GlobalPath global_path;

	int corner_cnt = corners.size();
	const auto &G = corner_graph;

	// 判断直达
	if (canWalkThrough(start, end)) {
		global_path.points.emplace_back(end);
		global_path.distance = start.distance(end);
		return global_path;
	}

	// 枚举起点 corner
	std::vector<Path> starters;
	for (int u = 0; u < corner_cnt; u++) {
		if (canWalkThrough(start, corners[u])) {
			starters.emplace_back(start.distance(corners[u]), u);
		}
	}
	if (starters.empty()) return global_path;

	// 枚举终点 corner
	Path result{};
	int end_corner = -1;
	for (int v = 0; v < corner_cnt; v++) {
		Path current{};
		if (canWalkThrough(end, corners[v])) {
			for (auto &path : starters) {
				current.update(path, G.paths[path.next][v]);
			}
		}
		current.dis += end.distance(corners[v]);
		if (current.dis < result.dis) {
			result = current;
			end_corner = v;
		}
	}
	if (end_corner == -1) return global_path;

	// 还原路径
	for (int u = result.next; u != end_corner; u = G.paths[u][end_corner].next) {
		global_path.points.emplace_back(corners[u]);
	}
	global_path.points.emplace_back(corners[end_corner]);
	global_path.points.emplace_back(end);
	global_path.distance = result.dis;

	return global_path;
}

AI_IMPL_END



// 选手角色
using AIClass = ai_impl::AIFighter;

// 为假则 play() 调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与 IAPI 的方法同时访问
// 异步标志，感觉取 false 就行
extern const bool asynchronous = false;

// 选手主动技能
extern const SoftwareType playerSoftware = AIClass::software;

// 选手被动技能
extern const HardwareType playerHardware = AIClass::hardware;

// 给外部代码调用的接口
void AI::play(IAPI &api) {
	static AIClass ai(api);
	ai.play(api);
}


