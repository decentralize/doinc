#include <cstdlib>
using std::malloc;
using std::free;
using std::calloc;

#include <iostream>
using std::cout;
using std::cerr;
using std::cin;
using std::endl;

#include <string>
using std::string;

#include <fstream>
using std::ifstream;
using std::istreambuf_iterator;

#include <unordered_map>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
using boost::asio::ip::udp;

#include <chrono>
using std::chrono::milliseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

#include "protocol/packet.pb.h"
#include "utils.h"
#include "endpoint.h"
#include "crypto.h"
#include "lua_runtime.h"

class server {
public:
  server(boost::asio::io_service& io_service,
    crypto::PrivateKey priv, crypto::PublicKey pub,
    string server_code, string client_code)
      : io_service_(io_service),
        socket_(io_service, udp::endpoint(udp::v4(), 0)),
        ns_timer_(io_service),
        nodes(),
        priv_key(priv),
        pub_key(pub),
        supplier(server_code),
        blueprint(client_code) {

    cout << "server.lua loaded OK" << endl;

    refresh();
  }

  void refresh() {
    udp::endpoint ns_endpoint = udp::endpoint(udp::v4(), 9000);
    queue_send_to(createPacket(protocol::Packet::NS_REGISTER), ns_endpoint);

    if(nodes.empty()) {
      cout << "Network empty - attempting to bootstrap" << endl;
      queue_send_to(createPacket(protocol::Packet::NS_REQUEST_NODE), ns_endpoint);
    } else {
      // Perform cleanup of nodes that have not registered in 30 minutes (1 800 000 milliseconds)
      int pre_count = nodes.size();
      std::vector<Endpoint> to_remove;
      for(auto it = nodes.begin(); it != nodes.end(); it++){
        milliseconds now_in_ms = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());

        int elapsed_seconds = (now_in_ms - it->second).count() / 1000;
        if(elapsed_seconds >= 240){
          to_remove.push_back(it->first);
          continue;
        } else if(elapsed_seconds >= 60) {
          udp::endpoint to_ping = it->first.get();
          queue_send_to(createPacket(protocol::Packet::PING), to_ping);
        }
      }
      for(auto it = to_remove.begin(); it != to_remove.end(); it++) {
        nodes.erase(*it);
      }
      if(nodes.size() != pre_count) {
        cout << "Network size: " << nodes.size() << endl;
      }
    }

    // Do this again some time
    ns_timer_.expires_from_now(boost::posix_time::seconds(30));
    ns_timer_.async_wait(boost::bind(&server::refresh, this));
  }

  void handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd) {
    if (error || bytes_recvd <= 1) {
      cerr << "Receive error: " << error << endl;
      cerr << "Received bytes: " << bytes_recvd << endl;
      queue_receive_from();
      return;
    }

    protocol::Packet in_p;
    in_p.ParseFromString(string(data_, bytes_recvd));
    if(!in_p.has_code() || !checkCRC(in_p)) {
      cerr << "CRC check failed on packet from " << sender_endpoint_ << endl;
      queue_send_to(createPacket(protocol::Packet::ERROR), sender_endpoint_);
      return;
    }

    auto code = in_p.code();
    if(code == protocol::Packet::ERROR || code == protocol::Packet::OK) {
      // Do not care
    } else if(code == protocol::Packet::GET_NODES) {
      if(nodes.empty()){
        queue_send_to(createPacket(protocol::Packet::NODE_LIST), sender_endpoint_);
      } else {
        protocol::NodeList nl;

        for(auto it = nodes.begin(); it != nodes.end(); it++) {
          std::ostringstream stream;
          udp::endpoint e = it->first.get();
          if(Endpoint(e) == Endpoint(sender_endpoint_)) {
            // Make absolutely sure we never send the asking node with the list
            continue;
          }

          protocol::Node* node = nl.add_node();
          node->set_addr(e.address().to_string());
          node->set_port(e.port());
        }

        string data_string;
        nl.SerializeToString(&data_string);

        queue_send_to(createPacket(protocol::Packet::NODE_LIST, data_string), sender_endpoint_);
      }

    } else if(code == protocol::Packet::PING) {
      milliseconds timestamp = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
      int pre_count = nodes.size();
      nodes[Endpoint(sender_endpoint_)] = timestamp;
      if(nodes.size() != pre_count) {
        cout << "Network size: " << nodes.size() << endl;
      }
      queue_send_to(createPacket(protocol::Packet::PONG), sender_endpoint_);

    } else if(code == protocol::Packet::PONG) {
      milliseconds timestamp = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
      int pre_count = nodes.size();
      nodes[Endpoint(sender_endpoint_)] = timestamp;
      if(nodes.size() != pre_count) {
        cout << "Network size: " << nodes.size() << endl;
      }
    } else if(code == protocol::Packet::NODE) {
      cout << "Bootstrap OK" << endl;

      // Extract relevant ip:port information from received string
      protocol::Node node;
      string node_string = in_p.data();
      node.ParseFromString(node_string);

      // Create asio endpoint from extracted ip:port
      boost::asio::ip::address ip;
      ip = boost::asio::ip::address::from_string(node.addr());
      udp::endpoint endp(ip, node.port());
      queue_send_to(createPacket(protocol::Packet::GET_NODES), endp);
    } else if(code == protocol::Packet::NODE_LIST) {
      cout << "Connected to DOINC network" << endl;

      protocol::NodeList nodelist;
      string nodes_string = in_p.data();
      nodelist.ParseFromString(nodes_string);

      auto nodes = nodelist.mutable_node();
      for(auto it = nodes->begin(); it != nodes->end(); it++) {
        protocol::Node node = *it;
        boost::asio::ip::address ip;
        ip = boost::asio::ip::address::from_string(node.addr());
        udp::endpoint endp(ip, node.port());
        queue_send_to(createPacket(protocol::Packet::PING), endp);
      }
      queue_send_to(createPacket(protocol::Packet::PING), sender_endpoint_);

    // Client
    } else if(code == protocol::Packet::GET_PUBKEY) {
      if(in_p.data().compare(crypto::HashedKey(pub_key)) == 0) {
        string pk = crypto::StringifyPublicKey(pub_key);
        queue_send_to(createPacket(protocol::Packet::PUBKEY, pk), sender_endpoint_);
      } else {
        queue_send_to(createPacket(protocol::Packet::ERROR, "not_me"), sender_endpoint_);
      }

    } else if(code == protocol::Packet::GET_BLUEPRINT) {
      crypto::Signature sig = crypto::Sign(priv_key, blueprint);

      protocol::SignedMessage msg;
      msg.set_data(blueprint);
      msg.set_signature(string((const char*)sig.data(), sig.size()));
      string msg_str;
      msg.SerializeToString(&msg_str);

      queue_send_to(createPacket(protocol::Packet::BLUEPRINT, msg_str), sender_endpoint_);

    } else if(code == protocol::Packet::GET_WORK) {
      if(supplier.IsDone()) {
        queue_send_to(createPacket(protocol::Packet::ERROR, "no_more_work"), sender_endpoint_);
      } else {
        string work = supplier.GetWork();
        crypto::Signature sig = crypto::Sign(priv_key, work);

        protocol::SignedMessage msg;
        msg.set_data(work);
        msg.set_signature(string((const char*)sig.data(), sig.size()));
        string msg_str;
        msg.SerializeToString(&msg_str);

        queue_send_to(createPacket(protocol::Packet::WORK, msg_str), sender_endpoint_);

      }
    } else if(code == protocol::Packet::PUSH_RESULTS) {
      string res = in_p.data();
      supplier.AcceptResult(res);

      queue_send_to(createPacket(protocol::Packet::OK), sender_endpoint_);

    } else {
      cerr << "Received unknown function code: " << code << endl;
      queue_send_to(createPacket(protocol::Packet::ERROR, "bad_packet"), sender_endpoint_);
    }

    queue_receive_from();
  }

  void handle_send_to(const boost::system::error_code& error, size_t bytes_sent) {
    if(supplier.IsDone()) {
      io_service_.stop();
      return;
    }
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
    string packet_string;
    p.SerializeToString(&packet_string);
    queue_send_to(packet_string, ep);
  }

  void queue_send_to(string s, udp::endpoint ep) {
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
  enum { max_length = 64*1024 };
  char data_[max_length];

  std::unordered_map<Endpoint, milliseconds> nodes;
  crypto::PrivateKey priv_key;
  crypto::PublicKey pub_key;
  LuaSupplier supplier;
  string blueprint;
};




int main(int argc, char* argv[]) {
  crypto::PrivateKey priv_key;
  crypto::PublicKey pub_key;

  // TODO: Change to PEM if you be wanting human readable keys
  {
    ifstream pk_test("pub.key");
    if (!pk_test.good()) {
      cout << "No keypair found (priv.key, pub.key)\n" <<
        "Generate new keypair? Y/N" << endl;
      string input;
      std::cin >> input;
      if(boost::iequals(input, "Y")){
        crypto::GenerateKeyPair(priv_key, pub_key);
        crypto::SavePrivateKey("priv.key", priv_key);
        crypto::SavePublicKey("pub.key", pub_key);
      } else {
        cout << "Generate an RSA keypair as pub.key and priv.key and re-run" << endl;
        return 1;
      }
    } else {
      crypto::LoadPrivateKeyFile("priv.key", priv_key);
      crypto::LoadPublicKeyFile("pub.key", pub_key);
      cout << "Loaded symmetric key pair" << endl;
    }
  }

  if(!crypto::Validate(priv_key)){
    cerr << "Panic: Invalid private key!" << endl;
    return 1;
  } else if (!crypto::Validate(pub_key)) {
    cerr << "Panic: Invalid public key!" << endl;
    return 1;
  }

  cout << "Project Key (give this to workers): " <<
      crypto::HashedKey(pub_key) << endl;

  // TEST: Signing and verification of messages with RSA
  // string test_message = "This be test, lol";

  // SecByteBlock signature = crypto::Sign(priv_key, test_message);

  // Verify
  // bool ok = crypto::Verify(pub_key, test_message, signature);

  // if(ok){
  //   cout << "Message successfully verified!" << endl;
  // } else {
  //   cout << "Unable to verify message, beware!" << endl;
  //   return 1;
  // }

  ifstream server_ifs("server.lua");
  if(!server_ifs.good()) {
    cerr << "server.lua does not exist" << endl;
    return 1;
  }
  string server_code((istreambuf_iterator<char>(server_ifs)), (istreambuf_iterator<char>()));

  ifstream client_ifs("client.lua");
  if(!client_ifs.good()) {
    cerr << "client.lua does not exist" << endl;
    return 1;
  }
  string client_code((istreambuf_iterator<char>(client_ifs)), (istreambuf_iterator<char>()));

  try {
    boost::asio::io_service io_service;
    server s(io_service, priv_key, pub_key, server_code, client_code);
    io_service.run();
  } catch(const SupplierProgramException &e) {
    cerr << "Invalid supplier program: " << e.what() << endl;
    return 1;
  } catch (std::exception& e) {
    cerr << "Exception: " << e.what() << "\n";
    return 1;
  }
  cout << endl << "Server done, shutting down..." << endl;
  return 0;
}
