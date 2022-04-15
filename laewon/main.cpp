#include "tcp.hpp"
#include "RDMA.hpp"
#include "myRDMA.hpp"
#include <sys/time.h>

#define num_of_server 2
#define MAX_SEND_BYTES static_cast<long>(10737418240) // 10GB
#define MAX_MSG_SIZE 10485760
#define MAX_ITERATION 100000

const char *server[num_of_server] = {"192.168.1.100", "192.168.1.101"};

uint64_t timeDiff(struct timeval stop, struct timeval start)
{
    return (stop.tv_sec * 1000000L + stop.tv_usec) - (start.tv_sec * 1000000L + start.tv_usec);
}

// static char *send_buffer[num_of_server];
// static char *recv_buffer[num_of_server];

char send_buffer[num_of_server][BufSize];
char recv_buffer[num_of_server][BufSize];
char msg[BufSize];

// char msg[BufSize];

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cout << argv[0] << " <MY IP> " << endl;
        exit(1);
    }

    char *opcode = argv[3];

    TCP tcp;
    myRDMA myrdma;
    int socks_cnt;

    /*
    for(int i=0; i<num_of_server; i++){
      send_buffer[i] = new char[BufSize];
      recv_buffer[i] = new char[BufSize];
    }
    */

    tcp.set_num_of_server(num_of_server);

    cout << "Connecting tcp" << endl;
    tcp.connect_tcp(argv[1], server);
    cout << "Connection completely success" << endl;

    socks_cnt = num_of_server - 1;

    myrdma.set_buffer((char(*)[BufSize])send_buffer, (char(*)[BufSize])recv_buffer);
    myrdma.create_rdma_info(socks_cnt);
    myrdma.send_info_change_qp(socks_cnt);

    cout << "======================================================" << endl;

    if (strcmp(argv[2], "s") == 0)
    {
        time_t timer = time(NULL);
        struct tm *t = localtime(&timer);

        ofstream writeFile;
        string date;
        date = "" + to_string(t->tm_year - 100) + "0" + to_string(t->tm_mon + 1) + to_string(t->tm_mday) +
                "_" +
                to_string(t->tm_hour) +
                to_string(t->tm_min) + to_string(t->tm_sec);
        string filename(opcode);
        filename = "./logs/" + filename;
        filename += date;
        filename += ".txt";
        writeFile.open(filename);

        if(!writeFile.is_open()){
            cerr << "file open error" << endl;
        }
        char *tmp = "M_Size #_of_Msg throughput latency\n";
        writeFile.write(tmp, strlen(tmp));

        /*
        myrdma.fucking_rdma(socks_cnt, "send", "Yisak is Handsome");

        for(int i = 0; i<socks_cnt;i++){
          cout << "recv_buffer["<< i <<"] SEND: "<<recv_buffer[i]<< endl;
        }
        */
        
        sleep(1);

        for (int msg_size = 1; msg_size <= MAX_MSG_SIZE; msg_size *= 2)
        {
            memset(send_buffer, msg_size - 1, 'A');
            msg[msg_size - 1] = '\0';
            long long iteration = MAX_SEND_BYTES / msg_size;
            iteration = (iteration > MAX_ITERATION) ? MAX_ITERATION : iteration;

            cerr << "<---- " << opcode << " : " << msg_size << "bytes 벤치마크 테스트 시작 ---------->" << endl;

            struct timeval start, stop;
            gettimeofday(&start, NULL);

            for (int iter = 0; iter < iteration; iter++)
            {
                myrdma.fucking_rdma(socks_cnt, opcode, msg, msg_size);
            }

            gettimeofday(&stop, NULL);

            uint64_t time = timeDiff(stop, start);
            printf("total time : %ld\n", time);
            double msec = ((double)time) / 1000000L * 1000;

            double msgRate = ((double)(iteration * 1000000L)) / time;
            double bandwidth = ((double)(iteration * msg_size)) / (1024 * 1024) / (((double)time) / 1000000L);
            double latency = ((double)msec) / iteration;
            printf("%.3f msg/sec\t%.3f MB/sec\n", msgRate, bandwidth);
            printf("latency : %.3fms\n", latency);
            fflush(stdout);

            // delete [] msg;

            
            char send_data[100];
            sprintf(send_data, "%d %.3f %.3f %.3f\n", msg_size, msgRate, bandwidth, latency);
            writeFile.write(send_data, strlen(send_data));


        }

        writeFile.close();
    }
    else
    {
        for (int msg_size = 1; msg_size <= MAX_MSG_SIZE; msg_size *= 2)
        {
            cerr << "<---- " << opcode << " : " << msg_size << "bytes 벤치마크 테스트 시작 ---------->" << endl;
            long long iteration = MAX_SEND_BYTES / msg_size;
            iteration = (iteration > MAX_ITERATION) ? MAX_ITERATION : iteration;

            for (int i = 0; i < socks_cnt; i++)
            {
                for (int iter = 0; iter < iteration; iter++)
                {
                    myrdma.recv_t(socks_cnt, opcode, msg_size);
                    // cerr << "SEND: " << recv_buffer[i] << endl;
                }
            }
        }
    }

    // sleep(10);
    myrdma.exit_rdma(socks_cnt);

    return 0;
}
