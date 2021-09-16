#include <czmq.h>
#include <json-c/json.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <algorithm>
#include "Utils/Item2.h"
#include <mutex>
#include <sstream>
#include <vector>
#include "Utils/BlockingQueue.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
using namespace std;

static void subscriber_thread(zsock_t *pipe, void *args){
    zsock_signal(pipe, 0);

    void *ptr = nullptr;
    char *str = nullptr;
    while(true){
        puts("ciaoo");
        str= zstr_recv(pipe);
        int done = streq(str, "$TERM");
        zstr_free(&str);
        if(done)
            break;
    }
}
int main(){

    zactor_t *actor = zactor_new(subscriber_thread, nullptr);
    sleep(4);
    const char *str = "$TERM";
    const void *ptr = str;
    if(zsock_bsend(actor, "s", str, ptr))
        zsock_wait(actor);
    zactor_destroy(&actor);
    return 0;
}
