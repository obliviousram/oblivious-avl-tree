#include <iostream>
#include <cmath>
#include "Bucket.h"
#include "Block.h"
#include "RandomForOram.h"
#include "OAVLTreePathEviction.h"
#include "OAVLTreeDeterministicEviction.h"
#include "OAVLTreeInterface.h"
#include "RandForOramInterface.h"
#include "UntrustedStorageInterface.h"
#include "ServerStorage.h"
#include "CorrectnessTester.h"


using namespace std;

int main() {
    CorrectnessTester* tester = new CorrectnessTester;
    //make sure ONLY ONE runTester is uncommented before compiling
    // tester->runTester1();
    tester->runTester2();
    delete tester;
}
