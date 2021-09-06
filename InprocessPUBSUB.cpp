#include <czmq.h>
#include <json-c/json.h>
#include <cmath>
//default endpoint
#include "PUB.h"
#include "SUB.h"
using namespace std;

int main(int argc, char **argv) {
    std::thread threadSUB([&argc, &argv]{
        int a=main_SUB(argc, argv);
        cout<<"exit code of sub: "<<a<<endl;
    });
    zclock_sleep(3000);
    main_PUB(argc, argv);
    threadSUB.join();

    return 0;
}
