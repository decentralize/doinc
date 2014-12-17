#include <iostream>
#include <boost/asio.hpp>

#include "protocol/packet.pb.h"

using boost::asio::ip::udp;

void server(boost::asio::io_service& io_service, unsigned short port) {
  udp::socket sock(io_service, udp::endpoint(udp::v4(), port));
  for(;;) {
    char data[1024];

    udp::endpoint sender_endpoint;
    sock.receive_from(
        boost::asio::buffer(data, 1024), sender_endpoint);

    protocol::Packet in_p;
    in_p.ParseFromString(data);
    std::cout << "Got packet: " << in_p.code() << std::endl;
    std::cout << "From: " << sender_endpoint << std::endl;

    protocol::Packet out_p;
    std::string data_string;
    out_p.set_code(protocol::Packet::OK);

    std::ostringstream stream;
    stream << sender_endpoint;
    out_p.set_data(stream.str());

    out_p.set_crc(0);
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
    server(io_service, 9000);
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
