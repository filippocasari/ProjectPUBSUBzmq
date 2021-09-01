#include <czmq.h>
#include <iostream>

int main(int argc, const char *argv[]){

    if((std::string) argv[1] =="-v")
        int verbose = 1;
    else
        return -1;
    zactor_t *speaker = zactor_new (zbeacon, nullptr);

    zstr_sendx(speaker, "VERBOSE", NULL);
    zsock_send (speaker, "si", "CONFIGURE", 9999);
    char *hostname = zstr_recv (speaker);
    if (!*hostname) {
        printf ("OK (skipping test, no UDP broadcasting)\n");
        zactor_destroy (&speaker);
    }
    free (hostname);
    byte announcement [2] = { 0xCA, 0xFE };
    zsock_send (speaker, "sbi", "PUBLISH", announcement, 2, 100);
    zclock_sleep(5000);
    zstr_sendx (speaker, "SILENCE", NULL);
    zactor_destroy (&speaker);
}




