#include <iostream>
#include <string>

#include "GamePlay.h"
#include "TimeTest.h"
#include "EvaluateLearn.h"
using namespace std;

int main(int argc, char **argv) {
	setlocale(LC_ALL, "");

	if (argc == 1) {
		string mode;
		cin >> mode;

		if (mode == "gameplay")
			GamePlay::GamePlay(argc, argv);
		else if (mode == "timetest")
			TimeTest::TimeTest();
		else if (mode == "evallearn")
			EvaluateLearn::SelfLearn();
		//else if (mode == "usi")
	}
	else {
		GamePlay::GamePlay(argc, argv);
	}

	cout << "\a";
	system("pause");
	return 0;
}