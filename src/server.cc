#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <rsa.h>
#include <osrng.h>
#include <files.h>
#include <cryptlib.h>
#include <sha.h>
#include <base64.h>

#include "protocol/packet.pb.h"
#include "utils.h"

using boost::asio::ip::udp;

class server {
public:
  server(boost::asio::io_service& io_service)
    : io_service_(io_service),
      socket_(io_service, udp::endpoint(udp::v4(), 0)),
      ns_timer_(io_service) {

    queue_ns_register();
  }

  void queue_ns_register() {
    std::cout << "Registrering with Name Server" << std::endl;
    udp::endpoint ns_endpoint = udp::endpoint(udp::v4(), 9000);
    queue_send_to(createPacket(protocol::Packet::NS_REGISTER), ns_endpoint);

    ns_timer_.expires_from_now(boost::posix_time::seconds(10));
    ns_timer_.async_wait(boost::bind(&server::queue_ns_register, this));
  }

  void handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd) {
    if (error || bytes_recvd <= 1) {
      std::cerr << "Receive error: " << error << std::endl;
      std::cerr << "Received bytes: " << bytes_recvd << std::endl;
      queue_receive_from();
      return;
    }

    protocol::Packet in_p;
    in_p.ParseFromString(std::string(data_, bytes_recvd));
    if(!checkCRC(in_p)) {
      std::cerr << "CRC check failed on packet from " << sender_endpoint_ << std::endl;
      queue_send_to(createPacket(protocol::Packet::ERROR), sender_endpoint_);
      return;
    }

    auto code = in_p.code();
    std::cout << "Got valid packet: " << code << std::endl;
    if(code == protocol::Packet::ERROR || code == protocol::Packet::OK) {
      // No care ok?
    //} else if(code == ) {
      // TODO etc
    } else {
      std::cerr << "Received unknown function code: " << code << std::endl;
    }

    queue_receive_from();
    //queue_send_to(boost::asio::buffer(data_, bytes_recvd), sender_endpoint_);
  }

  void handle_send_to(const boost::system::error_code& error, size_t bytes_sent) {
    queue_receive_from();
  }



  // == Implementation details, no business logic ==

  void queue_receive_from() {
    socket_.async_receive_from(
        boost::asio::buffer(data_, max_length), sender_endpoint_,
        boost::bind(&server::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

  void queue_send_to(protocol::Packet p, udp::endpoint ep) {
    std::string packet_string;
    p.SerializeToString(&packet_string);
    queue_send_to(packet_string, ep);
  }

  void queue_send_to(std::string s, udp::endpoint ep) {
    queue_send_to(boost::asio::buffer(s.data(), s.length()), ep);
  }

  template<typename ConstBufferSequence>
  void queue_send_to(const ConstBufferSequence & b, udp::endpoint ep) {
    socket_.async_send_to(b, ep,
      boost::bind(&server::handle_send_to, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
  }

private:
  boost::asio::io_service& io_service_;
  udp::socket socket_;
  udp::endpoint sender_endpoint_;
  boost::asio::deadline_timer ns_timer_;
  enum { max_length = 1024 };
  char data_[max_length];
};


void Save(const std::string& filename, const CryptoPP::BufferedTransformation& bt) {
    CryptoPP::FileSink file(filename.c_str());

    bt.CopyTo(file);
    file.MessageEnd();
}

void SavePrivateKey(const std::string& filename, const CryptoPP::RSA::PrivateKey& key) {

    CryptoPP::ByteQueue queue;
    key.Save(queue);

    Save(filename, queue);
}

void SavePublicKey(const std::string& filename, const CryptoPP::RSA::PublicKey& key) {

    CryptoPP::ByteQueue queue;
    key.Save(queue);

    Save(filename, queue);
}

void Load(const std::string& filename, CryptoPP::BufferedTransformation& bt)
{
  
  CryptoPP::FileSource file(filename.c_str(), true);

  file.TransferTo(bt);
  bt.MessageEnd();
}

void LoadPrivateKey(const std::string& filename, CryptoPP::RSA::PrivateKey& key) {
  
  CryptoPP::ByteQueue queue;

  Load(filename, queue);
  key.Load(queue);  
}

void LoadPublicKey(const std::string& filename, CryptoPP::RSA::PublicKey& key) {

  CryptoPP::ByteQueue queue;

  Load(filename, queue);
  key.Load(queue);  
}

int main(int argc, char* argv[]) {
  using namespace CryptoPP;

  AutoSeededRandomPool rnd_pool;
  RSA::PrivateKey priv_key;
  RSA::PublicKey pub_key;

  // TODO: Change to PEM if you be wanting human readable keys
  std::cout << "Generate new keypair? Y/N" << std::endl;
  std::string input = "";
  std::cin >> input;
  if(boost::iequals(input, "Y")){

    priv_key.GenerateRandomWithKeySize(rnd_pool, 3072);
    pub_key.AssignFrom(priv_key);

    SavePrivateKey("priv.key", priv_key);
    SavePublicKey("pub.key", pub_key);
  } else {

    std::cout << "Loading priv.key and pub.key" << std::endl;
    LoadPrivateKey("priv.key", priv_key);
    LoadPublicKey("pub.key", pub_key);
  }

  if(!priv_key.Validate(rnd_pool, 3)){
    std::cerr << "Invalid private key!" << std::endl;
    return 1;
  } else if (!pub_key.Validate(rnd_pool, 3)){
    std::cerr << "Invalid public key!" << std::endl;
    return 1;
  }

  // Hash pubkey for distribution
  std::string toHash;
  CryptoPP::StringSink sink(toHash);

  pub_key.Save(sink);


  byte thumbprint[CryptoPP::SHA::DIGESTSIZE];
  CryptoPP::SHA().CalculateDigest(thumbprint, (byte const*) toHash.data(), toHash.length());
  
  // Create string version of the thumbprint
  std::string thumbprint_string;
  CryptoPP::StringSource source(thumbprint, CryptoPP::SHA::DIGESTSIZE, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(thumbprint_string)));
  std::cout << "Thumbprint: " << thumbprint_string << std::endl;


  // TODO: Signing and verification of messages with RSA

  try {
    boost::asio::io_service io_service;
    server s(io_service);
    io_service.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return 1;
  }
  std::cout << "Server done, shutting down..." << std::endl;
  return 0;
}
