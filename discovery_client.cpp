#include <czmq.h>
#include <iostream>
using namespace std;

int main(int argc, const char *argv[]){
    int verbose;
    if((string) argv[1] =="-v")
        verbose = 1;
    else
        verbose =0;
    //zactor_t *client = zactor_new(zbeacon, nullptr);


    // Create listener beacon on port 9999 to lookup service
    zactor_t *listener = zactor_new (zbeacon, nullptr);
    assert (listener);

    if (verbose)
        zstr_sendx (listener, "VERBOSE", NULL);
    zsock_send (listener, "si", "CONFIGURE", 9999);
    char *hostname = zstr_recv (listener);

    free (hostname);

    // We will broadcast the magic value 0xCAFE

    // We will listen to anything (empty subscription)
    zsock_send (listener, "sb", "SUBSCRIBE", "", 0);

    // Wait for at most 1/2 second if there's no broadcasting
    zsock_set_rcvtimeo (listener, 10000);
    char *ipaddress = zstr_recv (listener);
    cout<<"IP ADDRESS RECEIVED :"<<ipaddress<<endl;
    if (ipaddress) {
        zframe_t *content = zframe_recv (listener);
        assert (zframe_size (content) == 2);
        assert (zframe_data (content) [0] == 0xCA);
        assert (zframe_data (content) [1] == 0xFE);
        zframe_destroy (&content);
        zstr_free (&ipaddress);
    }
    zactor_destroy (&listener);



    return 0;
}

