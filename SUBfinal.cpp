#include "Utils/LockingQueue.hpp"
#include <czmq.h>
#include <json-c/json.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <algorithm>
#include "Utils/Item2.h"
#include <mutex>
#include <condition_variable>

#define PATH_CSV "./ResultsCsv_1/"
#define NUM_PRODUCERS 1
#define NUM_CONSUMERS 4
#define QUEUE_CAPACITY 4
#define NUM_SUBS 1
#define ENDPOINT endpoint_tcp
#define NUM_MEX_MAX 10000
#define FOLDER_EXPERIMENT "./Results/"
#define SIGTERM_MSG "SIGTERM received.\n"
using namespace std;
//#define MSECS_MAX_WAITING 10000
const char *endpoint_tcp = "tcp://127.0.0.1:6000";
//const char *endpoint_inprocess = "inproc://example";
const char *string_json_path;
char *path_csv = nullptr;


const char *type_test;
char endpoint[25] = "\0";


int payload_managing(zmsg_t **msg, LockingQueue<Item2> *lockingQueue, const int64_t
*end, const int64_t *c) {
    //char *end_pointer_string;
    //long start;
    try {

        char *frame;
        auto *item =new Item2();
        cout << "size of msg: " <<zmsg_size(*msg)<<std::endl;
        frame = zmsg_popstr(*msg);
        if (strcmp(frame, "TIMESTAMP") == 0) {
            frame = zmsg_popstr(*msg);
            cout<<"frame: "<< frame<<endl;
            string start = frame;
            item->ts_start=start;
            item->num=*c;
            item->ts_end=*end;
            item->name_metric="end_to_end_delay";
            lockingQueue->push(*item);
            cout << "item No. " << *c << " pushed" << endl;
        }
        if (zmsg_size(*msg) == 0) {
            cout << "NO MORE MESSAGES" << endl;
            return -2;
        }
        while (zmsg_size(*msg) > 0) {
            //puts("estrapolating other payload...");
            frame = zmsg_popstr(*msg);
            cout<<"size of payload (byte): "<< (strlen(frame) * sizeof(char))<<endl;
            //zsys_info("PAYLOAD > %s", frame);
        }

        return 0;
    }
    catch (int e) {
        return -3;
    }
}


int create_new_consumers(LockingQueue<Item2> *lockingQueue) {
    bool console = false;
    int num_consumers = NUM_CONSUMERS;
    std::string name_of_experiment;
    json_object *PARAM;
    int msg_rate;
    int payload;
    PARAM = json_object_from_file(string_json_path);
    json_object_object_foreach(PARAM, key, val) {
        if (strcmp(key, "metrics_output_type") == 0) {
            char *value = const_cast<char *>(json_object_get_string(val));
            if (strcmp(value, "console") == 0)
                console = true;
            else
                cout<<"creating new file csv"<<endl;
        }
        if (strcmp(key, "experiment_name") == 0)
            name_of_experiment = json_object_get_string(val);
        if (strcmp(key, "num_consumer_threads") == 0)
            num_consumers = (int) strtol(json_object_get_string(val), nullptr, 10);
        if (strcmp(key, "msg_rate_sec") == 0) {
            msg_rate = (int) strtol(json_object_get_string(val), nullptr, 10);
        }
        if (strcmp(key, "payload_size_bytes") == 0) {
            payload = (int) strtol(json_object_get_string(val), nullptr, 10);
        }
    }
    vector<thread> consumers; // create a vector of consumers
    consumers.reserve(num_consumers);
    ofstream config_file;
    string name_of_csv_file = name_of_experiment /*+ '_' + std::to_string(zclock_time()) */ + ".csv";

    printf("Num of consumer threads: %d\n", num_consumers);
    string name_path_csv = path_csv + name_of_csv_file;
    config_file.open(name_path_csv, ios::app);
    config_file << "number,value,timestamp,message rate,payload size\n";
    sleep(1);
    //int c = 1;
    for (int i = 0; i < num_consumers; i++) { //same as producers
        cout<<"launch consumer No. " <<i<<endl;
        consumers.emplace_back(
                [&lockingQueue, &console, &name_of_csv_file, &msg_rate, &payload, &name_path_csv, &config_file, &num_consumers]() {

                    cout << "new consumer thread created with ID: " << this_thread::get_id() << endl;
                    //cout << "pid of consumer: " << getpid() << endl;
                    int64_t end_to_end_delay;
                    int c = 1;
                    while (true) {

                        Item2 item = Item2();
                        c++;
                        if (c >= NUM_MEX_MAX/4)
                            break;

                        lockingQueue->waitAndPop(item);
                        cout << "--------------THREAD No. " << this_thread::get_id() << " IS WORKING--------" << endl;
                        char *end_pointer;
                        int64_t start = strtoll(item.ts_start.c_str(), &end_pointer, 10);
                        //lock_guard<mutex> lock(access_to_file);
                        cout << "end : " << item.ts_end << " start: " << start << endl << "managing payload" << endl;
                        end_to_end_delay = item.ts_end - start;

                        if (console) {
                            cout << "Metric name: " << item.name_metric << endl << "value: "
                                      << end_to_end_delay <<endl;
                        } else {
                            //cout << "lockingQueue empty? " << lockingQueue->empty() << endl;

                            if (!config_file.is_open()) {
                                cout<<"opening file csv..."<<endl;
                                config_file.open(name_path_csv, ios::app);

                            } else {
                                cout<<"file csv is just opened"<<endl;
                                //string put_to_file = to_string(count) + "," +to_string(end_to_end_delay) + "," + to_string(item.ts_end) +
                                //       "," + to_string(msg_rate) + "," + to_string(payload) + "\n";
                                //const char *put_to_file_array_char = put_to_file.c_str();

                                config_file << to_string(item.num) + "," + to_string(end_to_end_delay) + "," +
                                               to_string(item.ts_end) +
                                               "," + to_string(msg_rate) + "," + to_string(payload) + "\n";
                                //config_file.close();


                                cout << "--------------THREAD No. " << this_thread::get_id()
                                     << " FINISHED ITS JOB----------" << endl;
                            }
                        }

                    }
                    cout<<"thread is closing..."<<endl;
                });
        sleep(1);

    }

    for (int i = 0; i < num_consumers; i++) {
        consumers[i].join();
    }
    config_file.close();
    return 0;
}

void
subscriber_thread(void *args, const char *topic) {
    
    // -----------------------------------------------------------------------------------------------------
    LockingQueue<Item2> lockingQueue; //initialize lockingQueue
    // -----------------------------------------CREATING CONSUMERS----------------------------------------------------
    thread thread_start_consumers([&lockingQueue]() {
        int succ = create_new_consumers(&lockingQueue);
        cout<<"consumers terminate? "<< succ<<endl;

    });// create new thread to manage payload
    thread_start_consumers.detach();
    //--------------------------------------------------------------------------------------------------------
    //auto *sub = static_cast<zsock_t *>(args); // create new sub socket
    cout<<"topic is "<<topic<<endl;
    zsock_t *sub = zsock_new_sub((char *) args, topic);

    //long time_of_waiting = 0;
    int64_t c;
    //bool terminated = false;

    int64_t end;
    string metric = "end_to_end_delay";
    //zpoller_t *poller = zpoller_new(sub, NULL);
    zmsg_t *msg = zmsg_new();
    int k = 1;
    int succ=0;
    while (succ!=-1) {
        succ=zsock_recv(sub, "s8m", &topic, &c, &msg);
        k++;
        //char *topic;

        //zpoller_wait(poller, 0);
        /*int rc=
        if (rc==-1)
            puts("ERROR TO RECEIVE");
        */

        if (msg == nullptr) {
            cout<<"exit, msg null"<<endl;
            break;
        }
        if (strcmp(type_test, "LAN") == 0)
            end = zclock_time();
        else if (strcmp(type_test, "LOCAL") == 0)
            end = zclock_usecs();
        else
            end = 0;

        cout<<"Recv on "<< topic<<endl;
        cout<<"message Received: No. "<< c<<endl;
        int a = payload_managing(&msg, &lockingQueue, &end, &c);
        cout << "managing payload exit code: " << a << endl;
        //zmsg_destroy(&msg);

    }
    if (k - (int) NUM_MEX_MAX>=0)
        cout << "test SUCCESS" << endl;
    else
        cout << "PACKET LOSS" << endl;
    cout<<"Messages Received : "<<k<<endl;
    sleep(3);
    zsock_destroy(&sub);
    //
    //terminate();
}


int main(int argc, char *argv[]) {

    char *cmdstring; // string of args
    if (argc == 1) // exit if argc is less then 2
    {
        cout<<"NO INPUT JSON FILE OR TOO MANY ARGUMENTS...EXIT"<<endl;
        return 1;
    } else {

        //size_t strsize = 0; //size of the string to allocate memory

        //strsize += (int) strlen(argv[1]);

        if (argc == 2) {
            cout << "Path for csv not chosen..." << endl;
            return 2;
        }
        //size_t strsize_2 = (int) strlen(argv[2]);
        path_csv = argv[2];
        DIR *dir = opendir(path_csv);
        if (dir) {
            cout << "path csv already exists" << endl;

        } else if (ENOENT == errno) {
            int a = mkdir(path_csv, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (a != 0) {
                return 3;
            }
        } else {
            return 4;
        }
        closedir(dir);
        cout << "PATH chosen: " << path_csv << endl;
        // initialize the string
        cmdstring = argv[1];
        printf("INPUT FILE JSON (NAME): %s\n", cmdstring);
    }
    //path of json file
    string_json_path = cmdstring; // file passed from the bash script or manually from terminal

    // start deserialization
    json_object *PARAM;
    const char *endpoint_inproc;
    char *endpoint_customized;

    int num_of_subs = NUM_SUBS;
    PARAM = json_object_from_file(string_json_path);
    const char *topic;
    if (PARAM != nullptr) {
        puts("PARAMETERS PUBLISHER: ");
        const char *type_connection;
        const char *port;
        const char *ip;
        const char *output_file;

        //int payload_size;
        //int num_mex;
        int int_value;

        const char *value;
        json_object_object_foreach(PARAM, key, val) {

            value = json_object_get_string(val);

            if (json_object_is_type(val, json_type_int)) {
                int_value = (int) json_object_get_int64(val);
                if (strcmp(key, "num_of_subs") == 0)
                    num_of_subs = int_value;
            }

            printf("\t%s: %s\n", key, value);
            if (strcmp(key, "connection_type") == 0) {
                type_connection = value;
                printf("connection type found: %s\n", type_connection);
            }
            if (strcmp(key, "type_test") == 0)
                type_test = value;
            if (strcmp(key, "ip") == 0) {
                ip = value;
                printf("ip found: %s\n", ip);
            }
            if (strcmp(key, "port") == 0) {
                port = value;
                printf("port found: %s\n", port);
            }
            if (strcmp(key, "metrics_output_type") == 0) {
                output_file = value;
                printf("output file found: %s\n", output_file);
            }
            if (strcmp(key, "topic") == 0)
                topic = value;
            if (strcmp(key, "endpoint_inproc") == 0)
                endpoint_inproc = value;
        }

        endpoint_customized = strcat(endpoint, type_connection);
        endpoint_customized = strcat(endpoint_customized, "://");
        if (strcmp(type_connection, "tcp") == 0) {
            endpoint_customized = strcat(endpoint_customized, ip);
            endpoint_customized = strcat(endpoint_customized, ":");
            endpoint_customized = strcat(endpoint_customized, port);
        } else if (strcmp(type_connection, "inproc") == 0) {
            endpoint_customized = strcat(endpoint_customized, endpoint_inproc);
        }
        printf("string for endpoint (from json file): %s\t", endpoint_customized);
    } else {
        puts("FILE JSON NOT FOUND...EXIT");
        return 2;
    }
    //zactor_t *sub_threads[num_of_subs];
    printf("Numbers of SUBS : %d\n", num_of_subs);
    /*for (int i = 0; i < num_of_subs; i++) {
        zclock_log("file json is being used");

        string name;
        name = topic + to_string(i);
        cout << "Starting new sub thread :" + name << endl;
        sub_threads[i] = zactor_new(subscriber_thread, endpoint_customized);
        cout << "new actor created..." << endl;

    }
     * destroying zactors of subs
     */

    subscriber_thread(endpoint_customized, topic);
    /*for (int i = 0; i < num_of_subs; i++) {
        printf("destroying zactor and zsocket: No.%d\n", i);
        zactor_destroy(&sub_threads[i]);
    }*/
    return 0;
}

