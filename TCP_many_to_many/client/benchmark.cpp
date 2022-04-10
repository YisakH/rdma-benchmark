#include <network/tcp.hpp>
#include <network/rdmaBenchmark.hpp>
#include <tools/IP.hpp>

#define num_of_server 2
#define MAX_MSG_SIZE 1048576

const char* server[num_of_server] = {"192.168.1.100", "192.168.1.101"};

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
  myRDMA myrdma;
  
  cout << "Connecting tcp" <<endl;
  tcp.connect_tcp(IP, server);
  cout << "Connection completely success" << endl;
  
  socks_cnt = tcp.s_cnt();
  
  myrdma.create_rdma_info(socks_cnt);
  myrdma.send_info_change_qp(socks_cnt);


  rdmaBenchmark rdmaBenchmark;

  for (int msg_size=1; msg_size<=MAX_MSG_SIZE; msg_size*=2)
    rdmaBenchmark.run_bench_write(socks_cnt, msg_size, isServer);


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

  myrdma.exit_rdma(socks_cnt);
  return 0;
}
