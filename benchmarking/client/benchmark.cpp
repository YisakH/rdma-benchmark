#include "network/customRDMA.hpp"

#define num_of_server 4
#define MAX_MSG_SIZE 1048576

const char* server[num_of_server] = {"192.168.0.100", "192.168.0.101",
                                      "192.168.0.102", "192.168.0.103"};

void check_host_name(int hostname) { //This function returns host name for local computer
   if (hostname == -1) {
      perror("gethostname");
      exit(1);
   }
}
void check_host_entry(struct hostent * hostentry) { //find host info from host name
   if (hostentry == NULL){
      perror("gethostbyname");
      exit(1);
   }
}
void IP_formatter(char *IPbuffer) { //convert IP string to dotted decimal format
   if (NULL == IPbuffer) {
      perror("inet_ntoa");
      exit(1);
   }
}

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

  cout << IP << endl;


  int socks_cnt, buf_size = 1048580;
  char NAME[6];
  char msg[BufSize];
  NAME[0]='[';
  NAME[1]='S';
  NAME[2]='N';
  NAME[3]='0';
  NAME[4]=IP[12];
  NAME[5]=']';

  TCP tcp = TCP();
  cout << "Server_t() 실행" <<endl;
  tcp.Server_t(server);
    
  sleep(2);
    
  cout << "Client_t() 실행"<<endl;
  tcp.Client_t(IP,server);
  sleep(2);
    
  socks_cnt = tcp.Scnt();


  customRDMA custom_rdma;

  custom_rdma.Create_Rdmainfo(socks_cnt);
  custom_rdma.SendInfo_ChangeQP(socks_cnt,IP[12]-'0');

  for (int msg_size=1; msg_size<=MAX_MSG_SIZE; msg_size*=2)
    custom_rdma.run_bench(socks_cnt, NAME, msg_size, isServer);


  if(custom_rdma.cnt_thread() != 0)
    cout << "--------다른 서버의 종료를 기다리고 있습니다.--------"<<endl;
  while(1){
    if(custom_rdma.cnt_thread() == 0){
      break;
    }
  }
  custom_rdma.exitRDMA(socks_cnt);
  return 0;
}
