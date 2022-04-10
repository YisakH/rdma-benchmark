#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include "map"
#define PORT1 40145
#define BUF_SIZE 100
#define NAME_SIZE 20
#define NumOfServer 3

using namespace std;
class TCP{
    public:
        void server(const char* server[]);
        void connect_tcp(const char* ip, const char* server[]);
        void Run(const char* iip);
        void client(const char* ip, int idx);
        void client_t(const char* ip,const char* server[]);
        void send_msg(const char* m,int ip);
        int recv_msg(int ip);
        int s_cnt();
        int *client_sock();
        map<string, string> read_rdma_info(int ip);
    private:
        int sock;
        struct sockaddr_in serv_addr;
        struct sockaddr_in serv_adr, clnt_adr;
        int serv_sock, clnt_sock; 
        int clnt_adr_size;
        std::vector<std::thread> workers;
        char buffer[1048676] = {0};
        int valread;
        string result;
        string read_char;
        int clnt_cnt=0;
        int sock_cnt=0;
        int new_sock[NumOfServer+1];
        int clnt_socks[NumOfServer+1];
};