#include "BTree.h"
#include<iostream>

BTree::BTree(int _D) {
	D = _D;
	sizeOfPage = 12 + D * 2 * 12;
	BTreeBuffer = new MyBuffer("btree", 2, sizeOfPage);
	MainFileBuffer = new MyBuffer("mainfile", 2, 24);
	BTreeBitMapBuffer = new MyBuffer("btreebitmap", 2, 6);
	MainFileBitMapBuffer = new MyBuffer("mainfilebitmap", 2, 6);
	if (!BTreeBuffer->checkIfFileExists()) {
		addPage(-1);
	}
}


BTree::~BTree() {
	delete BTreeBuffer;
	delete MainFileBuffer;
	delete BTreeBitMapBuffer;
	delete MainFileBitMapBuffer;
}


int BTree::searchForPlace(bool isPage) {
	MyBuffer *buff;
	if (isPage) {
		buff = BTreeBitMapBuffer;
	}
	else {
		buff = MainFileBitMapBuffer;
	}
	int index = 0;
	bool con = true;
	buff->setPosition(0);
	char bitsToCheck;
	unsigned char mask;
	unsigned char result;
	while (con) {
		con = buff->read(&bitsToCheck);
		if (con == false) {
			buff->write(0);
			break;
		}
		mask = 128;
		for (int i = 0; i < 8; i++) {
			result = ~bitsToCheck & mask;
			if (result == mask) {
				if (isPage) {
					return index * sizeOfPage;
				}
				else {
					return index * 24;
				}
			}
			mask >>= 1;
			index++;
		}
	}

	if (isPage) {
		return index * sizeOfPage;
	}
	else {
		return index * 24;
	}
}


void BTree::managePlace(int address, bool isPage, bool isTaken) {
	MyBuffer *buff;
	if (isPage) {
		buff = BTreeBitMapBuffer;
	}
	else {
		buff = MainFileBitMapBuffer;
	}
	int addressInMap = address;
	if (isPage) {
		addressInMap /= sizeOfPage;
	}
	else {
		addressInMap /= 24;
	}

	buff->setPosition(addressInMap / 8);
	char bits;
	buff->read(&bits);

	unsigned char mask = 128;
	mask >>= addressInMap % 8;

	if (isTaken) {
		bits |= mask;
	}
	else if (!isTaken) {
		bits &= ~mask;
	}
	buff->setPosition(addressInMap / 8);
	buff->write(bits);
}


int BTree::addPage(int addressOfParent) {
	int defaultVar = -1;
	unsigned char temp[8];
	int numberOfKeys = 0;

	int address = searchForPlace(true);
	managePlace(address, true, true);

	memcpy(temp, &numberOfKeys, 4);
	memcpy(&temp[4], &addressOfParent, 4);

	BTreeBuffer->setPosition(address);
	for (int i = 0; i < 8; i++) {
		BTreeBuffer->write(temp[i]);
	}

	memcpy(temp, &defaultVar, 4);
	for (int i = 0; i < 2 * D * 3 + 1; i++) {
		for (int j = 0; j < 4; j++) {
			BTreeBuffer->write(temp[j]);
		}
	}

	return address;
}


int BTree::getNumberOfKeys(int addressOfPage) {
	int numberOfKeys;
	char temp[4];
	BTreeBuffer->setPosition(addressOfPage);
	for (int i = 0; i < 4; i++) {
		BTreeBuffer->read(&temp[i]);
	}
	memcpy(&numberOfKeys, temp, 4);
	return numberOfKeys;
}


void BTree::setNumberOfKeys(int addressOfPage, int numberOfKeys) {
	char temp[4];
	memcpy(temp, &numberOfKeys, 4);
	BTreeBuffer->setPosition(addressOfPage);
	for (int i = 0; i < 4; i++) {
		BTreeBuffer->write(temp[i]);
	}
}


void BTree::readEntryFromBTree(int *entry, int addressOfEntry) {
	char temp[12];
	BTreeBuffer->setPosition(addressOfEntry);
	for (int i = 0; i < 12; i++) {
		BTreeBuffer->read(&temp[i]);
	}
	memcpy(entry, temp, 12);
}


void BTree::insertEntryToBTree(int *entry, int addressOfEntry) {
	char temp[12];
	memcpy(temp, entry, 12);
	BTreeBuffer->setPosition(addressOfEntry);
	for (int i = 0; i < 12; i++) {
		BTreeBuffer->write(temp[i]);
	}
}


int BTree::findPlaceOnPage(int addressOfPage, int key) {
	int numberOfKeys = getNumberOfKeys(addressOfPage);
	int index;
	int temp[3];
	for (index = 0; index < numberOfKeys; index++) {
		readEntryFromBTree(temp, addressOfPage + 12 + 12 * index);
		if (temp[0] >= key) {
			break;
		}
	}
	return index;
}


void BTree::moveToLeft(int addressOfPage, int index) {
	int numberOfKeys = getNumberOfKeys(addressOfPage);
	char temp[12];
	if (numberOfKeys == 2 * D) {
		numberOfKeys--;
	}
	for (int i = numberOfKeys; i > index; i--) {
		BTreeBuffer->setPosition(addressOfPage + 12 + 12 * (i - 1));
		for (int j = 0; j < 12; j++) {
			BTreeBuffer->read(&temp[j]);
		}
		BTreeBuffer->setPosition(addressOfPage + 12 + 12 * i);
		for (int j = 0; j < 12; j++) {
			BTreeBuffer->write(temp[j]);
		}
	}
}


void BTree::moveToRight(int addressOfPage, int index) {
	char temp[12];
	for (int i = index + 1; i < 2 * D; i++) {
		BTreeBuffer->setPosition(addressOfPage + 12 + 12 * i);
		for (int j = 0; j < 12; j++) {
			BTreeBuffer->read(&temp[j]);
		}
		BTreeBuffer->setPosition(addressOfPage + 12 + 12 * (i - 1));
		for (int j = 0; j < 12; j++) {
			BTreeBuffer->write(temp[j]);
		}
	}
	int var = -1;
	memcpy(&temp, &var, 4);
	BTreeBuffer->setPosition(addressOfPage + 12 + 12 * (2 * D - 1));
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 4; j++) {
			BTreeBuffer->write(temp[j]);
		}
	}
}


int BTree::getParent(int addressOfPage) {
	int parent;
	char temp[4];
	BTreeBuffer->setPosition(addressOfPage + 4);
	for (int i = 0; i < 4; i++) {
		BTreeBuffer->read(&temp[i]);
	}
	memcpy(&parent, temp, 4);
	return parent;
}


int BTree::searchByPointer(int addressOfPage) {
	int parentAddr = getParent(addressOfPage);
	int index;
	int temp[3];
	int keys = getNumberOfKeys(parentAddr);
	for (index = 0; index < keys; index++) {
		readEntryFromBTree(temp, parentAddr + 12 * index);
		if (temp[2] == addressOfPage) {
			break;
		}
	}
	return index;
}


bool BTree::compensate(int addressOfPage, int *entryToWrite) {
	if (addressOfPage == 0) {
		return false;
	}
	int addressOfParent = getParent(addressOfPage);
	int indexInParent = searchByPointer(addressOfPage);
	int temp[3];
	int numberOfKeys = getNumberOfKeys(addressOfPage);;
	if (entryToWrite[0] != -1) {
		numberOfKeys++;
	}
	bool selectedLeft = false;
	if (indexInParent > 0) {
		readEntryFromBTree(temp, addressOfParent + 12 * (indexInParent - 1));
		int keysInLeft = getNumberOfKeys(temp[2]);
		if (keysInLeft + numberOfKeys >= 2 * D && keysInLeft + numberOfKeys <= 4 * D) {
			selectedLeft = true;
		}
	}
	if (!selectedLeft && indexInParent < getNumberOfKeys(addressOfParent)) {
		readEntryFromBTree(temp, addressOfParent + 12 * (indexInParent + 1));
		int keysInRight = getNumberOfKeys(temp[2]);
		if (keysInRight + numberOfKeys < 2 * D || keysInRight + numberOfKeys > 4 * D) {
			return false;
		}
	}
	else if (!selectedLeft) {
		return false;
	}

	int addressOfLeft;
	int addressOfRight;
	int keysInLeft;
	int keysInRight;
	int separatorAddr;
	int separator[3];

	if (selectedLeft) {
		addressOfLeft = temp[2];
		keysInLeft = getNumberOfKeys(addressOfLeft);
		addressOfRight = addressOfPage;
		keysInRight = getNumberOfKeys(addressOfRight);
		readEntryFromBTree(separator, addressOfParent + 12 * indexInParent);
		separatorAddr = addressOfParent + 12 * indexInParent;
	}
	else {
		addressOfLeft = addressOfPage;
		keysInLeft = getNumberOfKeys(addressOfLeft);
		addressOfRight = temp[2];
		keysInRight = getNumberOfKeys(addressOfRight);
		readEntryFromBTree(separator, addressOfParent + 12 * (indexInParent + 1));
		separatorAddr = addressOfParent + 12 * (indexInParent + 1);
	}
	int totalKeys = keysInLeft + keysInRight;
	bool isWritten = false;
	if (entryToWrite[0] != -1) {
		totalKeys++;
	}
	else {
		isWritten = true;
	}
	int limitLeft = totalKeys / 2 + totalKeys % 2;
	int limitRight = totalKeys / 2;

	if (keysInLeft < limitLeft) {
		//copy separator
		insertEntryToBTree(separator, addressOfLeft + 12 + 12 * keysInLeft);
		int temp1[3];
		int temp2[3];
		readEntryFromBTree(temp1, addressOfLeft + 12 + 12 * keysInLeft);
		readEntryFromBTree(temp2, addressOfRight);
		temp1[2] = temp2[2];
		insertEntryToBTree(temp1, addressOfLeft + 12 + 12 * keysInLeft);
		keysInLeft++;
		setNumberOfKeys(addressOfLeft, keysInLeft);


		if (!isWritten) {
			int index = findPlaceOnPage(addressOfRight, entryToWrite[0]);
			if (index == 0) {
				separator[0] = entryToWrite[0];
				separator[1] = entryToWrite[1];
				insertEntryToBTree(separator, separatorAddr);
				readEntryFromBTree(temp1, addressOfRight);
				temp1[2] = entryToWrite[2];
				insertEntryToBTree(temp1, addressOfRight);
			}
			else {
				readEntryFromBTree(temp1, addressOfRight + 12);
				separator[0] = temp1[0];
				separator[1] = temp1[1];
				insertEntryToBTree(separator, separatorAddr);
				readEntryFromBTree(temp2, addressOfRight);
				temp2[2] = temp1[2];
				insertEntryToBTree(temp2, addressOfRight);
				moveToRight(addressOfRight, 0);
				index--;
				moveToLeft(addressOfRight, index);
				insertEntryToBTree(entryToWrite, addressOfRight + 12 + 12 * index);
			}
			isWritten = true;
		}
		else {
			readEntryFromBTree(temp1, addressOfRight + 12);
			separator[0] = temp1[0];
			separator[1] = temp1[1];
			insertEntryToBTree(separator, separatorAddr);
			readEntryFromBTree(temp2, addressOfRight);
			temp2[2] = temp1[2];
			insertEntryToBTree(temp2, addressOfRight);
			moveToRight(addressOfRight, 0);
			keysInRight--;
			setNumberOfKeys(addressOfRight, keysInRight);
		}

		while (keysInLeft < limitLeft) {
			//copy separator
			insertEntryToBTree(separator, addressOfLeft + 12 + 12 * keysInLeft);
			readEntryFromBTree(temp1, addressOfLeft + 12 + 12 * keysInLeft);
			readEntryFromBTree(temp2, addressOfRight);
			temp1[2] = temp2[2];
			insertEntryToBTree(temp1, addressOfLeft + 12 + 12 * keysInLeft);
			keysInLeft++;
			setNumberOfKeys(addressOfLeft, keysInLeft);

			readEntryFromBTree(temp1, addressOfRight + 12);
			separator[0] = temp1[0];
			separator[1] = temp1[1];
			insertEntryToBTree(separator, separatorAddr);
			readEntryFromBTree(temp2, addressOfRight);
			temp2[2] = temp1[2];
			insertEntryToBTree(temp2, addressOfRight);
			moveToRight(addressOfRight, 0);
			keysInRight--;
			setNumberOfKeys(addressOfRight, keysInRight);
		}
	}
	else {
		//copy separator
		moveToLeft(addressOfRight, 0);
		int temp1[3];
		int temp2[3];
		readEntryFromBTree(temp2, addressOfRight);
		temp1[0] = separator[0];
		temp1[1] = separator[1];
		temp1[2] = temp2[2];
		insertEntryToBTree(temp1, addressOfRight + 12);
		keysInRight++;
		setNumberOfKeys(addressOfRight, keysInRight);

		if (!isWritten) {
			int index = findPlaceOnPage(addressOfLeft, entryToWrite[0]);
			if (index == 2 * D) {
				separator[0] = entryToWrite[0];
				separator[1] = entryToWrite[1];
				insertEntryToBTree(separator, separatorAddr);
				readEntryFromBTree(temp1, addressOfRight);
				temp1[2] = entryToWrite[2];
				insertEntryToBTree(temp1, addressOfRight);
			}
			else {
				readEntryFromBTree(temp1, addressOfLeft + 12 + 12 * (keysInLeft - 1));
				separator[0] = temp1[0];
				separator[1] = temp1[1];
				insertEntryToBTree(separator, separatorAddr);
				readEntryFromBTree(temp2, addressOfRight);
				temp2[2] = temp1[2];
				insertEntryToBTree(temp2, addressOfRight);
				moveToLeft(addressOfLeft, index);
				insertEntryToBTree(entryToWrite, addressOfLeft + 12 + 12 * index);
			}
			isWritten = true;
		}
		else {
			readEntryFromBTree(temp1, addressOfLeft + 12 + 12 * (keysInLeft - 1));
			separator[0] = temp1[0];
			separator[1] = temp1[1];
			insertEntryToBTree(separator, separatorAddr);
			readEntryFromBTree(temp2, addressOfRight);
			temp2[2] = temp1[2];
			insertEntryToBTree(temp2, addressOfRight);
			moveToRight(addressOfLeft, keysInLeft - 1);
			keysInLeft--;
			setNumberOfKeys(addressOfLeft, keysInLeft);
		}

		while (keysInLeft > limitLeft) {
			moveToLeft(addressOfRight, 0);
			readEntryFromBTree(temp2, addressOfRight);
			temp1[0] = separator[0];
			temp1[1] = separator[1];
			temp1[2] = temp2[2];
			insertEntryToBTree(temp1, addressOfRight + 12);
			keysInRight++;
			setNumberOfKeys(addressOfRight, keysInRight);

			readEntryFromBTree(temp1, addressOfLeft + 12 + 12 * (keysInLeft - 1));
			separator[0] = temp1[0];
			separator[1] = temp1[1];
			insertEntryToBTree(separator, separatorAddr);
			readEntryFromBTree(temp2, addressOfRight);
			temp2[2] = temp1[2];
			insertEntryToBTree(temp2, addressOfRight);
			moveToRight(addressOfLeft, keysInLeft - 1);
			keysInLeft--;
			setNumberOfKeys(addressOfLeft, keysInLeft);
		}
	}

	setNumberOfKeys(addressOfLeft, limitLeft);
	setNumberOfKeys(addressOfRight, limitRight);
	setChildren(addressOfLeft);
	setChildren(addressOfRight);
	return true;
}


void BTree::setChildren(int addressOfPage) {
	int par = getNumberOfKeys(0);
	int numberOfKeys = getNumberOfKeys(addressOfPage);
	int addressOfChild;
	char addressOfPageChar[4];
	char temp[4];
	memcpy(addressOfPageChar, &addressOfPage, 4);
	par = getNumberOfKeys(0);
	for (int i = 0; i <= numberOfKeys; i++) {
		BTreeBuffer->setPosition(addressOfPage + 8 + 12 * i);
		for (int j = 0; j < 4; j++) {
			BTreeBuffer->read(&temp[j]);
		}
		memcpy(&addressOfChild, temp, 4);
		if (addressOfChild == -1) {
			break;
		}
		BTreeBuffer->setPosition(addressOfChild + 4);
		for (int j = 0; j < 4; j++) {
			BTreeBuffer->write(addressOfPageChar[j]);
		}
	}
	par = getNumberOfKeys(0);
	int d = 3;
}


void BTree::split(int addressOfPage, int *entryToWrite) {
	int addressOfFirstPage;
	if (addressOfPage == 0) {
		//Add new page
		addressOfFirstPage = addPage(0);
		//Copy infrormation (except parent) from root to new page
		int temp[3];
		//Copy header
		readEntryFromBTree(temp, addressOfPage);
		temp[1] = 0;
		insertEntryToBTree(temp, addressOfFirstPage);
		//Copy header
		for (int i = 0; i < 2 * D; i++) {
			readEntryFromBTree(temp, addressOfPage + 12 + 12 * i);
			insertEntryToBTree(temp, addressOfFirstPage + 12 + 12 * i);
		}
		//Set parents of children of page addressOfFirstPage
		setChildren(addressOfFirstPage);
		//Clear root
		temp[0] = 0;
		temp[1] = -1;
		temp[2] = addressOfFirstPage;
		insertEntryToBTree(temp, addressOfPage);
		temp[0] = -1;
		temp[2] = -1;
		for (int i = 0; i < 2 * D; i++) {
			insertEntryToBTree(temp, addressOfPage + 12 + 12 * i);
		}
	}
	else {
		addressOfFirstPage = addressOfPage;
	}
	int parentKeys = getNumberOfKeys(0);
	int addressOfParent = getParent(addressOfFirstPage);
	int addressOfSecondPage = addPage(addressOfParent);

	int tempEntry[3];
	bool newEntryIsWritten = false;
	//Search for a place for a new record
	for (int i = 0; i < 2 * D; i++) {
		readEntryFromBTree(tempEntry, addressOfFirstPage + 12 + 12 * i);
		if (entryToWrite[0] < tempEntry[0]) {
			//Save the last item
			readEntryFromBTree(tempEntry, addressOfFirstPage + 12 + 12 * (2 * D - 1));
			moveToLeft(addressOfFirstPage, i);
			//Insert newEntry
			insertEntryToBTree(entryToWrite, addressOfFirstPage + 12 + 12 * i);
			newEntryIsWritten = true;
			break;
		}
	}
	parentKeys = getNumberOfKeys(0);
	//Take the next item as a separator (index D)
	//If new record wasn't saved, save it as the last one
	int lastEntry[3];
	if (!newEntryIsWritten) {
		memcpy(lastEntry, entryToWrite, 12);
	}
	else {
		memcpy(lastEntry, tempEntry, 12);
	}
	//Copy separator
	readEntryFromBTree(entryToWrite, addressOfFirstPage + 12 + 12 * D);

	unsigned char tempChar[4];
	//Save child of separator to new page as child 0
	
	memcpy(tempChar, &entryToWrite[2], 4);

	BTreeBuffer->setPosition(addressOfSecondPage + 8);
	for (int i = 0; i < 4; i++) {
		BTreeBuffer->write(tempChar[i]);
	}

	parentKeys = getNumberOfKeys(0);
	//Set child of separator
	entryToWrite[2] = addressOfSecondPage;

	//Copy records to the second page
	for (int i = 0; i < D - 1; i++) {
		readEntryFromBTree(tempEntry, addressOfFirstPage + 12 + (D + 1) * 12 + 12 * i);
		insertEntryToBTree(tempEntry, addressOfSecondPage + 12 + 12 * i);
	}
	insertEntryToBTree(lastEntry, addressOfSecondPage + 12 + 12 * (D - 1));
	//Insert keys
	setNumberOfKeys(addressOfFirstPage, D);
	setNumberOfKeys(addressOfSecondPage, D);

	//Clear addressOfFirstPage
	tempEntry[0] = -1;
	tempEntry[1] = -1;
	tempEntry[2] = -1;
	for (int i = 0; i < D; i++) {
		insertEntryToBTree(tempEntry, addressOfFirstPage + 12 + 12 * D + 12 * i);
	}
	parentKeys = getNumberOfKeys(0);
	//Set children
	setChildren(addressOfSecondPage);
	parentKeys = getNumberOfKeys(0);
}


void BTree::writeToMainFile(int addressInMainFile, double a, double b, double c) {
	MainFileBuffer->setPosition(addressInMainFile);
	char temp[24];
	memcpy(temp, &a, 8);
	memcpy(&temp[8], &b, 8);
	memcpy(&temp[16], &c, 8);
	for (int i = 0; i < 24; i++) {
		MainFileBuffer->write(temp[i]);
	}
	managePlace(addressInMainFile, false, true);
}


bool BTree::addNewEntryToBTree(int key, double a, double b, double c) {
	BTreeBuffer->resetStats();
	int addressInMainFile = searchForPlace(false);
	int entry[3];
	int addressOfPage;
	int addressOfChild = 0;
	int index;
	while (addressOfChild != -1) {
		addressOfPage = addressOfChild;

		index = findPlaceOnPage(addressOfChild, key);
		//If found place is inside page
		if (index < getNumberOfKeys(addressOfChild)) {
			readEntryFromBTree(entry, addressOfChild + 12 + 12 * index);
			//If returned key is equal to key to insert -> return false
			if (entry[0] == key) {
				std::cout << "\n\nRead operations: " << BTreeBuffer->getReadOperations() << "\nWrite operations: " << BTreeBuffer->getWriteOperations() << "\n\n";
				return false;
			}
		}
		//If returned key is diffrent from key to insert -> read address of left parent
		BTreeBuffer->setPosition(addressOfChild + 12 + 12 * index - 4);
		char temp[4];
		for (int i = 0; i < 4; i++) {
			BTreeBuffer->read(&temp[i]);
		}
		memcpy(&addressOfChild, temp, 4);
	}
	writeToMainFile(addressInMainFile, a, b, c);

	int numberOfKeys = getNumberOfKeys(addressOfPage);
	//If page isn't full
	if (numberOfKeys < 2 * D) {
		if (index < numberOfKeys) {
			moveToLeft(addressOfPage, index);
		}

		BTreeBuffer->setPosition(addressOfPage + 12 + 12 * index);
		unsigned char temp[8];
		memcpy(temp, &key, 4);
		memcpy(&temp[4], &addressInMainFile, 4);
		for (int i = 0; i < 8; i++) {
			BTreeBuffer->write(temp[i]);
		}

		int numberOfKeys = getNumberOfKeys(addressOfPage);
		numberOfKeys++;
		memcpy(temp, &numberOfKeys, 4);
		BTreeBuffer->setPosition(addressOfPage);
		for (int i = 0; i < 4; i++) {
			BTreeBuffer->write(temp[i]);
		}
	}
	else {
		int temp[3];
		temp[0] = key;
		temp[1] = addressInMainFile;
		temp[2] = -1;

		while (true) {
			if (compensate(addressOfPage, temp)) {
				break;
			}
			int parentKeys = getNumberOfKeys(0);
			split(addressOfPage, temp);
			parentKeys = getNumberOfKeys(0);
			if (addressOfPage != 0) {
				addressOfPage = getParent(addressOfPage);
				int parentKeys = getNumberOfKeys(addressOfPage);
				if (parentKeys < 2 * D) {
					int index = findPlaceOnPage(addressOfPage, temp[0]);
					moveToLeft(addressOfPage, index);
					insertEntryToBTree(temp, addressOfPage + 12 + 12 * index);
					readEntryFromBTree(temp, addressOfPage);
					//Increase the number of keys
					temp[0]++;
					insertEntryToBTree(temp, addressOfPage);
					break;
				}
			}
			else {
				insertEntryToBTree(temp, addressOfPage + 12);
				readEntryFromBTree(temp, addressOfPage);
				temp[0]++;
				insertEntryToBTree(temp, addressOfPage);
				break;
			}
		}
	}

	std::cout << "\n\nRead operations: " << BTreeBuffer->getReadOperations() << "\nWrite operations: " << BTreeBuffer->getWriteOperations() << "\n\n";
	return true;
}


bool BTree::searchForEntry(int key, int *addressOfPage, int *index) {
	int entry[3];
	int addressOfChild = 0;
	bool foundEntry = false;
	while (addressOfChild != -1) {
		*addressOfPage = addressOfChild;

		*index = findPlaceOnPage(addressOfChild, key);
		//If found place is inside page
		if (*index < getNumberOfKeys(addressOfChild)) {
			readEntryFromBTree(entry, addressOfChild + 12 + 12 * *index);
			//If returned key is equal to key to insert -> return false
			if (entry[0] == key) {
				return true;
			}
		}
		//If returned key is diffrent from key to insert -> read address of left parent
		BTreeBuffer->setPosition(addressOfChild + 12 + 12 * *index - 4);
		char temp[4];
		for (int i = 0; i < 4; i++) {
			BTreeBuffer->read(&temp[i]);
		}
		memcpy(&addressOfChild, temp, 4);
	}
	return false;
}


void BTree::merge(int addressOfPage) {
	if (addressOfPage == 0 && getNumberOfKeys(addressOfPage) > 0) {
		return;
	}
	else if (addressOfPage == 0 && getNumberOfKeys(addressOfPage) == 0) {
		int tmp[3];
		readEntryFromBTree(tmp, addressOfPage);
		int childAddress = tmp[2];

		if (childAddress == -1) {
			return;
		}

		for (int i = 0; i <= 2 * D; i++) {
			readEntryFromBTree(tmp, childAddress + 12 * i);
			if (i == 0) {
				tmp[1] = -1;
			}
			insertEntryToBTree(tmp, addressOfPage + 12 * i);
		}
		managePlace(childAddress, true, false);
		setChildren(addressOfPage);
		return;
	}
	int addressOfParent = getParent(addressOfPage);
	int indexInParent = searchByPointer(addressOfPage);
	int temp[3];
	int numberOfKeys = getNumberOfKeys(addressOfPage);
	bool selectedLeft = false;
	if (indexInParent > 0) {
		readEntryFromBTree(temp, addressOfParent + 12 * (indexInParent - 1));
		int keysInLeft = getNumberOfKeys(temp[2]);
		if (keysInLeft + numberOfKeys < 2 * D && keysInLeft + numberOfKeys >= D - 1) {
			selectedLeft = true;
		}
	}
	int addressOfLeft;
	int addressOfRight;
	int keysInLeft;
	int keysInRight;
	int separatorAddr;
	int separator[3];
	int separatorIndexInParent;

	if (selectedLeft) {
		readEntryFromBTree(temp, addressOfParent + 12 * (indexInParent - 1));
		addressOfLeft = temp[2];
		addressOfRight = addressOfPage;
		separatorAddr = addressOfParent + 12 * indexInParent;
		separatorIndexInParent = indexInParent;
	}
	else {
		readEntryFromBTree(temp, addressOfParent + 12 * (indexInParent + 1));
		addressOfLeft = addressOfPage;
		addressOfRight = temp[2];
		separatorAddr = addressOfParent + 12 * (indexInParent + 1);
		separatorIndexInParent = indexInParent + 1;
	}
	keysInLeft = getNumberOfKeys(addressOfLeft);
	keysInRight = getNumberOfKeys(addressOfRight);
	readEntryFromBTree(separator, separatorAddr);

	readEntryFromBTree(temp, addressOfRight);
	separator[2] = temp[2];

	insertEntryToBTree(separator, addressOfLeft + 12 + 12 * keysInLeft);
	keysInLeft++;

	for (int i = 0; i < keysInRight; i++) {
		readEntryFromBTree(temp, addressOfRight + 12 + 12 * i);
		insertEntryToBTree(temp, addressOfLeft + 12 + 12 * (i + keysInLeft));
	}
	keysInLeft += keysInRight;
	setNumberOfKeys(addressOfLeft, keysInLeft);

	moveToRight(addressOfParent, separatorIndexInParent - 1);

	int keysInParent = getNumberOfKeys(addressOfParent);
	setNumberOfKeys(addressOfParent, keysInParent - 1);

	managePlace(addressOfRight, true, false);

	setChildren(addressOfLeft);
}


bool BTree::deleteEntry(int key) {
	BTreeBuffer->resetStats();
	int addressOfPage;
	int index;
	if (!searchForEntry(key, &addressOfPage, &index)) {
		std::cout << "\n\nRead operations: " << BTreeBuffer->getReadOperations() << "\nWrite operations: " << BTreeBuffer->getWriteOperations() << "\n\n";
		return false;
	}
	int entry[3];
	readEntryFromBTree(entry, addressOfPage + 12 + 12 * index);
	if (entry[2] == -1) {
		moveToRight(addressOfPage, index);
		int keys = getNumberOfKeys(addressOfPage);
		keys--;
		setNumberOfKeys(addressOfPage, keys);
	}
	else {
		//Look for the separator
		int tmp[3];
		readEntryFromBTree(tmp, addressOfPage + 12 + 12 * (index - 1));
		int addr;
		while (tmp[2] != -1) {
			int key = getNumberOfKeys(tmp[2]);
			addr = tmp[2];
			readEntryFromBTree(tmp, tmp[2] + 12 + 12 * (key - 1));
		}
		tmp[2] = entry[2];
		insertEntryToBTree(tmp, addressOfPage + 12 + 12 * index);
		tmp[0] = -1;
		tmp[1] = -1;
		tmp[2] = -1;
		insertEntryToBTree(tmp, addr + 12 + 12 * (getNumberOfKeys(addr) - 1));
		setNumberOfKeys(addr, getNumberOfKeys(addr) - 1);
		addressOfPage = addr;
	}
	int tmp[3];
	tmp[0] = -1;
	tmp[1] = -1;
	tmp[2] = -1;
	while (getNumberOfKeys(addressOfPage) < D) {
		if (!compensate(addressOfPage, tmp)) {
			int parAddr = getParent(addressOfPage);
			merge(addressOfPage);
			addressOfPage = parAddr;
		}
	}

	managePlace(entry[1], false, false);

	std::cout << "\n\nRead operations: " << BTreeBuffer->getReadOperations() << "\nWrite operations: " << BTreeBuffer->getWriteOperations() << "\n\n";

	return true;
}


void BTree::outPage(int address) {
	std::cout << "address: " << address << '\n';
	BTreeBuffer->setPosition(address);
	int numberOfKeys = getNumberOfKeys(address);
	int parent = getParent(address);
	int tempInt[3];
	char tempChar[12];
	for (int i = 0; i < 4; i++) {
		BTreeBuffer->read(&tempChar[i]);
	}
	memcpy(tempInt, tempChar, 4);
	if (parent != -1) {
		std::cout << "Number of keys: " << numberOfKeys << ", parent: " << parent << ", child: ";
	}
	else {
		std::cout << "Number of keys: " << numberOfKeys << ", root" << ", child: ";
	}
	if (tempInt[0] != -1) {
		std::cout << tempInt[0] << '\n';
	}
	else {
		std::cout << "empty\n";
	}
	for (int i = 0; i < 2 * D; i++) {
		for (int j = 0; j < 12; j++) {
			BTreeBuffer->read(&tempChar[j]);
		}
		memcpy(tempInt, tempChar, 12);
		if (tempInt[0] != -1) {
			std::cout << "Key: " << tempInt[0] << ", address in the file: " << tempInt[1] << ", child: ";
			if (tempInt[2] != -1) {
				std::cout << tempInt[2] << '\n';
			}
			else {
				std::cout << "empty\n";
			}
		}
	}
	std::cout << "\n\n";
}


bool BTree::search(int key, double *dataFromMainFile) {
	BTreeBuffer->resetStats();
	int addressOfPage;
	int index;
	if (!searchForEntry(key, &addressOfPage, &index)) {
		return false;
	}
	int entry[3];
	readEntryFromBTree(entry, addressOfPage + 12 + 12 * index);
	readFromMainFile(entry[1], dataFromMainFile);
	std::cout << "\n\nRead operations: " << BTreeBuffer->getReadOperations() << "\nWrite operations: " << BTreeBuffer->getWriteOperations() << "\n\n";
	return true;
}


void BTree::readFromMainFile(int addressInMainFile, double *destination) {
	MainFileBuffer->setPosition(addressInMainFile);
	char temp[24];
	for (int i = 0; i < 24; i++) {
		MainFileBuffer->read(&temp[i]);
	}
	memcpy(destination, temp, 24);
}


bool BTree::modify(int key, double a, double b, double c) {
	BTreeBuffer->resetStats();
	int addressOfPage;
	int index;
	if (!searchForEntry(key, &addressOfPage, &index)) {
		std::cout << "\n\nRead operations: " << BTreeBuffer->getReadOperations() << "\nWrite operations: " << BTreeBuffer->getWriteOperations() << "\n\n";
		return false;
	}
	int entry[3];
	readEntryFromBTree(entry, addressOfPage + 12 + 12 * index);
	writeToMainFile(entry[1], a, b, c);
	std::cout << "\n\nRead operations: " << BTreeBuffer->getReadOperations() << "\nWrite operations: " << BTreeBuffer->getWriteOperations() << "\n\n";
	return true;
}


int BTree::checkAddressInBitMap(int address) {
	MyBuffer *buff;
	buff = BTreeBitMapBuffer;

	int index = 0;
	buff->setPosition(address / (8 * sizeOfPage));
	char bitsToCheck;
	unsigned char mask;
	unsigned char result;

	if (!buff->read(&bitsToCheck)) {
		return -1;
	}
	mask = 128;
	mask >>= (address / sizeOfPage) % 8;

	result = ~bitsToCheck & mask;
	if (result == mask) {
		return 0;
	}
	else {
		return 1;
	}
}


void BTree::printTree() {
	int check = 0;
	int addr = 0;
	while (check != -1) {
		check = checkAddressInBitMap(addr);
		if (check == 1) {
			outPage(addr);
		}
		addr += sizeOfPage;
	}
}


void BTree::printPage(int address) {
	int entry[3];
	BTreeBuffer->setPosition(address);
	readEntryFromBTree(entry, address);
	int keys = getNumberOfKeys(address);
	double rec[3];
	if (entry[2] != -1) {
		printPage(entry[2]);
		for (int i = 0; i < keys; i++) {
			readEntryFromBTree(entry, address + 12 + 12 * i);
			readFromMainFile(entry[1], rec);
			std::cout << entry[0] << " | " << rec[0] << ' ' << rec[1] << ' ' << rec[2] << " || ";
			printPage(entry[2]);
		}
	}
	else {
		for (int i = 0; i < keys; i++) {
			readEntryFromBTree(entry, address + 12 + 12 * i);
			readFromMainFile(entry[1], rec);
			std::cout << entry[0] << " | " << rec[0] << ' ' << rec[1] << ' ' << rec[2] << " || ";
		}
	}
}


void BTree::printMainFile() {
	BTreeBuffer->resetStats();
	std::cout << "\n\n";
	printPage(0);
	std::cout << "\n\n";
	std::cout << "\n\nRead operations: " << BTreeBuffer->getReadOperations() << "\nWrite operations: " << BTreeBuffer->getWriteOperations() << "\n\n";
}


void BTree::dumpMainFile() {
	char temp[8];
	double var;
	MainFileBuffer->setPosition(0);
	bool con = true;

	while (con) {
		for (int i = 0; i < 8; i++) {
			if (!MainFileBuffer->read(&temp[i])) {
				con = false;
				break;
			}
		}
		if (con) {
			memcpy(&var, temp, 8);
			std::cout << var << "  ";
		}
	}
}


void BTree::dumpIndex() {
	char temp[4];
	int var;
	BTreeBuffer->setPosition(0);
	bool con = true;

	while (con) {
		for (int i = 0; i < 4; i++) {
			if (!BTreeBuffer->read(&temp[i])) {
				con = false;
				break;
			}
		}
		if (con) {
			memcpy(&var, temp, 4);
			std::cout << var << "  ";
		}
	}
}