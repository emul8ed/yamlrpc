#include "yamlrpc.h"

#include <iostream>
#include <string>
#include <vector>

using namespace yamlrpc;

void Skel::theMethod(std::vector<std::string> const &Vec) {
  std::cout << "Address of tuple member: " << std::hex << &Vec << "\n";
  std::cout << "Address of string: " << std::hex << &Vec[0] << "\n";
  for (auto const &Str : Vec) {
    std::cout << Str << "\n";
  }
}

auto Skel::returningMethod(std::vector<std::string> const &Vec) -> std::string {
  std::cout << "Returning method called\n";
  for (auto const &Str : Vec) {
    std::cout << Str << "\n";
  }
  return std::string("Item 0: " + Vec[0]);
}

auto Skel::sortVector(std::vector<std::string> const &Vec)
    -> std::vector<std::string> {
  std::vector<std::string> Ret = Vec;
  std::sort(Ret.begin(), Ret.end());
  return Ret;
}