#include "tcp.hpp"
#include "RDMA.hpp"
#include "myRDMA.hpp"
#include <sys/time.h>

#define num_of_server 2
#define TOTAL_SEND_BYTES 1073741824 // 1GB
#define MAX_MSG_SIZE 1048576
#define ITERATION 1000

const char *server[num_of_server] = {"192.168.1.100", "192.168.1.101"};

uint64_t timeDiff(struct timeval stop, struct timeval start)
{
  return (stop.tv_sec * 1000000L + stop.tv_usec) - (start.tv_sec * 1000000L + start.tv_usec);
}

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

  if (strcmp(argv[2], "s") == 0)
  {
    /*
    myrdma.fucking_rdma(socks_cnt, "send", "Yisak is Handsome");

    for(int i = 0; i<socks_cnt;i++){
      cout << "recv_buffer["<< i <<"] SEND: "<<recv_buffer[i]<< endl;
    }
    */

    for (int msg_size = 1; msg_size <= MAX_MSG_SIZE; msg_size *= 2)
    {
      memset(send_buffer, msg_size - 1, 'A');
      msg[msg_size - 1] = '\0';
      int iteration = TOTAL_SEND_BYTES / msg_size;

      cerr << "<---- " << opcode  << " : " << msg_size << "bytes 벤치마크 테스트 시작 ---------->" << endl;

      struct timeval start, stop;
      gettimeofday(&start, NULL);

      for (int iter = 0; iter < ITERATION; iter++)
      {
        myrdma.fucking_rdma(socks_cnt, opcode, "msg", msg_size);
      }

      gettimeofday(&stop, NULL);

      uint64_t time = timeDiff(stop, start);
      printf("total time : %ld\n", time);
      double msec = ((double)time) / 1000000L * 1000;

      double msgRate = ((double)(ITERATION * 1000000L)) / time;
      double bandwidth = ((double)(ITERATION * msg_size)) / (1024 * 1024) / (((double)time) / 1000000L);
      double latency = ((double)msec) / ITERATION;
      printf("%.3f msg/sec\t%.3f MB/sec\n", msgRate, bandwidth);
      printf("latency : %.3fms\n", latency);
      fflush(stdout);

      // delete [] msg;
    }
  }
  else
  {
    for (int msg_size = 1; msg_size <= MAX_MSG_SIZE; msg_size *= 2)
    {
      cerr << "<---- " << opcode  << " : " << msg_size << "bytes 벤치마크 테스트 시작 ---------->" << endl;
      int iteration = TOTAL_SEND_BYTES / msg_size;

      for (int i = 0; i < socks_cnt; i++)
      {
        for (int iter = 0; iter < ITERATION; iter++)
        {
          myrdma.recv_t(socks_cnt, opcode, msg_size);
          //cerr << "SEND: " << recv_buffer[i] << endl;
        }
      }
    }
  }

  // sleep(10);
  myrdma.exit_rdma(socks_cnt);

  return 0;
}
