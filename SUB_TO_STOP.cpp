#include <czmq.h>
#include <iostream>



int main(){
    zsock_t *sub = zsock_new_sub("tcp://172.20.10.3:6000", "CAR");
    char *topic;
    uint64_t n;
    zsock_recv(sub, "s8",&topic, &n );
    while(n!= 9999){
        zsock_recv(sub, "s8",&topic, &n );
        //std::cout<<"N: "<<n<<std::endl;
    }
    sleep(2);
    zsock_destroy(&sub);
    return 0;

}

