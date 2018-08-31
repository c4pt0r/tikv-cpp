#pragma once
#include <string>
#include <random>
#include <iterator>

namespace tikv {

std::string strip_url(const std::string& addr) {
  std::string stripped_address;
  std::string substr("://");
  std::string::size_type i = addr.find(substr);
  if(i != std::string::npos) {
    stripped_address = addr.substr(i+substr.length());
  }
  return stripped_address;
}

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

}