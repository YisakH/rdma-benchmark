#include "myRDMA.hpp"
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

#define ITERATION 1000
class rdmaBenchmark : public myRDMA
{
public:
    rdmaBenchmark();
    void run_bench(int socks_cnt, int msg_size, bool isServer);
    void run_bench_write(int socks_cnt, int msg_size, bool isServer);
    
private:
    //char *send_buffer[3];
    //char *recv_buffer[3];
    void bench_send(int socks_cnt, int msg_size, char *msg);
    void bench_write(int socks_cnt, int msg_size, char *msg);
    void bench_revb();
    void Recv_t(int socks_cnt);
    void Recv_RDMA(int i, int socks_cnt);
    void Send_RDMA(char *msg, int i, int msg_size);

};