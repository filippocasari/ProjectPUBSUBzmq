#include <czmq.h>
#include <json-c/json.h>
#include <cmath>
//default endpoint
#include "PUB.h"
#include "SUB.h"
using namespace std;

int main(int argc, char **argv) {
    main_PUB(argc, argv);
    main_SUB(argc, argv);
    return 0;
}
