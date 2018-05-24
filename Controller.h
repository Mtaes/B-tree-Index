#pragma once

#include "BTree.h"

class Controller {
	BTree *tree;
	void instructions();
	void test();
public:
	Controller();
	~Controller();
	void userInput();
};