#include "tcp.hpp"
#include "RDMA.hpp"
#include "myRDMA.hpp"
#include <time.h>
#include <sys/time.h>

#define MAX_SEND_BYTES static_cast<long>(10737418240) // 10GB
#define MAX_MSG_SIZE 10485760
#define MAX_ITERATION 1000000

uint64_t timeDiff(struct timeval stop, struct timeval start)
{
    return (stop.tv_sec * 1000000L + stop.tv_usec) - (start.tv_sec * 1000000L + start.tv_usec);
}

static std::mutex mutx;
myRDMA myrdma;


char* change(string temp){
  static char stc[BufSize];
  strcpy(stc, temp.c_str());
  return stc;
}

void myRDMA::send_rdma(char* msg, int i, int msg_size, vector<pair<struct timeval, struct timeval>> *bench_time){
    RDMA rdma;

    for (int msg_size = 1; msg_size <= MAX_MSG_SIZE; msg_size *= 2)
        {
            struct timeval start, end;
            bench_time->push_back({start, end});

            memset(send_buffer, msg_size - 1, 'A');
            msg[msg_size - 1] = '\0';
            long long iteration = MAX_SEND_BYTES / msg_size;
            iteration = (iteration > MAX_ITERATION) ? MAX_ITERATION : iteration;

            gettimeofday(&(bench_time->back().first), NULL);

            strcpy(myrdma.send_buffer[i],msg);

            //cerr << "전송 준비 완료" << endl;

            for (int iter = 0; iter < iteration; iter++)
            {
                rdma.post_rdma_send(get<4>(myrdma.rdma_info[0][i]),
                                    get<5>(myrdma.rdma_info[0][i]),
                                    myrdma.send_buffer[i],
                                    msg_size,
                                    myrdma.qp_key[i].first,
                                    myrdma.qp_key[i].second);
                if (rdma.pollCompletion(get<3>(myrdma.rdma_info[0][i])) == true)
                {
                    // cerr << "send success" << endl;
                }
                else
                {
                    printf("send error\n");
                    exit(-1);
                }
            }
            gettimeofday(&(bench_time->back().second), NULL);
        }
}

void myRDMA::write_rdma(char *msg, int i, int msg_size){
    RDMA rdma;
    TCP tcp;
   
    //strcpy(myrdma.send_buffer[i],msg);
    
    rdma.post_rdma_write(get<4>(myrdma.rdma_info[0][i]), get<5>(myrdma.rdma_info[0][i]), myrdma.send_buffer[i], 
                         msg_size, myrdma.qp_key[i].first, myrdma.qp_key[i].second);
    if(rdma.pollCompletion(get<3>(myrdma.rdma_info[0][i])) ==true){
        //cerr << "send success" << endl;
        //tcp.send_msg("1", myrdma.sock_idx[i]);
    }
    else
        cerr << "send failed" << endl;
}

void myRDMA::read_rdma(char *msg, int i, int msg_size){
    RDMA rdma;
    TCP tcp;
   
    //strcpy(myrdma.send_buffer[i],msg);
    
    rdma.post_rdma_read(get<4>(myrdma.rdma_info[0][i]), get<5>(myrdma.rdma_info[0][i]), myrdma.send_buffer[i], 
                         msg_size, myrdma.qp_key[i].first, myrdma.qp_key[i].second);
    if(rdma.pollCompletion(get<3>(myrdma.rdma_info[0][i])) ==true){
        //cerr << "send success" << endl;
        //tcp.send_msg("1", myrdma.sock_idx[i]);
    }
    else
        cerr << "read failed" << endl;
}

void myRDMA::write_rdma_with_imm(char *msg, int i, int msg_size){
    RDMA rdma;

    strcpy(myrdma.send_buffer[i],msg);
    
    rdma.post_rdma_write_with_imm(get<4>(myrdma.rdma_info[0][i]), get<5>(myrdma.rdma_info[0][i]), myrdma.send_buffer[i], 
                                msg_size, myrdma.qp_key[i].first, myrdma.qp_key[i].second);
    if(rdma.pollCompletion(get<3>(myrdma.rdma_info[0][i])) ==true){
        //cerr << "send success" << endl;
    }
    else
        cout << "send failed" << endl;
    
}
int myRDMA::send_recv_rdma(int i, int socks_cnt, int msg_size){
    RDMA rdma;

    for (int msg_size = 1; msg_size <= MAX_MSG_SIZE; msg_size *= 2)
    {
        long long iteration = MAX_SEND_BYTES / msg_size;
        iteration = (iteration > MAX_ITERATION) ? MAX_ITERATION : iteration;
        for (int iter = 0; iter < iteration; iter++)
        {

            rdma.post_rdma_recv(get<4>(myrdma.rdma_info[1][i]), get<5>(myrdma.rdma_info[1][i]),
                                get<3>(myrdma.rdma_info[1][i]), myrdma.recv_buffer[i], msg_size);
            rdma.pollCompletion(get<3>(myrdma.rdma_info[1][i]));
        }
        /*mutx.lock();

        cerr << "SEND:  recv_buffer[" <<i<<"] = ";
        printf("%s\n", myrdma.recv_buffer[i]);

        mutx.unlock();*/
        // cerr << myrdma.recv_buffer[i] << endl;
    }
    return 1;
}

int myRDMA::write_recv_rdma(int i, int socks_cnt, int msg_size){
    
    usleep(10);
    return 0;
}

int myRDMA::read_recv_rdma(int i, int socks_cnt, int msg_size){
    
    usleep(10);
    return 0;
}


void myRDMA::rdma_send_msg(int socks_cnt, const char* opcode, char* msg, int msg_size){

    std::vector<std::thread> worker;
    std::vector<std::vector<std::pair<struct timeval, struct timeval>>> bench_time(socks_cnt);

    if (strcmp(opcode,"send") == 0){
        //cerr << "send_rdma run" <<endl;
        for(int i=0;i<socks_cnt;i++){
            worker.push_back(thread(&myRDMA::send_rdma, myRDMA(),msg, i, msg_size, &bench_time[i]));
        }
    }
    else if(strcmp(opcode,"write") == 0){
        //cerr << "write_rdma run" << endl;
        for(int i=0;i<socks_cnt;i++){
            write_rdma(msg, i, msg_size);
        }
    }
    else if(strcmp(opcode,"write_with_imm") == 0){
        //cerr << "write_with_imm_rdma run" <<endl;
        for(int i=0;i<socks_cnt;i++){
            write_rdma_with_imm(msg, i, msg_size);
        }
    }
    else if(strcmp(opcode, "read") == 0){
        for(int i=0; i<socks_cnt; i++){
            read_rdma(msg, i, msg_size);
        }
    }
    else{
        cerr << "rdma_send_msg opcode error" << endl;
        exit(-1);
    }

    for(int i=0; i<socks_cnt; i++){
        worker[i].join();
    }
    cout << "정상 종료" << endl;
}
int myRDMA::recv_t(int socks_cnt, const char* opcode, int msg_size){
    std::vector<std::thread> worker;

    if (strcmp(opcode,"send") == 0){
        for(int i=0;i<socks_cnt;i++){
            worker.push_back(
                std::thread(&myRDMA::send_recv_rdma, myRDMA(),i, socks_cnt, msg_size)
                );
        }
    }
    else if(strcmp(opcode,"write") == 0){
        for(int i=0;i<socks_cnt;i++){
            write_recv_rdma(i, socks_cnt, msg_size);
        }
    }
    else if (strcmp(opcode,"write_with_imm") == 0){
        for(int i=0;i<socks_cnt;i++){
            send_recv_rdma(i, socks_cnt, msg_size);
        }
    }
    else if (strcmp(opcode,"read") == 0){
        for(int i=0;i<socks_cnt;i++){
            read_recv_rdma(i, socks_cnt, msg_size);
        }
    }
    else{
        cerr << "recv_t opcode error" << endl;
        exit(-1);
    }
    for(int i=0; i<socks_cnt; i++)
    {
        worker[i].join();
    }
    return 1;
}

void myRDMA::fucking_rdma(int socks_cnt, const char* opcode, char* msg, int msg_size){
    //char *ms;
    //ms = change(msg);
    thread snd_msg = thread(&myRDMA::rdma_send_msg,myRDMA(), socks_cnt, opcode, msg, msg_size);
    recv_t(socks_cnt, opcode, msg_size);
    //myRDMA::recv_t(socks_cnt,opcode);

    snd_msg.join();

}

void myRDMA::send_info_change_qp(int socks_cnt){
    TCP tcp;
    RDMA rdma;
    //Send RDMA info
    for(int k = 0;k<2;k++){
        int *clnt_socks = tcp.client_sock();
        cerr << "Sending rdma info[" << k << "]... ";
        if(k==0){
            for(int idx=0; idx < socks_cnt+1; idx++){
                if(clnt_socks[idx]!=0){
                    myrdma.sock_idx.push_back(idx);
                }
            }
        }
        for(int j=0;j<socks_cnt;j++){
            std::ostringstream oss;

            if(k==0)
                oss << &myrdma.send_buffer[j];
            else
                oss << &myrdma.recv_buffer[j];
            
            tcp.send_msg(change(oss.str()+"\n"),myrdma.sock_idx[j]);
            tcp.send_msg(change(to_string(get<5>(myrdma.rdma_info[k][j])->length)+"\n"),myrdma.sock_idx[j]);
            tcp.send_msg(change(to_string(get<5>(myrdma.rdma_info[k][j])->lkey)+"\n"),myrdma.sock_idx[j]);
            tcp.send_msg(change(to_string(get<5>(myrdma.rdma_info[k][j])->rkey)+"\n"),myrdma.sock_idx[j]);
            tcp.send_msg(change(to_string(get<6>(myrdma.rdma_info[k][j]))+"\n"),myrdma.sock_idx[j]);
            tcp.send_msg(change(to_string(get<7>(myrdma.rdma_info[k][j]))+"\n"),myrdma.sock_idx[j]);
            
        }
        cerr << "[ SUCCESS ]" <<endl;
        //Read RDMA info
        map<string, string> read_rdma_info;
        cerr << "Changing queue pair...  ";
        for(int i=0;i<myrdma.rdma_info[k].size();i++){
            read_rdma_info = tcp.read_rdma_info(myrdma.sock_idx[i]);
            //Exchange queue pair state
            rdma.changeQueuePairStateToInit(get<4>(myrdma.rdma_info[k^1][i]));
            rdma.changeQueuePairStateToRTR(get<4>(myrdma.rdma_info[k^1][i]), PORT, 
                                           stoi(read_rdma_info.find("qp_num")->second), 
                                           stoi(read_rdma_info.find("lid")->second));
                
            if(k^1==0){
                rdma.changeQueuePairStateToRTS(get<4>(myrdma.rdma_info[k^1][i]));
                myrdma.qp_key.push_back(make_pair(read_rdma_info.find("addr")->second,read_rdma_info.find("rkey")->second));
            }
        }
        cerr << "[ SUCCESS ]" << endl;
    }
    cerr << "Change Info success" << endl;
}
void myRDMA::create_rdma_info(int socks_cnt){
    RDMA rdma;
    TCP tcp;
    cerr << "Creating rdma info...   ";
    char (*buf)[BufSize];
    for(int j =0;j<2;j++){
        if(j==1){
            buf = &myrdma.recv_buffer[0];
            if(!buf){
                cerr << "Error please set_buffer() recv_buffer" << endl;
                exit(1);
            }
            //cerr << buf << endl;
        }
        else{
            buf = &myrdma.send_buffer[0];
            if(!buf){
                cerr << "\n";
                cerr << "Error please set_buffer() send_buffer" << endl;
                exit(1);
            }
            //cerr << buf << endl;
        }
        for(int i =0;i<socks_cnt;i++){
            struct ibv_context* context = rdma.createContext();
            struct ibv_pd* protection_domain = ibv_alloc_pd(context);
            int cq_size = 0x10;
            struct ibv_cq* completion_queue = ibv_create_cq(context, cq_size, nullptr, nullptr, 0);
            struct ibv_qp* qp = rdma.createQueuePair(protection_domain, completion_queue);
            struct ibv_mr *mr = rdma.registerMemoryRegion(protection_domain, 
                                                    buf[i], sizeof(buf[i]));
            uint16_t lid = rdma.getLocalId(context, PORT);
            uint32_t qp_num = rdma.getQueuePairNumber(qp);
            myrdma.rdma_info[j].push_back(make_tuple(context,protection_domain,cq_size,
                                            completion_queue,qp,mr,lid,qp_num));
        }
    }
    cerr << "[ SUCCESS ]" << endl;
}
void myRDMA::set_buffer(char send[][BufSize], char recv[][BufSize]){
    myrdma.send_buffer = &send[0];
    myrdma.recv_buffer = &recv[0];
}
int myRDMA::cnt_thread(){
    return myrdma.thread_cnt;
}
void myRDMA::exit_rdma(int socks_cnt){
    TCP tcp;
    for(int j=0;j<2;j++){
        for(int i=0;i<socks_cnt;i++){
            ibv_destroy_qp(get<4>(myrdma.rdma_info[j][i]));
            ibv_dereg_mr(get<5>(myrdma.rdma_info[j][i]));
            ibv_destroy_cq(get<3>(myrdma.rdma_info[j][i]));
            ibv_dealloc_pd(get<1>(myrdma.rdma_info[j][i]));
            ibv_close_device(get<0>(myrdma.rdma_info[j][i]));
        }
    }
    exit(0);
}


