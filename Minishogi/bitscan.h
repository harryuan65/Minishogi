#pragma once
#include"define.h"
#ifndef _BITSCAN_
#define _BITSCAN_

static const int index32[] = {
    0, 14,  1, 15,  9, 18,  2, 12,
    16, 10, 26, 28, 19, 22,  3, 30,
    13,  8, 17, 11, 25, 27, 21, 29,
    7, 24, 20,  6, 23,  5,  4, 31
};

static const U32 debrujin32 = U32(0x7ca26eb);

int BitScan(U32 board); // LSB
int BitScanRev(U32 board); // MSB;#pragma once

#endif // !_BITSCAN_
