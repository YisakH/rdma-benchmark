#include <network/tcp.hpp>
#include <network/myRDMA.hpp>

#define num_of_server 2
const char* server[num_of_server] = {"192.168.1.100", "192.168.1.101", "192.168.1.102"};

int main(int argc, char* argv[]){
  if(argc != 3)
  {
    std::cout << argv[0] << " <MY IP> " <<"<server/client>" <<std::endl;
    exit(1);
  }
  int socks_cnt;

  TCP tcp;
  myRDMA myrdma;
  
  cout << "Connecting tcp" <<endl;
  tcp.connect_tcp(argv[1], server);
  cout << "Connection completely success" << endl;
  
  socks_cnt = tcp.s_cnt();
  
  myrdma.create_rdma_info(socks_cnt);
  myrdma.send_info_change_qp(socks_cnt);
  
  if(strcmp(argv[2],"server")==0){  //SN01
    //for (int i = 0 ; i<1000; i++){
    //myrdma.send_t(socks_cnt);
    //for(int i=0;i<10000;i++){
    //  cout << i <<" ";
    myrdma.rdma_send_msg(socks_cnt,"send","abasdfasdf3w");
    myrdma.rdma_send_msg(socks_cnt,"send","basdfewq dsfasdf");
    myrdma.rdma_send_msg(socks_cnt,"send","casdfeawfewawe");
    //myrdma.rdma_send_msg(socks_cnt,"write", "Yisak is not Handsome.");
  }
  else{                             //SN02, SN03
    //myrdma.recv_t(socks_cnt,"send");                   
    myrdma.recv_t(socks_cnt,"send");
    sleep(10);        
  }
  //sleep(5);
  //myrdma.run_chat(socks_cnt);
  
  /*if(myrdma.cnt_thread() != 0)
    cout << "--------Waiting for another server to exit--------"<<endl;
  while(1){
    if(myrdma.cnt_thread() == 0){
      break;
    }
  }*/
  myrdma.exit_rdma(socks_cnt);
  return 0;
}
