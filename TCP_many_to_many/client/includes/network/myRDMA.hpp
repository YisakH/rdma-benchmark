#define BufSize 1048580
#include <vector>
#include <string>
#include <network/tcp.hpp>
#include <network/RDMA.hpp>
#include <time.h>

using namespace std;


class myRDMA{
    public:
        void send_rdma(char* msg, int i);
        void write_rdma(char *msg, int i);
        void write_rdma_with_imm(char *msg, int i);
        void send_recv_rdma(int i, int socks_cnt);
        void write_recv_rdma(int i, int socks_cnt);
        void rdma_send_msg(int socks_cnt, const char* opcode, char* msg);
        void send_t(int socks_cnt);
        void recv_t(int socks_cnt, const char* opcode);
        void run_chat(int socks_cnt);
        void create_rdma_info(int socks_cnt);
        void send_info_change_qp(int socks_cnt);
        void exit_rdma(int socks_cnt);
        int cnt_thread();

        std::vector<pair<string,string>> qp_key;
        int thread_cnt = 0;
        int check_exit = 0;
};