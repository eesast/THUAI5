#include <random>
#include<cmath>
#include "../include/AI.h"
#pragma warning(disable:C26495)

// 为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与IAPI的方法同时访问
extern const bool asynchronous= true;

#define framet (50)
#define hzt (300)
#define NNN (270)
//x*NNN+y
#define NN (250)
#define leng (200)
#define emax (0x3f3f3f3f)
#define emin (-10031207)
#define testime 10
using namespace THUAI5;

const int robotspeed[6] = { 0,3000,5000,4000,4000 };//(robotspeed[int(pr->softwareType)] *buff[pr->playID+pr->teamID*4][1]/1000)
const int jsspeed[6] = { 0,1000,2500,5000,2000 };//jsspeed[int(pr->type)]
const int attack_range[6] = { 0,900,4500,9000,2000 };
const int explosion_radius[6] = { 0,4000,2500,1500,7000 };

extern const THUAI5::SoftwareType playerSoftware = THUAI5::SoftwareType::Invisible;
int myvelocity = robotspeed[(int)SoftwareType::Invisible]-500;
extern const THUAI5::HardwareType playerHardware = THUAI5::HardwareType::PowerBank;

int emap[NNN][NNN];
long long startime; int nowt,xbirth = 4500,ybirth =7500 ,store=0,fpick=0,xgt= 29500,ygt= 17500;
std::shared_ptr<const THUAI5::Robot> self;
int buff[10][6],attackt[10];
namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

#ifndef CHRONO
#include<chrono>
#endif
inline long long nowtime()
{
	auto msec = std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();
	return msec;
}

inline int ftime() {
	return (int)(nowtime() - startime);
}

//返回从开局到现在的毫秒数
 //non- nowtime
inline double dist(int32_t x, int32_t y) {
	return sqrt(pow((x - self->x), 2) + pow((y - self->y), 2));
}

inline void buf(std::shared_ptr<const THUAI5::Robot> robot) {
	std::vector<BuffType> bv = robot->buff;
	int i = robot->playerID + self->teamID * 4;//Id of player
	BuffType k;
	
	while (bv.size()) {
		 k = bv.back();bv.pop_back();
		if (k == BuffType::MoveSpeed)buff[i][1] <<= 1; else buff[i][int(k)] = 1;
	}
	buff[i][0] = bv.size()- buff[i][4];
}

int lcy[6666]={0};
using namespace std;
const int grid[5][37] = { {0} ,{0}, {14,5,7,9,10,11,12,12,13,13,14,14,14,14,14}, {9,4,6,7,8,8,9,9,9,9},
{36, 10,13,16,18,20, 21,22,24,25,26, 27,28,29,29,30, 31,32,32,33,33, 34,34,34,35,35, 35,36,36,36,36, 36,36,36,36,36, 36} };

#define PI (3.141592653589793)
inline double distancetosegment(double center_x, double center_y, double rad, int x, int y)
{//compute the distance of grid(x,y) to segment
	double d = sqrt((center_x - (x * leng + leng / 2)) * (center_x - (x * leng + leng / 2)) + (center_y - (y * leng + leng / 2)) * (center_y - (y * leng + leng / 2)));
	double rad_need;
	if (center_x - (x * leng + leng / 2) < 1e-6 && center_x - (x * leng + leng / 2) > -1e-6) {
		rad_need = PI / 2 - rad;
	}
	else {
		double tan_mc = (center_y - (y * leng + leng / 2)) / (center_x - (x * leng + leng / 2));
		rad_need = atan(tan_mc) - rad;
	}
	if (d * abs(cos(rad_need)) <= 2000)return d * abs(sin(rad_need));
	else return(sqrt((d * abs(cos(rad_need)) - 2000) * (d * abs(cos(rad_need)) - 2000) + d * sin(rad_need) * d * sin(rad_need)));
}

inline int SeekIndexInGrid(int num, int type)
{
	//j=grid[x][y],x=type, seek for the first y whose j larger than num
	int i;
	for (i = 1; i <= grid[type][0]; ++i)
	{
		if (grid[type][i] >= num)
			return i;
	}
	return 0;
}

inline void ExplosionDanger(std::shared_ptr<const THUAI5::SignalJammer> sj, const IAPI& api, int rx, int ry, int t)
{
	double alpha = sj->facingDirection;
	int j = 0, k = 0, t_esc = emax, t_set = emax;
	//If not Linejammer
	if (((int)sj->type) > 1)
	{
		for (j = 1; j <= grid[(int)sj->type][0]; ++j)
		{
			for (k = 1; k <= int(grid[(int)sj->type][grid[(int)sj->type][0] - j]); ++k)
			{//t_esc means explosion time minus the time required for escape;
				int verti = (int)(grid[(int)sj->type][grid[(int)sj->type][0] - j] - k);
				int horiz = grid[(int)sj->type][0] - j - SeekIndexInGrid(k, (int)(sj->type));
				t_esc = t - ((verti < horiz) ? verti + 1 : horiz + 1) * leng * 1000 / myvelocity;//find which is better: a horizontal escape or vertical escape?
			//with defination of d=abs(x-rx)+abs(y-ry)
				t_set = t_esc - 2 * grid[(int)sj->type][0] + k + j;
				if ((int)(rx / leng) - j >= 0 && (int)(ry / leng) - k >= 0 && emap[(int)(rx / leng) - j][(int)(ry / leng) - k] > t_set)
					emap[(int)(rx / leng) - j][(int)(ry / leng) - k] = t_set;//only decrease the emap value
				if ((int)(rx / leng) - j >= 0 && (int)(ry / leng) + k < NNN && emap[(int)(rx / leng) - j][(int)(ry / leng) + k] > t_set)
					emap[(int)(rx / leng) - j][(int)(ry / leng) + k] = t_set;
				if ((int)(rx / leng) + j < NNN && (int)(ry / leng) + k < NNN && emap[(int)(rx / leng) + j][(int)(ry / leng) + k] > t_set)
					emap[(int)(rx / leng) + j][(int)(ry / leng) + k] = t_set;
				if ((int)(rx / leng) + j < NNN && (int)(ry / leng) - k >= 0 && emap[(int)(rx / leng) + j][(int)(ry / leng) - k] > t_set)
					emap[(int)(rx / leng) + j][(int)(ry / leng) - k] = t_set;
			}
		}
		for (k = 0; k <= grid[(int)sj->type][0]; ++k)
		{//under the same defination of t_set
			t_esc = (grid[(int)sj->type][0] - k + 1) * leng * 1000 / myvelocity;
			t_set = t_esc - 2 * grid[(int)sj->type][0] + k;
			if ((int)(rx / leng) + k < NNN && emap[(int)(rx / leng) + k][(int)(ry / leng)] > t_set)
				emap[(int)(rx / leng) + k][(int)(ry / leng)] = t_set;
			if ((int)(rx / leng) - k >= 0 && emap[(int)(rx / leng) - k][(int)(ry / leng)] > t_set)
				emap[(int)(rx / leng) - k][(int)(ry / leng)] = t_set;
			if ((int)(ry / leng) + k < NNN && emap[(int)(rx / leng)][(int)(ry / leng) + k] > t_set)
				emap[(int)(rx / leng)][(int)(ry / leng) + k] = t_set;
			if ((int)(ry / leng) - k < NNN && emap[(int)(rx / leng)][(int)(ry / leng) - k] > t_set)
				emap[(int)(rx / leng)][(int)(ry / leng) - k] = t_set;
		}
	}

	else//In the case of Linejammer
	{
		int xlower = (int)((rx - 2000.99) / leng - 1), xupper = (int)((rx + 2000) / leng) + 2;
		int ylower = (int)((ry - 2000.99) / leng - 1), yupper = (int)((ry + 2000) / leng) + 2;
		double SegmentDistance = 0;
		//calculations include a 21*21 area
		int xi = xlower, yi = ylower;
		bool flag11 = true, flag12 = true, flag21 = true, flag22 = true;
		const double threshold = 500 + leng + 0.5 * leng * 1.415;//above threshold, grid receives no damage
		while (xi <= xupper)
		{
			flag12 = flag12 = flag21 = flag22 = true;//representing (xi, yi)*(>0?, <=NN?)
			if (xi <= 0)flag11 = false;
			else if (xi > NN) flag12 = false;
			while (flag11 && flag12 && yi <= yupper)
			{
				if (yi <= 0) flag21 = false;
				else if (yi > NN) flag22 = false;
				SegmentDistance = distancetosegment(rx, ry, alpha, xi, yi);
				if (flag21 && flag22 && SegmentDistance <= threshold)
				{
					t_set = t - (500 - (int)(SegmentDistance + 1) / leng) * 1.1314 * 1000 / myvelocity;
					//1.1314 is the estimate of max(sin(x)+sin(45deg-x))*sqrt(2) in domain(0,45deg).
					if (emap[xi][yi] > t_set)
						emap[xi][yi] = t_set;
				}
				if (!flag22) break;
				++yi;
			}
			if (!flag12)break;
			++xi;
		}

	}
	return;
}

inline void danger(std::shared_ptr<const THUAI5::SignalJammer> sj, const IAPI& api) {
	//Initialize all components
	double psi = 0, rxx = 0, ryy = 0;
	int rx = sj->x, ry = sj->y, rx1 = rx, ry1 = ry;//store the original rx and ry
	int v = jsspeed[int(sj->type)];
	int range = attack_range[(int)sj->type];
	//Get direction of SJ
	double alpha = sj->facingDirection;
	int j = 0, k = 0, i = 0, t = 0;//Frame number i
	bool blindcheck = true;
	if (nowt == lcy[sj->guid] && !((int)api.GetPlaceType(rx1 / 1000, ry1 / 1000)))//if near blindzone, we assume it will explode immediately.
	{
		rx = rx1 - (int)round(v * cos(alpha) / 1000.0 * framet);
		ry = ry1 - (int)round(v * sin(alpha) / 1000.0 * framet);
		if ((int)api.GetPlaceType((int)(rx / 1000), (int)(ry / 1000)) >= 2 && (int)api.GetPlaceType((int)(rx / 1000), (int)(ry / 1000)) <= 4)
			blindcheck = false;
	}
	for (i = 0; v * i / 1000.0 * framet <= range - v * (nowt - lcy[sj->guid]) / 1000.0; ++i)//Discrete assembly of positions on i
	{
		t = framet * (i - 3); //possible explosion time in milliseconds, after 2 more frames
		rx = rx1 + (int)round(v * i * cos(alpha) / 1000.0 * framet);
		ry = ry1 + (int)round(v * i * sin(alpha) / 1000.0 * framet);
		if (!blindcheck)
			ExplosionDanger(sj, api, rx, ry, t);
		//Follow the SJ's track
		int delta_t = 1400 * 1000 / myvelocity;
		for (k = (int)(rx / leng - 3); k <= 3 + (int)((rx + 200) / leng); ++k)
		{
			for (j = (int)(ry / leng - 3); j <= 3 + (int)((ry + 200) / leng); ++j)
			{
				if (i < 3 && emap[k][j] > -50)emap[k][j] = -50;
				else
				{
					long long tempdistance = (long long)(abs(k - rx / leng) + abs(j - ry / leng));
					if (emap[k][j] > leng * tempdistance * 1000 / myvelocity + t - delta_t)
						emap[k][j] = leng * tempdistance * 1000 / myvelocity + t - delta_t;
				}

			}
		}
		//If encounters wall(get 13 points from the front semicircle);
		bool flag = false;
		for (j = 0; j < 13; ++j)
		{
			psi = alpha - PI / 2 + j * PI / 12;
			rxx = (int)(rx + 200 * cos(psi));
			ryy = (int)(ry + 200 * sin(psi));
			if ((int)(api.GetPlaceType(rxx / 1000, ryy / 1000)) == 1 || rxx < 0 || ryy < 0 || rxx >= 50000 || ryy >= 50000)
			{
				flag = true; break;
			}
		}
		if (flag)break;
	}
	ExplosionDanger(sj, api, rx, ry, t);
	return;
}

int d[NNN][NNN];
int xyf[NNN*NNN];
int Xnow, Ynow;
int sjnum = 0;

inline void locate() {
	Xnow = self->x / leng;
	Ynow = self->y / leng;
	if (self->x % leng ) {
		if ((!(self->y % leng)) && emap[Xnow][Ynow] < emap[Xnow][Ynow - 1]) --Ynow;
	}
	else {
		if (!(self->y % leng)) {
			if (emap[Xnow][Ynow] < emap[Xnow - 1][Ynow]) --Xnow;
		}
		else {
			if (emap[Xnow - 1][Ynow] <= emap[Xnow][Ynow] && emap[Xnow - 1][Ynow] <= emap[Xnow][Ynow - 1]) { 
				--Xnow;
				if (emap[Xnow][Ynow - 1] < emap[Xnow][Ynow])--Ynow;
			}
			  else {
				if (emap[Xnow - 1][Ynow - 1] <= emap[Xnow][Ynow] && emap[Xnow - 1][Ynow-1] <= emap[Xnow ][Ynow - 1])--Xnow, --Ynow;
				else if (emap[Xnow][Ynow - 1] < emap[Xnow][Ynow])--Ynow;
			  }
		}
	}
		return;
}
inline bool cget(int32_t x, int32_t y, IAPI& api) {
	int k = (int)(api.GetPlaceType(self->x/1000, self->y/1000));
	return( x / 1000 == self->x / 1000 && y / 1000 == self->y / 1000 &&( k<5 || k>12 || testime * 60 * 1000 - nowt < 60 * 1000));
}
int xq[NNN * NNN<<2],yq[NNN*NNN<<2];
int v[NNN][NNN],dp[NNN][NNN];
int myv;
inline void SPFA(IAPI& api) {
	memset(d, 0x3f, sizeof(d));// dist 数组
	memset(v, 0, sizeof(v));// 节点标记
	memset(dp, 0x3f, sizeof(dp));
	int l=0, r=1;
	dp[Xnow][Ynow] = d[Xnow][Ynow] = 0;//fortest
	v[Xnow][Ynow] = 1;
	xyf[Xnow*NNN+Ynow] = Xnow * NNN + Ynow;
	xq[1] = Xnow; yq[1] = Ynow;
	while (l<r) {
		++l;
		int x = xq[l], y = yq[l],
			kk = (int)api.GetPlaceType(x * leng / 1000, y * leng / 1000),
			b = (kk > 1 && kk < 5);
		    if (kk == 13)b = 2;
			v[x][y] = 0;

			int xx = x - 1, yy = y - 1,
				vd = 1 + 1.4142 * leng,
				vdp = 1 + 1.4142 * leng;
			    if (emap[x][yy] > framet && emap[xx][y] > framet &&(d[x][y] + vd) * 1000 < myvelocity *((long long)emap[xx][yy]-framet)) {
				if (b == 1 && (kk = (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000)) > 1 && kk < 5)vdp = vd * 0.8;
				else if (b == 2 && (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000) == 13)vdp = vd * 0.9;
				if (dp[xx][yy] > dp[x][y] + vdp) {
					d[xx][yy] = d[x][y] + vd;
					dp[xx][yy] = dp[x][y] + vdp;
					xyf[xx*NNN+yy] = x*NNN+y;
					if (!v[xx][yy])xq[++r] = xx, yq[r] = yy, v[xx][yy] = 1;
				}
			}

				xx += 2;
				if (emap[x][yy] > framet && emap[xx][y] > framet && (d[x][y] + vd) * 1000 < myvelocity *((long long)emap[xx][yy]-framet)) {
					if (b == 1 && (kk = (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000)) > 1 && kk < 5)vdp = vd * 0.8;
					else {
						if (b == 2 && (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000) == 13)vdp = vd * 0.9;
						else vdp = vd;
					}
					if (dp[xx][yy] > dp[x][y] + vdp) {
						d[xx][yy] = d[x][y] + vd;
						dp[xx][yy] = dp[x][y] + vdp;
						xyf[xx * NNN + yy] = x * NNN + y;
						if (!v[xx][yy])xq[++r] = xx, yq[r] = yy, v[xx][yy] = 1;
					}
				}

				yy += 2;
				if (emap[x][yy] > framet && emap[xx][y] > framet && (d[x][y] + vd) * 1000 < myvelocity *((long long)emap[xx][yy]-framet)) {
					if (b == 1 && (kk = (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000)) > 1 && kk < 5)vdp = vd * 0.8;
					else {
						if (b == 2 && (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000) == 13)vdp = vd * 0.9;
						else vdp = vd;
					}
					if (dp[xx][yy] > dp[x][y] + vdp) {
						d[xx][yy] = d[x][y] + vd;
						dp[xx][yy] = dp[x][y] + vdp;
						xyf[xx * NNN + yy] = x * NNN + y;
						if (!v[xx][yy])xq[++r] = xx, yq[r] = yy, v[xx][yy] = 1;
					}
				}

				xx -= 2;
				if (emap[x][yy] > framet && emap[xx][y] > framet && (d[x][y] + vd) * 1000 < myvelocity *((long long)emap[xx][yy]-framet)) {
					if (b == 1 && (kk = (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000)) > 1 && kk < 5)vdp = vd * 0.8;
					else {
						if (b == 2 && (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000) == 13)vdp = vd * 0.9;
						else vdp = vd;
					}
					if (dp[xx][yy] > dp[x][y] + vdp) {
						d[xx][yy] = d[x][y] + vd;
						dp[xx][yy] = dp[x][y] + vdp;
						xyf[xx * NNN + yy] = x * NNN + y;
						if (!v[xx][yy])xq[++r] = xx, yq[r] = yy, v[xx][yy] = 1;
					}
				}

				vd = leng;
				--yy;
				if ((d[x][y] + vd) * 1000 < myvelocity *((long long)emap[xx][yy]-framet)) {
					if (b == 1 && (kk = (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000)) > 1 && kk < 5)vdp = vd * 0.8;
					else {
						if (b == 2 && (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000) == 13)vdp = vd * 0.9;
						else vdp = vd;
					}
					if (dp[xx][yy] > dp[x][y] + vdp) {
						d[xx][yy] = d[x][y] + vd;
						dp[xx][yy] = dp[x][y] + vdp;
						xyf[xx * NNN + yy] = x * NNN + y;
						if (!v[xx][yy])xq[++r] = xx, yq[r] = yy, v[xx][yy] = 1;
					}
				}
				
				xx += 2;
				if ((d[x][y] + vd) * 1000 < myvelocity *((long long)emap[xx][yy]-framet)) {
					if (b == 1 && (kk = (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000)) > 1 && kk < 5)vdp = vd * 0.8;
					else {
						if (b == 2 && (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000) == 13)vdp = vd * 0.9;
						else vdp = vd;
					}
					if (dp[xx][yy] > dp[x][y] + vdp) {
						d[xx][yy] = d[x][y] + vd;
						dp[xx][yy] = dp[x][y] + vdp;
						xyf[xx * NNN + yy] = x * NNN + y;
						if (!v[xx][yy])xq[++r] = xx, yq[r] = yy, v[xx][yy] = 1;
					}
				}

				--xx; --yy;
				if ((d[x][y] + vd) * 1000 < myvelocity *((long long)emap[xx][yy]-framet)) {
					if (b == 1 && (kk = (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000)) > 1 && kk < 5)vdp = vd * 0.8;
					else {
						if (b == 2 && (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000) == 13)vdp = vd * 0.9;
						else vdp = vd;
					}
					if (dp[xx][yy] > dp[x][y] + vdp) {
						d[xx][yy] = d[x][y] + vd;
						dp[xx][yy] = dp[x][y] + vdp;
						xyf[xx * NNN + yy] = x * NNN + y;
						if (!v[xx][yy])xq[++r] = xx, yq[r] = yy, v[xx][yy] = 1;
					}
				}
				
				yy += 2;
				if ((d[x][y] + vd) * 1000 < myvelocity *((long long)emap[xx][yy]-framet)) {
					if (b == 1 && (kk = (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000)) > 1 && kk < 5)vdp = vd * 0.8;
					else {
						if (b == 2 && (int)api.GetPlaceType(xx * leng / 1000, yy * leng / 1000) == 13)vdp = vd * 0.9;
						else vdp = vd;
					}
					if (dp[xx][yy] > dp[x][y] + vdp) {
						d[xx][yy] = d[x][y] + vd;
						dp[xx][yy] = dp[x][y] + vdp;
						xyf[xx * NNN + yy] = x * NNN + y;
						if (!v[xx][yy])xq[++r] = xx, yq[r] = yy, v[xx][yy] = 1;
					}
				}
	}
}

inline void makewall(int x, int y) {
	int i, j;
	if (x>5) {
		i = x - 5;
		for (j = y - 2 - ((y > 8)*3); j < y + 6; ++j)
			emap[i][j] = min(emap[i][j], emin + 3);
		++i;
		for (j = y-2 - ((y > 8 ) << 1); j < y + 5; ++j)
			emap[i][j] = min(emap[i][j],emin + 2);
		++i;
		for (j = y-2 - ((y > 8 )); j < y + 4; ++j)
			emap[i][j] = min(emap[i][j], emin + 1);
	}

	if (x < 49000/leng) {
		i = x + 5;
		for (j = y - 2 - ((y > 8)*3 ); j < y + 6; ++j)
			emap[i][j] = min(emap[i][j], emin + 3);
		--i;
		for (j = y - 2 - ((y > 8) << 1); j < y + 5; ++j)
			emap[i][j] = min(emap[i][j], emin + 2);
		--i;
		for (j = y - 2 - ((y >8 )); j < y + 4; ++j)
			emap[i][j] = min(emap[i][j], emin + 1);
	}

	if (y>5) {
		j = y - 5;
		for (i = x - 2 - ((x > 8)<<1); i < x + 5; ++i)
			emap[i][j] = min(emap[i][j], emin + 3);
		++j;
		for (i = x -2 - (x > 8); i < x + 4; ++i)
			emap[i][j] = min(emap[i][j], emin + 2);
		++j;
		for (i = x -2 ; i < x + 3; ++i)
			emap[i][j] = min(emap[i][j], emin + 1);
	}

	if (y<49000/leng) {
		j = y + 5;
		for (i = x - 2 - ((x > 8) << 1); i < x + 5; ++i)
			emap[i][j] = min(emap[i][j], emin + 3);
		--j;
		for (i = x -2 - (x >8 ); i < x + 4; ++i)
			emap[i][j] = min(emap[i][j], emin + 2);
		--j;
		for (i = x -2 ; i < x + 3; ++i)
			emap[i][j] = min(emap[i][j], emin + 1);
	}

	for (i = x-2 ; i < x + 3 ; ++i)
	for (j = y-2 ; j < y + 3 ; ++j)
		emap[i][j] = emin ;
	return;
}

inline int collect(int32_t x, int32_t y) {
	int x0 = (x /1000) *(1000/leng),y0 = (y / 1000) * (1000 / leng);
	int minx = x0, miny = y0;
	int i;

	for (i = 1; i < 5; ++i)
		if (d[x0 + i][y0] < d[minx][miny])minx = x0 + i, miny = y0;

	for (i = 0; i < 5; ++i)
		if (d[x0 + i][y0 + 4] < d[minx][miny])minx = x0 + i, miny = y0 + 4;

	for (i = 1; i < 4; ++i)
		if (d[x0][y0 + i] < d[minx][miny])minx = x0, miny = y0 + i;

	for (i = 1; i < 4; ++i)
		if (d[x0 + 4][y0 + i] < d[minx][miny])minx = x0 + 4, miny = y0 + i;
	return(minx * NNN + miny);
}
inline int collectx(int32_t x, int32_t y) {
	int x0 = (x  / leng) - 6, y0 = (y  / leng) - 6;
	int minx = x0 , miny = y0 , i;

	for (i = 1; i < 13; ++i)
		if (d[x0 + i][y0] < d[minx][miny])minx = x0 + i, miny = y0;

	for (i = 0; i < 13; ++i)
		if (d[x0 + i][y0 + 12] < d[minx][miny])minx = x0 + i, miny = y0 + 12;

	for (i = 1; i < 12; ++i)
		if (d[x0][y0 + i] < d[minx][miny])minx = x0, miny = y0 + i;

	for (i = 1; i < 12; ++i)
		if (d[x0 + 12][y0 + i] < d[minx][miny])minx = x0 + 12, miny = y0 + i;
	return(minx * NNN + miny);
}

inline void attack(int32_t x, int32_t y, IAPI& api) {
	//If no SJs are available
	if (sjnum) api.Attack(atan2((double)(y - self->y), (double)(x - self->x))), --sjnum;
	return;
}
inline bool mov(int x, int y, IAPI& api) {

	double t0 = sqrt((y * leng + (leng >>1) - self->y) * (y * leng + (leng >> 1) - self->y) + (x * leng + (leng >> 1) - self->x) * (x * leng + (leng >> 1) - self->x)) / self->speed;
	bool movesuc = api.MovePlayer((int)(t0 * 1000), atan2((double)(y * leng + leng / 2 - self->y), (double)(x * leng + leng / 2 - self->x)));
	if (!movesuc) { 
	while (sjnum)attack(x * leng + (leng >> 1), y * leng + (leng >> 1),api);
	//api.MovePlayer((int)(t0 * 1000), -atan2((double)(y * leng + leng / 2 - self->y), (double)(x * leng + leng / 2 - self->x))); 
	 }
	return movesuc;
}
std::string str = "";
inline bool fri(std::shared_ptr<const THUAI5::Prop> pr, string str) {
	//every char in str contains a guid;
	//we need to judge whether pr is in str.
	char goal = (char)pr->guid;
	int t = str.length();
	for (int i = 0; i < t; ++i)if (goal == str[i])return true;
	return false;
}

inline void warning(int32_t x, int32_t y, int s) {
	int i, j;
	for (i = 0; i <= s / leng + 1; ++i)
	{
		if (x / leng + i <= NN)
		{
			if (emap[x / leng + i][y / leng] > -1000 * leng * (s / leng + 2 - i) / myvelocity)
				emap[x / leng + i][y / leng] = -1000 * leng * (s / leng + 2 - i) / myvelocity;
			for (j = 0; sqrt((i * leng) * (i * leng) + (j * leng) * (j * leng)) <= s + leng * 1.415; ++j)
			{
				if (y / leng + j <= NN && emap[x / leng + i][y / leng + j] > -(j < i ? (leng * (s / leng + 2 - i)) : (leng * (s / leng + 2 - j))) * 1000 / myvelocity)
					emap[x / leng + i][y / leng + j] = -(j < i ? (leng * (s / leng + 2 - i)) : (leng * (s / leng + 2 - j))) * 1000 / myvelocity;
				if (y / leng - j > 0 && emap[x / leng + i][y / leng - j] > -(j < i ? (leng * (s / leng + 2 - i)) : (leng * (s / leng + 2 - j))) * 1000 / myvelocity)
					emap[x / leng + i][y / leng - j] = -(j < i ? (leng * (s / leng + 2 - i)) : (leng * (s / leng + 2 - j))) * 1000 / myvelocity;
			}
		}
		if (x / leng - i > 0)
		{
			if (emap[x / leng - i][y / leng] > -1000 * leng * (s / leng + 2 - i) / myvelocity)
				emap[x / leng - i][y / leng] = -1000 * leng * (s / leng + 2 - i) / myvelocity;
			for (j = 0; sqrt((i * leng) * (i * leng) + (j * leng) * (j * leng)) <= s + leng * 1.415; ++j)
			{
				if (y / leng + j <= NN && emap[x / leng - i][y / leng + j] > -(j < i ? (leng * (s / leng + 1 - i)) : (leng * (s / leng + 1 - j))) / myvelocity)
					emap[x / leng - i][y / leng + j] = -(j < i ? (leng * (s / leng + 1 - i)) : (leng * (s / leng + 1 - j))) * 1000 / myvelocity;
				if (y / leng - j > 0 && emap[x / leng - i][y / leng - j] > -(j < i ? (leng * (s / leng + 1 - i)) : (leng * (s / leng + 1 - j))) / myvelocity)
					emap[x / leng - i][y / leng - j] = -(j < i ? (leng * (s / leng + 1 - i)) : (leng * (s / leng + 1 - j))) * 1000 / myvelocity;
			}
		}
	}
	return;
}

inline bool throwCPU(IAPI& api) {
	if (!self->cpuNum||(testime * 60000 - nowt -(hzt<<1) <= 30000))return 0;
	else if ((self->x - xbirth) * (self->x - xbirth) + (self->y - ybirth) * (self->y - ybirth) > 225000000)return 1;
	else {
		if (abs(self->x - xbirth) > abs(self->y - ybirth)) {
			double k = (double)(self->y - ybirth) / (self->x - xbirth) , d = 500 * sqrt(1 + k * k);
			int fx = (xbirth > self->x) ? 1 : -1;
			int x0 = self->x / 1000, x1 = xbirth / 1000;
			if (fx > 0)++x0;
			else ++x1;
			double yj = self->y + k * (x0*1000 - self->x);
			int i, n = abs(x0 - x1) + 1, y1;
			for (i = 0; i < n; ++i) {
				y1 = ((int)yj) / 1000 ;
				if ((yj - y1 * 1000 <= d) && ((int)api.GetPlaceType(x0 + i * fx , y1)  == 1 || (int)api.GetPlaceType(x0 + i * fx - 1 , y1) == 1 || (int)api.GetPlaceType(x0 + i * fx , y1 - 1) == 1 || (int)api.GetPlaceType(x0 + i * fx - 1, y1 - 1) == 1)) return 1;
				++y1;
				if ((y1 * 1000 -  yj<= d) && ((int)api.GetPlaceType(x0 + i * fx, y1) == 1 || (int)api.GetPlaceType(x0 + i * fx - 1, y1) == 1 || (int)api.GetPlaceType(x0 + i * fx, y1 - 1) == 1 || (int)api.GetPlaceType(x0 + i * fx - 1, y1 - 1) == 1)) return 1;
				yj += 1000 *  k * fx;
			}
			api.ThrowCPU((int)(sqrt((self->x - xbirth) * (self->x - xbirth) + (self->y - ybirth) * (self->y - ybirth)) / 3), (fx > 0) ? atan2(k, 1) : (atan2(k, 1) + PI), self->cpuNum);
			return 0;
		}
		else {
			double k = (double)(self->x - xbirth) / (self->y - ybirth), d = 500 * sqrt(1 + k * k);
			int fy = (ybirth > self->y) ? 1 : -1;
			int y0 = self->y / 1000 , y1 = ybirth / 1000;
			if (fy > 0)++y0;
			else ++y1;
			double xj = self->x + k * (y0*1000 - self->y);
			int i, n = abs(y0 - y1) + 1, x1;
			for (i = 0; i < n; ++i) {
				x1 = ((int)xj) / 1000 ;
				if ((xj - x1*1000 <= d)&& ((int)api.GetPlaceType(y0 + i * fy , x1) == 1 || (int)api.GetPlaceType(y0 + i * fy - 1 , x1) == 1 || (int)api.GetPlaceType(y0 + i * fy, x1 -1) == 1 || (int)api.GetPlaceType(y0 + i * fy - 1 , x1 - 1) == 1)) return 1;
				++x1;
				if ((x1 * 1000 - xj <= d) && ((int)api.GetPlaceType(y0 + i * fy, x1) == 1 || (int)api.GetPlaceType(y0 + i * fy - 1, x1) == 1 || (int)api.GetPlaceType(y0 + i * fy, x1 - 1) == 1 || (int)api.GetPlaceType(y0 + i * fy - 1, x1 - 1) == 1)) return 1;
				xj += 1000 * k * fy;
			}	
			api.ThrowCPU((int)(sqrt((self->x - xbirth) * (self->x - xbirth) + (self->y - ybirth) * (self->y - ybirth)) / 3), ((fy > 0)?(PI / 2): 3*PI / 2) - atan2(k, 1), self->cpuNum);
			return 0;
		}
	}
}
int fx, fy;
void AI::play(IAPI& api)
{
	std::ios::sync_with_stdio(false);
	self = api.GetSelfInfo();
	std::optional<string> ufo = api.TryGetMessage();
	if (self->playerID && ufo != nullopt)str.assign(*(ufo)); else str = "";
	if (self->isResetting) {
		if(self->playerID!=3)api.Send(self->playerID + 1, str);
		return;
	}

	sjnum = self->signalJammerNum;
	if (!nowt) {
		//forchange
		if (self->teamID) {
			store = 2;
            if ((int)api.GetPlaceType(44, 28) == 11)xbirth = 44500, ybirth = 28500;
			else xbirth = 46500, ybirth = 43500;
		}
		else if ((int)api.GetPlaceType(6, 3) == 5)xbirth = 6500, ybirth = 3500;
		nowt = 1; startime = nowtime(); //, buildmap()
		if (self->playerID == 1)xgt = 22500;
		else if (self->playerID == 2)xgt = 18500, ygt = 26500;
		else if (self->playerID == 3)xgt = 17500, ygt = 25000;
	}
	else nowt = ftime();

	locate();
	memset(buff, 0, sizeof(buff));
	for (int i = 0; i < 8; ++i)buff[i][1] = 1;
	buf(self);
	myv = (myvelocity * buff[self->playerID+ (self->teamID <<2)][1])/1000;

	memset(emap,0x3f, sizeof(emap));
	int k;
	for (int i = 0; i < 50; ++i)
		for (int j = 0; j < 50; ++j)
			if ((k = (int)(api.GetPlaceType(i, j))) == 1 || (k > 4 && k < 13 && k != 5 + self->playerID + ((self->teamID) << 2) ))
				makewall(i * 1000 / leng + 2, j * 1000 / leng + 2);

	std::vector<std::shared_ptr<const THUAI5::SignalJammer>>  sjv = api.GetSignalJammers();
	for (auto iv = sjv.begin(); iv != sjv.end(); ++iv) if ((*iv)->parentTeamID != self->teamID) {
		if (!lcy[(*iv)->guid])lcy[(*iv)->guid]=nowt;
		danger(*iv, api);
	}

	int flagt = 0,minet=10031207,bttack=0;
	std::vector<std::shared_ptr<const THUAI5::Robot>> rv = api.GetRobots();
	std::shared_ptr<const THUAI5::Robot> att=nullptr;
	for (auto iv = rv.begin(); iv != rv.end(); ++iv)  {
			std::shared_ptr<const THUAI5::Robot> pr = *iv;
			if (pr->isResetting)continue;
		if (pr->teamID != self->teamID) {
			buf(pr);
			int k = d[pr->x/leng][pr->y/leng]-1000,//fortest
				prsjv = (pr->signalJammerNum|| (pr->CD)<=framet) ?
				((pr->timeUntilCommonSkillAvailable || ((int)(pr->softwareType) != 3))
					? jsspeed[((int)(pr->hardwareType) == 3) ? 1 : (int)(pr->hardwareType) + 1] : jsspeed[4]
					) : 0,
				sjr= (pr->signalJammerNum || (pr->CD) <= framet) ?
				((pr->timeUntilCommonSkillAvailable || ((int)(pr->softwareType) != 3))
					? explosion_radius[((int)(pr->hardwareType) == 3) ? 1 : (int)(pr->hardwareType) + 1] : explosion_radius[4]
					) : 0,
				gv = robotspeed[(int)pr->softwareType] * buff[pr->playerID + ((pr->teamID) << 2)][1] ,
				t =k* 1000 /myvelocity;
		//	attackt[pr->playerID + ((pr->teamID) << 2)] = t;
			if ((dist(pr->x, pr->y) - 1000)*1000 <= (framet * jsspeed[(int)self->signalJammerType])) {
				api.UseProp(); api.UseCommonSkill();
				while (sjnum)  attack(pr->x, pr->y, api);

			}else if ((!buff[pr->playerID + ((pr->teamID) << 2)][0]) && t < minet && sjnum * 2500 >= pr->life
				&& pr->timeUntilCommonSkillAvailable - 24000 < 0 && nowt != 1 &&
				(self->timeUntilCommonSkillAvailable == 0 || (self->timeUntilCommonSkillAvailable - 24000 > t) || (buff[self->playerID + ((self->teamID) << 2)][3] && !buff[pr->playerID + ((pr->teamID) << 2)][4]))
				) {
				minet = t, att = pr;
			}

			 if ((k - sjr) * 1000 / (gv + prsjv) <= (hzt<<1)) {
				    ++bttack;
					if(sjnum>2)attack(pr->x, pr->y, api);
				    api.UseProp(); api.UseCommonSkill();
			}
			 makewall(pr->x / leng, pr->y / leng);
		}else if(pr->playerID!=self->playerID)makewall(pr->x / leng, pr->y / leng);
	}


    if(nowt != 1&&bttack!=1)
	for (auto iv = rv.begin(); iv != rv.end(); ++iv) {
		std::shared_ptr<const THUAI5::Robot> pr = *iv;
		if (pr->isResetting)continue;
		if (pr->teamID != self->teamID) {
			int k = d[pr->x / leng][pr->y / leng] - 1000,//fortest
				prsjv = (pr->signalJammerNum || (pr->CD) <= framet) ?
				((pr->timeUntilCommonSkillAvailable || ((int)(pr->softwareType) != 3))
					? jsspeed[((int)(pr->hardwareType) == 3) ? 1 : (int)(pr->hardwareType) + 1] : jsspeed[4]
					) : 0,
				sjr = (pr->signalJammerNum || (pr->CD) <= framet) ?
				((pr->timeUntilCommonSkillAvailable || ((int)(pr->softwareType) != 3))
					? explosion_radius[((int)(pr->hardwareType) == 3) ? 1 : (int)(pr->hardwareType) + 1] : explosion_radius[4]
					) : 0,
				gv = robotspeed[(int)pr->softwareType] * buff[pr->playerID + ((pr->teamID) << 2)][1];

					warning(pr->x, pr->y, ((gv + prsjv) * (framet>>1)/1000) + sjr);
			}
		}
	
	
	int dm = 10031207;
	int got, gt,gx,gy;
	SPFA(api);
	if (minet <= 3000) {
		
			if (bttack == 1) {
		        got = collectx(att->x, att->y), gy = got % NNN, gx = got / NNN;
				if (d[gx][gy] - 1000 <= 1000 * myvelocity) {
					api.UseProp();
					api.UseCommonSkill();
					bttack = 1;
				}
				else bttack = 0;
			}
	}

	std::vector<std::shared_ptr<const THUAI5::Prop>>  pv = api.GetProps();
	std::shared_ptr<const THUAI5::Prop> gtv = nullptr;
	bool cpnum=throwCPU(api);
	
	for (auto iv = pv.begin(); iv != pv.end(); ++iv) {
		std::shared_ptr<const THUAI5::Prop> pr = *iv;
   //     printf("bronya!\n");
		if (!pr->isMoving) {
			if (cget(pr->x, pr->y, api)) {

				//printf("homura!\n");
				if ((int)self->prop && pr->type != PropType::CPU) api.UseProp();
				if (((int)api.GetPlaceType(self->x / 1000, self->y / 1000) != 5 + store + ((self->teamID) << 2))
					||(testime * 60000 - nowt - hzt <= 30000 && !fpick))
					api.Pick(pr->type);
			}
			else {
				if (bttack == 1)continue;
				if ((int)api.GetPlaceType(pr->x / 1000, pr->y / 1000) == 5 + store + ((self->teamID) << 2))continue;
				if ((int)pv.size() <= (self->playerID) || !fri(pr, str)) {//fortest

					gt = collect(pr->x, pr->y), gy = gt % NNN, gx = gt/ NNN;

					//	printf("kiana!\n");
					if (emap[gx][gy] < 0)continue;
					if (d[gx][gy] < hzt * myv) {//fortest
						if (flagt) {
							if(dm>d[gx][gy])
							got = gt;
							gtv = pr;
							dm = d[gx][gy];
						}
						else {
							got = gt;
							gtv = pr;
							dm = d[gx][gy];
							flagt = 1;
						}
					}
					//				printf("mei!\n");
					if (!flagt&&!cpnum) {
						if (pr->type == PropType::CPU) {
							if (d[gx][gy] < dm * sqrt(pr->size) * 1.2) {//fortest
								dm = d[gx][gy] / sqrt(pr->size);
								got = gt;
								gtv = pr;
							}
						}
						else  if (d[gx][gy] < dm) {
							dm = d[gx][gy] * 1.2;
							got = gt;
							gtv = pr;
						}
					}
				}
			}
		}
	}
	//printf("honkai!\n");
	if (emap[Xnow][Ynow] >= hzt *3) {
		if (self->playerID == store) {
			if (!fpick) {
			    int gst = collect(xbirth, ybirth);
				if( testime * 60000 - nowt - d[(gst / NNN)][gst % NNN]/ myvelocity - hzt <= 30000) {
				got = gst;
				dm = d[gst / NNN][gst % NNN];
				gtv = nullptr;
				//	printf("%d %d %d %d %d\n",(int)api.GetPlaceType(self->x/1000,self->y/1000), self->x, self->y, gst / NNN, gst % NNN);
			}
			    else if (cpnum && dm == 10031207) {
				got = gst;
				dm = d[gst / NNN][gst % NNN];
			    }
			}
		}
		else if(cpnum && dm == 10031207 && testime * 60000 - nowt -(hzt<<1)> 30000) {
			got = collectx(xbirth, ybirth);
			dm = d[got / NNN][got % NNN];
		}
   }
	
	if (self->playerID == store) {
		if (testime * 60000 - nowt - hzt <= 30000&&!fpick&&
			(int)api.GetPlaceType(self->x / 1000, self->y / 1000) == 5 + store + ((self->teamID) << 2))
			fpick=1,  api.UseCPU(self->cpuNum);
	}
	if (testime * 60 * 1000 - nowt <= hzt)  api.UseCPU(self->cpuNum);

	//挡住CPU
	if (dm >= 10031207) got = (xgt / leng)*NNN+ (ygt / leng), dm = d[xgt / leng][ygt / leng];
	if (emap[Xnow][Ynow] <= (hzt*3) || dm>= 10031207) {//fortest
		int xk=(25000/leng>Xnow)?1:-1, yk= (25000 / leng > Ynow) ? 1 : -1,
			 k= emap[Xnow+xk][Ynow +yk], x = Xnow + xk, y= Ynow + yk, i = x, j = y -yk;

		if (emap[i][j] > k)k = emap[i][j], x = i, y = j;
		j-=yk; if (emap[i][j] > k)k = emap[i][j], x = i, y = j;
		i-=xk; j = y; if (emap[i][j] > k)k = emap[i][j], x = i, y = j;
		j -= yk<<1; if (emap[i][j] > k)k = emap[i][j], x = i, y = j;
		i -= xk; j = y; if (emap[i][j] > k)k = emap[i][j], x = i, y = j;
		j -= yk; if (emap[i][j] > k)k = emap[i][j], x = i, y = j;
		j -= yk; if (emap[i][j] > k)k = emap[i][j], x = i, y = j;
		if (emap[Xnow][Ynow] > k)k = emap[Xnow][Ynow], x = Xnow, y = Ynow;

		if (self->playerID != 3)	api.Send(self->playerID + 1, str);
		mov(x, y, api);
		return;
	}

    int gxy = got ;
//	printf("Madoka!\n");
    while (xyf[gxy] != Xnow*NNN+Ynow)
		gxy = xyf[gxy];

	//if (dm < 2000*myv&&self->timeUntilCommonSkillAvailable==0)api.UseCommonSkill();
//	printf("kiana!");
	mov(gxy/NNN, gxy%NNN, api);//fortest
	if(gtv!=nullptr)str.append(1, (char)gtv->guid);
	if (self->playerID != 3)	api.Send(self->playerID + 1, str);
}