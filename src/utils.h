#ifndef __UTILS_H_
#define __UTILS_H_

#include "protocol/packet.pb.h"

void makeCRC(protocol::Packet &p);
bool checkCRC(protocol::Packet &p);

#endif
