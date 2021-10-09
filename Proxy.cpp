#include <czmq.h>


int main(){

    zactor_t *proxy = zactor_new (zproxy, NULL);
    zstr_sendx(proxy, "VERBOSE", NULL);
    zsock_wait(proxy);
    zstr_sendx (proxy, "FRONTEND", "SUB", ">tcp://127.0.0.1:5600", "CAR");
    zsock_wait (proxy);
    zstr_sendx (proxy, "BACKEND", "PUB", "@tcp://127.0.0.1:5602", "CAR");
    zsock_wait (proxy);
    while(!zsys_interrupted and !zctx_interrupted){
        zsock_wait(proxy);
    }

    zactor_destroy(&proxy);

    return 0;
}

