#include <network/tcp.hpp>
#include <network/rdmaBenchmark.hpp>

#define num_of_server 2
#define MAX_MSG_SIZE 1048576

const char* server[num_of_server] = {"192.168.0.100", "192.168.0.101"};

int main(int argc, char* argv[]){
  if(argc != 3)
  {
    std::cout << argv[0] << " <MY IP> " <<"<server/client>" <<std::endl;
    exit(1);
  }
  int socks_cnt;

  TCP tcp;
  rdmaBenchmark rdma_benchmark;
  
  cout << "Connecting tcp" <<endl;
  tcp.connect_tcp(argv[1], server);
  cout << "Connection completely success" << endl;
  
  socks_cnt = tcp.s_cnt();
  
  rdma_benchmark.create_rdma_info(socks_cnt);
  rdma_benchmark.send_info_change_qp(socks_cnt);
  
  if(strcmp(argv[2],"server")==0){  //SN01
    rdma_benchmark.write_rdma("hello", 0);
  }
  else{                             //SN02, SN03
    sleep(10);
    rdma_benchmark.write_recv_rdma(0, 0);
  }

  rdma_benchmark.exit_rdma(socks_cnt);
  return 0;
}
