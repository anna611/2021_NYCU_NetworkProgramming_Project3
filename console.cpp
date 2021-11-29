#include <boost/asio/io_service.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <array>
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;

io_service ioservice;
tcp::resolver resolv{ioservice};
tcp::socket tcp_socket{ioservice};
std::array<char, 4096> bytes;

typedef struct{
	string host;
	string port;
	string file;
	int valid;
}client_info;

client_info client_record[5];

void read_handler(const boost::system::error_code &ec,
  std::size_t bytes_transferred)
{
  if (!ec)
  {
    //cerr <<"bytes:"<<bytes<<endl;
    std::cout.write(bytes.data(), bytes_transferred);
    tcp_socket.async_read_some(buffer(bytes), read_handler);
  }
}

void connect_handler(const boost::system::error_code &ec)
{
  if (!ec)
  {
    tcp_socket.async_read_some(buffer(bytes), read_handler);
    
  }
}

void resolve_handler(const boost::system::error_code &ec,
  tcp::resolver::iterator it)
{
  if (!ec)
    tcp_socket.async_connect(*it, connect_handler);
}

void ParseQuery(string query){
    vector<string> spiltQuery;
    stringstream ss(query);
    string tok;
    int index;
    while (getline(ss, tok, '&')) {
        spiltQuery.push_back(tok);
    }   
    for(int i = 0;i < spiltQuery.size();++i){
        index = i /3;
        //cerr <<"index:" <<index << endl;
        stringstream tempss(spiltQuery[i]);
        while (getline(tempss, tok, '=')) {
        }
        if(i % 3 == 0){
            client_record[index].host = tok;
            //cerr <<"host:" <<tok << endl;
        }
        else if(i % 3 == 1){
            client_record[index].port = tok;
            //cerr <<"port:" <<tok << endl;
        }
        else if(i % 3 == 2){
            client_record[index].file = tok;
            //cerr <<"file:" <<tok << endl;
        }
        client_record[index].valid = 1;
    }
    return;
}

int main()
{
    cout << "Content-type: text/html" << endl << endl;
    const char *tmp = getenv("QUERY_STRING");
    string env_var(tmp ? tmp : "");
    if (env_var.empty()) {
        cerr << "[ERROR] No such variable found!" << endl;
        exit(EXIT_FAILURE);
    }
    //cerr <<"Query:"<< env_var <<endl;
    ParseQuery(env_var);
    tcp::resolver::query q{client_record[0].host, client_record[0].port};
    resolv.async_resolve(q, resolve_handler);
    ioservice.run();
}