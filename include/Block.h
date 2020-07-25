//
//
//

#ifndef _BLOCK_H
#define _BLOCK_H

#include <algorithm>
using namespace std;

class Block {
    public:
    static const int BLOCK_SIZE = 4;
    int leaf_id = -1;
    int id = -1;
    int data[BLOCK_SIZE]; //0: key, i: value, 2: left height, 3: right height
    int leftChildPos = -1;
    int leftChildId = -1;
    int rightChildPos = -1;
    int rightChildId = -1;
    Block();
    Block(const Block& b);
    Block(int leaf_id, int id, int data[]);
    void printBlock();
    int getBalance();
    virtual ~Block();
};

#endif //_BLOCK_H
