//
// Created by Filippo Casari
//
#ifndef PROJECTPUBSUBZMQ_SUB_H
#define PROJECTPUBSUBZMQ_SUB_H
#include <czmq.h>
#include <json-c/json.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <algorithm>
#include "Utils/Item.h"
#include <mutex>
#include <sstream>
#include <vector>
#include "Utils/BlockingQueue.h"
#include <string>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <atomic>

//#define NUM_CONSUMERS 1 // deprecated. Just to say how many consumer threads to store values on Csv.
// it is not necessary because we do not want speed thread to write on a csv file

//#define NUM_SUBS 7 // You can set how many Sub you want
#define ENDPOINT endpoint_tcp // default endpoint
#define NUM_MEX_MAX 5000 // default messages
#define TIMEOUT 70000
//#define MSECS_MAX_WAITING 10000 // we would have implemented maximum milli secs to wait
const char *endpoint_tcp = "tcp://127.0.0.1:6000"; // default tcp endpoint
//const char *type_test; // type of the test TODO is it necessarily global?
using namespace std; // using standard library
mutex cout_mutex; // semaphore to write safely on standard output if we got multi thread consumers
mutex access_to_file; // semaphore to access safely to csv file
bool time_mono=false;
//atomic<bool> finished; // just a simple boolean to tell
// everyone that thread sub is finished and threads consumers must be turned off
// simple function to write on standard output thread safely
void write_safely(string *what_i_said) {
    // starting critical section
    cout_mutex.lock();
    cout << *what_i_said << endl;
    cout_mutex.unlock();
    // critical section ends here
}

// function to separate divide scope of functions
int payloadManaging(zmsg_t **msg, const int64_t
*end, const int64_t *c, BlockingQueue<Item> *lockingQueue,const bool *verbose) {

    auto *item = new Item(); // create new Items

    // try, if throws an exception, catch it
    try {
        char *frame; //  it is a string refers to the first frame of the message at the beginning

        // just a log (safe)
        string say = "size of msg: " + to_string(zmsg_size(*msg));
        //write_safely(&say);

        frame = zmsg_popstr(*msg); // pop a string from the message,
        // size of message decreases each time you pop something
        if (strcmp(frame, "TERMINATE") == 0) { // if message contains this particular string, return 1
            cout << "Message received :" << frame << endl;
            cout << "exit" << endl;
            return 1;
        }
        // if this frame is a string containing "TIMESTAMP", it means the following frames are about timestamps.
        if (strcmp(frame, "TIMESTAMP") == 0) {
            frame = zmsg_popstr(*msg); // another pop
            if (verbose) {
                //say = "frame: " + string(frame);
                //write_safely(&say);
            }
            char *pointer;
            int64_t start = strtoll(frame, &pointer,
                                    10); // this string contains temporally the sending timestamp of the publisher

            // let's assign values to Item instance
            item->ts_start = start;
            item->ts_end = *end;
            item->name_metric = "end_to_end_delay";
            item->num = *c;
            // push it into the queue
            lockingQueue->push(*item);
        }
        // if the message caught is empty, exit
        if (zmsg_size(*msg) == 0) {
            if (verbose) {
                say = "NO MORE MESSAGES";
                write_safely(&say);
            }

            return -2;
        }
        // in case of non-empty message, pop all the remaining frames
        while (zmsg_size(*msg) > 0) {

            zmsg_popstr(*msg);
            if (verbose) {
                //say = "size of payload (byte): " + to_string((strlen(frame) * sizeof(char)));
                //write_safely(&say);
            }
            // zsys_info("PAYLOAD > %s", frame); // commented 'cause cout is too busy
        }
        // destroy the message, it is not useful anymore
        zmsg_destroy(msg);
        // return 0 if everything went how we had guessed
        return 0;
    }
        // exit otherwise
    catch (int e) {
        return -3;
    }
}

// this function implements the sync service
//**** note that "ip" variable is passed even if it is not used. thus, it could be useful,
// in some cases we want our sync service not be on the localhost ****
int synchronizationService(const char *ip, const char *port) {
    // let's initialize a string
    string endpoint_sync = "tcp";
    endpoint_sync.append("://");
    endpoint_sync.append(ip);
    endpoint_sync.append(":");
    endpoint_sync.append(to_string(atoi(port) + 1)); // port number= port number (passed) +1.
    // Just to not introduce another variable
    // *** Note: atoi is not safe!
    // we have to find another function that do it better ***

    cout << "SUB> Endpoint for Sync service: " << endpoint_sync << endl;
    // *** WE ARE IMPLEMENTING REQ/REP pattern to get sync service ***
    zsock_t *syncservice = zsock_new_req(endpoint_sync.c_str());
    zsock_send(syncservice, "s", "INIT"); // send a simple string to say to pub: "I am ready, boy!"
    // now we must wait for the reply of the pub
    char *string;
    zsock_recv(syncservice, "s", &string);
    // destroy socket. We do not need it anymore
    sleep(2);
    zsock_destroy(&syncservice);
    // return 0; it is all okay
    return 0;
}


// *** the crucial function. It takes the endpoint to connect, the topic to subscribe to , path csv to write on,
// name of the experiment to save the file,
// how much is the payload and what is the message rate ***
void
subscriber(const char *endpoint_custom, char *topic, const char *path_csv, const char *name_of_experiment,
           const int *payload, const int *msg_rate, const bool *verbose) {

    // ------------------------------------- STARTING THE SUB -------------------------------------------------------------
    BlockingQueue<Item> lockingQueue; // declare The Queue, is a blocked-locking queue, inspired by Java,
    // but in this case works like a common queue. It can be crucial if we create more than one consumer because the queue is thread safe.
    // It is controlled by a lock ( see Blocking Queue code, BlockingQueue.h).
    //if(lockingQueue.size()<(int)NUM_MEX_MAX)
    //    lockingQueue.d_queue.resize((int)NUM_MEX_MAX);
    //finished = false; // say: "not finished" to all threads ( if there are ), deprecated
    int id=(int)random();
    //--------------------------------------------------------------------------------------------------------
    // NEW SUB
    auto *sub = zsock_new_sub(endpoint_custom, topic); // new sub socket from ZMQ
    // ------------------- DECLARE VARIABLES -----------------------------------------------------------------

    int64_t c = 0; // this identifies the number of the message
    int64_t end; // timestamp ( when the message was received )
    string metric = "end_to_end_delay"; // metric name
    int succ; // flag to say if receive function succeeded or not.
    zmsg_t *msg = zmsg_new(); // create an empty message
    ofstream config_file; // initializing a new csv file
    Item item; // declare new object Item to dequeue
    double end_to_end_delay; // this is the end to end delay
    string say; // string "say" just to print safely

    // ----------------- BEGINNING OF WHILE LOOP TO RECEIVE MESSAGES --------------------------------------
    //int64_t starting_point = zclock_mono();
    int counter=0;
    while (!zsys_interrupted and c<NUM_MEX_MAX-1) {
        // function to receive. It is blocking. It takes 1 string, 1 int64, 1 message ( that contains other payload)
        succ = zsock_recv(sub, "s8m", &topic, &c, &msg);
        // the message is null, size message incorrect
        counter++;
        if (msg == nullptr) {
            cout << "SUB> exit, msg null" << endl;
            break;
        }
        // timestamps of receiving
        end=zclock_usecs();
        if(counter%1000==0)
            cout<<"SUB No. "<<id<<", counter value: "<<counter<<endl;
        //cout<<"Recv on "<< topic<<endl;
        //cout<<"message Received: No. "<< c<<endl;
        // lets menage the payload... passing message, end timestamp, counter, queue
        payloadManaging(&msg, &end, &c, &lockingQueue, verbose);

        //cout << "managing payload exit code: " << a << endl;
        // if the last number received is Mex-1 or received function does not return 0, stop
        if (succ == -1) {
            cout << "SUB> TERMINATING " << endl;
            break;
        }
    }

    sleep(3); // sleep for a while before starting to dequeue
    // it is easier to work with string, so let's convert char arrays into string c++
    string name_of_csv_file = name_of_experiment; // name of the experiment ==> we get name of csv
    name_of_csv_file.append(".csv");
    string name_path_csv = path_csv;
    name_path_csv.append("/" + name_of_csv_file);
    cout << "OPENING FILE: " << name_path_csv << endl;
    // open the file csv and append the (column) names of metrics
    config_file.open(name_path_csv, ios::app); // open the csv file and try to append metrics
    config_file << "number,value,timestamp,message rate,payload size\n";
    config_file.close();
    // printing size of the Queue
    int size_queue= (int)lockingQueue.size();
    cout << "SUB> Size of the queue: " << size_queue << endl;

    // ---------------------- STARTING CONSUMER THREAD --------------------------------------
    config_file.open(name_path_csv, ios::app); // open the csv file and try to append metrics
    int i=0;
    long start_time=zclock_mono();
    while(lockingQueue.size()!=0) {
        if (verbose) {
            //say = "SUB> trying to pop new item...";
            //write_safely(&say);
        }
        // pop Item from the queue ( it is internally already thread safe)
        item = lockingQueue.pop();
        // now, compute the difference between two nodes
        end_to_end_delay =((double) item.ts_end - (double)item.ts_start)/1000.0;
        if (verbose) {
            //say = "SUB> opening file csv...";
            //write_safely(&say);
        }

        // try to write metrics on csv
        config_file << to_string(item.num) + "," + to_string(end_to_end_delay) + "," +
                       to_string(item.ts_end) +
                       "," + to_string(*msg_rate) + "," + to_string(*payload) + "\n";
        // when queue is empty, exit
        i++;

    }
    config_file.close(); // close the csv file. We do not need to write on it anymore
    long end_time=zclock_mono();
    cout << "all items dequeued" << endl;
    cout<<"csv lines: "<<i<<endl;
    cout<<"time to deque all items: "<<(end_time-start_time)<<endl;
    zsock_destroy(&sub); // destroy subscriber socket ( it is mandatory)
}

static void startNewSubThread(zsock_t *pipe, void *args) {

    zsock_signal(pipe, 0); // You must call this function when you work with z-actor

    const string *argv= (string *) args;
    cout << "SUB> ARGS RECEIVED: " << *argv << endl;

    // ---------------USING A STRANGE/DUMB METHOD TO PARSE THE ARGUMENTS ---------------
    bool verbose=false;
    auto pox = (int32_t)argv->find(',');
    auto pox2 = (int32_t )argv->find('&');
    auto argc = (int32_t ) strlen(argv->c_str());
    string substring = argv->substr(pox2, (int32_t)argv->size() - pox2);
    string csv = argv->substr(pox + 1, pox2 - 1 - pox);
    const char *path_csv = (const char *) csv.c_str();
    const char *v = (const char *) substring.c_str();
    if (argc == 1) // exit if argc is less than 2
        cout << "SUB> NO INPUT JSON FILE OR TOO MANY ARGUMENTS...EXIT" << endl;
    else {
        if (argc == 2) {
            cout << "SUB> Path for csv not chosen..." << endl;
        }
        //cout<<"POX of ',' : "<<pox<<endl;
        //cout<<"Length of string :"<<(int) str.length()<<endl;
        // open the directory if exists
        DIR *dir = opendir(path_csv);
        if (dir) {
            cout << "SUB> path csv already exists" << endl;
            closedir(dir);

        } else if (ENOENT == errno) {
            // make it otherwise
            int a = mkdir(path_csv, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (a != 0) {
                cout << "Error to create a directory" << endl;
            }
        } else {
            cout << "SUB> Error unknown, i could not be possible to open or create a directory for csv tests" << endl;
        }
        cout << "SUB> PATH chosen: " << path_csv << endl;
    }
    string json_file = argv->substr(0, pox);
    cout << "SUB> STRING OF JSON FILE IS: " << json_file << endl; // file passed
    // from the bash script or manually from terminal
    // start deserialization
    json_object *PARAM; // json object, see library JSON-C
    const char *endpoint_inproc;
    string endpoint_customized;

    //int num_of_subs = NUM_SUBS;
    PARAM = json_object_from_file(json_file.c_str());
    char *topic = nullptr;
    char *type_connection;
    const char *port;
    const char *ip;
    //const char *output_file;
    //bool console = false;
    //int num_consumers = NUM_CONSUMERS;
    char *name_of_experiment;
    int msg_rate;
    int payload;
    puts("SUB> PARAMETERS SUBSCRIBER: ");
    if (PARAM != nullptr) {
        if (strcmp(v, "-v") == 0)
            verbose = true;
        else
            verbose = false;
        //int payload_size;
        //int num_mex;
        int int_value;

        const char *value;
        json_object_object_foreach(PARAM, key, val) {

            value = json_object_get_string(val);
            if (verbose)
                cout << key << " : " << value << endl;
            if (json_object_is_type(val, json_type_int)) {
                int_value = (int) json_object_get_int64(val);
                if (strcmp(key, "msg_rate_sec") == 0)
                    msg_rate = int_value;
            }
            if (strcmp(key, "connection_type") == 0)
                type_connection = (char *) value;
            if (strcmp(key, "ip") == 0)
                ip = value;
            if (strcmp(key, "port") == 0) {
                port = value;
            }
            //if (strcmp(key, "metrics_output_type") == 0)
            //    output_file = value;
            if (strcmp(key, "experiment_name") == 0)
                name_of_experiment = (char *) json_object_get_string(val);
            if (strcmp(key, "payload_size_bytes") == 0)
                payload = (int) strtol(json_object_get_string(val), nullptr, 10);
            if (strcmp(key, "topic") == 0)
                topic = (char *) value;
            if (strcmp(key, "endpoint_inproc") == 0)
                endpoint_inproc = value;
        }
        endpoint_customized = string() + type_connection + "://";
        if (strcmp(type_connection, "tcp") == 0)
            endpoint_customized = endpoint_customized + ip + ":" + port;
        else if (strcmp(type_connection, "inproc") == 0)
            endpoint_customized = endpoint_customized + endpoint_inproc;
        else
            cout << "invalid endpoint" << endl;
        cout << "SUB>string for endpoint (from json file):\t" << endpoint_customized << endl;
    } else {
        cout << "FILE JSON NOT FOUND...EXIT" << endl;

    }
    synchronizationService(ip, port);
    cout << "SUB> Synchronization success" << endl;
    subscriber(endpoint_customized.c_str(), topic,
               path_csv, name_of_experiment, &payload, &msg_rate, &verbose);
    cout << "SUB> END OF SUBSCRIBER" << endl;
}

#endif //PROJECTPUBSUBZMQ_SUB_H
