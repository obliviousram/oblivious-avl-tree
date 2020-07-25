//
//
//

#ifndef _OAVLTREEINTERFACE_H
#define _OAVLTREEINTERFACE_H
#include "Block.h"
#include <vector>

class OAVLTreeInterface {
public:
    struct Root
    {
        int id = -1;
        int leaf_id = -1;
    };
    
    virtual void insert(int key, int val) = 0;
    virtual int search(int key) = 0;

    virtual int P(int leaf, int level) = 0;
    
    virtual vector<Block> getStash() = 0;
    virtual int getStashSize() = 0;
    virtual int getNumLeaves() = 0;
    virtual int getNumLevels() = 0;
    virtual int getNumBlocks() = 0;
    virtual int getNumBuckets() = 0;
    virtual void dumpStash() = 0;
};


#endif //_OAVLTREEINTERFACE_H
