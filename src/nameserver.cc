#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

void server(boost::asio::io_service& io_service, unsigned short port) {
  udp::socket sock(io_service, udp::endpoint(udp::v4(), port));
  for(;;) {
    char data[1024];
    udp::endpoint sender_endpoint;
    size_t length = sock.receive_from(
        boost::asio::buffer(data, 1024), sender_endpoint);
    std::cout << sender_endpoint << std::endl;
    sock.send_to(boost::asio::buffer(data, length), sender_endpoint);
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
