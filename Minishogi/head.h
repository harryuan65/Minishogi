#ifndef _HEAD_
#define _HEAD_
#define _CRT_SECURE_NO_WARNINGS
#define WINDOWS_10
#define PV_DISABLE
//#define BEST_ENDGAME_SEARCH
//#define TRANSPOSITION_DISABLE
using namespace std;

#include <algorithm>
#include <assert.h>
#include <atlstr.h>
#include <conio.h>
#include <direct.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <time.h>
#include <vector>
#include <Windows.h>

struct TranspositNode;
struct PV;
class Board;

#include "Zobrist.h"
#include "define.h"
#include "library.h"
#include "Observer.h"
#include "bitscan.h"
#include "board.h"
#include "MoveGene.h"
#include "AI.h"

#endif