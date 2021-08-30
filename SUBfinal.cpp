#include "Utils/LockingQueue.hpp"
#include <czmq.h>
#include <json-c/json.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <algorithm>
#include "Utils/Item2.h"
#include <mutex>
#include <sstream>
#include "Utils/BlockingQueue.h"

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
static volatile sig_atomic_t flag = 0;
static void sig_stop(int signum) { flag = 1; }
const char *type_test;
mutex cout_mutex;
mutex access_to_file;
BlockingQueue<Item2> lockingQueue; //initialize lockingQueue
ofstream config_file;
void write_something(string *what_i_said){
    cout_mutex.lock();
    cout<<*what_i_said<<endl;
    cout_mutex.unlock();
}

int payload_managing(zmsg_t **msg, const int64_t
*end, const int64_t *c) {
    //char *end_pointer_string;
    //long start;
    try {
        char *frame;
        auto *item =new Item2();
        string say = "size of msg: " +to_string(zmsg_size(*msg));
        write_something(&say);
        frame = zmsg_popstr(*msg);
        if (strcmp(frame, "TIMESTAMP") == 0) {
            frame = zmsg_popstr(*msg);
            say ="frame: "+ string(frame);
            write_something(&say);
            string start = frame;
            item->ts_start=start;
            item->num=*c;
            item->ts_end=*end;
            item->name_metric="end_to_end_delay";
            lockingQueue.push(*item);
            say = "item No. " + to_string(*c) +" pushed";
            write_something(&say);
        }
        if (zmsg_size(*msg) == 0) {
            say = "NO MORE MESSAGES";
            write_something(&say);
            return -2;
        }
        while (zmsg_size(*msg) > 0) {
            //puts("estrapolating other payload...");
            frame = zmsg_popstr(*msg);
            say = "size of payload (byte): "+ to_string((strlen(frame) * sizeof(char)));
            write_something(&say);
            //zsys_info("PAYLOAD > %s", frame);
        }

        return 0;
    }
    catch (int e) {
        return -3;
    }
}



int create_new_consumers() {
    bool console = false;
    int num_consumers = NUM_CONSUMERS;
    string name_of_experiment;
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
    string name_of_csv_file = name_of_experiment /*+ '_' + std::to_string(zclock_time()) */ + ".csv";
    printf("Num of consumer threads: %d\n", num_consumers);
    string name_path_csv = path_csv + name_of_csv_file;
    access_to_file.lock();
    config_file.open(name_path_csv, ios::app);
    config_file << "number,value,timestamp,message rate,payload size\n";
    access_to_file.unlock();
    sleep(1);

    //int c = 1;
    for (int i = 0; i < num_consumers; i++) { //same as producers
        cout<<"launch consumer No. " <<i<<endl;
        consumers.emplace_back(
                [ &console, &msg_rate, &payload, &name_path_csv, &num_consumers]() {
                    signal(SIGKILL, sig_stop);
                    access_to_file.lock();
                    config_file.open(name_path_csv, ios::app);
                    access_to_file.unlock();
                    auto name = this_thread::get_id();
                    stringstream id;
                    id <<name;
                    string say = "new consumer thread created with ID: "+id.str();
                    write_something(&say);
                    //cout << "pid of consumer: " << getpid() << endl;
                    int64_t end_to_end_delay;
                    int c = 1;
                    while (!zsys_interrupted and !flag) {

                        Item2 item = Item2();
                        c++;
                        if (c >= (int) (NUM_MEX_MAX/num_consumers)-1)
                            break;

                        item=lockingQueue.pop();
                        say ="--------------THREAD No. "+id.str()+" IS WORKING--------";
                        write_something(&say);
                        char *end_pointer;
                        int64_t start = strtoll(item.ts_start.c_str(), &end_pointer, 10);
                        //lock_guard<mutex> lock(access_to_file);
                        say =  "end : " + to_string(item.ts_end) + " start: " + to_string(start) + "\nmanaging payload";
                        write_something(&say);
                        end_to_end_delay = item.ts_end - start;

                        if (console) {
                            say = "Metric name: " + item.name_metric +"\nvalue: "
                                    + to_string(end_to_end_delay);
                            write_something(&say);
                        } else {
                            //cout << "lockingQueue empty? " << lockingQueue->empty() << endl;
                            access_to_file.lock();
                            if (!config_file.is_open()) {
                                say ="opening file csv...";
                                write_something(&say);
                                config_file.open(name_path_csv, ios::app);

                            } else{
                                say="file csv is just opened";
                                write_something(&say);
                            }
                            //string put_to_file = to_string(count) + "," +to_string(end_to_end_delay) + "," + to_string(item.ts_end) +
                            //       "," + to_string(msg_rate) + "," + to_string(payload) + "\n";
                            //const char *put_to_file_array_char = put_to_file.c_str();

                            config_file << to_string(item.num) + "," + to_string(end_to_end_delay) + "," +
                                           to_string(item.ts_end) +
                                           "," + to_string(msg_rate) + "," + to_string(payload) + "\n";
                            //config_file.close();
                            config_file.close();
                            access_to_file.unlock();
                            say = "--------------THREAD No. " +id.str()+" FINISHED ITS JOB----------" ;
                            write_something(&say);
                        }
                    }
                    say = "thread is closing...";
                    write_something(&say);
                });

        this_thread::sleep_for(chrono::seconds(1) );

    }

    for (auto& th : consumers)
        th.join();
    config_file.close();
    return 0;
}

void
subscriber_thread(string *endpoint_custom, const char *topic) {
    
    // -----------------------------------------------------------------------------------------------------

    // -----------------------------------------CREATING CONSUMERS----------------------------------------------------
    thread thread_start_consumers([]() {
        int succ = create_new_consumers();
        cout<<"consumers terminate? "<< succ<<endl;

    });// create new thread to manage payload
    //--------------------------------------------------------------------------------------------------------
    //auto *sub = static_cast<zsock_t *>(args); // create new sub socket
    cout<<"topic is "<<topic<<endl;
    zsock_t *sub = zsock_new_sub(endpoint_custom->c_str(), topic);

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
        int a = payload_managing(&msg, &end, &c);
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
    if (thread_start_consumers.joinable())
        thread_start_consumers.join();

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
            closedir(dir);

        } else if (ENOENT == errno) {
            int a = mkdir(path_csv, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (a != 0) {
                cout<<"Error to create a directory"<<endl;
                return 3;
            }
        } else {
            cout<<"Error unknown, i could not be possible to open or create a directory for csv tests"<<endl;
            return 4;
        }

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
    string endpoint_customized;

    int num_of_subs = NUM_SUBS;
    PARAM = json_object_from_file(string_json_path);
    const char *topic;
    if (PARAM != nullptr) {
        puts("PARAMETERS PUBLISHER: ");
        char *type_connection;
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
                type_connection = (char *) value;
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
        endpoint_customized = string()+type_connection+"://";

        if (strcmp(type_connection, "tcp") == 0)
            endpoint_customized = endpoint_customized+ip+":"+port;
        else if (strcmp(type_connection, "inproc") == 0)
            endpoint_customized = endpoint_customized+endpoint_inproc;
        else
            cout<<"invalid endpoint"<<endl;
        cout<<"string for endpoint (from json file):\t"<< endpoint_customized<<endl;
    } else {
        cout<<"FILE JSON NOT FOUND...EXIT"<<endl;
        return 2;
    }
    //zactor_t *sub_threads[num_of_subs];
    cout<<"Numbers of SUBS : "<< num_of_subs<<endl;
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

    subscriber_thread(&endpoint_customized, topic);
    /*for (int i = 0; i < num_of_subs; i++) {
        printf("destroying zactor and zsocket: No.%d\n", i);
        zactor_destroy(&sub_threads[i]);
    }*/
    return 0;
}

