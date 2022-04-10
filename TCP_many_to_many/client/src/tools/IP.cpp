#include <stdlib.h>

void check_host_name(int hostname) { //This function returns host name for local computer
   if (hostname == -1) {
      exit(1);
   }
}
void check_host_entry(struct hostent * hostentry) { //find host info from host name
   if (hostentry == NULL){
      exit(1);
   }
}
void IP_formatter(char *IPbuffer) { //convert IP string to dotted decimal format
   if (NULL == IPbuffer) {
      exit(1);
   }
}