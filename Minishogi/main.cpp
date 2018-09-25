#include "usi.h"

int main(int argc, char **argv) {
	setlocale(LC_ALL, "");
	Zobrist::Initialize();
	USI::Options.Initialize();
	USI::loop(argc, argv);

	return 0;
}