#pragma once

#include "tcp.hpp"
#include "RDMA.hpp"

#include <time.h>

#define BufSize 1048580
    
class myRDMA{
public:

    static std::mutex mtex;
    std::vector<pair<string,string>> qp_key;
    vector<int> server_ip;
    int thread_cnt = 0;
    int check_exit = 0;
    char *send_buffer[NumOfServer];
    char *recv_buffer[NumOfServer];
    
    std::vector<tuple<struct ibv_context*,
                            struct ibv_pd*, 
                            int, struct ibv_cq*,
                            struct ibv_qp*,
                            struct ibv_mr*,
                            uint16_t,
                            uint32_t>> rdmaInfo[2];

    myRDMA();

    void Send_RDMA(char *msg, int i, char *name);
    void Recv_RDMA(int i, int socks_cnt);
    void Send_t(int socks_cnt, char *name);
    void Recv_t(int socks_cnt);
    void run_chat(int socks_cnt, char* name);
    void Create_Rdmainfo(int socks_cnt);
    void SendInfo_ChangeQP(int socks_cnt, int ip);
    void exitRDMA(int socks_cnt);
    int cnt_thread();
};