//
//
//
#include "Block.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

Block::Block(const Block& b) {
    for (int i = 0; i<BLOCK_SIZE; i++) {
		this->data[i] = b.data[i];
	}
    this->leaf_id = b.leaf_id;
    this->id = b.id;
    this->leftChildPos = b.leftChildPos;
    this->leftChildId = b.leftChildId;
    this->rightChildPos = b.rightChildPos;
    this->rightChildId = b.rightChildId;
}

Block::Block() { }
Block::~Block() { }

Block::Block(int leaf_id, int id, int data[]) : leaf_id(leaf_id), id(id)
{
   for (int i = 0; i < BLOCK_SIZE; i++){
       this->data[i] = data[i];
   }
}

void Block::printBlock(){
	string data_holder = "";
	for (int i = 0; i<BLOCK_SIZE; i++) {
		data_holder += to_string(this->data[i]);
		data_holder += " ";
	}
	cout << "id: " << to_string(this->id) << " leaf id: " << to_string(this->leaf_id) << " data: " << data_holder << endl;
}

int Block::getBalance() {
    return this->data[2] - this->data[3];
}