#ifndef _HEAD_
#define _HEAD_
#define _CRT_SECURE_NO_WARNINGS
#define WINDOWS_10 //WINDOWS_7 or WINDOWS_10
//#define PERFECT_ENDGAME_PV

using namespace std;

#include <algorithm>
#include <assert.h>
#include <conio.h>
#include <direct.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <iomanip>
#include <map> //之後不用
#include <stdlib.h>
#include <string>
#include <sstream>
#include <time.h>
#include <vector>
#include <Windows.h>

struct TranspositNode;
struct PV;
class Board;

#include "define.h"
#include "library.h"
#include "Observer.h"
#include "bitscan.h"
#include "board.h"
#include "MoveGene.h"
#include "AI.h"

#endif