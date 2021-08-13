#include <czmq.h>
#include <iostream>
using namespace std;

int main(int argc, const char *argv[]){
    char *end_pointer= nullptr;
    int verbose = (int) strtol(argv[1], &end_pointer, 0);
    //zactor_t *client = zactor_new(zbeacon, nullptr);
    zactor_t *speaker = zactor_new (zbeacon, nullptr);
    assert (speaker);
    if (verbose)
        zstr_sendx (speaker, "VERBOSE", NULL);
    zsock_send (speaker, "si", "CONFIGURE", 9999);
    char *hostname = zstr_recv (speaker);
    if (!*hostname) {
        printf ("OK (skipping test, no UDP broadcasting)\n");
        zactor_destroy (&speaker);
    }
    free (hostname);

    // Create listener beacon on port 9999 to lookup service
    zactor_t *listener = zactor_new (zbeacon, nullptr);
    assert (listener);

    if (verbose)
        zstr_sendx (listener, "VERBOSE", NULL);
    zsock_send (listener, "si", "CONFIGURE", 9999);
    hostname = zstr_recv (listener);
    assert (*hostname);
    free (hostname);

    // We will broadcast the magic value 0xCAFE
    byte announcement [2] = { 0xCA, 0xFE };
    zsock_send (speaker, "sbi", "PUBLISH", announcement, 2, 100);
    // We will listen to anything (empty subscription)
    zsock_send (listener, "sb", "SUBSCRIBE", "", 0);

    // Wait for at most 1/2 second if there's no broadcasting
    zsock_set_rcvtimeo (listener, 500);
    char *ipaddress = zstr_recv (listener);
    if (ipaddress) {
        zframe_t *content = zframe_recv (listener);
        assert (zframe_size (content) == 2);
        assert (zframe_data (content) [0] == 0xCA);
        assert (zframe_data (content) [1] == 0xFE);
        zframe_destroy (&content);
        zstr_free (&ipaddress);
        zstr_sendx (speaker, "SILENCE", NULL);
    }
    zactor_destroy (&listener);
    zactor_destroy (&speaker);


    return 0;
}

