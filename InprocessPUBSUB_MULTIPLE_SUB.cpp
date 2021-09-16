//
// Created by Filippo Casari on 09/09/21.
//

#include <czmq.h>
#include <json-c/json.h>
#include <cmath>
//default endpoint
#include "PUB.h"
#include "SUB.h"

using namespace std;
#define NUM_SUB 2
int main(int argc, char **argv) {
    //vector<thread> SUBS;
    //SUBS.reserve(NUM_SUB);
    //vector<thread> actors;
    //actors.reserve(NUM_SUB);
    zactor_t *actors[NUM_SUB];
    //zsock_t *sub = zsock_new_sub("inproc://CAR", "CAR");
    for (int i = 0; i < NUM_SUB; i++) {
        string i_str = "_" + to_string(i);
        //SUBS.emplace_back([&argc, &argv, &i_str]() {
        char **new_argv = argv;
        string temp = (argv[2] + i_str);
        new_argv[2] = (char *) temp.c_str();
        cout << "New argv[2] passed: " << new_argv[2] << endl;

        string comma = ",";
        string comma2 ="&";

        string final_string = argv[1];
        final_string+=comma;
        final_string+=argv[2]+comma2+argv[3];
        cout<<"FINAL STRING: "<<final_string<<endl;

        actors[i]= zactor_new(main_SUB_M, (void *) &final_string);

        sleep(2);
        //cout << "exit code of sub: " << a << endl;
        //});

        //this_thread::sleep_for(chrono::milliseconds(500));
        //cout << "CREATING NEW SUB" << endl;
    }
    zclock_sleep(6000);
    main_PUB(argc, argv);
    for(auto & actor : actors){
        zactor_destroy(&actor);
    }
    //creating_publisher.join();



    return 0;
}