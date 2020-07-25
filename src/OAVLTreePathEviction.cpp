//
//
//

#include "OAVLTreePathEviction.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>


OAVLTreePathEviction::OAVLTreePathEviction(UntrustedStorageInterface* storage, RandForOramInterface* rand_gen,
                                           int bucket_size, int num_blocks) {
    this->storage = storage;
    this->rand_gen = rand_gen;
    this->bucket_size = bucket_size;
    this->num_blocks = num_blocks;
    this->num_levels = ceil(log10(num_blocks) / log10(2)) + 1;
    this->num_buckets = pow(2, num_levels)-1;
    if (this->num_buckets*this->bucket_size < this->num_blocks) //deal with precision loss
    {
        throw new runtime_error("Not enough space for the acutal number of blocks.");
    }
    this->num_leaves = pow(2, num_levels-1);
    Bucket::resetState();
    Bucket::setMaxSize(bucket_size);
    this->rand_gen->setBound(num_leaves);
    this->storage->setCapacity(num_buckets);
    this->id_gen = new IdGenerator;
    this->stash = vector<Block>();
    

    for(int i = 0; i < num_buckets; i++){

        Bucket init_bkt = Bucket();
        for(int j = 0; j < bucket_size; j++){
            init_bkt.addBlock(Block());
        }
        storage->writeBucket(i, Bucket(init_bkt));
    }

}

void OAVLTreePathEviction::moveToLocal(int leaf, int blockid) {
    bool inOram = false;
    for (int i = 0; i < num_levels; i++) {
        vector<Block> blocks = storage->readBucket(P(leaf, i)).getBlocks();
        Bucket writeBack;
        for (Block &b: blocks) {
            if (b.id == blockid) {
                local.push_back(Block(b));
                inOram = true;
                writeBack.addBlock(Block());
            } else writeBack.addBlock(b);
        }
        storage->writeBucket(P(leaf, i), writeBack);
    }
    if (inOram == false) {
        for (auto it = stash.begin(); it != stash.end(); it++) {
            if (it->id == blockid) {
                local.push_back(*it);
                stash.erase(it);
                break;
            } 
        }
    }
}

void OAVLTreePathEviction::insert(int key, int val) {
    Block to_insert;
    to_insert.leaf_id = rand_gen->getRandomLeaf();
    to_insert.id = id_gen->getNextId();
    to_insert.data[0] = key;
    to_insert.data[1] = val;
    to_insert.data[2] = 0;
    to_insert.data[3] = 0;
    if (root.id == -1) {
        stash.push_back(to_insert);
        root.leaf_id = to_insert.leaf_id;
        root.id = to_insert.id;
        performDummyFinds(num_levels);
        evictTwoPaths();
        performDummyEvictions(num_levels - 1);
        return;
    }
    moveToLocal(root.leaf_id, root.id);
    int b = 0;
    while (true) {
        if (local[b].data[0] < key) {
            if (local[b].rightChildId != -1) {
                moveToLocal(local[b].rightChildPos, local[b].rightChildId);
            } else {
                local[b].rightChildId = to_insert.id;
                local[b].rightChildPos = to_insert.leaf_id;
                local.push_back(to_insert);
                break;
            }
        } else {
            if (local[b].leftChildId != -1) {
                moveToLocal(local[b].leftChildPos, local[b].leftChildId);
            } else {
                local[b].leftChildId = to_insert.id;
                local[b].leftChildPos = to_insert.leaf_id;
                local.push_back(to_insert);
                break;
            }
        }
        b = (int)local.size() - 1;
    }
    for (int i = (int)local.size() - 2; i >= 0; i--) {
        local[i].leaf_id = rand_gen->getRandomLeaf();
        if (local[i].leftChildId == local[i + 1].id) {
            local[i].leftChildPos = local[i + 1].leaf_id;
        } else local[i].rightChildPos = local[i + 1].leaf_id;
    }
    updateHeight();
    root.id = local[0].id;
    root.leaf_id = local[0].leaf_id;

    for (int i = (int)local.size() - 3; i > 0; i--) {
        if (local[i].getBalance() > 1) {
            if (local[i+1].getBalance() >= 0) {
                bool isLeftChild = false;
                if (local[i-1].leftChildId == local[i].id) isLeftChild = true;
                local[i].leftChildId = local[i+1].rightChildId;
                local[i].leftChildPos = local[i+1].rightChildPos;
                local[i+1].rightChildId = local[i].id;
                local[i+1].rightChildPos = local[i].leaf_id;
                if (isLeftChild) {
                    local[i-1].leftChildId = local[i+1].id;
                    local[i-1].leftChildPos = local[i+1].leaf_id;
                } else {
                    local[i-1].rightChildId = local[i+1].id;
                    local[i-1].rightChildPos = local[i+1].leaf_id;
                }
                local[i].data[2] = local[i+1].data[3];
                local[i+1].data[3] = 1+ max(local[i].data[2],local[i].data[3]);
                if (isLeftChild) local[i-1].data[2] = 1+ max(local[i+1].data[2],local[i+1].data[3]);
                else local[i-1].data[3] = 1+ max(local[i+1].data[2],local[i+1].data[3]);
                for (int j = i-2; j >= 0; j--) {
                    if (local[j].leftChildId == local[j+1].id) {
                        local[j].data[2] = 1 + max(local[j+1].data[2], local[j+1].data[3]);
                    } else local[j].data[3] = 1 + max(local[j+1].data[2], local[j+1].data[3]);
                }
                root.id = local[0].id;
                root.leaf_id = local[0].leaf_id;
            } else { //double rotation
               bool isLeftChild = false;
               if (local[i-1].leftChildId == local[i].id) isLeftChild = true;
               local[i].leftChildId = local[i+2].rightChildId;
               local[i].leftChildPos = local[i+2].rightChildPos;
               local[i].data[2] = local[i+2].data[3];
               local[i+1].rightChildId = local[i+2].leftChildId;
               local[i+1].rightChildPos = local[i+2].leftChildPos;
               local[i+1].data[3] = local[i+2].data[2];
               local[i+2].rightChildId = local[i].id;
               local[i+2].rightChildPos = local[i].leaf_id;
               local[i+2].data[3] = 1 + max(local[i].data[2], local[i].data[3]);
               local[i+2].leftChildId = local[i+1].id;
               local[i+2].leftChildPos = local[i+1].leaf_id;
               local[i+2].data[2] = 1 + max(local[i+1].data[2], local[i+1].data[3]);
               if (isLeftChild) {
                   local[i-1].leftChildId = local[i+2].id;
                   local[i-1].leftChildPos = local[i+2].leaf_id;
                   local[i-1].data[2] = 1+ max(local[i+2].data[2],local[i+2].data[3]);
               } else {
                   local[i-1].rightChildId = local[i+2].id;
                   local[i-1].rightChildPos = local[i+2].leaf_id;
                   local[i-1].data[3] = 1+ max(local[i+2].data[2],local[i+2].data[3]);
               }
               for (int j = i-2; j >= 0; j--) {
                   if (local[j].leftChildId == local[j+1].id) {
                       local[j].data[2] = 1 + max(local[j+1].data[2], local[j+1].data[3]);
                   } else local[j].data[3] = 1 + max(local[j+1].data[2], local[j+1].data[3]);
               }
               root.id = local[0].id;
               root.leaf_id = local[0].leaf_id;
           }
        }
        
        
        if (local[i].getBalance() < -1) {
            if (local[i+1].getBalance() <= 0) {
                bool isLeftChild = false;
                if (local[i-1].leftChildId == local[i].id) isLeftChild = true;
                local[i].rightChildId = local[i+1].leftChildId;
                local[i].rightChildPos = local[i+1].leftChildPos;
                local[i+1].leftChildId = local[i].id;
                local[i+1].leftChildPos = local[i].leaf_id;
                if (isLeftChild) {
                    local[i-1].leftChildId = local[i+1].id;
                    local[i-1].leftChildPos = local[i+1].leaf_id;
                } else {
                    local[i-1].rightChildId = local[i+1].id;
                    local[i-1].rightChildPos = local[i+1].leaf_id;
                }
                local[i].data[3] = local[i+1].data[2];
                local[i+1].data[2] = 1+ max(local[i].data[2],local[i].data[3]);
                if (isLeftChild) local[i-1].data[2] = 1+ max(local[i+1].data[2],local[i+1].data[3]);
                else local[i-1].data[3] = 1+ max(local[i+1].data[2],local[i+1].data[3]);
                for (int j = i-2; j >= 0; j--) {
                    if (local[j].leftChildId == local[j+1].id) {
                        local[j].data[2] = 1 + max(local[j+1].data[2], local[j+1].data[3]);
                    } else local[j].data[3] = 1 + max(local[j+1].data[2], local[j+1].data[3]);
                }
                root.id = local[0].id;
                root.leaf_id = local[0].leaf_id;
            } else { //double rotation
                bool isLeftChild = false;
                if (local[i-1].leftChildId == local[i].id) isLeftChild = true;
                local[i].rightChildId = local[i+2].leftChildId;
                local[i].rightChildPos = local[i+2].leftChildPos;
                local[i].data[3] = local[i+2].data[2];
                local[i+1].leftChildId = local[i+2].rightChildId;
                local[i+1].leftChildPos = local[i+2].rightChildPos;
                local[i+1].data[2] = local[i+2].data[3];
                local[i+2].leftChildId = local[i].id;
                local[i+2].leftChildPos = local[i].leaf_id;
                local[i+2].data[2] = 1 + max(local[i].data[2], local[i].data[3]);
                local[i+2].rightChildId = local[i+1].id;
                local[i+2].rightChildPos = local[i+1].leaf_id;
                local[i+2].data[3] = 1 + max(local[i+1].data[2], local[i+1].data[3]);
                if (isLeftChild) {
                    local[i-1].leftChildId = local[i+2].id;
                    local[i-1].leftChildPos = local[i+2].leaf_id;
                    local[i-1].data[2] = 1+ max(local[i+2].data[2],local[i+2].data[3]);
                } else {
                    local[i-1].rightChildId = local[i+2].id;
                    local[i-1].rightChildPos = local[i+2].leaf_id;
                    local[i-1].data[3] = 1+ max(local[i+2].data[2],local[i+2].data[3]);
                }
                for (int j = i-2; j >= 0; j--) {
                    if (local[j].leftChildId == local[j+1].id) {
                        local[j].data[2] = 1 + max(local[j+1].data[2], local[j+1].data[3]);
                    } else local[j].data[3] = 1 + max(local[j+1].data[2], local[j+1].data[3]);
                }
                root.id = local[0].id;
                root.leaf_id = local[0].leaf_id;
            }
        }
    }

    //rotate root
    if (local[0].getBalance() > 1) {
        if (local[1].getBalance() >= 0) {
            local[0].leftChildId = local[1].rightChildId;
            local[0].leftChildPos = local[1].rightChildPos;
            local[1].rightChildId = local[0].id;
            local[1].rightChildPos = local[0].leaf_id;
            local[0].data[2] = local[1].data[3];
            local[1].data[3] = 1+ max(local[0].data[2],local[0].data[3]);
            root.id = local[1].id;
            root.leaf_id = local[1].leaf_id;
        } else { //double rotation
            local[0].leftChildId = local[2].rightChildId;
            local[0].leftChildPos = local[2].rightChildPos;
            local[0].data[2] = local[2].data[3];
            local[1].rightChildId = local[2].leftChildId;
            local[1].rightChildPos = local[2].leftChildPos;
            local[1].data[3] = local[2].data[2];
            local[2].rightChildId = local[0].id;
            local[2].rightChildPos = local[0].leaf_id;
            local[2].data[3] = 1 + max(local[0].data[2], local[0].data[3]);
            local[2].leftChildId = local[1].id;
            local[2].leftChildPos = local[1].leaf_id;
            local[2].data[2] = 1 + max(local[1].data[2], local[1].data[3]);
            root.id = local[2].id;
            root.leaf_id = local[2].leaf_id;
        }
    }
    if (local[0].getBalance() < -1) {
        if (local[1].getBalance() <= 0) {
            local[0].rightChildId = local[1].leftChildId;
            local[0].rightChildPos = local[1].leftChildPos;
            local[1].leftChildId = local[0].id;
            local[1].leftChildPos = local[0].leaf_id;
            local[0].data[3] = local[1].data[2];
            local[1].data[2] = 1+ max(local[0].data[2],local[0].data[3]);
            root.id = local[1].id;
            root.leaf_id = local[1].leaf_id;
        } else { //double rotation
            local[0].rightChildId = local[2].leftChildId;
            local[0].rightChildPos = local[2].leftChildPos;
            local[0].data[3] = local[2].data[2];
            local[1].leftChildId = local[2].rightChildId;
            local[1].leftChildPos = local[2].rightChildPos;
            local[1].data[2] = local[2].data[3];
            local[2].leftChildId = local[0].id;
            local[2].leftChildPos = local[0].leaf_id;
            local[2].data[2] = 1 + max(local[0].data[2], local[0].data[3]);
            local[2].rightChildId = local[1].id;
            local[2].rightChildPos = local[1].leaf_id;
            local[2].data[3] = 1 + max(local[1].data[2], local[1].data[3]);
            root.id = local[2].id;
            root.leaf_id = local[2].leaf_id;
        }
    }
    int l = local.size();
    performDummyFinds(num_levels - l);
    while (!local.empty())
    {
        stash.push_back(local[local.size()-1]);
        local.pop_back();
        evictTwoPaths();
    }
    performDummyEvictions(num_levels - l);
}

void OAVLTreePathEviction::updateHeight() {
    for (int i = local.size() - 2; i >= 0; i--) {
        if (local[i].leftChildId == local[i+1].id) {
            local[i].data[2] = 1 + max(local[i+1].data[2], local[i+1].data[3]);
        } else local[i].data[3] = 1 + max(local[i+1].data[2], local[i+1].data[3]);
    }
}

int OAVLTreePathEviction::search(int key) {
    if (root.id == -1) throw SearchFailureException();
    moveToLocal(root.leaf_id, root.id);
    Block b = local[0];
    while (b.data[0] != key) {
        if (b.data[0] < key) {
            if (b.rightChildId != -1) {
                moveToLocal(b.rightChildPos, b.rightChildId);
            } else {
                evictAfterSearch();
                throw SearchFailureException();
            }
        } else {
            if (b.leftChildId != -1) {
                moveToLocal(b.leftChildPos, b.leftChildId);
            } else {
                evictAfterSearch();
                throw SearchFailureException();
            }
        }
        b = local[local.size() - 1];
    }
    int val = b.data[1];
    int l = local.size();
    performDummyFinds(num_levels - l);
    evictAfterSearch();
    performDummyEvictions(num_levels - l);
    return val;
}

void OAVLTreePathEviction::evictAfterSearch() {
    while (local.size() > 1)
    {
        local[local.size()-1].leaf_id = rand_gen->getRandomLeaf();
        if (local[local.size()-2].leftChildId == local[local.size()-1].id) {
            local[local.size()-2].leftChildPos = local[local.size()-1].leaf_id;
        } else local[local.size()-2].rightChildPos = local[local.size()-1].leaf_id;
        stash.push_back(local[local.size()-1]);
        local.pop_back();
        evictTwoPaths();
    }
    local[0].leaf_id = rand_gen->getRandomLeaf();
    stash.push_back(local[0]);
    root.id = local[0].id;
    root.leaf_id = local[0].leaf_id;
    local.pop_back();
    evictTwoPaths();
}

void OAVLTreePathEviction::evictTwoPaths() {
    int random_leaf_one = this->rand_gen->getRandomLeaf()/2;
    int random_leaf_two = this->rand_gen->getRandomLeaf()/2 + this->num_leaves/2;
    this->evictByLeaf(random_leaf_one);
    this->evictByLeaf(random_leaf_two);
}

void OAVLTreePathEviction::evictByLeaf(int leaf) {
    for (int l = 0; l < num_levels; l++) {
      std::vector<Block> blocks = this->storage->readBucket(this->P(leaf, l)).getBlocks();
      for (const Block& b : blocks) {
        if (b.id != -1) {
          this->stash.push_back(b);
        }
      }
    }
    for (int l = num_levels-1; l >= 0; l--) {
      auto evictedId = std::vector<int>();
      Bucket bucket;
      int Pxl = P(leaf, l);
      int counter = 0;

      for (const Block& be_evicted : this->stash) {
        if (counter >= this->bucket_size) {
          break;
        }
        if (Pxl == this->P(be_evicted.leaf_id, l)) {
          bucket.addBlock(be_evicted);
          evictedId.push_back(be_evicted.id);
          counter++;
        }
      }

      // removing from the stash; careful not to modify the stash while iterating over it
      for (int i = 0; i < evictedId.size(); i++) {
        for (int j = 0; j < this->stash.size(); j++) {
          if (this->stash.at(j).id == evictedId.at(i)) {
            this->stash.erase(this->stash.begin() + j);
            break;
          }
        }
      }

      // pad out the bucket with dummy blocks
      while (counter < bucket_size) {
        bucket.addBlock(Block());
        counter++;
      }

      this->storage->writeBucket(Pxl, bucket);
    }
}

int OAVLTreePathEviction::P(int leaf, int level) {
    /*
    * This function should be deterministic. 
    * INPUT: leaf in range 0 to num_leaves - 1, level in range 0 to num_levels - 1. 
    * OUTPUT: Returns the location in the storage of the bucket which is at the input level and leaf.
    */
    return (1<<level) - 1 + (leaf >> (this->num_levels - level - 1));
}


/*
The below functions are to access various parameters, as described by their names.
INPUT: No input
OUTPUT: Value of internal variables given in the name.
*/


vector<Block> OAVLTreePathEviction::getStash() {
    return this->stash;
}
    
int OAVLTreePathEviction::getStashSize() {
    return (int)(this->stash).size();
}
    
int OAVLTreePathEviction::getNumLeaves() {
    return this->num_leaves;

}

int OAVLTreePathEviction::getNumLevels() {
    return this->num_levels;

}

int OAVLTreePathEviction::getNumBlocks() {
    return this->num_blocks;

}

int OAVLTreePathEviction::getNumBuckets() {
    return this->num_buckets;

}

void OAVLTreePathEviction::dumpStash() {
    for (auto bl: getStash()) {
        if (bl.id != -1) {
            cout << bl.id << " " << bl.data[0] << " "<< bl.data[1] << " "<< bl.data[2] << " "<< bl.data[3]  << " "<< bl.leftChildId  << " "<< bl.rightChildId << endl;
        }
    }
}

void OAVLTreePathEviction::performDummyFinds(int num) {
    for (int i = 0; i < num; ++i) {
        auto leaf = rand_gen->getRandomLeaf();
        for (int i = 0; i < num_levels; i++) {
            vector<Block> blocks = storage->readBucket(P(leaf, i)).getBlocks();
            Bucket writeBack;
            for (Block &b: blocks) {
                writeBack.addBlock(b);
            }
            storage->writeBucket(P(leaf, i), writeBack);
        }
    }
}
void OAVLTreePathEviction::performDummyEvictions(int num) {
    for (int i = 0; i < num; ++i) evictTwoPaths();
}