#ifndef _USI_H_
#define _USI_H_

#include <assert.h>
#include <atomic>
#include <map>
#include <string>

#include "Types.h"
#include "Thread.h"

struct OptionsMap;

class Option {
    typedef void (Fn)(const Option&);

public:
    Option(Fn* = nullptr);

    // bool型用
    Option(bool v, Fn* = nullptr);

    // string型用  
    Option(const char* v, Fn* = nullptr);

    // combobox用
    Option(const std::vector<std::string> combo, const std::string& v, Fn* f = nullptr);

    // int型用
    Option(int v, int min_, int max_, Fn* = nullptr);
 
    Option& operator = (const std::string& v);

    // Option -> int
    operator int() const {
        assert(type_ == "check" || type_ == "spin");
        return (type_ == "spin" ? stoi(current_value_) : current_value_ == "true");
    }

    // Option -> string
    operator std::string() const { assert(type_ == "string" || type_ == "combo"); return current_value_; }

private:
    friend std::ostream& operator<<(std::ostream&, const OptionsMap&);
    std::string default_value_, current_value_, type_;
    std::vector<std::string> combo_;
    int min_, max_;
    Fn* on_change_;
};

// OptionsMap 以ASCII code排序
struct CaseInsensitiveLess {
    bool operator() (const std::string& s1, const std::string& s2) const     {
        // lexicographical_compare 順序比較
        return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(),
            [](char c1, char c2) { return tolower(c1) < tolower(c2); });
    }
};

struct OptionsMap : public std::map<std::string, Option, CaseInsensitiveLess> {
public:
    void Initialize();
    bool IsLegalOption(const std::string name) { return this->find(name) != std::end(*this); }
};

struct LimitsType {
    LimitsType() { std::memset(this, 0, sizeof(LimitsType)); }
    inline bool IsTimeManagement() const { return !(move_time | depth | nodes | infinite); }

    std::vector<Move> search_moves;

    // time 残り時間(この1局について) [ms]
    int time[2];

    // 増加時間
    int inc[2];

    // 讀秒(ms換算で)
    int byoyomi;

    // 固定深度搜尋之深度(0為不固定)
    int depth;

    // 固定思考時間之時間(0為無限制) 單位:ms
    int move_time;

    // 思考時間限制 flag(true為無限制)
    bool infinite;

    // ponder flag
    Key ponder;

	// 搜索根盤面 hash key
	Key rootKey;

    // 固定node數搜尋之數量
    uint64_t nodes;

    // go開始時間
    TimePoint start_time;
};

class Minishogi;

namespace USI {
    extern OptionsMap Options;
    extern LimitsType Limits;

    void loop(int argc, char** argv);
	void position(Minishogi& pos, std::istringstream& ss_cmd);
	void go(const Minishogi& pos, std::istringstream& ss_cmd);
	void timetest(std::istringstream& ss_cmd);
	void perft(Minishogi &pos, std::istringstream& ss_cmd);
	void make_opening(std::istringstream& ss_cmd);
	void setoption(std::istringstream& ss_cmd);

    std::string value(Value s);
    std::string pv(const RootMove &rm, const Thread &th, Value alpha, Value beta);
	std::string move_list(ExtMove *begin, ExtMove *end, Minishogi &pos);
}

std::ostream& operator<<(std::ostream& os, const OptionsMap& om);

#endif