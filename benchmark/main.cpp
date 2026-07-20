#include "application.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  return benchmarking::runApplication(argc, argv, std::cout, std::cerr);
}
