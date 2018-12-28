//
//  socket.cpp
//  EasySockets
//
//  Created by Marc Mendelson on 27.12.18.
//  Copyright © 2018 Ezra Knobloch. All rights reserved.
//

#include "socket.hpp"

E_Socket::E_Socket(){
    this->E_Socket_FD = -1;
    this->E_Socket_State = E_SOCKET_INVALID;
    this->E_Socket_Type = E_SOCKET_UNKNOWN;
}

E_Socket::E_Socket(int Socket_Proto, int Socket_Type){
    this->E_Socket_FD = socket(Socket_Proto, Socket_Type, 0);
    if(this->E_Socket_FD < 0 || this->SetNonBlock() < 0){
        this->E_Socket_State = E_SOCKET_INVALID;
    }
    else{
        this->E_Socket_State = E_SOCKET_OK;
    }
    
    if(Socket_Proto == AF_INET && Socket_Type == SOCK_STREAM){
        this->E_Socket_Type = E_SOCKET_TCP;
    }
    else{
        this->E_Socket_Type = E_SOCKET_UNKNOWN;
    }
}

E_Socket::E_Socket(int E_Type){
    
    switch(E_Type){
        case E_SOCKET_TCP:
            this->E_Socket_FD = socket(AF_INET, SOCK_STREAM, 0);
            if(this->E_Socket_FD < 0 || this->SetNonBlock() < 0){
                this->E_Socket_State = E_SOCKET_INVALID;
                this->E_Socket_Type = E_SOCKET_UNKNOWN;
            }
            else{
                this->E_Socket_State = E_SOCKET_OK;
                this->E_Socket_Type = E_SOCKET_TCP;
            }
            break;
        default:
            this->E_Socket_FD = -1;
            this->E_Socket_State = E_SOCKET_INVALID;
            break;
    }
    
}

E_Socket::~E_Socket(){
    
    if(this->E_Socket_FD != -1){
        close(this->E_Socket_FD);
    }
    
}

int E_Socket::SetNonBlock(){
    
    if(fcntl(this->E_Socket_FD, F_SETFL, fcntl(this->E_Socket_FD, F_GETFL, 0) | O_NONBLOCK) < 0){
        this->E_Socket_State = E_SOCKET_INVALID;
        
        return -1;
    }
    
    return 0;
    
}

int E_Socket::GetState(){
    return this->E_Socket_State;
}

void E_Socket::SetState(int E_State){
    this->E_Socket_State = E_State;
}

int E_Socket::GetType(){
    return this->E_Socket_Type;
}

void E_Socket::SetType(int E_Type){
    this->E_Socket_Type = E_Type;
}

int E_Socket::GetFD(){
    return this->E_Socket_FD;
}

void E_Socket::SetFD(int E_Socket_FD, int E_Type, int E_State){
    this->E_Socket_FD = E_Socket_FD;
    if(this->SetNonBlock() < 0){
        this->E_Socket_State = E_SOCKET_INVALID;
        this->E_Socket_Type = E_SOCKET_UNKNOWN;
    }
    else{
        this->E_Socket_State = E_State;
        this->E_Socket_Type = E_Type;
    }
}

int E_Socket::Listen(int Listen_Port){
    
    if(this->E_Socket_Type == E_SOCKET_UNKNOWN){
        return -1;
    }
    
    struct sockaddr_in addr;
    
    if(this->E_Socket_Type == E_SOCKET_TCP){
        addr.sin_family = AF_INET;
    }
    else{
        return -1;
    }
    addr.sin_port = htons(Listen_Port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(this->E_Socket_FD, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        this->E_Socket_Type = E_SOCKET_UNKNOWN;
        this->E_Socket_State = E_SOCKET_INVALID;
        return errno;
    }
    if(listen(this->E_Socket_FD, 1) < 0){
        this->E_Socket_Type = E_SOCKET_UNKNOWN;
        this->E_Socket_State = E_SOCKET_INVALID;
        return errno;
    }
    
    this->E_Socket_State = E_SOCKET_LISTEN;
    
    return 0;
    
}

int E_Socket::Listen(int Listen_Port, u_int32_t Listen_On){
    
    if(this->E_Socket_Type == E_SOCKET_UNKNOWN){
        return -1;
    }
    
    struct sockaddr_in addr;
    
    if(this->E_Socket_Type == E_SOCKET_TCP){
        addr.sin_family = AF_INET;
    }
    else{
        return -1;
    }
    addr.sin_port = htons(Listen_Port);
    addr.sin_addr.s_addr = htonl(Listen_On);
    
    if(bind(this->E_Socket_FD, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        this->E_Socket_Type = E_SOCKET_UNKNOWN;
        this->E_Socket_State = E_SOCKET_INVALID;
        return errno;
    }
    if(listen(this->E_Socket_FD, 1) < 0){
        this->E_Socket_Type = E_SOCKET_UNKNOWN;
        this->E_Socket_State = E_SOCKET_INVALID;
        return errno;
    }
    
    this->E_Socket_State = E_SOCKET_LISTEN;
    
    return 0;
    
}

int E_Socket::Listen(int Listen_Port, int E_Listen_On){
    
    if(this->E_Socket_Type == E_SOCKET_UNKNOWN){
        return -1;
    }
    
    struct sockaddr_in addr;
    
    if(this->E_Socket_Type == E_SOCKET_TCP){
        addr.sin_family = AF_INET;
    }
    else{
        return -1;
    }
    addr.sin_port = htons(Listen_Port);
    switch(E_Listen_On){
        case E_LISTEN_ANY:
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
            break;
        case E_LISTEN_LOCAL:
            addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            break;
        default:
            addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            break;
    }
    
    if(bind(this->E_Socket_FD, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        this->E_Socket_Type = E_SOCKET_UNKNOWN;
        this->E_Socket_State = E_SOCKET_INVALID;
        return errno;
    }
    if(listen(this->E_Socket_FD, 1) < 0){
        this->E_Socket_Type = E_SOCKET_UNKNOWN;
        this->E_Socket_State = E_SOCKET_INVALID;
        return errno;
    }
    
    this->E_Socket_State = E_SOCKET_LISTEN;
    
    return 0;
    
}

int E_Socket::AcceptConnectionNonBlocking(std::string* Client_IPv4){
    
    if(this->E_Socket_State != E_SOCKET_LISTEN){
        return -1;
    }
    
    struct sockaddr* addr = (struct sockaddr*)malloc(sizeof(struct sockaddr));
    uint len = sizeof(struct sockaddr);
    
    int sock = -1;
    
    if((sock = accept(this->E_Socket_FD, addr, &len)) > 0){
        if(Client_IPv4 != NULL){
            if(addr->sa_family == AF_INET){
                *Client_IPv4 = std::string(inet_ntoa(((struct sockaddr_in*)addr)->sin_addr));
            }
            else{
                *Client_IPv4 = "";
            }
        }
    }
    else if(sock < 0 && (errno != EWOULDBLOCK && errno != EAGAIN)){
        this->E_Socket_Type = E_SOCKET_UNKNOWN;
        this->E_Socket_State = E_SOCKET_INVALID;
    }
    
    free(addr);
    
    return sock;
    
}

int E_Socket::AcceptConnectionBlocking(std::string* Client_IPv4){
    
    if(this->E_Socket_State != E_SOCKET_LISTEN){
        return -1;
    }
    
    struct sockaddr* addr = (struct sockaddr*)malloc(sizeof(struct sockaddr));
    uint len = sizeof(struct sockaddr);
    
    int sock = -1;
    
    bool keep_trying = true;
    
    while(keep_trying){
        
        if((sock = accept(this->E_Socket_FD, addr, &len)) > 0){
            if(Client_IPv4 != NULL){
                if(addr->sa_family == AF_INET){
                    *Client_IPv4 = std::string(inet_ntoa(((struct sockaddr_in*)addr)->sin_addr));
                }
                else{
                    *Client_IPv4 = "";
                }
            }
            keep_trying = false;
        }
        else if(errno == EAGAIN || errno == EWOULDBLOCK){
            usleep(500);
            // continue loop
        }
        else{
            keep_trying = false;
            this->E_Socket_Type = E_SOCKET_UNKNOWN;
            this->E_Socket_State = E_SOCKET_INVALID;
            sock = -1;
        }
        
    }
    
    return sock;
    
}

long E_Socket::ReadNonBlocking(char* Dest_Buffer, long Max_Bytes){
    
    if(!Dest_Buffer || this->E_Socket_State == E_SOCKET_INVALID){
        return -1;
    }
    
    long bytes = read(this->E_Socket_FD, Dest_Buffer, Max_Bytes);
    
    if(bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)){
        return 0;
    }
    else if(bytes < 0){
        this->E_Socket_State = E_SOCKET_INVALID;
        this->E_Socket_Type = E_SOCKET_UNKNOWN;
        return -1;
    }
    else if(bytes == 0){
        this->E_Socket_State = E_SOCKET_INVALID;
        this->E_Socket_Type = E_SOCKET_UNKNOWN;
        return -1;
    }
    else{
        return bytes;
    }
    
}

E_Server::E_Server(int Listen_Port){
    
    this->E_Server_Socket = new E_Socket(E_SOCKET_TCP);
    if(this->E_Server_Socket->Listen(Listen_Port) < 0){
        
        delete this->E_Server_Socket;
        this->E_Server_Status = E_SERVER_SOCKET_ERROR;
        
        return;
        
    }
    
    this->E_Clients = NULL;
    this->E_Clients_Len = 0;
    this->E_Clients_Lock = false;
    
    this->E_Processing_Threads = NULL;
    this->E_Processing_Len = 0;
    this->E_Processing_Lock = false;
    
    this->E_Server_Run_Loop_t = NULL;
    
    this->E_Server_On_Connect = NULL;
    this->E_Server_On_Error = NULL;
    this->E_Server_On_Data = NULL;
    this->E_Server_On_Disconnected = NULL;
    
    this->E_Server_Status = E_SERVER_OK;
    
}

int E_Server::E_Clients_Expand(int Size){
    
    while(this->E_Clients_Lock){
        usleep(200);
    }
    
    this->E_Clients_Lock = true;
    
    if(this->E_Clients_Len == 0){
        this->E_Clients = (E_Client*)malloc(Size * sizeof(struct E_Client));
    }
    else if(this->E_Clients_Len != 0 && this->E_Clients != NULL){
        
        void* ptr = realloc(this->E_Clients, (this->E_Clients_Len + Size) * sizeof(struct E_Client));
        if(ptr == NULL){
            this->E_Clients_Lock = false;
            
            return -1;
        }
        this->E_Clients = (struct E_Client*)ptr;
        
    }
    
    if(this->E_Clients == NULL){
        
        this->E_Clients_Len = 0;
        this->E_Clients_Lock = false;
        
        return -1;
        
    }
    
    for(long i = this->E_Clients_Len; i < this->E_Clients_Len + Size; i++){
        this->E_Clients[i].E_Client_Socket = NULL;
        new(&this->E_Clients[i].Client_IPv4)std::string; // placement new; create object in allocated space. (this was a pain to find...)
        this->E_Clients[i].Client_IPv4 = "";
    }
    
    this->E_Clients_Len += Size;
    
    this->E_Clients_Lock = false;
    return 0;
    
}

void E_Server::E_Clients_Sort(){
    
    while(this->E_Clients_Lock){
        usleep(200);
    }
    
    this->E_Clients_Lock = true;
    
    long last_free_index = -1;
    
    for(long i = 0; i < this->E_Clients_Len; i++){
        
        if(this->E_Clients[i].E_Client_Socket == NULL && last_free_index == -1){
            
            last_free_index = i;
            
        }
        else if(this->E_Clients[i].E_Client_Socket != NULL){
            
            if(last_free_index != -1){
                
                memmove(&this->E_Clients[last_free_index], &this->E_Clients[i], sizeof(struct E_Client));
                
                this->E_Clients[i].E_Client_Socket = NULL;
                this->E_Clients[i].Client_IPv4 = "";
                
                if(this->E_Clients[last_free_index+1].E_Client_Socket == NULL){
                    last_free_index++;
                }
                else{
                    last_free_index = i;
                }
                
            }
            
        }
        
    }
    
    this->E_Clients_Lock = false;
    
}

int E_Server::E_Clients_Shrink(){
    
    while(this->E_Clients_Lock){
        usleep(200);
    }
    
    this->E_Clients_Lock = true;
    
    for(long i = 0; i < this->E_Clients_Len; i++){
        
        if(this->E_Clients[i].E_Client_Socket != NULL){
            if(this->E_Clients[i].E_Client_Socket->GetState() == E_SOCKET_UNKNOWN){
                
                delete this->E_Clients[i].E_Client_Socket;
                
                this->E_Clients[i].E_Client_Socket = NULL;
                if(i+1 % 8 == 0){ // really not sure what i did here...
                    new(&this->E_Clients[i].Client_IPv4)std::string;
                    this->E_Clients[i].Client_IPv4 = "";
                }
                else{
                    this->E_Clients[i].Client_IPv4 = "";
                }
                
            }
        }
        
    }
    
    this->E_Clients_Lock = false;
    
    this->E_Clients_Sort();
    
    while(this->E_Clients_Lock){
        usleep(200);
    }
    
    this->E_Clients_Lock = true;
    
    long used = 0;
    
    for(long i = 0; i < this->E_Clients_Len; i++){
        if(this->E_Clients[i].E_Client_Socket != NULL){
            used++;
        }
        else{
            break;
        }
    }
    
    while(used % 8 != 0){
        used++;
    }
    
    if(this->E_Clients_Len - used < 8){ // no need to shrink, almost full
        this->E_Clients_Lock = false;
        return 0;
    }
    
    if(used == 0){
        this->E_Clients_Len = used;
        free(this->E_Clients);
        this->E_Clients = NULL;
        this->E_Clients_Lock = false;
        return 0;
    }
    
    void* new_ptr = realloc(this->E_Clients, used * sizeof(E_Client));
    
    if(new_ptr == NULL){
        this->E_Clients_Lock = false;
        return -1;
    }
    
    this->E_Clients = (E_Client*)new_ptr;
    this->E_Clients_Len = used;
    
    this->E_Clients_Lock = false;
    return 0;
    
}

int E_Server::E_Clients_Add(E_Socket* E_Client_Socket, std::string Client_IPv4){
    
    while(this->E_Clients_Lock){
        usleep(200);
    }
    
    this->E_Clients_Lock = true;
    
    bool some_free = false;
    
    for(long i = 0; i < this->E_Clients_Len; i++){
        if(this->E_Clients[i].E_Client_Socket == NULL){
            some_free = true;
            break;
        }
    }
    
    if(!some_free){
        this->E_Clients_Lock = false;
        if(this->E_Clients_Expand(8) < 0){
            return -1;
        }
        while(this->E_Clients_Lock){
            usleep(200);
        }
        this->E_Clients_Lock = true;
    }
    
    for(long i = 0; i < this->E_Clients_Len; i++){
        if(this->E_Clients[i].E_Client_Socket == NULL){
            
            this->E_Clients[i].E_Client_Socket = E_Client_Socket;
            this->E_Clients[i].Client_IPv4 = Client_IPv4;
            
            this->E_Clients_Lock = false;
            
            return 0;
            
        }
    }
    
    this->E_Clients_Lock = false;
    return -1;
    
}

void E_Server::SetOnConnect(void(*E_Server_On_Connect)(E_Socket*, std::string)){
    
    this->E_Server_On_Connect = E_Server_On_Connect;
    
}

void E_Server::SetOnError(void(*E_Server_On_Error)(int)){
    
    this->E_Server_On_Error = E_Server_On_Error;
    
}

void E_Server::SetOnData(void(*E_Server_On_Data)(E_Client*, char*, long, uint64_t, bool*)){
    
    this->E_Server_On_Data = E_Server_On_Data;
    
}

void E_Server::SetOnDisconnected(void(*E_Server_On_Disconnected)(std::string, uint64_t, bool*)){
    
    this->E_Server_On_Disconnected = E_Server_On_Disconnected;
    
}

int E_Server::Start(){
    
    if(this->E_Server_Status == E_SERVER_OK && this->E_Server_Run_Loop_t == NULL){
        this->E_Server_Status = E_SERVER_RUNNING;
        if(pthread_create(&this->E_Server_Run_Loop_t, NULL, &E_Server_Run_Loop, this) < 0){
            this->E_Server_Run_Loop_t = NULL;
            this->E_Server_Status = E_SERVER_THREAD_ERROR;
            return -1;
        }
        usleep(0.5 * 1000000); // waiting 0.5 secs to see if the server actually runs
        if(this->E_Server_Status == E_SERVER_EXITED){
            return -1;
        }
        return 0;
    }
    
    return -1;
    
}

void E_Server::Stop(){
    
    if((this->E_Server_Status == E_SERVER_RUNNING || this->E_Server_Status == E_SERVER_EXITED) && this->E_Server_Run_Loop_t != NULL){
        this->E_Server_Status = E_SERVER_PENDING_SHUTDOWN;
        pthread_join(this->E_Server_Run_Loop_t, NULL);
        this->E_Server_Run_Loop_t = NULL;
    }
    else if(this->E_Server_Run_Loop_t != NULL){
        pthread_join(this->E_Server_Run_Loop_t, NULL);
        this->E_Server_Run_Loop_t = NULL;
    }
    
    if((this->E_Server_Status == E_SERVER_RUNNING || this->E_Server_Status == E_SERVER_PENDING_SHUTDOWN) && this->E_Processing_Threads != NULL){
        this->E_Processing_Sort();
        
        while(this->E_Processing_Lock){
            usleep(200);
        }
        this->E_Processing_Lock = true;
        
        for(long i = 0; i < this->E_Processing_Len; i++){
            *this->E_Processing_Threads[i].is_finished = true;
            pthread_join(this->E_Processing_Threads[i].thread, NULL);
        }
        
        this->E_Processing_Lock = false;
        this->E_Processing_Shrink();
        
        this->E_Server_Status = E_SERVER_PENDING_SHUTDOWN;
    }
    
    this->E_Server_Status = E_SERVER_PENDING_SHUTDOWN;
    
}

E_Server::~E_Server(){
    
    if(this->E_Server_Status != E_SERVER_PENDING_SHUTDOWN){
        this->Stop();
    }
    
    E_Clients_Sort();
    
    while(this->E_Clients_Lock){
        usleep(200);
    }
    this->E_Clients_Lock = true;
    
    for(long i = 0; i < this->E_Clients_Len; i++){
        if(this->E_Clients[i].E_Client_Socket != NULL){
            delete this->E_Clients[i].E_Client_Socket;
        }
    }
    this->E_Clients_Lock = false;
    
    delete this->E_Server_Socket;
    if(this->E_Clients != NULL){
        free(this->E_Clients);
    }
    free(this->E_Processing_Threads);
    
}

int E_Server::E_Processing_Expand(int Size){
    
    while(this->E_Processing_Lock){
        usleep(200);
    }
    this->E_Processing_Lock = true;
    
    if(this->E_Processing_Len == 0){
        
        this->E_Processing_Threads = (E_Thread*)malloc(Size * (sizeof(E_Thread)));
        
        if(this->E_Processing_Threads == NULL){
            this->E_Processing_Lock = false;
            return -1;
        }
        
    }
    else if(this->E_Processing_Len != 0 && this->E_Processing_Threads != NULL){
        
        void* ptr = realloc(this->E_Processing_Threads, this->E_Processing_Len + Size);
        
        if(ptr == NULL){
            this->E_Processing_Lock = false;
            return -1;
        }
        
        this->E_Processing_Threads = (E_Thread*)ptr;
        
    }
    
    for(long i = this->E_Processing_Len; i < this->E_Processing_Len + Size; i++){
        this->E_Processing_Threads[i].thread = NULL;
        this->E_Processing_Threads[i].is_finished = NULL;
    }
    
    this->E_Processing_Len += Size;
    
    this->E_Processing_Lock = false;
    return 0;
    
}

void E_Server::E_Processing_Sort(){
    
    while(this->E_Processing_Lock){
        usleep(200);
    }
    this->E_Processing_Lock = true;
    
    long last_free_index = -1;
    
    for(long i = 0; i < this->E_Processing_Len; i++){
        
        if(this->E_Processing_Threads[i].thread == NULL && last_free_index == -1){
            
            last_free_index = i;
            
        }
        else if(this->E_Processing_Threads[i].thread != NULL){
            
            if(last_free_index != -1){
                
                memmove(&this->E_Processing_Threads[last_free_index], &this->E_Processing_Threads[i], sizeof(E_Thread));
                
                this->E_Processing_Threads[i].thread = NULL;
                this->E_Processing_Threads[i].is_finished = NULL;
                
                if(this->E_Processing_Threads[last_free_index+1].thread == NULL){
                    last_free_index++;
                }
                else{
                    last_free_index = i;
                }
                
            }
            
        }
        
    }
    
    this->E_Processing_Lock = false;
    
}

int E_Server::E_Processing_Shrink(){
    
    while(this->E_Processing_Lock){
        usleep(200);
    }
    this->E_Processing_Lock = true;
    
    for(long i = 0; i < this->E_Processing_Len; i++){
        
        if(this->E_Processing_Threads[i].is_finished != NULL){
            if(*this->E_Processing_Threads[i].is_finished){
                
                pthread_join(this->E_Processing_Threads[i].thread, NULL);
                
                this->E_Processing_Threads[i].thread = NULL;
                free(this->E_Processing_Threads[i].is_finished);
                this->E_Processing_Threads[i].is_finished = NULL;
                
            }
        }
        
    }
    
    this->E_Processing_Lock = false;
    this->E_Processing_Sort();
    
    while(this->E_Processing_Lock){
        usleep(200);
    }
    this->E_Processing_Lock = true;
    
    long used = 0;
    
    for(long i = 0; i < this->E_Processing_Len; i++){
        if(this->E_Processing_Threads[i].is_finished != NULL){
            used++;
        }
        else{
            break;
        }
    }
    
    while(used % 8 != 0){
        used++;
    }
    
    if(this->E_Processing_Len - used < 8){
        this->E_Processing_Lock = false;
        return 0;
    }
    
    void *ptr= realloc(this->E_Processing_Threads, used * sizeof(E_Thread));
    
    if(ptr == NULL){
        this->E_Processing_Lock = false;
        return -1;
    }
    
    this->E_Processing_Threads = (E_Thread*)ptr;
    this->E_Processing_Len = used;
    
    this->E_Processing_Lock = false;
    
    return 0;
    
}

int E_Server::E_Processing_Add(pthread_t Thread, bool* is_finished){
    
    while(this->E_Processing_Lock){
        usleep(200);
    }
    this->E_Processing_Lock = true;
    
    bool some_free = false;
    
    for(long i = 0; i < this->E_Processing_Len; i++){
        if(this->E_Processing_Threads[i].is_finished == NULL){
            some_free = true;
            break;
        }
    }
    
    if(!some_free){
        this->E_Processing_Lock = false;
        if(this->E_Processing_Expand(8) < 0){
            return -1;
        }
        while(this->E_Processing_Lock){
            usleep(200);
        }
        this->E_Processing_Lock = true;
    }
    
    some_free = false;
    
    for(long i = 0; i < this->E_Processing_Len; i++){
        
        if(this->E_Processing_Threads[i].is_finished == NULL){
            
            this->E_Processing_Threads[i].thread = Thread;
            this->E_Processing_Threads[i].is_finished = is_finished;
            
            some_free = true;
            
            break;
            
        }
        
    }
    
    this->E_Processing_Lock = false;
    
    if(!some_free){
        return -1;
    }
    
    return 0;
    
}

void* E_Server::E_Server_On_Data_Thread(void* Arg_Struct){
    
    E_Server_On_Data_Struct* data = (E_Server_On_Data_Struct*)Arg_Struct;
    E_Server* _this = (E_Server*)data->_this;
    
    // suppose the below call terminates when is_finished is set to true.
    // if not its up to the user function to let the server shutdown...
    // in the cleanup we will all wait for you..
    (*_this->E_Server_On_Data)(data->Client, data->buffer, data->buffer_len, data->mach_time, data->is_finished);
    
    if(data->buffer != NULL){
        free(data->buffer);
    }
    
    // in case user forgot..
    *data->is_finished = true;
    
    return NULL;
    
}

void* E_Server::E_Server_On_Disconnected_Thread(void* Arg_Struct){
    
    E_Server_On_Data_Struct* data = (E_Server_On_Data_Struct*)Arg_Struct;
    E_Server* _this = (E_Server*)data->_this;
    
    (*_this->E_Server_On_Disconnected)(std::string(data->buffer, data->buffer_len), data->mach_time, data->is_finished);
    
    if(data->buffer != NULL){
        free(data->buffer);
    }
    
    *data->is_finished = true;
    
    return NULL;
    
}

void* E_Server::E_Server_Run_Loop(void* data){
    
    E_Server* _this = static_cast<E_Server*>(data);
    
    while(_this->E_Server_Status == E_SERVER_RUNNING){
        
        /*
         * BEGIN accepting new clients
         */
        std::string cli = "";
        int sock = _this->E_Server_Socket->AcceptConnectionNonBlocking(&cli);
        
        if(sock == -1 && _this->E_Server_Socket->GetState() == E_SOCKET_INVALID){ // Server should be Stop()'ed and closed if this happens..
            _this->E_Server_Status = E_SERVER_SOCKET_ERROR;
            if(_this->E_Server_On_Error != NULL){
                (*_this->E_Server_On_Error)(_this->E_Server_Status);
            }
            // TODO: request server shutdown here somehow.
            break;
        }
        else if(sock == -1){
            //usleep(200);
        }
        else{
            
            E_Socket* Client = new E_Socket();
            
            Client->SetFD(sock);
            if(Client->GetState() == E_SOCKET_INVALID){ // only happens if the socket fd cant be set to nonblocking
                if(_this->E_Server_On_Error != NULL){
                    (*_this->E_Server_On_Error)(E_SERVER_ADDCLI_ERROR);
                }
                delete Client;
            }
            else{
                
                Client->SetType(E_SOCKET_TCP);
                Client->SetState(E_SOCKET_CONN);
                
                if(_this->E_Clients_Add(Client, cli) < 0){
                    if(_this->E_Server_On_Error != NULL){
                        (*_this->E_Server_On_Error)(E_SERVER_ADDCLI_ERROR);
                    }
                    delete Client;
                }
                else{
                    if(_this->E_Server_On_Connect != NULL){
                        (*_this->E_Server_On_Connect)(Client, cli);
                    }
                }
                
            }
            
        }
        /*
         * END accepting new clients
         */
        
        /*
         * BEGIN reading from client
         */
        if(_this->E_Server_On_Data != NULL){
            
            _this->E_Clients_Sort();
            while(_this->E_Clients_Lock){
                usleep(200);
            }
            _this->E_Clients_Lock = true;
            
            for(long i = 0; i < _this->E_Clients_Len; i++){
                
                if(_this->E_Clients[i].E_Client_Socket != NULL){
                    if(_this->E_Clients[i].E_Client_Socket->GetState() == E_SOCKET_CONN){
                        
                        char* buff = (char*)malloc(512 * sizeof(char));
                        
                        if(buff != NULL){
                            
                            void* ptr = malloc(sizeof(bool));
                            if(ptr != NULL){
                                
                                long bytes = _this->E_Clients[i].E_Client_Socket->ReadNonBlocking(buff, 512);
                                
                                if(bytes > 0){
                                    pthread_t thread;
                                    E_Server_On_Data_Struct* data = (E_Server_On_Data_Struct*)malloc(sizeof(E_Server_On_Data_Struct));
                                    
                                    data->Client = &_this->E_Clients[i];
                                    data->buffer = buff;
                                    data->buffer_len = bytes;
                                    data->mach_time = mach_absolute_time();
                                    data->is_finished = (bool*)ptr;
                                    *data->is_finished = false;
                                    data->_this = _this;
                                    
                                    if(pthread_create(&thread, NULL, &_this->E_Server_On_Data_Thread, data) != -1){
                                        if(_this->E_Processing_Add(thread, data->is_finished) < 0 && _this->E_Server_On_Error != NULL){
                                            (*_this->E_Server_On_Error)(E_SERVER_ADD_USERTHREAD_ERROR);
                                        }
                                    }
                                    else if(_this->E_Server_On_Error != NULL){
                                        (*_this->E_Server_On_Error)(E_SERVER_CREATE_USERTHREAD_ERROR);
                                    }
                                }
                                else if(bytes == -1 && _this->E_Server_On_Disconnected != NULL){
                                    pthread_t thread;
                                    E_Server_On_Data_Struct* data = (E_Server_On_Data_Struct*)malloc(sizeof(E_Server_On_Data_Struct));
                                    
                                    strncpy(buff, _this->E_Clients[i].Client_IPv4.c_str(), _this->E_Clients[i].Client_IPv4.length());
                                    
                                    data->Client = NULL;
                                    data->buffer = buff;
                                    data->buffer_len = _this->E_Clients[i].Client_IPv4.length();
                                    data->mach_time = mach_absolute_time();
                                    data->is_finished = (bool*)ptr;
                                    *data->is_finished = false;
                                    data->_this = _this;
                                    
                                    if(pthread_create(&thread, NULL, &_this->E_Server_On_Disconnected_Thread, data) != -1){
                                        if(_this->E_Processing_Add(thread, data->is_finished) < 0 && _this->E_Server_On_Error != NULL){
                                            (*_this->E_Server_On_Error)(E_SERVER_ADD_USERTHREAD_ERROR);
                                        }
                                    }
                                    else if(_this->E_Server_On_Error != NULL){
                                        (*_this->E_Server_On_Error)(E_SERVER_CREATE_USERTHREAD_ERROR);
                                    }
                                }
                                
                            }
                            
                        }
                        
                    }
                    
                }
                else{
                    break; // should be sorted so no client comes after the last filled one
                }
                
            }
            
            _this->E_Clients_Lock = false;
            
        }
        /*
         * END reading from clients
         */
        
        /*
         * BEGIN cleanup arrays
         */
        if(_this->E_Processing_Shrink() < 0 && _this->E_Server_On_Error != NULL){
            (*_this->E_Server_On_Error)(E_SERVER_SHRINK_USERPROCESS_ERROR);
        }
        if(_this->E_Clients_Shrink() < 0 && _this->E_Server_On_Error != NULL){
            (*_this->E_Server_On_Error)(E_SERVER_SHRINK_INTERNSOCK_ERROR);
        }
        /*
         * END cleanup arrays
         */
        
        usleep(50000); //rest a bit
        
    }
    
    _this->E_Server_Status = E_SERVER_EXITED;
    
    return NULL;
    
}

