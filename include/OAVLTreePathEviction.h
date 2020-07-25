//
//
//

#ifndef _OAVLTREEPATHEVICTION_H
#define _OAVLTREEPATHEVICTION_H
#include "OAVLTreeInterface.h"
#include "RandForOramInterface.h"
#include "UntrustedStorageInterface.h"
#include "IdGenerator.h"
#include "Exceptions.h"
#include <cmath>


class OAVLTreePathEviction : public OAVLTreeInterface {
    public:
    OAVLTreePathEviction(UntrustedStorageInterface* storage,
            RandForOramInterface* rand_gen, int bucket_size, int num_blocks);
            
    int P(int leaf, int level);

    vector<Block> getStash();
    int getStashSize();
    int getNumLeaves();
    int getNumLevels();
    int getNumBlocks();
    int getNumBuckets();
    void dumpStash();

    void insert(int key, int val);
    int search(int key);

    private:
    UntrustedStorageInterface* storage;
    RandForOramInterface* rand_gen;

    int bucket_size;
    int num_levels;
    int num_leaves;
    int num_blocks;
    int num_buckets;

    vector<Block> stash;
    vector<Block> local;
    void evictAfterSearch();
    void evictTwoPaths();
    void updateHeight();
    void evictByLeaf(int leaf);
    IdGenerator* id_gen;
    Root root;
    void moveToLocal(int leaf, int blockid);
    void performDummyFinds(int num);
    void performDummyEvictions(int num);
};


#endif 
