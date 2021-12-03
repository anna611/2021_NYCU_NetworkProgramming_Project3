//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

using boost::asio::io_service;
using boost::asio::ip::tcp;
using namespace std;

io_service io_context;

class session
  : public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket)
    : socket_(std::move(socket))
  {

  }

  void start()
  {
    do_read();
  }

private:
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
  char REQUEST_METHOD[20];
  char REQUEST_URI[1000];
  char QUERY_STRING[1000];
  char SERVER_PROTOCOL[100];
  char HTTP_HOST[100];
  char SERVER_ADDR[100];
  char SERVER_PORT[10];
  char REMOTE_ADDR[100];
  char REMOTE_PORT[10];
  char temp[100];
  char EXEC_FILE[100] = "./";
  void setclientenv(){
      setenv("REQUEST_METHOD", REQUEST_METHOD, 1);
      setenv("REQUEST_URI", REQUEST_URI, 1);
      setenv("QUERY_STRING", QUERY_STRING, 1);
      setenv("SERVER_PROTOCOL", SERVER_PROTOCOL, 1);
      setenv("HTTP_HOST", HTTP_HOST, 1);
      setenv("SERVER_ADDR", SERVER_ADDR, 1);
      setenv("SERVER_PORT", SERVER_PORT, 1);
      setenv("REMOTE_ADDR", REMOTE_ADDR, 1);
      setenv("REMOTE_PORT", REMOTE_PORT, 1);
      setenv("EXEC_FILE", EXEC_FILE, 1);
  }
  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          sscanf(data_, "%s %s %s %s %s", REQUEST_METHOD, REQUEST_URI,
                 SERVER_PROTOCOL, temp, HTTP_HOST);
          if (!ec)
          {
            do_write(length);
          }
        });
  }

  void do_write(std::size_t length)
  {
    auto self(shared_from_this());
    strcpy(data_ ,"HTTP/1.1 200 OK\n");
    boost::asio::async_write(socket_, boost::asio::buffer(data_, strlen(data_)),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            strcpy(SERVER_ADDR,
                   socket_.local_endpoint().address().to_string().c_str());
            sprintf(SERVER_PORT, "%u", socket_.local_endpoint().port());
            strcpy(REMOTE_ADDR,
                   socket_.remote_endpoint().address().to_string().c_str());
            sprintf(REMOTE_PORT, "%u", socket_.remote_endpoint().port());
            int index_q = 0;
            int index_e = 2;
            int checkquery = 0;
            memset(QUERY_STRING, 0, 1000);
            for(int i = 0;i < (int)strlen(REQUEST_URI);i++){
                if(REQUEST_URI[i] == '?'){
                    checkquery = 1;
                    i++;
                }
                if(REQUEST_URI[i] == '\0'){
                    break;
                }
                if(checkquery == 1){
                  QUERY_STRING[index_q] = REQUEST_URI[i];
                  index_q++;
                }
                else{
                    EXEC_FILE[index_e] = REQUEST_URI[i];
                    index_e++;
                }
            }
            //cerr <<"execfile:" <<EXEC_FILE << endl;
            setclientenv();
            io_context.notify_fork(io_service::fork_prepare);
            if (fork() > 0) {
              io_context.notify_fork(io_service::fork_parent);
              socket_.close();
            } else {
              io_context.notify_fork(io_service::fork_child);
              int sock = socket_.native_handle();
              //dup2(sock, STDERR_FILENO);
              dup2(sock, STDIN_FILENO);
              dup2(sock, STDOUT_FILENO);
              socket_.close();
              execlp(EXEC_FILE, EXEC_FILE, NULL);
               
            }
            do_read();
          }
        });
  }
};

class server
{
public:
  server(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            std::make_shared<session>(std::move(socket))->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;

    server s(io_context, std::atoi(argv[1]));

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}