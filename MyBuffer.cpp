#include <fstream>
#include "MyBuffer.h"

using namespace std;


MyBuffer::MyBuffer(string _pathToFile, int _numberOfBuffers, int _sizeOfBuffer) {
	pathToFile = _pathToFile;
	numberOfBuffers = _numberOfBuffers;
	sizeOfBuffer = _sizeOfBuffer;
	currentAddressInFile = 0;
	writeOperations = 0;
	readOperations = 0;
	numberOfCurrentBuffer = 0;
	tableOfBuffers = new char*[numberOfBuffers];
	buffersToWrite = new bool[numberOfBuffers];
	relevantBytes = new int[numberOfBuffers];
	addressesOfPagesInBuffers = new int[numberOfBuffers];
	for (int i = 0; i < numberOfBuffers; i++) {
		tableOfBuffers[i] = new char[sizeOfBuffer];
		buffersToWrite[i] = false;
		relevantBytes[i] = 0;
		addressesOfPagesInBuffers[i] = -1;
	}
}


MyBuffer::~MyBuffer() {
	for (int i = 0; i < numberOfBuffers; i++) {
		writeToFile(i);
		delete[] tableOfBuffers[i];
	}
	delete[] tableOfBuffers;
	delete[] buffersToWrite;
	delete[] relevantBytes;
	delete[] addressesOfPagesInBuffers;
}


bool MyBuffer::writeToFile(int indexOfBuffer) {
	if (!buffersToWrite[indexOfBuffer]) {
		return true;
	}
	FILE *file;
	file = fopen(pathToFile.c_str(), "rb+");
	if (file == NULL) {
		file = fopen(pathToFile.c_str(), "wb+");
	}
	fseek(file, addressesOfPagesInBuffers[indexOfBuffer], SEEK_SET);
	fwrite(tableOfBuffers[indexOfBuffer], 1, relevantBytes[indexOfBuffer], file);
	fclose(file);
	buffersToWrite[indexOfBuffer] = false;
	writeOperations++;
	return true;
}


bool MyBuffer::readFromFile(int indexOfBuffer) {
	FILE *file;
	file = fopen(pathToFile.c_str(), "rb+");
	if (file == NULL) {
		file = fopen(pathToFile.c_str(), "wb+");
	}
	fseek(file, (currentAddressInFile / sizeOfBuffer) * sizeOfBuffer, SEEK_SET);
	relevantBytes[indexOfBuffer] = fread(tableOfBuffers[indexOfBuffer], 1, sizeOfBuffer, file);
	fclose(file);
	buffersToWrite[indexOfBuffer] = false;
	addressesOfPagesInBuffers[indexOfBuffer] = (currentAddressInFile / sizeOfBuffer) * sizeOfBuffer;
	readOperations++;
	return true;
}


int MyBuffer::getBuffer() {
	bool inBuffer = false;
	int indexOfBuffer;
	int addressOfPage;
	for (indexOfBuffer = 0; indexOfBuffer < numberOfBuffers; indexOfBuffer++) {
		addressOfPage = addressesOfPagesInBuffers[indexOfBuffer];
		if (addressOfPage != -1 && addressOfPage <= currentAddressInFile && currentAddressInFile < addressOfPage + sizeOfBuffer) {
			inBuffer = true;
			break;
		}
	}

	if (inBuffer == false) {
		writeToFile(numberOfCurrentBuffer);
		indexOfBuffer = numberOfCurrentBuffer;
		readFromFile(indexOfBuffer);
		numberOfCurrentBuffer = (numberOfCurrentBuffer + 1) % numberOfBuffers;
	}

	return indexOfBuffer;
}


bool MyBuffer::read(char *destination) {
	int i = getBuffer();
	int addressInBuffer = currentAddressInFile - addressesOfPagesInBuffers[i]; 
	if (addressInBuffer >= relevantBytes[i]) {
		return false;
	}
	memcpy(destination, &tableOfBuffers[i][addressInBuffer], 1);
	currentAddressInFile++;
	return true;
}


bool MyBuffer::write(char source) {
	int i = getBuffer();
	int addressInBuffer = currentAddressInFile - addressesOfPagesInBuffers[i];
	if (addressInBuffer > relevantBytes[i]) {
		return false;
	}
	else if (addressInBuffer == relevantBytes[i]) {
		relevantBytes[i]++;
	}
	memcpy(&tableOfBuffers[i][addressInBuffer], &source, 1);
	currentAddressInFile++;
	buffersToWrite[i] = true;
	return true;
}


void MyBuffer::setPosition(int position) {
	currentAddressInFile = position;
}


int MyBuffer::getWriteOperations() {
	return writeOperations;
}


int MyBuffer::getReadOperations() {
	return readOperations;
}


void MyBuffer::resetStats() {
	writeOperations = 0;
	readOperations = 0;
}


void MyBuffer::flush() {
	for (int i = 0; i < numberOfBuffers; i++) {
		writeToFile(i);
	}
}


bool MyBuffer::checkIfFileExists() {
	if (FILE *file = fopen(pathToFile.c_str(), "r")) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}