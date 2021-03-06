#include <network/myRDMA.hpp>

static std::mutex mutx;


char* change(string temp){
  static char stc[BufSize];
  strcpy(stc, temp.c_str());
  return stc;
}

myRDMA::myRDMA()
{
    for (int i  =0 ; i<3; i++){
        send_buffer[i] = new char[BufSize];
        recv_buffer[i] = new char[BufSize];
    }
}
myRDMA::~myRDMA()
{
    for (int i  =0 ; i<3; i++){
        delete(send_buffer[i]);
        delete(recv_buffer[i]);
    }
}

void myRDMA::send_rdma(char* msg, int i){
    RDMA rdma;
    TCP tcp;
    if(strcmp(msg,"exit\n")==0)
        strcpy(send_buffer[i],msg);

    else{
        strcpy(send_buffer[i],msg);
    }
    rdma.post_rdma_send(get<4>(rdma_info[0][i]), get<5>(rdma_info[0][i]), send_buffer[i], 
                         sizeof(send_buffer[i]), qp_key[i].first, qp_key[i].second);
    if(rdma.pollCompletion(get<3>(rdma_info[0][i])) == true)
        cout << "send success" << endl;
    else
        cout << "send failed" << endl;
}

void myRDMA::write_rdma(char *msg, int i){
    RDMA rdma;
    TCP tcp;
    if(strcmp(msg,"exit\n")==0)
        strcpy(send_buffer[i],msg);

    else{
        strcpy(send_buffer[i],msg);
    }
    string strsoso = qp_key[i].first;
    string strnono = qp_key[i].second;

    rdma.post_rdma_write(get<4>(rdma_info[0][i]), get<5>(rdma_info[0][i]), send_buffer[i], 
                         BufSize, qp_key[i].first, qp_key[i].second);
    if(rdma.pollCompletion(get<3>(rdma_info[0][i])) ==true){
        cout << "send success" << endl;
        tcp.send_msg("1", sock_idx[i]);
    }
    else
        cout << "send failed" << endl;
}

void myRDMA::write_rdma_with_imm(char *msg, int i){
    RDMA rdma;
    TCP tcp;

    if(strcmp(msg,"exit\n")==0)
        strcpy(send_buffer[i],msg);

    else{
        strcpy(send_buffer[i],msg);
    }

    rdma.post_rdma_write_with_imm(get<4>(rdma_info[0][i]), get<5>(rdma_info[0][i]), send_buffer[i], 
                                sizeof(send_buffer[i]), qp_key[i].first, qp_key[i].second);
    rdma.pollCompletion(get<3>(rdma_info[0][i]));
    
}
void myRDMA::send_recv_rdma(int i, int socks_cnt){
    TCP tcp;
    RDMA rdma;
    printf("hello~");
    while(1){
        rdma.post_rdma_recv(get<4>( rdma_info[1][i]), get<5>( rdma_info[1][i]), 
                            get<3>( rdma_info[1][i]), recv_buffer[i], BufSize);
        rdma.pollCompletion(get<3>( rdma_info[1][i]));
        mutx.lock();

        cout << "SEND:  recv_buffer[" <<i<<"] = ";
        printf("%s\n",  recv_buffer[i]);

        mutx.unlock();
    }
}

void myRDMA::write_recv_rdma(int i, int socks_cnt){
    TCP tcp;
    RDMA rdma;
    int cnt = 0;
    while(1){
        cnt +=1;
        if(tcp.recv_msg(sock_idx[i]) != 0){
            mutx.lock();
            cout << cnt<<": WRITE: recv_buffer[" <<i<<"] = ";
            printf("%s\n",  recv_buffer[i]); 
            mutx.unlock();
        }
    }

}

void myRDMA::send_t(int socks_cnt){
    char msg[BufSize];

    while(1){
        fgets(msg,BufSize,stdin);
        /*if(myrdma.thread_cnt != socks_cnt && strcmp(msg,"exit\n")!=0){
            cout << "No exit."<<endl;
            continue;
        }*/

        for(int i=0;i<socks_cnt;i++){
            myRDMA::send_rdma(msg,i);
        }

        if(strcmp(msg,"exit\n")==0){
             check_exit = 1;
            break;
        }
    }
}
void myRDMA::rdma_send_msg(int socks_cnt, const char* opcode, char* msg){
    //char mmsg[BufSize];
    //fgets(mmsg,BufSize,stdin);
    if (strcmp(opcode,"send") == 0){
        cout << "send_rdma run" << endl;
        for(int i=0;i<socks_cnt;i++){
            myRDMA::send_rdma(msg,i);
        }
    }
    else{
        cout << "write_rdma run" << endl;
        for(int i=0;i<socks_cnt;i++){
            myRDMA::write_rdma(msg,i);
        }
    }
}
void myRDMA::recv_t(int socks_cnt, const char* opcode){
    std::vector<std::thread> worker;
    if (strcmp(opcode,"send") == 0){
        for(int i=0;i<socks_cnt;i++){
            worker.push_back(std::thread(&myRDMA::send_recv_rdma,myRDMA(),i,socks_cnt));
             thread_cnt++;
        }
    }
    else{
        for(int i=0;i<socks_cnt;i++){
            worker.push_back(std::thread(&myRDMA::write_recv_rdma,myRDMA(),i,socks_cnt));
             thread_cnt++;
        }
    }
    printf("hello~1\n");
    printf("vector len : %d\n", worker.size());
    printf("socks_cnt : %d\n", socks_cnt);
    while(worker.size()>0){
        worker.back().join();
        printf("hello~2\n");
    }
}

void myRDMA::run_chat(int socks_cnt){
    cout << "-----Start chat-----" <<endl;
    std::thread snd_msg = std::thread(&myRDMA::send_t,myRDMA(),socks_cnt);
    myRDMA::recv_t(socks_cnt,"send");

    snd_msg.join();
}

void myRDMA::send_info_change_qp(int socks_cnt, std::vector<tuple<
                struct ibv_context*,
                struct ibv_pd*, 
                int, struct ibv_cq*,
                struct ibv_qp*, struct ibv_mr*,
                uint16_t, uint32_t>> * rdma_info){
    TCP tcp;
    RDMA rdma;
    //Send RDMA info
    for(int k = 0;k<2;k++){
        int *clnt_socks = tcp.client_sock();
        cout << "Sending rdma info[" << k << "]... ";
        if(k==0){
            for(int idx=0; idx < NumOfServer; idx++){
                if(clnt_socks[idx]!=0){
                     sock_idx.push_back(idx);
                }
            }
        }
        for(int j=0;j<socks_cnt;j++){
            std::ostringstream oss;

            if(k==0)
                oss << & send_buffer[j];
            else
                oss << & recv_buffer[j];
            
            tcp.send_msg(change(oss.str()+"\n"), sock_idx[j]);
            tcp.send_msg(change(to_string(get<5>( rdma_info[k][j])->length)+"\n"), sock_idx[j]);
            tcp.send_msg(change(to_string(get<5>( rdma_info[k][j])->lkey)+"\n"), sock_idx[j]);
            tcp.send_msg(change(to_string(get<5>( rdma_info[k][j])->rkey)+"\n"), sock_idx[j]);
            tcp.send_msg(change(to_string(get<6>( rdma_info[k][j]))+"\n"), sock_idx[j]);
            tcp.send_msg(change(to_string(get<7>( rdma_info[k][j]))+"\n"), sock_idx[j]);
            
        }
        cout << "[ SUCCESS ]" <<endl;
        //Read RDMA info
        map<string, string> read_rdma_info;
        cout << "Changing queue pair...  ";
        for(int i=0;i< rdma_info[k].size();i++){
            read_rdma_info = tcp.read_rdma_info( sock_idx[i]);
            //Exchange queue pair state

            string s1 = read_rdma_info.find("qp_num")->second;
            string s2 = read_rdma_info.find("lid")->second;

            rdma.changeQueuePairStateToInit(get<4>( rdma_info[k^1][i]));
            rdma.changeQueuePairStateToRTR(get<4>( rdma_info[k^1][i]), PORT, 
                                           stoi(read_rdma_info.find("qp_num")->second), 
                                           stoi(read_rdma_info.find("lid")->second));


            int real_k = k ^ 1;
            if(k==1){
                rdma.changeQueuePairStateToRTS(get<4>( rdma_info[k^1][i]));
                qp_key.push_back(make_pair(read_rdma_info.find("addr")->second,read_rdma_info.find("rkey")->second));
            }
        }
        cout << "[ SUCCESS ]" << endl;
    }
    cout << "Completely success" << endl;
}

//
// ?????? ?????? ???????????? ????????? ?????? ????????? ??? ???
//
std::vector<tuple<struct ibv_context*, struct ibv_pd*, 
                int, struct ibv_cq*,
                struct ibv_qp*, struct ibv_mr*,
                uint16_t, uint32_t>>* myRDMA::create_rdma_info(int socks_cnt){
    RDMA rdma;
    TCP tcp;
    cout << "Creating rdma info...   ";
    char* buf[3];
    for(int j =0;j<2;j++){
        if(j==1){
            buf[0] = recv_buffer[0];
            buf[1] = recv_buffer[1];
            buf[2] = recv_buffer[2];
        }
        else{
            buf[0] = send_buffer[0];
            buf[1] = send_buffer[1];
            buf[2] = send_buffer[2];
        }
        for(int i =0;i<socks_cnt;i++){
            struct ibv_context* context = rdma.createContext();
            struct ibv_pd* protection_domain = ibv_alloc_pd(context);
            int cq_size = 0x10;
            struct ibv_cq* completion_queue = ibv_create_cq(context, cq_size, nullptr, nullptr, 0);
            struct ibv_qp* qp = rdma.createQueuePair(protection_domain, completion_queue);
            struct ibv_mr *mr = rdma.registerMemoryRegion(protection_domain, 
                                                    buf[i], BufSize);
            uint16_t lid = rdma.getLocalId(context, PORT);
            uint32_t qp_num = rdma.getQueuePairNumber(qp);
             rdma_info[j].push_back(make_tuple(context,protection_domain,cq_size,
                                            completion_queue,qp,mr,lid,qp_num));
        }
    }
    cout << "[ SUCCESS ]" << endl;

    return rdma_info;
}
int myRDMA::cnt_thread(){
    return  thread_cnt;
}
void myRDMA::exit_rdma(int socks_cnt){
    TCP tcp;
    for(int j=0;j<2;j++){
        for(int i=0;i<socks_cnt;i++){
            ibv_destroy_qp(get<4>( rdma_info[j][i]));
            ibv_dereg_mr(get<5>( rdma_info[j][i]));
            ibv_destroy_cq(get<3>( rdma_info[j][i]));
            ibv_dealloc_pd(get<1>( rdma_info[j][i]));
            ibv_close_device(get<0>( rdma_info[j][i]));
        }
    }
    /*cout <<""<<endl;
    //}
    cout<<"|?????????????????????|"<<endl;
    cout<<"|   ????????????   |"<<endl;
    cout<<"|?????????????????????|"<<endl; 
    cout<<"(???__???) ||     "<<endl;
    cout<<"( ????????? ) ||      "<<endl;
    cout<<"/ . . . .???    "<<endl;*/

    exit(0);
}


