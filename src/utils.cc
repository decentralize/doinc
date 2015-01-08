#include "utils.h"
#include "protocol/packet.pb.h"

#include <string>
using std::string;

#include <google/protobuf/stubs/common.h>
#include <boost/crc.hpp>


::google::protobuf::int64 calculateCRC(protocol::Packet &p) {
  boost::crc_32_type result;
  result.process_bytes(p.data().data(), p.data().length());
  return result.checksum();
}

void makeCRC(protocol::Packet &p) {
  p.set_crc(calculateCRC(p));
}
bool checkCRC(protocol::Packet &p) {
  return p.crc() == calculateCRC(p);
}

protocol::Packet createPacket(protocol::Packet::FunctionCode code) {
  return createPacket(code, string());
}

protocol::Packet createPacket(protocol::Packet::FunctionCode code, string payload) {
  protocol::Packet p;
  p.set_code(code);
  p.set_data(payload);
  makeCRC(p);
  return p;
}
