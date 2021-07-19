#include "Utils/linkedblockingqueueItems.h"
#include <czmq.h>
#include <json-c/json.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <algorithm>
#include "Utils/Item2.h"

#define PATH_CSV "./ResultsCsv_1/"
#define NUM_PRODUCERS 1
#define NUM_CONSUMERS 4
#define QUEUE_CAPACITY 10
#define NUM_SUBS 1
#define ENDPOINT endpoint_tcp
#define NUM_MEX_MAX 1000
#define FOLDER_EXPERIMENT "./Results/"
#define SIGTERM_MSG "SIGTERM received.\n"
using namespace std;
//#define MSECS_MAX_WAITING 10000
const char *endpoint_tcp = "tcp://127.0.0.1:6000";
//const char *endpoint_inprocess = "inproc://example";
const char *string_json_path;
char *path_csv = nullptr;
bool is_open = false;


void sig_term_handler(int signum, siginfo_t *info, void *ptr) {
    write(STDERR_FILENO, SIGTERM_MSG, sizeof(SIGTERM_MSG));
}


void catch_sigterm() {
    static struct sigaction _sigact;

    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = sig_term_handler;
    _sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &_sigact, nullptr);
}


void payload_managing(zmsg_t *msg, BlockingQueue<Item2> *queue, long
end) {
//char *end_pointer_string;
//long start;
    char *frame;
    Item2 item;
    cout << "size of msg: " <<
         zmsg_size(msg)
         <<
         std::endl;

    while (
            zmsg_size(msg)
            > 0) {
        frame = zmsg_popstr(msg);
        if (
                strcmp(frame,
                       "TIMESTAMP") == 0) {
            frame = zmsg_popstr(msg);
            zsys_info("> %s", frame);
            string start = frame;
            item = Item2(start, end, "end_to_end_delay");
            frame = zmsg_popstr(msg);
            queue->
                    Push(item);
            //zsys_info("PAYLOAD > %s", frame);
            break;
        } else {
            puts("error...this message does not contain any timestamp");
            break;
        }
    }

    zmsg_destroy(&msg);

}

void create_new_consumers(BlockingQueue<Item2> *queue) {
    catch_sigterm();
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
                puts("creating new file csv");
        }
        if (strcmp(key, "experiment_name") == 0)
            name_of_experiment = json_object_get_string(val);
        if (strcmp(key, "num_consumer_threads") == 0)
            num_consumers = (int) strtol(json_object_get_string(val), nullptr, 10);
        if(strcmp(key, "msg_rate_sec")==0){
            msg_rate= (int) strtol(json_object_get_string(val), nullptr, 10);
        }
        if(strcmp(key, "payload_size_bytes")==0){
            payload= (int) strtol(json_object_get_string(val), nullptr, 10);
        }
    }
    vector<thread> consumers; // create a vector of consumers
    consumers.reserve(num_consumers);
    ofstream config_file;

    string name_of_csv_file = name_of_experiment /*+ '_' + std::to_string(zclock_time()) */ + ".csv";
    int count = 0;
    printf("Num of consumer threads: %d\n", num_consumers);
    for (int i = 0; i < num_consumers; i++) { //same as producers
        consumers.emplace_back([&queue, console, &config_file, name_of_csv_file, &count, &msg_rate, &payload]() {

            Item2 item = Item2();

            while (queue->Pop(item) && !zctx_interrupted && !zsys_interrupted) {
                puts("created new item...");
                long end_to_end_delay;
                long start = std::stol(item.ts_start);
                cout << "end : " << item.ts_end << " start: " << start;
                end_to_end_delay = item.ts_end - start;
                puts("\nmanaging message...");
                if (console) {
                    std::cout << "Metric name: " << item.name_metric << std::endl << "value: "
                              << end_to_end_delay << std::endl;
                } else {

                    config_file.open(path_csv + name_of_csv_file, ios::app);
                    if (!is_open) {
                        config_file << "metric,number,value,timestamp,message rate,payload size\n";
                        is_open = true;
                    }

                    config_file << item.name_metric + "," + to_string(count) + "," +
                                   to_string(end_to_end_delay) + "," + to_string(item.ts_end) +","+ to_string(msg_rate)+","+
                                                                                                                        to_string(payload)+
                                   "\n";
                    config_file.close();
                    count++;
                }
            }

        });
    }
    for_each(consumers.begin(), consumers.end(), [](thread &thread) {
        thread.join();
    });
}

static void
subscriber_thread(zsock_t *pipe, void *args) {
    catch_sigterm();
    // -----------------------------------------------------------------------------------------------------
    BlockingQueue<Item2> queue(QUEUE_CAPACITY); //initialize queue
    // -----------------------------------------CREATING CONSUMERS----------------------------------------------------
    thread thread_start_consumers([&queue]() {
        create_new_consumers(&queue);
        cout << "start new threads consumers\n";
    });// create new thread to manage payload

    //--------------------------------------------------------------------------------------------------------
    auto *sub = static_cast<zsock_t *>(args); // create new sub socket
    puts("sub connected");

    int count = 0;
    //long time_of_waiting = 0;
    int c = 0;
    while (!zctx_interrupted && c < NUM_MEX_MAX && !zsys_interrupted) {
        c++;
        char *topic;
        zmsg_t *msg;
        long end;
        zsock_recv(sub, "sm", &topic, &msg);
        end = zclock_usecs();
        zsys_info("Recv on %s", topic);
        string metric = "end_to_end_delay";
        thread producer([msg, &queue, &end]() {
            printf("start new thread producer\nadding value...\n");
            payload_managing(msg, &queue, end);
        });
        free(topic);
        count++;
        producer.join();
    }
    thread_start_consumers.join();
    zsock_destroy(&sub);
}


int main(int argc, char *argv[]) {
    catch_sigterm();
    char *cmdstring; // string of args
    if (argc == 1) // exit if argc is less then 2
    {
        printf("NO INPUT JSON FILE OR TOO MANY ARGUMENTS...EXIT\n");
        return 1;
    } else {

        size_t strsize = 0; //size of the string to allocate memory

        strsize += (int) strlen(argv[1]);

        if (argc==2)
        {
            cout<<"Path for csv not chosen..."<<endl;
            return 2;
        }
        size_t strsize_2= (int) strlen(argv[2]);
        path_csv = argv[2];
        DIR* dir = opendir(path_csv);
        if (dir) {
            cout<<"path csv already exists"<<endl;
            closedir(dir);
        } else if (ENOENT == errno) {
            int a = mkdir(path_csv, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (a !=0){
                return 3;
            }
        }else{
            return 4;
        }
        cout<<"PATH chosen: "<<path_csv<<endl;
         // initialize the string
        cmdstring=argv[1];
        printf("INPUT FILE JSON (NAME): %s\n", cmdstring);
    }
    //path of json file
    string_json_path = cmdstring; // file passed from the bash script or manually from terminal

    // start deserialization
    json_object *PARAM;
    const char *endpoint_inproc;
    char *endpoint_customized;
    const char *topic;
    int num_of_subs = NUM_SUBS;
    PARAM = json_object_from_file(string_json_path);

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
        char endpoint[30] = "\0";
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
    zactor_t *sub_threads[num_of_subs];
    zsock_t *subscribers[num_of_subs];
    printf("Numbers of SUBS : %d\n", num_of_subs);
    for (int i = 0; i < num_of_subs; i++) {
        zclock_log("file json is being used");
        subscribers[i] = zsock_new_sub(endpoint_customized, topic);
        string name;
        name = topic + to_string(i);
        cout << "Starting new sub thread :" + name << endl;
        sub_threads[i] = zactor_new(subscriber_thread, subscribers[i]);
        cout << "new actor created..." << endl;

    }
    /*
     * destroying zactors of subs
     */
    for (int i = 0; i < num_of_subs; i++) {
        printf("destroying zactor and zsocket: No.%d\n", i);
        zactor_destroy(&sub_threads[i]);
        zsock_destroy(&subscribers[i]);
    }


    return 0;
}

