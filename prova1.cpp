#include "linkedblockingqueue.h"
#include <czmq.h>
#include <json-c/json.h>
#include <fstream>

#define NUM_PRODUCERS 1
#define NUM_CONSUMERS 5
#define QUEUE_CAPACITY 5


#define NUM_SUB 1
#define ENDPOINT endpoint_tcp
#define MSECS_MAX_WAITING 10000
const char *endpoint_tcp = "tcp://127.0.0.1:6000";
const char *endpoint_inprocess = "inproc://example";


void create_new_consumers(blocking_queue<long> *queue) {
    bool console=false;
    const char *string_json_path = "/home/filippocasari/CLionProjects/ProjectPUBSUBzmq/parameters.json";
    json_object *PARAM;
    PARAM = json_object_from_file(string_json_path);
    json_object_object_foreach(PARAM, key, val)
    {
        if(strcmp(key, "metrics_output_type")==0){
            char *value = const_cast<char *>(json_object_get_string(val));
            if(strcmp(value, "cosnole")==0){
                console=true;
            }
        }
    }
    std::vector<std::thread> consumers; // create a vector of consumers
    consumers.reserve(NUM_CONSUMERS);
    std::cout<<"creating new file csv\n";
    std::ofstream myfile;
    std::string name_of_csv_file= "experiment_"+std::to_string(zclock_time())+".csv";
    int count=0;
    for (int i = 0; i < NUM_CONSUMERS; i++) { //same as producers
        consumers.emplace_back([&queue, console, &myfile, name_of_csv_file, &count]() {

            //int count = 0; //count for tracking, if count is "x" => shut down the thread
            while (long a = (int) queue->pop()) { // polling thread
                if(console){
                    std::cout << "\nTHREAD CONSUMER, POPPING ELEMENT : " << a << std::endl;
                }
                else
                {
                    myfile.open (name_of_csv_file, std::ios::app);
                    myfile <<"end_to_end_delay,"+std::to_string(count)+","+std::to_string(a)+"\n";
                    myfile.close();
                    count++;

                }
            }
        });
    }

    std::for_each(consumers.begin(), consumers.end(), [](std::thread &thread) {
        thread.join();
    });
}

void add_value(const char *matric_name, long value, blocking_queue<long> *queue){
    // create producers
    std::vector<std::thread> producers; //vector of producers
    producers.reserve(NUM_PRODUCERS);
    for(int i = 0; i < NUM_PRODUCERS; i++) {
            producers.emplace_back([&queue, value]() {

                std::cout << "THREAD PRODUCER:  IS PUSHING" << std::endl;
                std::cout <<"Pushed VALUE: "<<value<<std::endl;
                queue->push(value); // push element "end to end delay" (in this case is a long)
            });
        }
    std::for_each(producers.begin(), producers.end(), [](std::thread &thread) {
        thread.join();
    });
}

static void
subscriber_thread(zsock_t *pipe, void *args) {

    // -----------------------------------------------------------------------------------------------------
    blocking_queue<long> queue(QUEUE_CAPACITY); //initialize queue

    // create producers
    std::vector<std::thread> producers;
    std::thread t([&queue](){
        std::cout << "start new threads consumers\n";
        create_new_consumers(&queue);
    });//vector of producers

    //--------------------------------------------------------------------------------------------------------
    auto *sub = static_cast<zsock_t *>(args);

    //assert(sub);
    puts("sub connected");

    int count = 0;
    long time_of_waiting = 0;
    while (!zctx_interrupted /*&& time_of_waiting<MSECS_MAX_WAITING*/) {
        char *topic;
        char *frame;
        zmsg_t *msg;

        zsock_recv(sub, "sm", &topic, &msg);
        zsys_info("Recv on %s", topic);

        char *end_pointer_string;
        long end;
        long start;
        long end_to_end_delay;
        while (zmsg_size(msg) > 0) {

            frame = zmsg_popstr(msg);
            if (strcmp(frame, "TIMESTAMP") == 0) {

                frame = zmsg_popstr(msg);
                zsys_info("> %s", frame);
                start = strtol(frame, &end_pointer_string, 10);
                frame = zmsg_popstr(msg);
                end = zclock_usecs();
                zsys_info("PAYLOAD > %s", frame);
                end_to_end_delay = end - start;
                printf("END TO END DELAY : %ld [micro secs]\n", end_to_end_delay);
                add_value("end_to_end_delay", end_to_end_delay, &queue);
                zsys_info("> %s", frame);
                //free(frame);
                break;
            }
        }

            free(topic);
            zmsg_destroy(&msg);
            count++;
        }
        zsock_destroy (&sub);
}




int main() {

    const char *string_json_path = "/home/filippocasari/CLionProjects/ProjectPUBSUBzmq/parameters.json";
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
        //endpoint_customized = strcat(endpoint_customized, ip);
        //endpoint_customized = strcat(endpoint_customized, ":");
        //endpoint_customized = strcat(endpoint_customized, port);
        printf("string for endpoint (from json file): %s\t", endpoint_customized);


    }


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

        char name[15];
        sprintf(name, "%s-%d", topic, i);
        printf("Starting new sub thread : %s\n", name);

    }

    for (int i = 0; i < num_of_subs; i++) {
        zactor_destroy(&sub_threads[i]);
        zsock_destroy(&subscribers[i]);
    }

    return 0;
}