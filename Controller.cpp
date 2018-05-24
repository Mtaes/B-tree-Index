#include "Controller.h"
#include <iostream>
#include <fstream>

using namespace std;

Controller::Controller() {
	tree = new BTree(2);
}


Controller::~Controller() {
	delete tree;
}


void Controller::instructions() {
	cout << "\na - add record\n";
	cout << "d - delete record\n";
	cout << "s - search for a record\n";
	cout << "m - modify record\n";
	cout << "t - load test file\n";
	cout << "o - print main file\n";
	cout << "v - print files\n";
	cout << "q - quit\n\n";
}


void Controller::userInput() {
	int mode = 0;
	char in = 'a';
	int key;
	double rec[3];
	while (in != 'q') {
		instructions();
		tree->printTree();
		cin >> in;
		switch (in) {
		case 'a':
			cout << "Enter key to be inserted.\n";
			cin >> key;
			if (key < 1) {
				cout << "Wrong key.\n";
				break;
			}
			cout << "Enter three values to be inserted:\n";
			double a, b, c;
			cout << "a: ";
			cin >> a;
			cout << "b: ";
			cin >> b;
			cout << "c: ";
			cin >> c;
			if (!tree->addNewEntryToBTree(key, a, b, c))
				cout << "The key already exists!\n";
			break;
		case 'd':
			cout << "Enter key to be deleted.\n";
			key;
			cin >> key;
			if (key < 1) {
				cout << "Wrong key.\n";
				break;
			}
			if (!tree->deleteEntry(key))
				cout << "The key does not exist!\n";
			break;
		case 's':
			cout << "Enter key to be searched.\n";
			key;
			cin >> key;
			if (key < 1) {
				cout << "Wrong key.\n";
				break;
			}
			if (!tree->search(key, rec))
				cout << "The key does not exist!\n";
			else
				cout << rec[0] << " " << rec[1] << " " << rec[2] << "\n";
			break;
		case 'm':
			cout << "Enter key to be modified.\n";
			key;
			cin >> key;
			if (key < 1) {
				cout << "Wrong key.\n";
				break;
			}
			cout << "Enter three values:\n";
			cout << "a: ";
			cin >> rec[0];
			cout << "b: ";
			cin >> rec[1];
			cout << "c: ";
			cin >> rec[2];
			if (!tree->modify(key, rec[0], rec[1], rec[2]))
				cout << "The key does not exist!\n";
			else
				cout << "OK\n";
			break;
		case 't':
			test();
			break;
		case 'o':
			tree->printMainFile();
			break;
		case 'v':
			cout << "\n\n index file:\n";
			tree->dumpIndex();
			cout << "\n\n main file:\n";
			tree->dumpMainFile();
			cout << "\n\n";
			break;
		default:
			break;
		}
	}
}


void Controller::test() {
	char com;
	int key;
	double rec[3];
	fstream fin("test.txt", fstream::in);
	while (fin >> com) {
		switch (com) {
		case 'a':
			fin >> key;
			if (key < 1) {
				cout << "Wrong key.\n";
				break;
			}
			double a, b, c;
			fin >> a;
			fin >> b;
			fin >> c;
			if (!tree->addNewEntryToBTree(key, a, b, c))
				cout << "The key already exists!\n";
			break;
		case 'd':
			fin >> key;
			if (key < 1) {
				cout << "Wrong key.\n";
				break;
			}
			if (!tree->deleteEntry(key))
				cout << "The key does not exist!\n";
			break;
		case 's':
			fin >> key;
			if (key < 1) {
				cout << "Wrong key.\n";
				break;
			}
			if (!tree->search(key, rec))
				cout << "The key does not exist!\n";
			else
				cout << rec[0] << " " << rec[1] << " " << rec[2] << "\n";
			break;
		case 'm':
			fin >> key;
			if (key < 1) {
				cout << "Wrong key.\n";
				break;
			}
			fin >> rec[0];
			fin >> rec[1];
			fin >> rec[2];
			if (!tree->modify(key, rec[0], rec[1], rec[2]))
				cout << "The key does not exist!\n";
			else
				cout << "OK\n";
			break;
		default:
			break;
		}
	}
	fin.close();
}