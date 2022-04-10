#include <network/customRDMA.hpp>

uint64_t timeDiff(struct timeval stop, struct timeval start) {
	return (stop.tv_sec * 1000000L + stop.tv_usec) - (start.tv_sec * 1000000L + start.tv_usec);
}


void customRDMA::run_bench(int socks_cnt, char *name, int msg_size, bool isServer)
{
    cerr << "<-------  " << msg_size << "bytes 벤치마크 테스트 시작-------------->" << endl;
    
    if(isServer)
    {
        cerr << "나는 수신만 할게" << endl;
        Recv_RDMA(0, socks_cnt);
    }
    else
    {
        cerr << "나는 송신만 할게" << endl;

        char msg[BufSize];
        memset(msg, 'A', msg_size -1);
        msg[msg_size] = '\0';

        struct timeval start, stop;
		gettimeofday(&start, NULL);
        
        bench_send(socks_cnt, name, msg_size, msg);

		gettimeofday(&stop, NULL);


        uint64_t time = timeDiff(stop, start);
        printf("total time : %ld\n", time);
        double msec = ((double)time) / 1000000L * 1000;

        double msgRate = ((double)(ITERATION * 1000000L)) / time;
        double bandwidth = ((double) (ITERATION * msg_size)) / (1024*1024) / (((double) time) / 1000000L);
        double latency = ((double) msec) / ITERATION;
        printf("%.3f msg/sec\t%.3f MB/sec\n", msgRate, bandwidth);
        printf("latency : %.3fms\n", latency);
        fflush(stdout);
    }
    
    /*
    std::thread snd_msg = std::thread(&customRDMA::bench_send,customRDMA(),socks_cnt,name, msg_size);
    customRDMA::Recv_t(socks_cnt);

    snd_msg.join();
    */
}

void customRDMA::bench_send(int socks_cnt, char *name, int msg_size, char *msg)
{

    for (int iter =0; iter<ITERATION; iter++) {
        //fgets(msg,MsgSize,stdin);
        // 메시지 입력받을 이유 x

        for(int i=0;i<socks_cnt;i++){
            customRDMA::Send_RDMA(msg,i,name, msg_size);
        }
    }
}

void customRDMA::Send_RDMA(char *msg, int i,char *name, int msg_size){
    RDMA rdma;
    TCP tcp;
    //char name_msg[BufSize+strlen(name)];

    strcpy(send_buffer[i],msg);


    rdma.post_rdma_send(get<4>(rdmaInfo[0][i]), get<5>(rdmaInfo[0][i]), send_buffer[i], 
                         msg_size, qp_key[i].first, qp_key[i].second);
    rdma.pollCompletion(get<3>(rdmaInfo[0][i]));
}


void customRDMA::bench_revb()
{

}

void customRDMA::Recv_t(int socks_cnt){
    std::vector<std::thread> worker;

    for(int i=0;i<socks_cnt;i++){
        worker.push_back(std::thread(&customRDMA::Recv_RDMA,customRDMA(),i,socks_cnt));
        thread_cnt++;
    }
    sleep(1);
    for(int i=0;i<socks_cnt;i++){
        worker[i].detach();
    }
}

void customRDMA::Recv_RDMA(int i, int socks_cnt){
    TCP tcp;
    RDMA rdma;

    sleep(5);
    
    for (int iter=0; iter<ITERATION; iter++) {
        //char msg[BufSize];

        rdma.post_rdma_recv(get<4>(rdmaInfo[1][i]), get<5>(rdmaInfo[1][i]), 
                            get<3>(rdmaInfo[1][i]),recv_buffer[i], BufSize);

        rdma.pollCompletion(get<3>(rdmaInfo[1][i]));
    }
}