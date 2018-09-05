#include "utils.h"

namespace tikv {

std::string to_hex(const std::string& data) {
  std::stringstream ss;
  ss << std::hex << "[";
  for(int i=0;i<data.size();++i)
    ss << std::setw(2) << std::showbase << std::setfill('0') << (unsigned int)(data.c_str()[i]) << " ";
  ss << "]";
  return ss.str();
}

std::string strip_url(const std::string& addr) {
  std::string stripped_address;
  std::string substr("://");
  std::string::size_type i = addr.find(substr);
  if(i != std::string::npos) {
    stripped_address = addr.substr(i+substr.length());
  }
  return stripped_address;
}

}
