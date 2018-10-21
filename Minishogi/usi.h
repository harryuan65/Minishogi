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

// 大文字小文字を区別せず、アルファベット順の大小比較をする。これはstd::mapのために定義されている。
// これにより、OptionsMapはアルファベット順にオーダリングされることになる。
struct CaseInsensitiveLess {
    bool operator() (const std::string& s1, const std::string& s2) const     {
        // lexicographical_compare はコンテナを比較する
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
    //bool useTimeManagement() const { return !(move_time | depth | nodes | infinite); }

    std::vector<Move> search_moves;

    // time 残り時間(この1局について) [ms]
    int time[2];

    // 増加時間
    int inc[2];

    // 秒読み(ms換算で)
    int byoyomi;

    // 探索深さを固定するとき用(0以外を指定してあるなら)
    int depth;

    // 固定思考時間(0以外が指定してあるなら) : 単位は[ms]
    int move_time;

    // 思考時間無制限かどうかのフラグ。非0なら無制限。
    bool infinite;

    // ponder中であるかのフラグ
    Key ponder;

	// 探索盤面 hash
	Key rootKey;

    // この探索ノード数だけ探索したら探索を打ち切る。
    uint64_t nodes;

    // go開始時刻
    TimePoint start_time;
};

class Minishogi;

namespace USI {
    extern OptionsMap Options;
    extern LimitsType Limits;

    void loop(int argc, char** argv);
	void position(Minishogi& pos, std::istringstream& up);
	void go(const Minishogi& pos, std::istringstream& ss_cmd);
	void timetest(std::istringstream& is);
	void perft(Minishogi &pos, int depth);
	void setoption(std::istringstream& ss_cmd);

    std::string value(Value s);
    std::string pv(const RootMove &rm, const Thread &th, Value alpha, Value beta);
	std::string move_list(ExtMove *begin, ExtMove *end, Minishogi &pos);
}

std::ostream& operator<<(std::ostream& os, const OptionsMap& om);

#endif