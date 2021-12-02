#include <boost/asio/io_service.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/bind/bind.hpp>
#include <array>
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <memory>
#include <utility>
#include <fstream>

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost::algorithm;
using namespace std;

io_service ioservice;
tcp::resolver resolv{ioservice};
const string filePath_ = "./test_case/";
ifstream file_;
bool checkexit = false;
tcp::socket tcp_socket{ioservice};
std::array<char, 4096> bytes;
//std::enable_shared_from_this;

typedef struct{
	string host;
	string port;
	string file;
	int valid = 0;
}client_info;

client_info client_record[5];

class session
    :public std::enable_shared_from_this<session>
{
    public:
        session(){
            tcp::resolver::query q{client_record[0].host, client_record[0].port};
            resolv.async_resolve(q, boost::bind( &session::resolve_handler, this,std::placeholders::_1,std::placeholders::_2));
        }
        void encode(std::string& data) {
            std::string buffer;
            buffer.reserve(data.size());
            for(size_t pos = 0; pos != data.size(); ++pos) {
                switch(data[pos]) {
                    case '&':  buffer.append("&amp;");       break;
                    case '\"': buffer.append("&quot;");      break;
                    case '\'': buffer.append("&apos;");      break;
                    case '<':  buffer.append("&lt;");        break;
                    case '>':  buffer.append("&gt;");        break;
                    case '\n': buffer.append("&NewLine;");   break;
                    case '\r': buffer.append("");   break;
                    default:   buffer.append(&data[pos], 1); break;
                }
            }
            data.swap(buffer);
        }

        void output_shell(int index ,string content){
            encode(content);
            string output = "<script>document.getElementById('s"+to_string(index)+"').innerHTML += '"+content+"';</script>";
            cout << output << flush;
        }
        void output_cmd(int index,string content){
            encode(content);
            string output = "<script>document.getElementById('s"+to_string(index)+"').innerHTML += '<b>"+content+"</b>';</script>";
            cout <<output <<flush;
        }

        bool do_write(){
            //auto self(shared_from_this());
            string input;
            bool state = false;
            getline(file_,input);
            if(input == "exit"){
                state = true;
            }
            input = input + "\n";
            boost::asio::async_write(tcp_socket, boost::asio::buffer(input.c_str(), input.size()),
                [this](boost::system::error_code ec, std::size_t ){

                });
            if(checkexit == false)
                output_cmd(0,input);

            return state;
        }

        void read_handler(const boost::system::error_code &ec)
        {
        if (!ec)
        {
            string str(bytes.data());
            output_shell(0,str);
            //std::cout.write(bytes.data(), bytes_transferred);
            if(boost::algorithm::contains(bytes.data(),"%")){
                checkexit = do_write();
                if(checkexit == true){
                    client_record[0].valid = 0;
                }
            }
            memset(bytes.data(), 0, 4096);
            tcp_socket.async_read_some(buffer(bytes), boost::bind( &session::read_handler, this,std::placeholders::_1));
        }
        }
        void connect_handler(const boost::system::error_code &ec)
        {
        if (!ec)
        {
            tcp_socket.async_read_some(buffer(bytes), boost::bind( &session::read_handler, this,std::placeholders::_1));
            
        }
        }

        void resolve_handler(const boost::system::error_code &ec,
        tcp::resolver::iterator it)
        {
        if (!ec)
            tcp_socket.async_connect(*it,  boost::bind( &session::connect_handler, this,std::placeholders::_1));
        }
};








void ParseQuery(string query){
    vector<string> spiltQuery;
    stringstream ss(query);
    string tok;
    int index;
    while (getline(ss, tok, '&')) {
        spiltQuery.push_back(tok);
    }   
    for(size_t i = 0;i < spiltQuery.size();++i){
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

void Printpanel(){
    string htmlContent = "";
    htmlContent+=
"<!DOCTYPE html>"
"<html lang=\"en\">"
"  <head>"
"    <meta charset=\"UTF-8\" />"
"    <title>NP Project 3 Console</title>"
"    <link"
"     rel=\"stylesheet\""
"     href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\""
"     integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\""
"      crossorigin=\"anonymous\""
"    />"
"   <link"
"      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\""
"      rel=\"stylesheet\""
"    />"
"    <link"
"      rel=\"icon\""
"        type=\"image/png\""
"      href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\""
"    />"
"    <style>"
"      * {"
"        font-family: 'Source Code Pro', monospace;"
"        font-size: 1rem !important;"
"      }"
"      body {"
"        background-color: #212529;"
"      }"
"      pre {"
"        color: #cccccc;"
"      }"
"      b {"
"        color: #01b468;"
"      }"
"    </style>"
"  </head>"
"  <body>"
"    <table class=\"table table-dark table-bordered\">"
"      <thead>"
"        <tr> ";
        for(int i = 0;i < 5;i++){
            if(client_record[i].valid == 1){
                htmlContent += "<th scope=\"col\">"+client_record[i].host+":"+client_record[i].port+"</th>" ;
            }
        }
        htmlContent +=
"        </tr>"
"      </thead>"
"      <tbody>"
"        <tr>";
        for(int i = 0;i < 5;i++){
            if(client_record[i].valid == 1){
                htmlContent += "<td><pre id=\"s"+to_string(i)+"\" class=\"mb-0\"></pre></td>";
            }
        }
        htmlContent +=  
"        </tr>"
"      </tbody>"
"    </table>"
"  </body>"
"</html>";
    cout << htmlContent << flush;
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
    Printpanel();
    for(int i = 0;i < 5;i ++){
        if(client_record[i].valid == 1){
            file_.open(filePath_+client_record[i].file);
            if(!file_.is_open()){
                cerr << "no file:" << client_record[i].file <<endl;
            }
        }
    }
    session s;
    ioservice.run();
}