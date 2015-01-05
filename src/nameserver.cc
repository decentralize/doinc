#include <iostream>
#include <boost/asio.hpp>
#include <chrono>
#include <cstdlib>

#include "protocol/packet.pb.h"
#include "utils.h"

using boost::asio::ip::udp;

void nameserver(boost::asio::io_service& io_service, unsigned short port) {
  udp::socket sock(io_service, udp::endpoint(udp::v4(), port));

  // TODO:
  //   list of endpoint/timestamp pair: DONE
  //   register packets adds or refreshes the endpoint to the list: DONE
  //   ask for node yields a random node from the list: DONE
  //   cleanup (remove old) is run when a client asks for a node, before one is selected: DONE, but not properly tested

  std::map<udp::endpoint, std::chrono::milliseconds> endpoints;

  for(;;) {
    char data[1024];

    udp::endpoint sender_endpoint;
    sock.receive_from(
        boost::asio::buffer(data, 1024), sender_endpoint);

    std::cout << std::endl;

    protocol::Packet in_p;
    in_p.ParseFromString(data);
    std::cout << "Got packet: " << in_p.code() << std::endl;
    std::cout << "CRC: " << in_p.crc() << std::endl;
    std::cout << "Calculated CRC: " << calculateCRC(in_p) << std::endl;
    std::cout << "From: " << sender_endpoint << std::endl;

    if(!checkCRC(in_p)) {
      std::cerr << "CRC mismatch!" << std::endl;
      exit(1);
      return;
    }

    // Construct packet to send back
    protocol::Packet out_p;

    // Stream used to construct return packet
    std::ostringstream stream;

    // Check which message was received
    // NS_REGISTER = 2
    if(in_p.code() == protocol::Packet::NS_REGISTER){

    // Update endpoint/timestamp for particular sender_endpoint
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
    endpoints[sender_endpoint] = ms;

    std::cout << "This sender: " << sender_endpoint << std::endl;
    std::cout << "Was updated with this timestamp: " << ms.count() << std::endl;

    // Should not send endpoint data, fix later.
    stream << sender_endpoint;
    out_p.set_data(stream.str());
    out_p.set_code(protocol::Packet::OK);

    // NS_REQUEST_NODE = 3
    } else if (in_p.code() == protocol::Packet::NS_REQUEST_NODE){
      if(endpoints.empty()){

        std::cout << "No nodes registered" << std::endl;
        out_p.set_code(protocol::Packet::ERROR);
      } else {
        // Perform cleanup of nodes that have not registered in 30 minutes (1 800 000 milliseconds)
        std::map<udp::endpoint, std::chrono::milliseconds>::iterator it;
        for(it = endpoints.begin(); it != endpoints.end(); it++){
          std::chrono::milliseconds now_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());

          if((now_in_ms - it->second).count() >= 1800000){
            endpoints.erase(it->first);
          }
        }


        // Choose a random node from the list to send back
        it = endpoints.begin();
        std::advance(it, rand() % endpoints.size());
        udp::endpoint random_endpoint = it->first;

        // TODO: Should also send project id according to spec
        std::cout << "Random endpoint: " << random_endpoint << std::endl;
        stream << random_endpoint;
        out_p.set_data(stream.str());
        out_p.set_code(protocol::Packet::NODE);
      }
    }

    //out_p.set_code(protocol::Packet::OK);

    // std::ostringstream stream;
    // stream << sender_endpoint;
    // out_p.set_data(stream.str());

    std::string data_string;

    makeCRC(out_p);
    out_p.SerializeToString(&data_string);

    strncpy(data, data_string.c_str(), sizeof(data));
    // Null terminate, for safety
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
