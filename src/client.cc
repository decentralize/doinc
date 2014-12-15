#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;
using boost::asio::ip::address;

int main(int argc, char* argv[]) {
  try {
    boost::asio::io_service io_service;

    udp::endpoint receiver_endpoint = udp::endpoint(address::from_string("130.239.40.31"), 9000);

    udp::socket socket(io_service);
    socket.open(udp::v4());

    boost::array<char, 255> send_buf  = { "hej idiots" };
    socket.send_to(boost::asio::buffer(send_buf), receiver_endpoint);

    boost::array<char, 255> recv_buf;
    udp::endpoint sender_endpoint;
    size_t len = socket.receive_from(
        boost::asio::buffer(recv_buf), sender_endpoint);

    std::cout.write(recv_buf.data(), len);
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
