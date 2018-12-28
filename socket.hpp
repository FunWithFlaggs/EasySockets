//
//  socket.hpp
//  EasySockets
//
//  Created by Ezra Knobloch on 27.12.18.
//  Copyright © 2018 Ezra Knobloch. All rights reserved.
//

#ifndef socket_hpp
#define socket_hpp

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <mach/mach_time.h> // replace this with something else to run on linux

// E_Type
#define E_SOCKET_UNKNOWN 1
#define E_SOCKET_TCP 2

// socket states
#define E_SOCKET_INVALID 1
#define E_SOCKET_OK 2
#define E_SOCKET_CONN 3
#define E_SOCKET_LISTEN 4

//E_Listen_On
#define E_LISTEN_ANY 1
#define E_LISTEN_LOCAL 2

//E_Server_Status
#define E_SERVER_OK 1
#define E_SERVER_SOCKET_ERROR 2
#define E_SERVER_PENDING_SHUTDOWN 3
#define E_SERVER_THREAD_ERROR 4
#define E_SERVER_RUNNING 5
#define E_SERVER_EXITED 6

//E_Server_Error (for callback)
#define E_SERVER_ADDCLI_ERROR 1
#define E_SERVER_ADD_USERTHREAD_ERROR 2
#define E_SERVER_SHRINK_USERPROCESS_ERROR 3
#define E_SERVER_SHRINK_INTERNSOCK_ERROR 4
#define E_SERVER_CREATE_USERTHREAD_ERROR 5

/*
 * The base class describing a socket.
 *
 * at this time this wrapper is only capable of creating
 * listening tcp sockets. All sockets (except the "empty" one)
 * are non-blocking, tho there is a pseudo blocking call
 * for each feature (accept, read, etc.) if needed.
 *
 * functions generally return -1 on error.
 */
class E_Socket{
    
public:
    
    /*
     * creates an "empty" socket, internally set as invalid.
     * use this if you need to set the file descriptor,
     * state and type manually.
     * NOTE: if the fd is set manually it will still be set to non-blocking.
     */
    E_Socket();
    /*
     * creates a socket either with defined proto and type
     * (proto is eg. AF_INET, type is eg. SOCK_STREAM)
     * or with an "easy" type (see #define's above).
     * NOTE: currently only TCP sockets are supported, you need to
     * avoid creating a different socket here!
     */
    E_Socket(int Socket_Proto, int Socket_Type);
    E_Socket(int E_Type);
    
    /*
     * the underlying socket is closed on object destruction.
     */
    ~E_Socket();
    
    // see #define's above
    int GetState();
    void SetState(int E_State);
    // see #define's above
    int GetType();
    void SetType(int E_Type);
    
    // get the underlying socket file descriptor (which can be invalid, see E_Socket()).
    int GetFD();
    /*
     * sets the underlying socket file descriptor. SetNonBlock() is called on it then.
     * If it fails to set the socket to non-blocking the socket is internally
     * set as invalid (which will lead to all functions working on it
     * (like accepting, reading, listening) to return -1 and fail.
     */
    void SetFD(int E_Socket_FD, int E_Type = E_SOCKET_TCP, int E_State = E_SOCKET_CONN);
    
    /*
     * put the underlying socket into listening state defining a port to
     * listen on. Optionally pass an address to listen on, see
     * #define's above or netinet/in.h (sin_addr.s_addr)
     */
    int Listen(int Listen_Port);
    int Listen(int Listen_Port, u_int32_t Listen_On);
    int Listen(int Listen_Port, int E_Listen_On);
    
    /*
     * tries to accept one connection on the underlying socket.
     * returns immediately either with -1 or with the sockets fd
     * for the new connection.
     *
     * -1 can be returned either if:
     *  - the socket is not in listening mode
     *  - there is no connection to accept
     *  - an error occurred
     *
     * on error the internal type and state are changed accordingly
     * so you can check those.
     *
     * the Client_IPv4 will be filled with the remote IPv4 address
     * as string (like you would expect to read it humanly).
     * if it is empty you most likely are not using a AF_INET socket.
     */
    int AcceptConnectionNonBlocking(std::string* Client_IPv4);
    /*
     * identical to the non-blocking function except that it blocks
     * until either:
     *  - a connection was accepted and the fd can be returned
     *  - an error occurred and the internal type and state are set accordingly
     */
    int AcceptConnectionBlocking(std::string* Client_IPv4);
    
    // return 0 on no data, -1 on true error, else bytes read
    /*
     * tries to read data from the underlying socket, return immediately
     * either with:
     *  - 0 when no data was read
     *  - -1 whn there was an error
     *  - the amount of bytes read
     *
     * the internal state and type are set accordingly on error.
     *
     * data read is put into Dest_Buffer, up to Max_Bytes bytes of data.
     * (so this can crash when you supply wrong values!)
     */
    long ReadNonBlocking(char* Dest_Buffer, long Max_Bytes);
    
private:
    
    int E_Socket_FD;
    int E_Socket_State;
    int E_Socket_Type;
    
    int SetNonBlock();
    
};

/*
 * a class defining one client connected to the tcp server below.
 */
struct E_Client{
    
    /*
     * the sockets must be regulary checked for their state
     * to determine if they still can be used and if not
     * they need to be closed/destroyed!!!
     */
    E_Socket* E_Client_Socket;
    std::string Client_IPv4;
    
};

/*
 * this struct stores the thread in which a callback is running along with it's
 * running flag (see the setters in the server class for details).
 */
struct E_Thread{
    
    pthread_t thread;
    /*
     * thread needs to check this in order to guarantee a lagfree/deadlock-free running
     * they also need to set this to true when finished, else this would result in a
     * complete server deadlock...
     */
    bool* is_finished;
    
};

/*
 * this struct is used as helping hand to call the callbacks for the server events.
 * it mostly holds the arguments needed to call the function.
 */
struct E_Server_On_Data_Struct{
    
    E_Client* Client;
    char* buffer;
    long buffer_len;
    uint64_t mach_time;
    bool* is_finished;
    void* _this;
    
};

/*
 * the base class describing a multithreaded tcp server.
 *
 * the server class makes use of the above E_Socket class to maintain
 * an array of connected client structures (E_Client class).
 * it is extremly simple to setup and run a multi purpose tcp server
 * using this class, you only need a few lines of code.
 *
 * there are different events you can register callbacks for, some of them
 * run in their own thread, some not (and thus should not take too much time
 * to not slow down the whole server).
 * TODO for here: let all callbacks run in their own thread. (except the error function)
 *
 * OnConnect()      - runs in server thread
 * OnError()        - runs in server thread
 * OnData()         - runs in its own thread
 * OnDisconnected() - runs in its own thread
 *
 */
class E_Server{
    
public:
    
    /*
     * initialize the server and configure it for the specified port.
     * this will already create a socket and set it to listen on the
     * given port.
     * the server sets its internal state accordingly.
     */
    E_Server(int Listen_Port);
    /*
     * if not already done the server is stopped.
     * then each client structure is beeng deleted and thus all remaining
     * connections are closed (using close()).
     * the server socket is closed and the internal thread array is free'd.
     */
    ~E_Server();
    
    /*
     * starts the server's main run loop.
     *
     * on success 0 is returned, on error -1.
     * the internal status is set accordingly.
     */
    int Start();
    /*
     * the main run loop's thread gets joined.
     * all remaining callback thread's get joined.
     * the internal state is set to pending_shutdown.
     */
    void Stop();
    
    /*
     * the setters for the callback functions.
     * NOTE: be sure to create a callback function according to theese below,
     * else you will get crashes/undefined stuff happening.
     *
     * NOTE: this is important to callbacks running in their own thread:
     * the last argument is a pointer to a boolean. This is a flag, it
     * indicates if the thread is finished (true) or not (false).
     * this flag may be set by some other functions (eg. Stop()) to
     * indicate that the thread should exit (early).
     * if your thread does not respond to this this may slow down
     * server shutdown or other parts of the server core (that may get
     * added later on).
     * you should also set that flag to true when your function ends, altho
     * this is also done by the server in case you forget...
     */
    void SetOnConnect(void(*E_Server_On_Connect)(E_Socket*, std::string));
    void SetOnError(void(*E_Server_On_Error)(int));
    void SetOnData(void(*E_Server_On_Data)(E_Client*, char*, long, uint64_t, bool*));
    void SetOnDisconnected(void(*E_Server_On_Disconnected)(std::string, uint64_t, bool*));
    
private:
    
    E_Socket* E_Server_Socket;
    E_Client* E_Clients; // this is a pointer to an array
    
    long E_Clients_Len;
    bool E_Clients_Lock;
    
    E_Thread* E_Processing_Threads; // this is a pointer to an array
    
    long E_Processing_Len;
    bool E_Processing_Lock;
    
    int E_Server_Status;
    
    pthread_t E_Server_Run_Loop_t;
    
    /*
     * expands the underlying E_Clients array by Size objects.
     * the newly created objects are initialized with default values
     * which are empty or NULL.
     * uses realloc() to expand the allocated space.
     */
    int E_Clients_Expand(int Size);
    /*
     * sorts the underlying E_Clients array so that used objects are
     * on the "left" and empty objects are on the "right".
     * uses memmove() to move objects in memory.
     */
    void E_Clients_Sort();
    /*
     * goes through the underlying E_Clients array and delete's E_Client's
     * that have a unknown state, thus some sort of garbage collection.
     * then sorts the array by calling E_Clients_Sort() and if needed
     * realloc()'es the array to shrink its size.
     */
    int E_Clients_Shrink(); // also checks E_Socket state
    /*
     * adds the given E_Client to the underlying array expanding its
     * size if needed.
     */
    int E_Clients_Add(E_Socket* E_Client_Socket, std::string Client_IPv4);
    
    /*
     * expands the underlying E_Processing_Threads array by Size objects.
     * the newly created objects are initialized with default values
     * which are empty or NULL;
     * this will also init the array if it was not done before.
     * uses realloc() to expand the allocated space.
     */
    int E_Processing_Expand(int Size);
    /*
     * sorts the underlying E_Processing_Threads array so that used
     * objects are on the "left" and empty objects are on the "right".
     * useses memmove() to move objects in memory.
     */
    void E_Processing_Sort();
    /*
     * goes through the underlying E_Processing_Threads array and
     * joins callback threads that have a is_running flag set to true.
     * then calls E_Processing_Sort() and if needed realloc()'es the
     * array to shrink its size.
     * this is some sort of garbage collection for the callback threads array.
     */
    int E_Processing_Shrink();
    /*
     * adds the given E_Thread to the underlying array expanding its size
     * if needed.
     */
    int E_Processing_Add(pthread_t Thread, bool* is_finished);
    
    /*
     * this is the server's main run loop, it runs until the server's
     * status is not E_SERVER_RUNNING.
     *
     * 1. runs AcceptConnectionNonBlocking() on the server socket.
     *      1.1 if this fails and the server socket is invalid,
     *          the loop is exited.
     * 2. if there is a new connection, create an E_Client and
     *    add it to the underlying array.
     *      2.1 call OnConnect() if it was defined, runs inside
     *          this thread and can lead to huge server delays.
     * 3. if OnData() is defined, go through the E_Clients array
     *    and try to read data from each connected socket using
     *    ReadNonBlocking().
     * 4. if there was some data (512 byte chunks), run OnData()
     *    in its own thread and add that thread to the underlying array.
     *      4.1 if there was an error reading from the socket call
     *          OnDisconnected() (if it was defined) in its own thread
     *          (and add that thread to the underlying array).
     * 5. after going through the E_Clients array, call
     *    E_Processing_Shrink() and E_Clients_Shrink().
     */
    static void* E_Server_Run_Loop(void* data);
    
    // function pointers to the user supplied callbacks implementing the events.
    void (*E_Server_On_Connect)(E_Socket*, std::string Client_IPv4);
    void (*E_Server_On_Error)(int E_Server_Status_Or_Error);
    void (*E_Server_On_Data)(E_Client* E_Client_Struct, char* Received_Data, long Data_Len, uint64_t Event_Time, bool* Stop_Requested);
    void (*E_Server_On_Disconnected)(std::string Client_IPv4, uint64_t Event_Time, bool* Stop_Request);
    
    // functions used to run the above callbacks in their own threads.
    static void* E_Server_On_Data_Thread(void* Arg_Struct);
    static void* E_Server_On_Disconnected_Thread(void* Arg_Struct);
    
};

#endif /* socket_hpp */
