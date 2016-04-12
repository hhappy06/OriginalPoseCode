#ifndef MYDEFINE_H
#define MYDEFINE_H

#define NUI_IMAGE_PLAYER_INDEX_SHIFT 3
#define NUI_IMAGE_PLAYER_INDEX_MASK  7
#define OUT_COLS     160
#define OUT_ROWS     120
#define DEPTH_COLS   320
#define DEPTH_ROWS   240
//#define TREECOUNT    3
//#define TREENODECNT  1023
#define CLASSCOUNT   31
#define DepthUnusual 8000.0f
#define DEPTH_FACTOR 2048.0f

#define TREECOUNT  3
#define TREE_DEPTH  18
#define TREENODECNT  ((1<<(TREE_DEPTH+1)) - 1)
#define VALUE_COUNT  835114
const int FOREST_DATA_SIZE = 0x01f2af4a;




#endif