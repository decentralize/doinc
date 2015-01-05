#include <iostream>
#include <boost/asio.hpp>

#include "protocol/packet.pb.h"
#include "utils.h"

using boost::asio::ip::udp;

void nameserver(boost::asio::io_service& io_service, unsigned short port) {
  udp::socket sock(io_service, udp::endpoint(udp::v4(), port));

  // TODO:
  //   list of endpoint/timestamp pair
  //   register packets adds or refreshes the endpoint to the list
  //   ask for node yields a random node from the list
  //   cleanup (remove old) is run when a client asks for a node, before one is selected

  for(;;) {
    char data[1024];

    udp::endpoint sender_endpoint;
    sock.receive_from(
        boost::asio::buffer(data, 1024), sender_endpoint);

    protocol::Packet in_p;
    in_p.ParseFromString(data);
    std::cout << "Got packet: " << in_p.code() << std::endl;
    std::cout << "From: " << sender_endpoint << std::endl;

    if(!checkCRC(in_p)) {
      std::cerr << "CRC mismatch!" << std::endl;
      exit(1);
      return;
    }

    protocol::Packet out_p;
    std::string data_string;
    out_p.set_code(protocol::Packet::OK);

    std::ostringstream stream;
    stream << sender_endpoint;
    out_p.set_data(stream.str());

    makeCRC(out_p);
    out_p.SerializeToString(&data_string);

    strncpy(data, data_string.c_str(), sizeof(data));
    // Null terminate for safety
    data[sizeof(data) - 1] = 0;

    sock.send_to(boost::asio::buffer(data, data_string.length()), sender_endpoint);
  }
}

int main(int argc, char* argv[]){
  try {
    boost::asio::io_service io_service;
    nameserver(io_service, 9000);
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
