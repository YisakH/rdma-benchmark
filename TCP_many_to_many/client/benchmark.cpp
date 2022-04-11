#include <network/tcp.hpp>
#include <network/rdmaBenchmark.hpp>
#include <tools/IP.hpp>

#define num_of_server 2
#define MAX_MSG_SIZE 1048576

const char* server[num_of_server] = {"192.168.0.100", "192.168.0.101"};

int main(int argc, char* argv[]){
  char host[256];
  char *IP;
  struct hostent *host_entry;
  int hostname;
  
  hostname = gethostname(host, sizeof(host)); //find the host name
  check_host_name(hostname);
  host_entry = gethostbyname(host); //find host information
  check_host_entry(host_entry);
  IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));

  bool isServer = (IP[12]%2) == 1;
  int socks_cnt;

  TCP tcp;
  rdmaBenchmark rdma_benchmark;

  char char_arr[20];

  strcpy(char_arr, IP);
  printf("my IP : %s\n", IP);

  
  cout << "Connecting tcp" <<endl;
  tcp.connect_tcp(char_arr, server);
  cout << "Connection completely success" << endl;
  
  socks_cnt = tcp.s_cnt();
  
  std::vector<tuple<struct ibv_context*, struct ibv_pd*, 
                int, struct ibv_cq*,
                struct ibv_qp*, struct ibv_mr*,
                uint16_t, uint32_t>>* rdmaInfo_ptr = rdma_benchmark.create_rdma_info(socks_cnt);
  rdma_benchmark.send_info_change_qp(socks_cnt, rdmaInfo_ptr);




  
  for (int msg_size=1; msg_size<=MAX_MSG_SIZE; msg_size*=2)
    //rdma_benchmark.run_bench(socks_cnt, msg_size, isServer);
    rdma_benchmark.run_bench_write(socks_cnt, msg_size, isServer, rdmaInfo_ptr);


  /*
  if(strcmp(argv[2],"server")==0){

    myrdma.rdma_send_msg(socks_cnt,"send","abasdfasdf3w");
    myrdma.rdma_send_msg(socks_cnt,"send","basdfewq dsfasdf");
    myrdma.rdma_send_msg(socks_cnt,"send","casdfeawfewawe");
    //myrdma.rdma_send_msg(socks_cnt,"write", "Yisak is not Handsome.");
  }
  else{                             //SN02, SN03
    //myrdma.recv_t(socks_cnt,"send");                   
    myrdma.recv_t(socks_cnt,"send");
    sleep(10);        
  }*/

   rdma_benchmark.exit_rdma(socks_cnt);
  return 0;
}
