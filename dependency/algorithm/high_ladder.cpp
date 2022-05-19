#include <iostream>
#include <algorithm>
#include <cmath>
using namespace std;

template <typename T>
using mypair = pair<T, T>;

double PHI(double x) // THUAI3: Sigmoid; THUAI4: Tanh; THUAI5: Normal Distribution CDF
{
    return erf(x / sqrt(2));
}

// orgScore 是天梯中两队的分数；competitionScore 是这次游戏两队的得分
mypair<int> cal(mypair<int> orgScore, mypair<int> competitionScore)
{

    // 调整顺序，让第一个元素成为获胜者，便于计算

    bool reverse = false;    // 记录是否需要调整

    if (competitionScore.first < competitionScore.second)
    {
        reverse = true;
    }
    else if (competitionScore.first == competitionScore.second)
    {
        if (orgScore.first == orgScore.second)      // 完全平局，不改变天梯分数
        {
            return orgScore;
        }

        if (orgScore.first > orgScore.second)        // 本次游戏平局，但一方天梯分数高，另一方天梯分数低，需要将两者向中间略微靠拢，因此天梯分数低的定为获胜者
        {
            reverse = true;
        }
        else
        {
            reverse = false;
        }
    }

    if (reverse)   // 如果需要换，换两者的顺序
    {
        swap(competitionScore.first, competitionScore.second);
        swap(orgScore.first, orgScore.second);
    }


    // 转成浮点数
    mypair<double> orgScoreLf;
    mypair<double> competitionScoreLf;
    orgScoreLf.first = orgScore.first;
    orgScoreLf.second = orgScore.second;
    competitionScoreLf.first = competitionScore.first;
    competitionScoreLf.second = competitionScore.second;
    mypair<int> resScore;

    const double deltaWeight = 90.0;       // 差距悬殊判断参数，比赛分差超过此值就可以认定为非常悬殊了，天梯分数增量很小，防止大佬虐菜鸡的现象造成两极分化

    double delta = (orgScoreLf.first - orgScoreLf.second) / deltaWeight;
    cout << "Normal CDF delta: " << PHI(delta) << endl;
    {

        const double firstnerGet = 1e-4;           // 胜利者天梯得分权值
        const double secondrGet = 7e-5;            // 失败者天梯得分权值
        const double possibleMaxScore = 1500.0;         // 估计的最大得分数量

        double deltaScore = 100.0;        // 两队竞争分差超过多少时就认为非常大
        double correctRate = (orgScoreLf.first - orgScoreLf.second) / 100.0;       // 订正的幅度，该值越小，则在势均力敌时天梯分数改变越大
        double correct = 0.5 * (PHI((competitionScoreLf.first - competitionScoreLf.second - deltaScore) / deltaScore - correctRate) + 1.0);      // 一场比赛中，在双方势均力敌时，减小天梯分数的改变量

        resScore.first = orgScore.first + round(competitionScoreLf.first * competitionScoreLf.first * firstnerGet * (1 - PHI(delta)) * correct);  // 胜者所加天梯分
        if (competitionScoreLf.second < possibleMaxScore)
            resScore.second = orgScore.second - round((possibleMaxScore - competitionScoreLf.second) * (possibleMaxScore - competitionScoreLf.second) * secondrGet * (1 - PHI(delta)) * correct);  // 败者所扣天梯分
        else
            resScore.second = orgScore.second; // 败者拿maxScore分，已经很强了，不扣分
    }

    // 如果换过，再换回来
    if (reverse)
    {
        swap(resScore.first, resScore.second);
    }

    return resScore;
}

void Print(mypair<int> score)
{
    std::cout << " team1: " << score.first << std::endl
        << "team2: " << score.second << std::endl;
}

int main()
{
    int x, y;
    std::cout << "origin score of team 1 and 2: " << std::endl;
    std::cin >> x >> y;
    auto ori = mypair<int>(x, y);
    std::cout << "game score of team 1 and 2: " << std::endl;
    std::cin >> x >> y;
    auto sco = mypair<int>(x, y);
    Print(cal(ori, sco));
}