#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <unordered_map>
using std::unordered_map;

#include <boost/asio.hpp>
using boost::asio::ip::udp;

#include <chrono>
using std::chrono::milliseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

#include "protocol/packet.pb.h"
#include "utils.h"
#include "endpoint.h"

void nameserver(boost::asio::io_service& io_service, unsigned short port) {
  udp::socket sock(io_service, udp::endpoint(udp::v4(), port));

  // TODO:
  //   list of endpoint/timestamp pair: DONE
  //   register packets adds or refreshes the endpoint to the list: DONE
  //   ask for node yields a random node from the list: DONE
  //   cleanup (remove old) is run when a client asks for a node, before one is selected: DONE, but not properly tested

  unordered_map<Endpoint, milliseconds> endpoints;

  for(;;) {
    char data[1024];

    udp::endpoint sender_endpoint;
    sock.receive_from(
        boost::asio::buffer(data, 1024), sender_endpoint);

    protocol::Packet in_p;
    in_p.ParseFromString(data);
    cout << "Got packet: " << in_p.code();
    cout << ", from: " << sender_endpoint << endl;

    if(!checkCRC(in_p)) {
      std::cerr << "CRC mismatch!" << endl;
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
      cout << "Total nodes: " << endpoints.size() << endl;
      milliseconds timestamp = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
      endpoints[Endpoint(sender_endpoint)] = timestamp;

      cout << "This sender: " << sender_endpoint << endl;
      cout << "Was updated with this timestamp: " << timestamp.count() << endl;
      cout << "Total nodes: " << endpoints.size() << endl;

      protocol::Node node;
      node.set_addr(sender_endpoint.address().to_string());
      node.set_port(sender_endpoint.port());
      std::string node_string;
      node.SerializeToString(&node_string);

      out_p.set_data(node_string);
      out_p.set_code(protocol::Packet::OK);

    // NS_REQUEST_NODE = 3
    } else if (in_p.code() == protocol::Packet::NS_REQUEST_NODE){
      if(endpoints.empty()){
        cout << "No nodes registered" << endl;
        out_p.set_code(protocol::Packet::ERROR);
      } else {
        // Perform cleanup of nodes that have not registered in 30 minutes (1 800 000 milliseconds)
        unordered_map<Endpoint, milliseconds>::iterator it;
        for(it = endpoints.begin(); it != endpoints.end(); it++){
          milliseconds now_in_ms = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());

          if((now_in_ms - it->second).count() >= 1800000){
            endpoints.erase(it->first);
          }
        }

        // If sender exists in nodelist, temporarily remove it so that
        // it's not returned to itself when a request is made
        bool exists_in_list = false;
        if(endpoints.count(Endpoint(sender_endpoint)) > 0){
          exists_in_list = true;
          endpoints.erase(Endpoint(sender_endpoint));
        }

        // Make sure that if the sender endpoints was the only one
        // registered the programs does not try to iterate over an empty list
        if(endpoints.empty()){
          cout << "No other nodes than sender registered" << endl;
          out_p.set_code(protocol::Packet::ERROR);
        } else {

          // Choose a random node from the list to send back
          it = endpoints.begin();
          std::advance(it, rand() % endpoints.size());
          Endpoint random_endpoint = it->first;

          cout << "Random endpoint: " << random_endpoint.tostring() << endl;
          protocol::Node node;
          node.set_addr(random_endpoint.get().address().to_string());
          node.set_port(random_endpoint.get().port());
          std::string node_string;
          node.SerializeToString(&node_string);
          out_p.set_data(node_string);
          out_p.set_code(protocol::Packet::NODE);
        }

        // If a node was temporarily removed, put it back with a new timestamp
        if(exists_in_list){
          milliseconds timestamp = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
          endpoints[Endpoint(sender_endpoint)] = timestamp;
        }
      }
    }

    std::string data_string;

    makeCRC(out_p);
    out_p.SerializeToString(&data_string);

    sock.send_to(boost::asio::buffer(data_string.data(), data_string.length()), sender_endpoint);
  }
}

int main(int argc, char* argv[]){
  try {
    boost::asio::io_service io_service;
    nameserver(io_service, 9000);
  } catch (std::exception& e) {
    cerr << "Exception: " << e.what() << endl;
  }

  return 0;
}
