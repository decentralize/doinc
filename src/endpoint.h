#ifndef __ENDPOINT_H_
#define __ENDPOINT_H_

#include <string>
using std::string;

#include <iostream>
using std::ostringstream;

#include <boost/asio.hpp>
using boost::asio::ip::udp;

class Endpoint {
private:
  const udp::endpoint ep;

public:
  Endpoint(udp::endpoint e) : ep(e) {}
  string tostring() const {
    ostringstream oss;
    oss << ep;
    return oss.str();
  }
  bool operator==(const Endpoint e) const {
    return tostring() == e.tostring();
  }
  udp::endpoint get() const {
    return ep;
  }
};

namespace std {
  template <>
  struct hash<Endpoint> {
    std::size_t operator()(const Endpoint& k) const {
      return hash<string>()(k.tostring());
    }
  };
}

#endif
