#include <cstdlib>
using std::malloc;
using std::free;
using std::calloc;

#include <iostream>
using std::cout;
using std::cerr;
using std::cin;
using std::endl;

#include <fstream>
using std::ifstream;

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
using boost::asio::ip::udp;

#include <rsa.h>
using CryptoPP::RSA;

#include <files.h>
using CryptoPP::FileSink;
using CryptoPP::FileSource;

#include <cryptlib.h>
using CryptoPP::BufferedTransformation;
using CryptoPP::ByteQueue;
using CryptoPP::StringSink;
using CryptoPP::StringSource;

#include <base64.h>
using CryptoPP::Base64Encoder;

#include <secblock.h>
using CryptoPP::SecByteBlock;

#include <osrng.h>
using CryptoPP::AutoSeededRandomPool;

#include <sha.h>
using CryptoPP::SHA;
using CryptoPP::SHA1;

#include <pssr.h>
using CryptoPP::RSASS;
using CryptoPP::PSS;

#include <chrono>
using std::chrono::milliseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

#include <google/protobuf/repeated_field.h>

#include "protocol/packet.pb.h"
#include "utils.h"

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
    
    // DEBUG
    queue_send_to(createPacket(protocol::Packet::NS_REQUEST_NODE), ns_endpoint);

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
    } else if(code == protocol::Packet::GET_NODES) {
      // TODO: TEST THIS PROPERLY!


      std::cout << "Received GET_NODES" << std::endl;
      if(nodes.empty()){
        std::cout << "No known nodes available, sending back empty packet" << std::endl;
        queue_send_to(createPacket(protocol::Packet::NODE_LIST), sender_endpoint_);
      } else {
        protocol::NodeList nl;

        std::map<udp::endpoint, milliseconds>::iterator it;
        for(it = nodes.begin(); it != nodes.end(); it++) {
          std::ostringstream stream;
          stream << it->first;
          nl.add_data(stream.str());
        }

        std::string data_string;
        nl.SerializeToString(&data_string);

        queue_send_to(createPacket(protocol::Packet::NODE_LIST, data_string), sender_endpoint_);
      }

    } else if(code == protocol::Packet::PING) {
      std::cout << "Received PING, sending back PONG" << std::endl;
      milliseconds timestamp = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
      nodes[sender_endpoint_] = timestamp;
      queue_send_to(createPacket(protocol::Packet::PONG), sender_endpoint_);

    } else if(code == protocol::Packet::PONG) {
      std::cout << "Received PONG" << std::endl;
      milliseconds timestamp = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
      nodes[sender_endpoint_] = timestamp;

    } else if(code == protocol::Packet::NODE) {  
      std::cout << "Received node from nameserver, sending PING" << std::endl;

      // Extract relevant ip:port information from received string
      std::string endpoint_string = in_p.data();
      std::size_t delim = endpoint_string.find(":");
      std::string ip_string = endpoint_string.substr(0, delim);
      std::string port_string = endpoint_string.substr(delim + 1);

      std::cout << "ip:port = " << ip_string << ":" << port_string << std::endl;

      // Create asio endpoint from extracted ip:port
      boost::asio::ip::address ip;
      unsigned short port;
      ip = boost::asio::ip::address::from_string(ip_string);
      port = boost::lexical_cast<unsigned short>(port_string);
      udp::endpoint endp(ip, port);
      queue_send_to(createPacket(protocol::Packet::PING), endp);


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

  std::map<udp::endpoint, milliseconds> nodes;
};


void SaveToFile(const std::string& filename, const BufferedTransformation& bt) {
  FileSink file(filename.c_str());

  bt.CopyTo(file);
  file.MessageEnd();
}

void SavePrivateKey(const std::string& filename, const RSA::PrivateKey& key) {
  ByteQueue queue;
  key.Save(queue);

  SaveToFile(filename, queue);
}

void SavePublicKey(const std::string& filename, const RSA::PublicKey& key) {
  ByteQueue queue;
  key.Save(queue);

  SaveToFile(filename, queue);
}

void LoadFromFile(const std::string& filename, BufferedTransformation& bt) {
  FileSource file(filename.c_str(), true);

  file.TransferTo(bt);
  bt.MessageEnd();
}

void LoadPrivateKey(const std::string& filename, RSA::PrivateKey& key) {
  ByteQueue queue;

  LoadFromFile(filename, queue);
  key.Load(queue);
}

void LoadPublicKey(const std::string& filename, RSA::PublicKey& key) {
  ByteQueue queue;

  LoadFromFile(filename, queue);
  key.Load(queue);
}

int main(int argc, char* argv[]) {
  AutoSeededRandomPool rnd_pool;
  RSA::PrivateKey priv_key;
  RSA::PublicKey pub_key;

  // TODO: Change to PEM if you be wanting human readable keys
  {
    ifstream pk_test("pub.key");
    if (!pk_test.good()) {
      std::cout << "No keypair found (priv.key, pub.key)\n" <<
        "Generate new keypair? Y/N" << std::endl;
      std::string input;
      std::cin >> input;
      if(boost::iequals(input, "Y")){
        priv_key.GenerateRandomWithKeySize(rnd_pool, 3072);
        pub_key.AssignFrom(priv_key);

        SavePrivateKey("priv.key", priv_key);
        SavePublicKey("pub.key", pub_key);
      }
    } else {
      LoadPrivateKey("priv.key", priv_key);
      LoadPublicKey("pub.key", pub_key);
      std::cout << "Loaded symmetric key pair" << std::endl;
    }
  }

  if(!priv_key.Validate(rnd_pool, 3)){
    std::cerr << "Panic: Invalid private key!" << std::endl;
    return 1;
  } else if (!pub_key.Validate(rnd_pool, 3)){
    std::cerr << "Panic: Invalid public key!" << std::endl;
    return 1;
  }

  // Hash pubkey for distribution
  std::string toHash;
  StringSink sink(toHash);
  pub_key.Save(sink);
  byte thumbprint[SHA::DIGESTSIZE];
  SHA().CalculateDigest(thumbprint, (const byte*) toHash.data(), toHash.length());

  // Create string version of the thumbprint
  std::string thumbprint_string;
  StringSource source(thumbprint, SHA::DIGESTSIZE, true, new Base64Encoder(new StringSink(thumbprint_string)));
  std::cout << "Project Key (give this to workers): " << thumbprint_string << std::endl;

  // TEST: Signing and verification of messages with RSA
  std::string test_message = "This be test, lol";

  // Sign
  RSASS<PSS, SHA1>::Signer signer(priv_key);

  size_t sig_length = signer.MaxSignatureLength();
  SecByteBlock signature(sig_length);

  sig_length = signer.SignMessage(rnd_pool, (const byte*) test_message.data(), test_message.length(), signature);
  signature.resize(sig_length);

  // Verify
  RSASS<PSS, SHA1>::Verifier verifier(pub_key);

  bool result = verifier.VerifyMessage((const byte*)test_message.data(), test_message.length(), signature, signature.size());

  if(result){
    std::cout << "Message successfully verified!" << std::endl;
  } else {
    std::cout << "Unable to verify message, beware!" << std::endl;
  }

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
