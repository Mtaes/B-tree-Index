#pragma once

#include "MyBuffer.h"

class BTree {
	int D;
	int sizeOfPage;
	MyBuffer *BTreeBuffer;
	MyBuffer *MainFileBuffer;
	MyBuffer *BTreeBitMapBuffer;
	MyBuffer *MainFileBitMapBuffer;

	//Search for a place to insert data based on bitmaps
	//isPage == true -> search for a place for new page in b-tree
	//isPage == false -> search in main file
	int searchForPlace(bool isPage);
	//address -> address in file
	//isTaken true -> position will be marked as taken
	//isTaken false -> position will be marked as empty
	void managePlace(int address, bool isPage, bool isTaken);
	//Add new page to b-tree and return its address
	int addPage(int addressOfParent);
	int getNumberOfKeys(int addressOfPage);
	void setNumberOfKeys(int addressOfPage, int numberOfKeys);
	//Read entry from b-tree -> key, address in main file, child
	void readEntryFromBTree(int *entry, int addressOfEntry);
	void insertEntryToBTree(int *entry, int addressOfEntry);
	//Search on page for key equal or greater than key in argument of this function
	//Returns number of position on page
	int findPlaceOnPage(int addressOfPage, int key);
	void moveToLeft(int addressOfPage, int index);
	void moveToRight(int addressOfPage, int index);
	int getParent(int addressOfPage);
	int searchByPointer(int addressOfPage);
	bool compensate(int addressOfPage, int *entryToWrite);
	void setChildren(int addressOfPage);
	//Splits page and insert new record
	//To entryToWrite inserts record to be written to parent
	void split(int addressOfPage, int *entryToWrite);
	//Save parent before calling this function
	void merge(int addressOfPage);
	void writeToMainFile(int addressInMainFile, double a, double b, double c);
	bool searchForEntry(int key, int *addressOfPage, int *index);
	void readFromMainFile(int addressInMainFile, double *destination);
	//1 -> taken
	//0 -> empty
	//-1 -> end of file
	int checkAddressInBitMap(int address);
	void outPage(int address);
	void printPage(int address);

public:
	BTree(int _D);
	~BTree();

	bool addNewEntryToBTree(int key, double a, double b, double c);
	bool search(int key, double *dataFromMainFile);
	bool deleteEntry(int key);
	bool modify(int key, double a, double b, double c);
	void printTree();
	void printMainFile();
	void dumpMainFile();
	void dumpIndex();
};