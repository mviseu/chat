#include "Connect.h"
#include <iostream>


int main() {
	try {
	 	std::cout << "hello" << std::endl;
		Connect();
		return 0;
	} catch(...) {
		throw;
	}
}