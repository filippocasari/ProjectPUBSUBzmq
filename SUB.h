//
// Created by Filippo Casari on 03/09/21.
//

#ifndef PROJECTPUBSUBZMQ_SUB_H
#define PROJECTPUBSUBZMQ_SUB_H
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
#include <string>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#define NUM_CONSUMERS 4

#define NUM_SUBS 1
#define ENDPOINT endpoint_tcp
#define NUM_MEX_MAX 10000

using namespace std;
//#define MSECS_MAX_WAITING 10000
const char *endpoint_tcp = "tcp://127.0.0.1:6000";
//const char *endpoint_inprocess = "inproc://example";
const char *string_json_path;
//bool multithread=false;
//bool multiple_sub =false;
const char *type_test;
mutex cout_mutex;
mutex access_to_file;
BlockingQueue<Item2> lockingQueue; //initialize lockingQueue
//ofstream config_file_common;
atomic<bool> finished;
void write_safely(string *what_i_said){
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

        string say = "size of msg: " +to_string(zmsg_size(*msg));
        write_safely(&say);
        frame = zmsg_popstr(*msg);
        if(strcmp(frame, "TERMINATE")==0){
            cout<<"Message received :"<<frame<<endl;
            cout<<"exit"<<endl;
            return 1;
        }

        if (strcmp(frame, "TIMESTAMP") == 0) {
            frame = zmsg_popstr(*msg);
            if(verbose){
                say ="frame: "+ string(frame);
                write_safely(&say);
            }
            string start = frame;
            Item2 item(frame,end, "end_to_end_delay",c   ) ;
            lockingQueue.push(item);
            if(verbose){
                //say="Size of the queue: "+to_string(lockingQueue.size())+"\nitem No. " + to_string(*c) +" pushed";
                //write_safely(&say);
            }
        }
        if (zmsg_size(*msg) == 0) {
            if(verbose){
                say = "NO MORE MESSAGES";
                write_safely(&say);
            }

            return -2;
        }
        while (zmsg_size(*msg) > 0) {
            //puts("estrapolating other payload...");
            frame = zmsg_popstr(*msg);
            if(verbose){
                say = "size of payload (byte): "+ to_string((strlen(frame) * sizeof(char)));
                write_safely(&say);
            }
            //zsys_info("PAYLOAD > %s", frame);
        }
        zmsg_destroy(msg);
        return 0;
    }
    catch (int e) {
        return -3;
    }
}

int syncronization( const char* ip, const char* port) {
    string endpoint_sync = "tcp";
    endpoint_sync.append("://");

    //only for tcp, not for in process connection

    endpoint_sync.append(get_ip());
    endpoint_sync.append( ":");
    endpoint_sync.append(to_string(atoi(port)+1));

    cout<<"Endpoint for Sync service: "<<endpoint_sync<<endl;
    zsock_t *syncservice = zsock_new_req(endpoint_sync.c_str());

    zsock_send(syncservice, "s", "INIT");
    char *string;
    zsock_recv(syncservice, "s",&string);
    free(string);
    zsock_destroy(&syncservice);
    return 0;


}

static void create_new_consumers(void *args) {
    const char *path_csv = static_cast<const char *>(args);
    //zsock_signal(pipe, 0);
    bool console = false;
    int num_consumers = NUM_CONSUMERS;
    char *name_of_experiment;
    json_object *PARAM;
    int msg_rate;
    int payload;
    int number_of_messages;
    PARAM = json_object_from_file(string_json_path);
    json_object_object_foreach(PARAM, key, val) {
        if (strcmp(key, "metrics_output_type") == 0)
        {
            char *value = const_cast<char *>(json_object_get_string(val));
            if (strcmp(value, "console") == 0)
                console = true;
            else
                cout<<"creating new file csv"<<endl;
        }
        if (strcmp(key, "experiment_name") == 0)
            name_of_experiment = (char*)json_object_get_string(val);
        if (strcmp(key, "num_consumer_threads") == 0)
            num_consumers = (int) strtol(json_object_get_string(val), nullptr, 10);
        if (strcmp(key, "msg_rate_sec") == 0)
            msg_rate = (int) strtol(json_object_get_string(val), nullptr, 10);
        if (strcmp(key, "payload_size_bytes") == 0)
            payload = (int) strtol(json_object_get_string(val), nullptr, 10);
        if(strcmp(key, "number_of_messages")==0)
            number_of_messages=(int) strtol(json_object_get_string(val), nullptr, 10);
    }
    string name_of_csv_file = name_of_experiment /*+ '_' + std::to_string(zclock_time()) */ ;
    name_of_csv_file.append(".csv");
    printf("Num of consumer threads: %d\n", num_consumers);
    string name_path_csv = path_csv + name_of_csv_file;
    if(num_consumers>1)
        access_to_file.lock();
    //config_file_common.open(name_path_csv, ios::app);
    //config_file_common << "number,value,timestamp,message rate,payload size\n";
    //config_file_common.close();
    if(num_consumers>1)
        access_to_file.unlock();

    sleep(1);
    auto name = this_thread::get_id();
    stringstream id;
    id <<name;
    string say = "new consumer thread created with ID: "+id.str();
    write_safely(&say);
    int64_t end_to_end_delay;
    int c = 0;
    Item2 item;
    int number_of_iterations=(int)number_of_messages/num_consumers;
    while (c<number_of_iterations && !zsys_interrupted) {

        item=lockingQueue.pop();

        say ="--------------THREAD No. "+id.str()+" IS WORKING--------";
        write_safely(&say);
        char *end_pointer;
        int64_t start = strtoll(item.ts_start.c_str(), &end_pointer, 10);
        if(verbose){
            say =  "end : " + to_string(item.ts_end) + " start: " + to_string(start) + "\nmanaging payload";
            write_safely(&say);
        }

        end_to_end_delay = item.ts_end - start;

        if (console) {
            say = "Metric name: " + item.name_metric +"\nvalue: "
                    + to_string(end_to_end_delay);
            write_safely(&say);
        } else {
            //cout << "lockingQueue empty? " << lockingQueue->empty() << endl;
            if(verbose){
                say ="opening file csv...";
                write_safely(&say);
            }
            if(num_consumers>1)
                access_to_file.lock();

            //config_file_common.open(name_path_csv, ios::app);
            //config_file_common << to_string(item.num) + "," + to_string(end_to_end_delay) + "," +
            to_string(item.ts_end) +
            "," + to_string(msg_rate) + "," + to_string(payload) + "\n";
            //config_file_common.close();
            if(num_consumers>1)
                access_to_file.unlock();
            say = "--------------THREAD No. " +id.str()+" FINISHED ITS JOB----------\nValue of c: "+to_string(c) ;
            write_safely(&say);
        }
        c++;
        if(finished.load(std::memory_order_relaxed)){
            if(lockingQueue.size()==0)
                break;
        }


    }
    say = "thread is closing...";
    write_safely(&say);

}

void
subscriber_thread(const char *endpoint_custom, char *topic, const char *path_csv, const char *name_of_experiment, const int *payload, const int *msg_rate) {

    // -----------------------------------------------------------------------------------------------------

    // -----------------------------------------CREATING CONSUMERS----------------------------------------------------
    finished=false;
    cout<<"STARTING NEW CONSUMERS"<<endl;
    //zactor_new(create_new_consumers, (void *) path_csv);

    //--------------------------------------------------------------------------------------------------------
    //auto *sub = static_cast<zsock_t *>(args); // create new sub socket
    cout<<"topic is "<<topic<<endl;

    //sub= zsock_new_sub(endpoint_custom->c_str(), topic);

    //long time_of_waiting = 0;
    int64_t c =0;
    //bool terminated = false;
    auto *sub = zsock_new_sub(endpoint_custom, topic);
    int64_t end;
    string metric = "end_to_end_delay";
    zmsg_t *msg = zmsg_new();
    int succ;

    while(!zsys_interrupted) {
        succ=zsock_recv(sub, "s8m", &topic, &c, &msg);

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
        //increment_counter.lock();
        //increment_counter.unlock();
        int a = payload_managing(&msg, &end, &c);
        cout << "managing payload exit code: " << a << endl;
        if(c==(NUM_MEX_MAX-1) or succ==-1){
            cout<<"TERMINATING"<<endl;
            zclock_sleep(1000);
            break;
        }
        //zmsg_destroy(&msg);
    }

    sleep(3);
    string name_of_csv_file = name_of_experiment /*+ '_' + std::to_string(zclock_time()) */  ;
    name_of_csv_file.append(".csv");
    string name_path_csv = path_csv ;
    name_path_csv.append("/" +name_of_csv_file);
    cout<<"OPENING FILE: "<<name_path_csv<<endl;
    ofstream config_file;
    sleep(3);
    config_file.open(name_path_csv, ios::app);
    config_file << "number,value,timestamp,message rate,payload size\n";
    config_file.close();
    Item2 item;
    int64_t end_to_end_delay;
    string say;
    cout<<"Size of the queue: "<<lockingQueue.size()<<endl;
    while(true){
        if(verbose){
            say ="trying to pop new item...";
            write_safely(&say);
        }
        item=lockingQueue.pop();
        char *end_pointer;
        int64_t start = strtoll(item.ts_start.c_str(), &end_pointer, 10);
        end_to_end_delay = item.ts_end - start;
        if(verbose){
            say ="opening file csv...";
            write_safely(&say);
        }
        //access_to_file.lock();
        config_file.open(name_path_csv, ios::app);
        config_file << to_string(item.num) + "," + to_string(end_to_end_delay) + "," +
                       to_string(item.ts_end) +
                       "," + to_string( *msg_rate) + "," + to_string(*payload) + "\n";
        config_file.close();
        //access_to_file.unlock();
        if(lockingQueue.size()==0)
            break;
    }
    cout<<"all items dequeued"<<endl;
    //thread_start_consumers.join();
    zsock_destroy(&sub);
    //terminate();
}


int main_S(int argc, char **argv) {
    for(int i=1; i<argc; i++){
        cout<<"ARGV["<<i<<"]: "<<argv[i]<<endl;
    }
    char *cmdstring; // string of args
    char *path_csv;
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
            if (a != 0)
            {
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
    char *topic;
    char *type_connection;
    const char *port;
    const char *ip;
    const char *output_file;
    const char *v =  (char *) argv[3];
    if(strcmp(v, "-v") == 0)
        verbose=true;
    else
        verbose=false;
    if (PARAM != nullptr) {
        puts("PARAMETERS PUBLISHER: ");

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
                topic = (char *)value;
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
    cout<<"Numbers of SUBS : "<< num_of_subs<<endl;


    //subscriber_thread(reinterpret_cast<const char *>(&endpoint_customized), topic, path_csv);
    int success=syncronization( ip, port);
    assert(success==0);
    cout<<"Syncronization success"<<endl;
    cout << "END OF SUBSCRIBER" << endl;
    return 0;
}




static void main_SUB_M(zsock_t *pipe, void *args) {

    zsock_signal(pipe, 0);
    const string *argv = (string *) args;
    cout<<"ARGS RECEIVED: "<<argv->c_str()<<endl;
    char *temp = (char *) argv->c_str();

    string str = temp;
    size_t pox = str.find(',');
    int pox2 = (int ) str.find('&');
    int argc= (int) strlen(reinterpret_cast<const char *>(argv->c_str()));
    string substring = str.substr(pox2, str.length()-pox2);
    string csv = str.substr(pox + 1, pox2-1-pox);
    const char *path_csv = (const char *) csv.c_str();
    const char *v =  (const char *) substring.c_str();
    char *cmdstring= nullptr; // string of args
    if (argc == 1) // exit if argc is less then 2
        cout<<"NO INPUT JSON FILE OR TOO MANY ARGUMENTS...EXIT"<<endl;
    else {

        //size_t strsize = 0; //size of the string to allocate memory

        //strsize += (int) strlen(args[1]);

        if (argc == 2) {
            cout << "Path for csv not chosen..." << endl;
        }
        cout<<"POX of ',' : "<<pox<<endl;
        cout<<"Length of string :"<<(int) str.length()<<endl;


        cout<<"PATH: "<<path_csv<<endl;

        DIR *dir = opendir(path_csv);
        if (dir) {
            cout << "path csv already exists" << endl;
            closedir(dir);

        } else if (ENOENT == errno) {
            int a = mkdir(path_csv, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (a != 0)
            {
                cout<<"Error to create a directory"<<endl;
            }
        } else {
            cout<<"Error unknown, i could not be possible to open or create a directory for csv tests"<<endl;
        }

        cout << "PATH chosen: " << path_csv << endl;
        // initialize the string
        cmdstring = (char *) str.substr(0,  pox).c_str();
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
    char *topic= nullptr;
    char *type_connection;
    const char *port;
    const char *ip;
    const char *output_file;
    //bool console = false;
    //int num_consumers = NUM_CONSUMERS;
    char *name_of_experiment;
    int msg_rate;
    int payload;
    puts("PARAMETERS SUBSCRIBER: ");
    if (PARAM != nullptr) {


        if(strcmp(v, "-v") == 0)
            verbose=true;
        else
            verbose=false;
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
                if (strcmp(key, "msg_rate_sec") == 0)
                    msg_rate = int_value;
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
            if (strcmp(key, "experiment_name") == 0)
                name_of_experiment =  (char*) json_object_get_string(val);
            if (strcmp(key, "payload_size_bytes") == 0)
                payload = (int) strtol(json_object_get_string(val), nullptr, 10);
            if (strcmp(key, "topic") == 0)
                topic = (char *)value;
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

    }
    cout<<"Numbers of SUBS : "<< num_of_subs<<endl;
    int success=syncronization( ip, port);
    assert(success==0);
    cout<<"Syncronization success"<<endl;
    subscriber_thread(reinterpret_cast<const char *>(&endpoint_customized), topic,
                      path_csv, name_of_experiment, &payload, &msg_rate);
    cout << "END OF SUBSCRIBER" << endl;
}
#endif //PROJECTPUBSUBZMQ_SUB_H
