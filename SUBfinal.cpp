
#include <czmq.h>
#include <json-c/json.h>
#include <fstream>
#include <iostream>

#include <algorithm>
#include "Utils/Item.h"
#include <mutex>
#include "Utils/BlockingQueue.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include "SUB.h"

using namespace std;
//atomic<bool> g_finished;

void writeSafely(string *what_i_said) {
    cout_mutex.lock();
    cout << *what_i_said << endl;
    cout_mutex.unlock();
}

int launchSynchronizationService(const char *ip, const char *port) {
    string endpoint_sync = "tcp://";
    endpoint_sync.append(ip);
    endpoint_sync.append(":");
    endpoint_sync.append(to_string(atoi(port) + 1));

    cout << "PUB>Endpoint for Sync service: " << endpoint_sync << endl;
    zsock_t *syncservice = zsock_new_req(endpoint_sync.c_str());

    zsock_send(syncservice, "s", "INIT");
    char *string;
    zsock_recv(syncservice, "s", &string);

    zsock_destroy(&syncservice);
    return 0;


}

int main(int argc, char **argv) {
    bool verbose=false;
    for (int i = 1; i < argc; i++) {
        cout << "ARGV[" << i << "]: " << argv[i] << endl;
    }
    char *g_path_csv;
    char *string_json_path; // string of args
    if (argc == 1) // exit if argc is less then 2
    {
        cout << "NO INPUT JSON FILE OR TOO MANY ARGUMENTS...EXIT" << endl;
        return 1;
    } else {

        //size_t strsize = 0; //size of the string to allocate memory

        //strsize += (int) strlen(argv[1]);

        if (argc == 2) {
            cout << "Path for csv not chosen..." << endl;
            return 2;
        }
        //size_t strsize_2 = (int) strlen(argv[2]);

        g_path_csv = argv[2];
        DIR *dir = opendir(g_path_csv);
        if (dir) {
            cout << "path csv already exists" << endl;
            closedir(dir);

        } else if (ENOENT == errno) {
            int a = mkdir(g_path_csv, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (a != 0) {
                cout << "Error to create a directory" << endl;
                return 3;
            }
        } else {
            cout << "Error unknown, i could not be possible to open or create a directory for csv tests" << endl;
            return 4;
        }

        cout << "PATH chosen: " << g_path_csv << endl;
        // initialize the string
        string_json_path = argv[1]; // file passed from the bash script or manually from terminal
        printf("INPUT FILE JSON (NAME): %s\n", string_json_path);
    }
    //path of json file


    // start deserialization
    json_object *PARAM;
    const char *endpoint_inproc;
    string endpoint_customized;

    //int num_of_subs = NUM_SUBS;
    PARAM = json_object_from_file(string_json_path);
    char *topic;
    char *type_connection;
    const char *port;
    const char *ip;
    //const char *output_file;
    int msg_rate;
    int payload;
    int num_mex;
    char *name_of_experiment;
    const char *v = (char *) argv[3];
    if (strcmp(v, "-v") == 0)
        verbose = true;

    if (PARAM != nullptr) {
        puts("READING PARAMETERS OF SUBSCRIBER... ");
        int int_value;
        char *value;

        json_object_object_foreach(PARAM, key, val) {
            value = (char *) json_object_get_string(val);
            cout << value << endl;
            if (json_object_is_type(val, json_type_int)) {
                int_value = (int) json_object_get_int64(val);
                if (strcmp(key, "number_of_messages") == 0)
                    num_mex = int_value;
                else if (strcmp(key, "msg_rate_sec") == 0)
                    msg_rate = (int) strtol(value, nullptr, 10);
                else if (strcmp(key, "payload_size_bytes") == 0)
                    payload = (int) strtol(value, nullptr, 10);
            } else if (strcmp(key, "connection_type") == 0) {
                type_connection = (char *) value;
                endpoint_customized = type_connection;
                endpoint_customized.append("://");
            }
            else if (strcmp(key, "ip") == 0)
                ip = value;
            else if (strcmp(key, "port") == 0)
                port = value;
            else if (strcmp(key, "metrics_output_type") == 0)
                cout << "deprecated output file " << endl;
            else if (strcmp(key, "topic") == 0)
                topic = value;
            else if (strcmp(key, "endpoint_inproc") == 0)
                endpoint_inproc = value;
            else if (strcmp(key, "experiment_name") == 0)
                name_of_experiment = value;
            if (verbose)
                printf("\t%s: %s\n", key, value);
        }
        if (strcmp(type_connection, "tcp") == 0)
            endpoint_customized.append(ip).append(":").append(port);
        if (strcmp(type_connection, "inproc") == 0)
            endpoint_customized.append(endpoint_inproc);

    } else {
        cout << "FILE JSON NOT FOUND...EXIT" << endl;
        return 2;
    }
    cout << "PUB>endpoint : " << endpoint_customized << endl;
    int success = launchSynchronizationService(ip, port);
    assert(success == 0);
    cout << "Synchronization success" << endl;

    subscriber(endpoint_customized.c_str(), topic,g_path_csv,name_of_experiment, &payload, &msg_rate, &verbose);
    cout << "END OF SUBSCRIBER" << endl;
    return 0;
}

