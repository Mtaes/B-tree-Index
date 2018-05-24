#pragma once

#include <string>

class MyBuffer {
	std::string pathToFile;
	int numberOfBuffers;
	int sizeOfBuffer;
	char **tableOfBuffers;
	bool *buffersToWrite;
	int *relevantBytes;
	//Addresses (in file) of pages in buffer
	//Address -1 means that buffer is empty
	int *addressesOfPagesInBuffers;
	int currentAddressInFile;
	int writeOperations;
	int readOperations;

	bool writeToFile(int indexOfBuffer);
	bool readFromFile(int indexOfBuffer);
	//Returns index of buffer that can be used
	int getBuffer();
	int numberOfCurrentBuffer;

public:
	MyBuffer(std::string _pathToFile, int _numberOfBuffers, int _sizeOfBuffer);
	~MyBuffer();
	bool read(char *destination);
	bool write(char source);
	void setPosition(int position);
	int getWriteOperations();
	int getReadOperations();
	void resetStats();
	void flush();
	bool checkIfFileExists();
};