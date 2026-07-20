#include "application.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  return runApplication(argc, argv, std::cout, std::cerr);
}
