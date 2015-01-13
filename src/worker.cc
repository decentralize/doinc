#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <sstream>
using std::ostringstream;

#include <fstream>
using std::ifstream;
using std::istreambuf_iterator;

#include <string>
using std::string;

// For sleep
#include <chrono>
#include <thread>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
using boost::asio::deadline_timer;
using boost::asio::ip::udp;
#include <boost/date_time/posix_time/posix_time.hpp>
using boost::posix_time::seconds;

#include "lua_runtime.h"
#include "protocol/packet.pb.h"
#include "utils.h"
#include "crypto.h"

// The client object runs the io_service to block thread execution until the
// actor completes.
class client
{
public:
  client(const udp::endpoint& listen_endpoint)
    : socket_(io_service_, listen_endpoint),
      deadline_(io_service_)
  {
    // No deadline is required until the first socket operation is started. We
    // set the deadline to positive infinity so that the actor takes no action
    // until a specific deadline is set.
    deadline_.expires_at(boost::posix_time::pos_infin);

    // Start the persistent actor that checks for deadline expiry.
    check_deadline();
  }

  protocol::Packet receive(boost::posix_time::time_duration timeout) {
    protocol::Packet p;
    char data[64*1024];
    boost::system::error_code ec;

    int bytes_recvd = receive(boost::asio::buffer(data, 64*1024), timeout, ec);
    if(ec) {
      throw(ec);
    }

    p.ParseFromString(string(data, bytes_recvd));
    return p;
  }

  void send_to(protocol::Packet p, udp::endpoint ep) {
    string packet_string;
    p.SerializeToString(&packet_string);
    send_to(packet_string, ep);
  }

  void send_to(string s, udp::endpoint ep) {
    send_to(boost::asio::buffer(s.data(), s.length()), ep);
  }

  template<typename ConstBufferSequence>
  void send_to(const ConstBufferSequence & b, udp::endpoint ep) {
    socket_.send_to(b, ep);
  }

  std::size_t receive(const boost::asio::mutable_buffer& buffer,
      boost::posix_time::time_duration timeout, boost::system::error_code& ec) {
    // Set a deadline for the asynchronous operation.
    deadline_.expires_from_now(timeout);

    // Set up the variables that receive the result of the asynchronous
    // operation. The error code is set to would_block to signal that the
    // operation is incomplete. Asio guarantees that its asynchronous
    // operations will never fail with would_block, so any other value in
    // ec indicates completion.
    ec = boost::asio::error::would_block;
    std::size_t length = 0;

    // Start the asynchronous operation itself. The handle_receive function
    // used as a callback will update the ec and length variables.
    socket_.async_receive(boost::asio::buffer(buffer),
        boost::bind(&client::handle_receive, _1, _2, &ec, &length));

    // Block until the asynchronous operation has completed.
    do io_service_.run_one(); while (ec == boost::asio::error::would_block);

    return length;
  }

private:
  void check_deadline()
  {
    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
    if (deadline_.expires_at() <= deadline_timer::traits_type::now())
    {
      // The deadline has passed. The outstanding asynchronous operation needs
      // to be cancelled so that the blocked receive() function will return.
      //
      // Please note that cancel() has portability issues on some versions of
      // Microsoft Windows, and it may be necessary to use close() instead.
      // Consult the documentation for cancel() for further information.
      socket_.cancel();

      // There is no longer an active deadline. The expiry is set to positive
      // infinity so that the actor takes no action until a new deadline is set.
      deadline_.expires_at(boost::posix_time::pos_infin);
    }

    // Put the actor back to sleep.
    deadline_.async_wait(boost::bind(&client::check_deadline, this));
  }

  static void handle_receive(
      const boost::system::error_code& ec, std::size_t length,
      boost::system::error_code* out_ec, std::size_t* out_length)
  {
    *out_ec = ec;
    *out_length = length;
  }

private:
  boost::asio::io_service io_service_;
  udp::socket socket_;
  deadline_timer deadline_;
};

void check(protocol::Packet& p) {
  if(!p.has_code()) {
    throw("Corrupted packet - no code");
  }
  if(!checkCRC(p)) {
    ostringstream str;
    str << "CRC mismatch: " << calculateCRC(p) << " != " << p.crc();
    throw(str.str().data());
  }
}

int main(int argc, char** argv) {
  client c(udp::endpoint(udp::v4(), 0));

  if(argc < 3) {
    cerr << "Usage: " << endl;
    cerr << "\t" << "worker <ip:port> <project_id>   work for a project" << endl;
    cerr << "\t\t" << "<ip:port> is the address to the nameserver ";
    cerr << "used for finding the network" << endl;
    return 1;
  }

  string addr_str(argv[1]);
  string pubkey_hash(argv[2]);
  boost::asio::ip::address ip;
  int port;
  {
    std::size_t pos = addr_str.find(":");
    if(pos == string::npos) {
      cerr << "Invalid ns address" << endl;
      return 1;
    }
    ip = boost::asio::ip::address::from_string(addr_str.substr(0,pos));
    port = atoi(addr_str.substr(pos+1).c_str());
    if(port < 1) {
      cerr << "Invalid port number" << endl;
      return 1;
    }
  }
  udp::endpoint ns(ip, port);

  std::vector<udp::endpoint> nodes_to_search;
  std::shared_ptr<udp::endpoint> supplier;
  crypto::PublicKey pk;
  std::shared_ptr<LuaSandbox> sandbox;
  string result;

  for(;;) {
    try {
      if(!supplier) {
        if(nodes_to_search.empty()) {
          cout << "Bootstrapping" << endl;
          c.send_to(createPacket(protocol::Packet::NS_REQUEST_NODE), ns);
          protocol::Packet node_resp = c.receive(seconds(5));
          check(node_resp);

          protocol::Node node;
          string node_string = node_resp.data();
          node.ParseFromString(node_string);

          // Create asio endpoint from extracted ip:port
          boost::asio::ip::address ip;
          ip = boost::asio::ip::address::from_string(node.addr());
          udp::endpoint endp(ip, node.port());

          cout << "Trying to connect to node: " << endp << endl;
          // Contact that IP for nodes
          c.send_to(createPacket(protocol::Packet::GET_NODES), endp);
          protocol::Packet nodes_resp = c.receive(seconds(5));
          check(nodes_resp);

          cout << "Connected to network" << endl;

          protocol::NodeList nodelist;
          string nodes_string = nodes_resp.data();
          nodelist.ParseFromString(nodes_string);

          auto nodes = nodelist.mutable_node();
          for(auto it = nodes->begin(); it != nodes->end(); it++) {
            protocol::Node node = *it;
            boost::asio::ip::address ip;
            ip = boost::asio::ip::address::from_string(node.addr());
            nodes_to_search.push_back(udp::endpoint(ip, node.port()));
          }
          nodes_to_search.push_back(endp);
        } else {
          for(auto it = nodes_to_search.begin(); it != nodes_to_search.end(); it++) {
            udp::endpoint endp = *it;
            cout << "Trying node " << endp << endl;
            c.send_to(createPacket(protocol::Packet::GET_PUBKEY, pubkey_hash), endp);
            protocol::Packet resp = c.receive(seconds(5));
            check(resp);

            if(resp.code() == protocol::Packet::PUBKEY) {
              crypto::LoadPublicKey(resp.data(), pk);
              supplier.reset(new udp::endpoint(endp));
              break;
            }
          }
          if(!supplier) {
            cerr << "Could not find supplier with that ID. Retrying in 30s" << endl;
            nodes_to_search.clear();
            std::this_thread::sleep_for(std::chrono::seconds(30));
          }
        }

      } else {
        if(!sandbox) {
          cout << "Requesting blueprint" << endl;
          c.send_to(createPacket(protocol::Packet::GET_BLUEPRINT), *supplier);
          protocol::Packet resp = c.receive(seconds(5));
          check(resp);

          protocol::SignedMessage msg;
          msg.ParseFromString(resp.data());
          string blueprint = msg.data();
          crypto::Signature sig((unsigned char*)msg.signature().data(), msg.signature().size());
          if(!crypto::Verify(pk, blueprint, sig)) {
            cerr << "Bad blueprint signature" << endl;
            continue;
          }

          LuaSandbox* sb;
          try {
            sb = new LuaSandbox(blueprint);
          } catch(const BlueprintException& e) {
            cerr << "Could not load blueprint: " << e.what() << endl;
            return 1;
          }
          sandbox.reset(sb);

        } else if(result.length() > 0) {
          cout << "Sending back work result" << endl;
          c.send_to(createPacket(protocol::Packet::PUSH_RESULTS, result), *supplier);
          protocol::Packet resp = c.receive(seconds(5));
          check(resp);
          if(resp.code() == protocol::Packet::OK) {
            result = string();
          } else {
            cerr << "Server did not accept work" << endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
          }

        } else {
          cout << "Requesting work" << endl;
          c.send_to(createPacket(protocol::Packet::GET_WORK), *supplier);
          protocol::Packet resp = c.receive(seconds(5));
          check(resp);

          if(resp.code() == protocol::Packet::ERROR) {
            cerr << "Stopping: " << resp.data() << endl;
            return 1;
          } else if(resp.code() != protocol::Packet::WORK) {
            cerr << "Unexpected packet: " << resp.code() << endl;
            continue;
          }

          protocol::SignedMessage msg;
          msg.ParseFromString(resp.data());
          string work = msg.data();
          crypto::Signature sig((unsigned char*)msg.signature().data(), msg.signature().size());
          if(!crypto::Verify(pk, work, sig)) {
            cerr << "Bad work signature" << endl;
            continue;
          }

          try {
            result = sandbox->PerformWork(work);
          } catch(const WorkException& e) {
            cerr << "Error performing work: " << e.what() << endl;
            return 1;
          }

          if(result.length() >= 64*1024) {
            cerr << "Work result too large to send" << endl;
            return 1;
          }
        }
        // TODO: Ask node for blueprint
        // TODO: Initialize lua sandbox
        // TODO: Load blueprint

        // TODO: Ask node for work, verify, work, push results, log, repeat
      }
    } catch(const boost::system::error_code& ec) {
      if(supplier) {
        supplier.reset();
      } else if(!nodes_to_search.empty()) {
        nodes_to_search.clear();
      }
      std::this_thread::sleep_for(std::chrono::seconds(3));
    } catch(const char* e) {
      cerr << "Error: " << e << endl;
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
  }

  return 0;
}
