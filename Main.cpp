#include "Controller.h"


int main() {
	Controller *con = new Controller;
	con->userInput();
	delete con;
	return 0;
}