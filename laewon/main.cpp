#include "tcp.hpp"
#include "RDMA.hpp"
#include "myRDMA.hpp"
#include <sys/time.h>

#define num_of_server 6
#define MAX_SEND_BYTES static_cast<long>(10737418240) // 10GB
#define MAX_MSG_SIZE 10485760
#define MAX_ITERATION 10000

const char *server[num_of_server] = {"192.168.1.100", "192.168.1.101", "192.168.1.102", "192.168.1.103", "192.168.1.104", "192.168.1.105"};

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

    int msg_size = atoi(argv[2]);

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

    myrdma.fucking_rdma(socks_cnt, opcode, "msg", msg_size);

    // sleep(10);
    myrdma.exit_rdma(socks_cnt);

    return 0;
}
