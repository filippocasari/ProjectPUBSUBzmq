#include <czmq.h>


int main(){

    zactor_t *proxy = zactor_new (zproxy, NULL);
    zstr_sendx(proxy, "VERBOSE", NULL);
    zsock_wait(proxy);
    zstr_sendx (proxy, "FRONTEND", "XSUB", ">tcp://169.254.124.70:5600", NULL);
    zsock_wait (proxy);
    zstr_sendx (proxy, "BACKEND", "XPUB", ">tcp://169.254.124.70:5602", NULL);
    zsock_wait (proxy);

    while(!zsys_interrupted and !zctx_interrupted){
        zsock_wait(proxy);
    }

    zactor_destroy(&proxy);

    return 0;
}

