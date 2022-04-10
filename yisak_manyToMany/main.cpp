#include "includes/myTcp.hpp"

int main(int argc, char *argv[])
{
    if(argc <= 1){
        printf("argument must be required\n");
        exit(1);
    }

    manyToMany mymanyToMany(argv[1]);
    printf("%s\n", argv[1]);

    mymanyToMany.server();
    sleep(1);
    mymanyToMany.client(NoOfNode);


    //string input = "";
    char input[1024];

    sleep(5);

    while(true){
        char input[1024];
        fgets(input,1024,stdin);
        mymanyToMany.send_msg(input);
    }

    return 0;
}