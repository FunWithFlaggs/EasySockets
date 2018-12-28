# **EasySockets with C++**
###### *networking within a few lines of code*

### **What is this?**
This is a set of classes and structs that wrap all the C code needed for
networking. This gives you the possibility<br /> to focus on your code while
using a higher level of abstraction for your connections. No need to mess with C
<br />socket code or own multithreaded server implementations.

### **But what can i actually do with this?**
At the moment the focus is on listening TCP sockets, with more to come
when i get some free time to work on it.<br />You can:
- Create TCP sockets and listen on specific ports using a higher level of
abstraction.<br />
  - read and write from and to that socket.
- Create a multithreaded event driven TCP server using only a few lines.

### **Basic example**
The following example creates a TCP server on port 8080 which will echo the
data it receives<br /> prefixed with a HTTP response header.

No need to create any socket by hand. The server runs in its own thread and can handle
multiple clients and requests<br /> at the same time. Just register your function
on the desired event and let the rest be handled.

```C++
#include "socket.hpp"

void OnConnect(E_Socket* client_socket, std::string Client_IPv4){

    std::cout << "[D] got a connection from " << Client_IPv4 << std::endl;

}

void OnData(E_Client* easy_client, char* data, long data_len, uint64_t event_time, bool* is_finished){

    std::string http_response_header = "HTTP/1.0 200 OK\r\n";
    http_response_header += "Server: EasySockets Demo Server\r\n";
    http_response_header += "Content-Type: text/html\r\n";
    http_response_header += "Connection: close\r\n";
    http_response_header += "\r\n";

    http_response_header += std::string(data, data_len); // just echo what we received

    long bytes_written = write(easy_client->E_Client_Socket->GetFD(), http_response_header.c_str(), http_response_header.length());

    if(bytes_written != http_response_header.length()){
        std::cout << "[!] did only write " << bytes_written << " out of " << http_response_header.length() << " bytes :(" << std::endl;
    }
    else{
        std::cout << "[D] successfully written response." << std::endl;
    }

    // let the garbage collection close the socket.
    easy_client->E_Client_Socket->SetState(E_SOCKET_INVALID);
    easy_client->E_Client_Socket->SetType(E_SOCKET_UNKNOWN);

    // tell that we are finished with our work.
    *is_finished = true;

}

void OnDisconnected(std::string Client_IPv4, uint64_t event_time, bool* is_finished){

    std::cout << "[D] connection to " << Client_IPv4 << " closed." << std::endl;

}

int main(int argc, const char * argv[]) {

    E_Server* easy_tcp_server = new E_Server(8080);

    easy_tcp_server->SetOnConnect(&OnConnect);
    easy_tcp_server->SetOnData(&OnData);
    easy_tcp_server->SetOnDisconnected(&OnDisconnected);

    if(easy_tcp_server->Start() < 0){

        std::cout << "[!] could not start server, do i have access to the port?" << std::endl;

        delete easy_tcp_server;

        return -1;

    }
    else{
        std::cout << "[D] server started!" << std::endl;
    }

    sleep(30);

    easy_tcp_server->Stop();

    delete easy_tcp_server;

    return 0;
}
```
### **Where does the code compile?**
As ``<mach/mach_time.h>`` is included and ``mach_absolute_time()`` is used to
get the time when an event occurred this<br />code compiles only on Mac OS
and iOS. However you can change that to whatever is better for you to get this
running<br />on Linux too.

### **Bugs**
This code is far from perfection. It is the result from countless times of
re-implementing network code. I created this to<br />save me that time and
hopefully it will also save you some time.<br />However, this code may contain
bugs and errors (and i hope you report them to me), so just keep that in
mind.<br />If you have any improvement ideas or want to inform me about a bug,
please contact me at hencethisismyname [at] protonmail [dot] com.<br />
I am always happy to learn something new!
