#include <network/rdmaBenchmark.hpp>
#include <stdio.h>

uint64_t timeDiff(struct timeval stop, struct timeval start) {
	return (stop.tv_sec * 1000000L + stop.tv_usec) - (start.tv_sec * 1000000L + start.tv_usec);
}

void rdmaBenchmark::run_bench(int socks_cnt, int msg_size, bool isServer)
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
        
        bench_send(socks_cnt, msg_size, msg);

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
}

void rdmaBenchmark::run_bench_write(int socks_cnt, int msg_size, bool isServer, std::vector<tuple<struct ibv_context*,
                struct ibv_pd*, 
                int, struct ibv_cq*,
                struct ibv_qp*, struct ibv_mr*,
                uint16_t, uint32_t>> * rdma_info)
{
    cerr << "<-------  " << msg_size << "bytes 벤치마크 테스트 시작-------------->" << endl;
    
    if(isServer)
    {
        cerr << "나는 수신만 할게" << endl;
        write_recv_rdma(0, 1);
    }
    else
    {
        cerr << "나는 송신만 할게" << endl;

        char msg[BufSize];
        memset(msg, 'A', msg_size -1);
        msg[msg_size] = '\0';

        struct timeval start, stop;
		gettimeofday(&start, NULL);
        
        bench_write(socks_cnt, msg_size, msg, rdma_info);

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
}

void rdmaBenchmark::bench_send(int socks_cnt, int msg_size, char *msg)
{

    for (int iter =0; iter<ITERATION; iter++) {
        //fgets(msg,MsgSize,stdin);
        // 메시지 입력받을 이유 x

        for(int i=0;i<socks_cnt;i++){
            RDMA rdma;
            TCP tcp;
            //char name_msg[BufSize+strlen(name)];

            strcpy(send_buffer[i],msg);

            rdma.post_rdma_send(get<4>(rdma_info[0][i]), get<5>(rdma_info[0][i]), send_buffer[i], 
                                msg_size, qp_key[i].first, qp_key[i].second);
            rdma.pollCompletion(get<3>(rdma_info[0][i]));
        }
    }
}

void rdmaBenchmark::bench_write(int socks_cnt, int msg_size, char *msg, std::vector<tuple<struct ibv_context*, struct ibv_pd*, 
                int, struct ibv_cq*,
                struct ibv_qp*, struct ibv_mr*,
                uint16_t, uint32_t>> *rdma_info)
{

    for (int iter =0; iter<ITERATION; iter++) {
        //fgets(msg,MsgSize,stdin);
        // 메시지 입력받을 이유 x

        for(int i=0;i<socks_cnt;i++){
            RDMA rdma;
            TCP tcp;
            //char name_msg[BufSize+strlen(name)];

            strcpy(send_buffer[i],msg);


            rdma.post_rdma_write_with_imm(get<4>(rdma_info[0][i]), get<5>(rdma_info[0][i]), send_buffer[i], 
                         msg_size, qp_key[i].first, qp_key[i].second);
    
            rdma.pollCompletion(get<3>(rdma_info[0][i]));
        }
    }
}

void rdmaBenchmark::bench_revb()
{

}

void rdmaBenchmark::Recv_t(int socks_cnt){
    std::vector<std::thread> worker;

    for(int i=0;i<socks_cnt;i++){
        worker.push_back(std::thread(&rdmaBenchmark::Recv_RDMA,rdmaBenchmark(),i,socks_cnt));
        thread_cnt++;
    }
    sleep(1);
    for(int i=0;i<socks_cnt;i++){
        worker[i].detach();
    }
}

void rdmaBenchmark::Recv_RDMA(int i, int socks_cnt){
    TCP tcp;
    RDMA rdma;
    for (int iter=0; iter<ITERATION; iter++) {
        //char msg[BufSize];

        rdma.post_rdma_recv(get<4>(rdma_info[1][i]), get<5>(rdma_info[1][i]), 
                            get<3>(rdma_info[1][i]),recv_buffer[i], BufSize);

        rdma.pollCompletion(get<3>(rdma_info[1][i]));
    }
}