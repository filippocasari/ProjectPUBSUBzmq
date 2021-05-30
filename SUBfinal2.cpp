#include "Utils/linkedblockingqueueItems.h"
#include <czmq.h>
#include <json-c/json.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <algorithm>
#include "Utils/Item.h"

#define NUM_PRODUCERS 1
#define NUM_CONSUMERS 5
#define QUEUE_CAPACITY 100


#define NUM_SUB 1
#define ENDPOINT endpoint_tcp
//#define MSECS_MAX_WAITING 10000
const char *endpoint_tcp = "tcp://127.0.0.1:6000";
//const char *endpoint_inprocess = "inproc://example";
const char *string_json_path;


long managing_payload(Item *item) {

    char *end_pointer_string;
    long start;
    long end_to_end_delay = 0;
    char *frame;
    zmsg_t *msg =item->msg;
    std::cout << "size of msg: " << zmsg_size(msg) << std::endl;

    while (zmsg_size(msg) > 0) {

        frame = zmsg_popstr(msg);
        if (strcmp(frame, "TIMESTAMP") == 0) {
            frame = zmsg_popstr(msg);
            zsys_info("> %s", frame);
            start = strtol(frame, &end_pointer_string, 10);
            frame = zmsg_popstr(msg);
            zsys_info("PAYLOAD > %s", frame);
            end_to_end_delay = item->timestamp - start;
            printf("END TO END DELAY : %ld [micro secs]\n", end_to_end_delay);
            //zsys_info("> %s", frame);
            //free(frame);
            break;
        } else {
            std::cout << "error...this message does not contain any timestamp";
            break;
        }
    }
    return end_to_end_delay;
}

void create_new_consumers(BlockingQueue<Item> *queue) {
    bool console = false;
    std::string name_of_experiment;
    json_object *PARAM;
    PARAM = json_object_from_file(string_json_path);
    json_object_object_foreach(PARAM, key, val) {
        if (strcmp(key, "metrics_output_type") == 0) {
            char *value = const_cast<char *>(json_object_get_string(val));
            if (strcmp(value, "console") == 0)
                console = true;
            else
                std::cout << "creating new file csv\n";

        }
        if (strcmp(key, "experiment_name") == 0)
            name_of_experiment = json_object_get_string(val);

    }
    std::vector<std::thread> consumers; // create a vector of consumers
    consumers.reserve(NUM_CONSUMERS);
    std::ofstream myfile;
    std::string name_of_csv_file = name_of_experiment /*+ '_' + std::to_string(zclock_time()) */ + ".csv";
    int count = 0;

    for (int i = 0; i < NUM_CONSUMERS; i++) { //same as producers
        consumers.emplace_back([&queue, console, &myfile, name_of_csv_file, &count]() {

            Item item = Item();
            while (queue->Pop(item)) {

                puts("created new item...");
                long end_to_end_delay = 0;

                end_to_end_delay = managing_payload(&item);
                puts("managing message...");
                if (console) {
                    std::cout << "Metric name: " << item.name_metric << std::endl << "value: "
                              << end_to_end_delay;
                } else {
                    myfile.open(name_of_csv_file, std::ios::app);
                    myfile << item.name_metric + "," + std::to_string(count) + "," +
                              std::to_string(end_to_end_delay) +
                              "\n";
                    myfile.close();
                    count++;
                }
                item=Item();
            }

        });
    }
    std::for_each(consumers.begin(), consumers.end(), [](std::thread &thread) {
        thread.join();
    });
}

void add_value(std::string *metric_name, zmsg_t *msg, BlockingQueue<Item> *queue, const long end) {
    // create producers
    Item item = Item(msg, end, *metric_name);

    std::cout << "THREAD PRODUCER: IS PUSHING" << std::endl;
    std::cout << "Item.msg\n";
    zmsg_print(msg);
    std::cout << "timestamp caught: " << item.timestamp;
    queue->Push(item); // push element
    std::cout << "enqueued item " << std::endl;
}

static void
subscriber_thread(zsock_t *pipe, void *args) {
    // -----------------------------------------------------------------------------------------------------
    BlockingQueue<Item> queue(QUEUE_CAPACITY); //initialize queue
    // -----------------------------------------------------------------------------------------------------
    // create producers

    std::cout << "start new threads consumers\n";

    std::thread thread_start_consumers([&queue]() {
        create_new_consumers(&queue);

    });// create new thread to manage payload


    //--------------------------------------------------------------------------------------------------------
    auto *sub = static_cast<zsock_t *>(args); // create new sub socket
    puts("sub connected");

    int count = 0;
    //long time_of_waiting = 0;
    while (!zctx_interrupted /*&& time_of_waiting<MSECS_MAX_WAITING*/) {
        char *topic;
        zmsg_t * msg;
        long end;
        zsock_recv(sub, "sm", &topic, &msg);
        end = zclock_usecs();
        zsys_info("Recv on %s", topic);
        std::thread thread_producer([&msg, &queue, &end]() {
            std::cout << "start new thread producer\nadding value...\n";
            std::string metric = "end_to_end_delay";
            add_value(&metric, msg, &queue, end);
        });// create new thread to manage payload
        thread_producer.join();
        free(topic);
        zmsg_destroy(&msg);
        count++;
    }
    zsock_destroy (&sub);
}


int main(int argc, char *argv[]) {
    char *cmdstring; // string of args
    if (argc < 1) // exit if argc is less then 1
    {
        printf("NO INPUT JSON FILE OR TOO MANY ARGUMENTS...EXIT\n");
        return 1;
    } else {
        int i;
        size_t strsize = 0; //size of the string to allocate memory
        for (i = 1; i < argc; i++) {
            strsize += strlen(argv[i]);
            if (argc > i + 1)
                strsize++;
        }
        strsize = (int) strsize; // converting into an int value

        cmdstring = static_cast<char *>(malloc(strsize)); // malloc for the string cmd string
        cmdstring[0] = '\0'; // initialize the string

        for (i = 1; i < argc; i++) {
            strcat(cmdstring, argv[i]);
            if (argc > i + 1)
                strcat(cmdstring, " "); //concat the string with a blank
        }
        printf("INPUT FILE JSON (NAME): %s\n", cmdstring);
    }
    //path of json file
    string_json_path = cmdstring; // file passed from the bash script or manually from terminal
    // start deserialization
    json_object *PARAM;
    const char *endpoint_inproc;
    char *endpoint_customized;
    const char *topic;
    int num_of_subs;
    PARAM = json_object_from_file(string_json_path);

    if (PARAM != nullptr) {
        puts("PARAMETERS PUBLISHER: ");
        const char *type_connection;
        const char *port;
        const char *ip;
        const char *output_file;
        int payload_size;
        int num_mex;
        int int_value;

        const char *value;
        json_object_object_foreach(PARAM, key, val) {

            if (json_object_is_type(val, json_type_int)) {
                int_value = (int) json_object_get_int64(val);
                if (strcmp(key, "number_of_messages") == 0)
                    num_mex = int_value;
                if (strcmp(key, "payload_size_bytes") == 0)
                    payload_size = int_value;
                if (strcmp(key, "num_of_subs") == 0)
                    num_of_subs = int_value;

            } else {
                value = json_object_get_string(val);
            }


            printf("\t%s: %s\n", key, json_object_to_json_string(val));
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
    }
    // default settings for default mode
    if (PARAM == nullptr)
        num_of_subs = NUM_SUB;
    zactor_t *sub_threads[num_of_subs];
    zsock_t *subscribers[num_of_subs];
    for (int i = 0; i < num_of_subs; i++) {
        if (PARAM != nullptr) {
            zclock_log("file json is being used");
            subscribers[i] = zsock_new_sub(endpoint_customized, topic);
        } else {
            subscribers[i] = zsock_new_sub(ENDPOINT, "ENGINE");
        }
        sub_threads[i] = zactor_new(subscriber_thread, subscribers[i]);
        std::string name;
        name = topic + std::to_string(i);
        std::cout << "Starting new sub thread :" + name << std::endl;
    }

    for (int i = 0; i < num_of_subs; i++) {
        zactor_destroy(&sub_threads[i]);
        zsock_destroy(&subscribers[i]);
    }

    return 0;
}