#define BufSize 1048580
#include <vector>
#include <string>
#include <network/tcp.hpp>
#include <network/RDMA.hpp>
#include <time.h>

using namespace std;


class myRDMA{
    public:
        std::vector<tuple<struct ibv_context*, struct ibv_pd*, 
                int, struct ibv_cq*,
                struct ibv_qp*, struct ibv_mr*,
                uint16_t, uint32_t>> rdma_info[2];
        char *send_buffer[3];
        char *recv_buffer[3];
        vector<int> sock_idx;

        myRDMA();
        ~myRDMA();
        char send_buffer[3][BufSize];
        char recv_buffer[3][BufSize];
        vector<int> sock_idx;

        void send_rdma(char* msg, int i);
        void write_rdma(char *msg, int i);
        void write_rdma_with_imm(char *msg, int i);
        void send_recv_rdma(int i, int socks_cnt);
        void write_recv_rdma(int i, int socks_cnt);
        void rdma_send_msg(int socks_cnt, const char* opcode, char* msg);
        void send_t(int socks_cnt);
        void recv_t(int socks_cnt, const char* opcode);
        void run_chat(int socks_cnt);
        std::vector<tuple<struct ibv_context*, struct ibv_pd*, 
                int, struct ibv_cq*,
                struct ibv_qp*, struct ibv_mr*,
                uint16_t, uint32_t>>* create_rdma_info(int socks_cnt);
        void send_info_change_qp(int socks_cnt, std::vector<tuple<struct ibv_context*, struct ibv_pd*, 
                int, struct ibv_cq*,
                struct ibv_qp*, struct ibv_mr*,
                uint16_t, uint32_t>> * rdmaInfo_ptr);
        void exit_rdma(int socks_cnt);
        int cnt_thread();

        std::vector<pair<string,string>> qp_key;
        int thread_cnt = 0;
        int check_exit = 0;
};
