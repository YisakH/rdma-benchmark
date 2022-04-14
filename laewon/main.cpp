#include "tcp.hpp"
#include "RDMA.hpp"
#include "myRDMA.hpp"
#include <sys/time.h>

#define num_of_server 2
#define MAX_SEND_BYTES static_cast<long>(10737418240) // 10GB
#define MAX_MSG_SIZE 10485760
#define MAX_ITERATION 1000000

const char *server[num_of_server] = {"192.168.1.100", "192.168.1.101"};

// static char *send_buffer[num_of_server];
// static char *recv_buffer[num_of_server];

char send_buffer[num_of_server][BufSize];
char recv_buffer[num_of_server][BufSize];
char msg[BufSize - 1];

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

    if (!writeFile.is_open())
    {
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

    writeFile.close();

    // sleep(10);
    myrdma.exit_rdma(socks_cnt);

    return 0;
}