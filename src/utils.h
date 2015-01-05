#ifndef __UTILS_H_
#define __UTILS_H_

#include "protocol/packet.pb.h"

#include <string>

::google::protobuf::int64 calculateCRC(protocol::Packet &p); // TODO private
void makeCRC(protocol::Packet &p);
bool checkCRC(protocol::Packet &p);

protocol::Packet createPacket(protocol::Packet::FunctionCode code);
protocol::Packet createPacket(protocol::Packet::FunctionCode code, std::string payload);

#endif
