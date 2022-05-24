#define BufSize 10485800
#include <vector>
#include <string>

using namespace std;
class myRDMA{
    public:
        void send_rdma(char* msg, int i, int msg_size);
        void write_rdma(char *msg, int i, int msg_size);
        void read_rdma(char *msg, int i, int msg_size);
        int read_recv_rdma(int i, int socks_cnt, int msg_size);
        void write_rdma_with_imm(char *msg, int i, int msg_size);
        int send_recv_rdma(int i, int socks_cnt, int msg_size);
        int write_recv_rdma(int i, int socks_cnt, int msg_size);
        void rdma_send_msg(int socks_cnt, const char* opcode, char* msg, int msg_size);
        void send_t(int socks_cnt);
        int recv_t(int socks_cnt, const char* opcode, int msg_size);
        void rdma_run(int socks_cnt, const char* opcode, char* msg, int msg_size);
        void create_rdma_info(int socks_cnt);
        void send_info_change_qp(int socks_cnt);
        void set_buffer(char send[][BufSize], char recv[][BufSize]);
        void exit_rdma(int socks_cnt);
        int cnt_thread();
        RDMA rdma;
    private:
        std::vector<tuple<struct ibv_context*, struct ibv_pd*, 
                        int, struct ibv_cq*,
                        struct ibv_qp*, struct ibv_mr*,
                        uint16_t, uint32_t>> rdma_info[2];
        std::vector<pair<string,string>> qp_key;
        int thread_cnt = 0;
        int check_exit = 0;
        char (*send_buffer)[BufSize];
        char (*recv_buffer)[BufSize];
        vector<int> sock_idx;
};