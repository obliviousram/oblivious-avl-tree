# README
A reference implementation for the Oblivious AVL Tree algorithm. For a description of the algorithm, please refer to https://eprint.iacr.org/2014/185.pdf

All source files are in the `src/` directory.
- `Block`, `Bucket`: contains implementation for the Block and Bucket data structures as described
  in the paper.
- `UntrustedStorageInterface`: defines the interface that the implemenation uses to
  communicate with the untrusted cloud storage provider. A possible implementation is defined in
  `ServerStorage`.
- `RandForOramInterface`: the implementation gets random leaf ids from here. A possible
  implementation is defined in `RandomForOram`. Can be overridden e.g. for testing purposes.
- `OAVLTreeInterface`: The interface with which to use this Oblivious AVL Tree implementation.
  `OAVLTreePathEviction` contains the reference implementation itself.


- `main`: Runs testcases.

To compile and run:

1. Uncomment out the desired test in `src/main.cpp`; comment out any other tests.
2. Run `make` in this directory.
3. Run the test with `./avl`.

C++17 is required. Known to compile with `gcc version 7.4.0 (Ubuntu 7.4.0-1ubuntu1~18.04.1)` and `gcc version 7.3.1 20180130 (Red Hat 7.3.1-2) (GCC)`.

rm -f ./obj/*.o *~ avl
g++ -Iinclude -std=c++11 -g -Werror -O3 -c -o obj/Block.o src/Block.cpp
g++ -Iinclude -std=c++11 -g -Werror -O3 -c -o obj/Bucket.o src/Bucket.cpp
g++ -Iinclude -std=c++11 -g -Werror -O3 -c -o obj/CorrectnessTester.o src/CorrectnessTester.cpp
g++ -Iinclude -std=c++11 -g -Werror -O3 -c -o obj/OAVLTreeDeterministicEviction.o src/OAVLTreeDeterministicEviction.cpp
g++ -Iinclude -std=c++11 -g -Werror -O3 -c -o obj/OAVLTreePathEviction.o src/OAVLTreePathEviction.cpp
g++ -Iinclude -std=c++11 -g -Werror -O3 -c -o obj/RandomForOram.o src/RandomForOram.cpp
g++ -Iinclude -std=c++11 -g -Werror -O3 -c -o obj/ServerStorage.o src/ServerStorage.cpp
g++ -Iinclude -std=c++11 -g -Werror -O3 -c -o obj/csprng.o src/csprng.cpp
g++ -Iinclude -std=c++11 -g -Werror -O3 -c -o obj/main.o src/main.cpp
g++ -Iinclude -std=c++11 -g -Werror -O3 -o avl obj/Block.o obj/Bucket.o obj/CorrectnessTester.o obj/OAVLTreeDeterministicEviction.o obj/OAVLTreePathEviction.o obj/RandomForOram.o obj/ServerStorage.o obj/csprng.o obj/main.o