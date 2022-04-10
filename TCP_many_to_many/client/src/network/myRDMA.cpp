#include "tcp.hpp"
#include "RDMA.hpp"
#include "myRDMA.hpp"
#include <time.h>
static std::mutex mutx;
myRDMA myrdma;


char* change(string temp){
  static char stc[BufSize];
  strcpy(stc, temp.c_str());
  return stc;
}

void myRDMA::send_rdma(char* msg, int i){
    RDMA rdma;
    TCP tcp;
    if(strcmp(msg,"exit\n")==0)
        strcpy(myrdma.send_buffer[i],msg);

    else{
        strcpy(myrdma.send_buffer[i],msg);
    }
    rdma.post_rdma_send(get<4>(myrdma.rdma_info[0][i]), get<5>(myrdma.rdma_info[0][i]), myrdma.send_buffer[i], 
                         sizeof(myrdma.send_buffer[i]), myrdma.qp_key[i].first, myrdma.qp_key[i].second);
    if(rdma.pollCompletion(get<3>(myrdma.rdma_info[0][i])) == true)
        cout << "send success" << endl;
    else
        cout << "send failed" << endl;
}
void myRDMA::write_rdma(char *msg, int i){
    RDMA rdma;
    TCP tcp;
    if(strcmp(msg,"exit\n")==0)
        strcpy(myrdma.send_buffer[i],msg);

    else{
        strcpy(myrdma.send_buffer[i],msg);
    }

    rdma.post_rdma_write(get<4>(myrdma.rdma_info[0][i]), get<5>(myrdma.rdma_info[0][i]), myrdma.send_buffer[i], 
                         sizeof(myrdma.send_buffer[i]), myrdma.qp_key[i].first, myrdma.qp_key[i].second);
    if(rdma.pollCompletion(get<3>(myrdma.rdma_info[0][i])) ==true){
        cout << "send success" << endl;
        tcp.send_msg("1", myrdma.sock_idx[i]);
    }
    else
        cout << "send failed" << endl;
}
void myRDMA::write_rdma_with_imm(char *msg, int i){
    RDMA rdma;
    TCP tcp;

    if(strcmp(msg,"exit\n")==0)
        strcpy(myrdma.send_buffer[i],msg);

    else{
        strcpy(myrdma.send_buffer[i],msg);
    }

    rdma.post_rdma_write_with_imm(get<4>(myrdma.rdma_info[0][i]), get<5>(myrdma.rdma_info[0][i]), myrdma.send_buffer[i], 
                                sizeof(myrdma.send_buffer[i]), myrdma.qp_key[i].first, myrdma.qp_key[i].second);
    rdma.pollCompletion(get<3>(myrdma.rdma_info[0][i]));
    
}
void myRDMA::send_recv_rdma(int i, int socks_cnt){
    TCP tcp;
    RDMA rdma;
    while(1){
        rdma.post_rdma_recv(get<4>(myrdma.rdma_info[1][i]), get<5>(myrdma.rdma_info[1][i]), 
                            get<3>(myrdma.rdma_info[1][i]),myrdma.recv_buffer[i], sizeof(myrdma.recv_buffer[i]));
        rdma.pollCompletion(get<3>(myrdma.rdma_info[1][i]));
        mutx.lock();

        cout << "SEND:  recv_buffer[" <<i<<"] = ";
        printf("%s\n", myrdma.recv_buffer[i]);

        mutx.unlock();
    }
    //}
    /*while(1){
        char msg[BufSize];

        rdma.post_rdma_recv(get<4>(myrdma.rdma_info[1][i]), get<5>(myrdma.rdma_info[1][i]), 
                            get<3>(myrdma.rdma_info[1][i]),myrdma.recv_buffer[i], sizeof(myrdma.recv_buffer[i]));
        rdma.pollCompletion(get<3>(myrdma.rdma_info[1][i]));
        
        if(strcmp(myrdma.recv_buffer[i],"exit\n")==0){
            myrdma.thread_cnt--;
            if(myrdma.thread_cnt==socks_cnt-1 && myrdma.check_exit == 0){
                sleep(0.5);
                cout << "----  Another server has exit  ----"<< endl;
                cout << "----     Please type exit      ----" << endl;
            }
            break;
        }

        mutx.lock();

        cout << "recv_buffer[" <<i<<"] = ";
        printf("%s", myrdma.recv_buffer[i]);

        mutx.unlock();
    }*/
}

void myRDMA::write_recv_rdma(int i, int socks_cnt){
    TCP tcp;
    RDMA rdma;
    int cnt = 0;
    while(1){
        cnt +=1;
        if(tcp.recv_msg(myrdma.sock_idx[i]) != 0){
            mutx.lock();
            cout << cnt<<": WRITE: recv_buffer[" <<i<<"] = ";
            printf("%s\n", myrdma.recv_buffer[i]); 
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
            myrdma.check_exit = 1;
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
            myrdma.thread_cnt++;
        }
    }
    else{
        for(int i=0;i<socks_cnt;i++){
            worker.push_back(std::thread(&myRDMA::write_recv_rdma,myRDMA(),i,socks_cnt));
            myrdma.thread_cnt++;
        }
    }
    for(int i=0;i<socks_cnt;i++){
        worker[i].join();
    }
}

void myRDMA::run_chat(int socks_cnt){
    cout << "-----Start chat-----" <<endl;
    std::thread snd_msg = std::thread(&myRDMA::send_t,myRDMA(),socks_cnt);
    myRDMA::recv_t(socks_cnt,"send");

    snd_msg.join();
}

void myRDMA::send_info_change_qp(int socks_cnt){
    TCP tcp;
    RDMA rdma;
    //Send RDMA info
    for(int k = 0;k<2;k++){
        int *clnt_socks = tcp.client_sock();
        cout << "Sending rdma info[" << k << "]... ";
        if(k==0){
            for(int idx=0; idx < NumOfServer; idx++){
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
        cout << "[ SUCCESS ]" <<endl;
        //Read RDMA info
        map<string, string> read_rdma_info;
        cout << "Changing queue pair...  ";
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
        cout << "[ SUCCESS ]" << endl;
    }
    cout << "Completely success" << endl;
}
void myRDMA::create_rdma_info(int socks_cnt){
    RDMA rdma;
    TCP tcp;
    cout << "Creating rdma info...   ";
    char (*buf)[BufSize];
    for(int j =0;j<2;j++){
        if(j==1){
            buf = &myrdma.recv_buffer[0];
        }
        else
            buf = &myrdma.send_buffer[0];
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
    cout << "[ SUCCESS ]" << endl;
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
    /*cout <<""<<endl;
    //}
    cout<<"|￣￣￣￣￣￣￣|"<<endl;
    cout<<"|   채팅종료   |"<<endl;
    cout<<"|＿＿＿＿＿＿＿|"<<endl; 
    cout<<"(＼__／) ||     "<<endl;
    cout<<"( •ㅅ• ) ||      "<<endl;
    cout<<"/ . . . .づ    "<<endl;*/

    exit(0);
}


