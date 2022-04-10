#include "myRDMA.hpp"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#define ITERATION 1000
class customRDMA : public myRDMA
{
public:
    void run_bench(int socks_cnt, char *name, int msg_size, bool isServer);
    
private:
    //int thread_cnt = 0;
    //int check_exit = 0;
    /*
    std::vector<tuple<struct ibv_context*,
                        struct ibv_pd*, 
                        int, struct ibv_cq*,
                        struct ibv_qp*,
                        struct ibv_mr*,
                        uint16_t,
                        uint32_t>> rdmaInfo[2];*/

    //std::vector<pair<string,string>> qp_key;
    
    //char send_buffer[NumOfServer][BufSize];
    //char recv_buffer[NumOfServer][BufSize];


    void bench_send(int socks_cnt, char *name, int msg_size, char *msg);
    void bench_revb();
    void Recv_t(int socks_cnt);
    void Recv_RDMA(int i, int socks_cnt);
    void Send_RDMA(char *msg, int i,char *name, int msg_size);

};