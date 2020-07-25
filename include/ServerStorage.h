//
//
//

#ifndef PORAM_SERVERSTORAGE_H
#define PORAM_SERVERSTORAGE_H
#include "OAVLTreeInterface.h"
#include "RandForOramInterface.h"
#include "UntrustedStorageInterface.h"
#include <cmath>

class ServerStorage : public UntrustedStorageInterface {
public:
    
    static bool is_initialized;
    static bool is_capacity_set;
    //Bucket* buckets;
    std::vector<Bucket> buckets;
    
    ServerStorage();
    void setCapacity(int totalNumOfBuckets);
    Bucket readBucket(int position);
    void writeBucket(int position, const Bucket& bucket_to_write);
    //functions only of debug
    void dumpAllValidBlocks();
private: 

    int capacity;

};


#endif //PORAM_ORAMREADPATHEVICTION_H
