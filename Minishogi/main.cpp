#include "usi.h"

int main(int argc, char **argv) {
	setlocale(LC_ALL, "");
	USI::Options.Initialize();
	Zobrist::Initialize();
	Evaluate::GlobalEvaluater.Load(USI::Options["EvalDir"]);
	GlobalTT.Resize(1);
	USI::loop(argc, argv);

	return 0;
}