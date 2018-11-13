#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <math.h>
#include <filesystem>

#include "usi.h"
#include "Evaluate.h"
#include "Observer.h"
using namespace std;
using namespace Evaluate;
namespace fs = std::experimental::filesystem;

vector<string> GetEvalFilesPath() {
    vector<string> filenames;

	for (auto &p : fs::directory_iterator(KPPT_DIRPATH))
		if (fs::is_directory(p))
			filenames.push_back(KPPT_DIRPATH + fs::path(p).filename().string());

	filenames.push_back("none");

    return filenames;
}

string GetEvalConfig(vector<string> filenames) {
	string confPath = KPPT_DIRPATH + "config.txt";
    ifstream ifs(confPath);

    if (!ifs) {
        cout << "info string " << confPath << " not found." << endl;
		filenames.front();
    }

    string str;
    getline(ifs, str);
    string kkptPath = KPPT_DIRPATH + str;

    if (find(filenames.begin(), filenames.end(), kkptPath) != filenames.end())
        return kkptPath;
    else
        return filenames.front();
}

void OptionsMap::Initialize() {
	vector<string> filenames = GetEvalFilesPath();

    //(*this)["Hash"]            = Option(64, 1, 65536, [](const Option& opt) {  });
	(*this)["HashEntry"]       = Option(27, 1, 32);
	(*this)["Depth"]           = Option(10, 1, 16);
    (*this)["USI_Ponder"]      = Option(true);
	(*this)["FullMovePonder"]  = Option(false);
    (*this)["NetworkDelay"]    = Option(0, 0, 60000);
    (*this)["ResignValue"]     = Option(-VALUE_MATE, -VALUE_MATE, VALUE_MATE);
	(*this)["EvalDir"]         = Option(filenames, GetEvalConfig(filenames), [](const Option& opt) { Evaluate::GlobalEvaluater.Load(opt); });
	(*this)["EvalStandardize"] = Option(true);
	(*this)["MaxCheckPly"]     = Option(16, 0, 1024);
}

ostream& operator<<(ostream& os, const OptionsMap& om) {
    for (auto it = om.begin(); it != om.end(); ++it) {
        const Option& o = it->second;

#ifdef BACKGROUND_SEARCH_DISABLE
		if (it->first == "USI_Ponder")
			continue;
#endif

        os << "\noption name " << it->first << " type " << o.type_;

        if (o.type_ != "button")
            os << " default " << o.default_value_;

        if (o.type_ == "combo")
            for (auto c : o.combo_)
                os << " var " << c;

        if (o.type_ == "spin")
            os << " min " << o.min_ << " max " << o.max_;
    }
    return os;
}

Option::Option(Fn* f) : type_("button"), min_(0), max_(0), on_change_(f) {}

Option::Option(const char* v, Fn* f) : type_("string"), min_(0), max_(0), on_change_(f) {
    default_value_ = current_value_ = v;
}

Option::Option(const vector<string> combo, const string& v, Fn* f) : type_("combo"), min_(0), max_(0), on_change_(f), combo_(combo) {
    default_value_ = current_value_ = v;
}

Option::Option(bool v, Fn* f) : type_("check"), min_(0), max_(0), on_change_(f) {
    default_value_ = current_value_ = (v ? "true" : "false");
}

Option::Option(int v, int minv, int maxv, Fn* f) : type_("spin"), min_(minv), max_(maxv), on_change_(f) {
    default_value_ = current_value_ = to_string(v);
}

Option& Option::operator=(const string& v) { 
    assert(!type_.empty());

    if ((type_ != "button" && v.empty()) ||
        (type_ == "check" && v != "true" && v != "false") ||
        (type_ == "spin" && (stoi(v) < min_ || stoi(v) > max_)))
        return *this;

    if (type_ != "button")
        current_value_ = v;

    if (on_change_ != nullptr)
        (*on_change_)(*this);

    return *this;
}
