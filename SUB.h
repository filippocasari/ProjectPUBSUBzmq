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

#define NUM_CONSUMERS 1 // deprecated. Just to say how many consumer threads to store values on Csv.
// it is not necessary because we do not want speed thread to write on a csv file

#define NUM_SUBS 7 // You can set how many Sub you want
#define ENDPOINT endpoint_tcp // default endpoint
#define NUM_MEX_MAX 10000 // default messages
#define TIMEOUT 60000
//#define MSECS_MAX_WAITING 10000 // we would have implemented maximum milli secs to wait
const char *endpoint_tcp = "tcp://127.0.0.1:6000"; // default tcp endpoint
const char *type_test; // type of the test TODO is it necessarily global?

mutex cout_mutex; // semaphore to write safely on standard output if we got multi thread consumers
mutex access_to_file; // semaphore to access safely to csv file

//atomic<bool> finished; // just a simple boolean to tell
// everyone that thread sub is finished and threads consumers must be turned off

using namespace std; // using standard library

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
*end, const int64_t *c, BlockingQueue<Item> *lockingQueue) {

    auto *item = new Item(); // create new Items

    // try, if throws an exception, catch it
    try {
        char *frame; //  it is a string refers to the first frame of the message at the beginning

        // just a log (safe)
        string say = "size of msg: " + to_string(zmsg_size(*msg));
        write_safely(&say);

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

            frame = zmsg_popstr(*msg);
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
    endpoint_sync.append("0.0.0.0");
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
    zsock_destroy(&syncservice);
    // return 0; it is all okay
    return 0;
}

// despracated... it creates consumer threads to write to csv file
static void startNewConsumers(void *args) {
    const char *path_csv = static_cast<const char *>(args);
    //zsock_signal(pipe, 0);
    bool console = false;
    int num_consumers = NUM_CONSUMERS;
    char *name_of_experiment;
    json_object *PARAM;
    int msg_rate;
    int payload;
    int number_of_messages;
    //PARAM = json_object_from_file();
    json_object_object_foreach(PARAM, key, val) {
        if (strcmp(key, "metrics_output_type") == 0) {
            char *value = const_cast<char *>(json_object_get_string(val));
            if (strcmp(value, "console") == 0)
                console = true;
            else
                cout << "SUB> creating new file csv" << endl;
        }
        if (strcmp(key, "experiment_name") == 0)
            name_of_experiment = (char *) json_object_get_string(val);
        if (strcmp(key, "num_consumer_threads") == 0)
            num_consumers = (int) strtol(json_object_get_string(val), nullptr, 10);
        if (strcmp(key, "msg_rate_sec") == 0)
            msg_rate = (int) strtol(json_object_get_string(val), nullptr, 10);
        if (strcmp(key, "payload_size_bytes") == 0)
            payload = (int) strtol(json_object_get_string(val), nullptr, 10);
        if (strcmp(key, "number_of_messages") == 0)
            number_of_messages = (int) strtol(json_object_get_string(val), nullptr, 10);
    }
    string name_of_csv_file = name_of_experiment /*+ '_' + std::to_string(zclock_time()) */ ;
    name_of_csv_file.append(".csv");
    printf("SUB> Num of consumer threads: %d\n", num_consumers);
    string name_path_csv = path_csv + name_of_csv_file;
    if (num_consumers > 1)
        access_to_file.lock();
    //config_file_common.open(name_path_csv, ios::app);
    //config_file_common << "number,value,timestamp,message rate,payload size\n";
    //config_file_common.close();
    if (num_consumers > 1)
        access_to_file.unlock();

    sleep(1);
    auto name = this_thread::get_id();
    stringstream id;
    id << name;
    string say = "SUB> new consumer thread created with ID: " + id.str();
    write_safely(&say);
    int64_t end_to_end_delay;
    int c = 0;
    Item *item = new Item();
    int number_of_iterations = (int) number_of_messages / num_consumers;
    while (c < number_of_iterations && !zsys_interrupted) {

        //item=lockingQueue.pop();
        zclock_sleep(10);

        int64_t start = item->ts_start;

        end_to_end_delay = item->ts_end - start;

        if (console) {
            say = "Metric name: " + item->name_metric + "\nvalue: "
                  + to_string(end_to_end_delay);
            write_safely(&say);
        } else {
            //cout << "lockingQueue empty? " << lockingQueue->empty() << endl;
            if (verbose) {
                say = "opening file csv...";
                write_safely(&say);
            }

            //config_file_common.open(name_path_csv, ios::app);
            //config_file_common << to_string(item.num) + "," + to_string(end_to_end_delay) + "," +
            to_string(item->ts_end) +
            "," + to_string(msg_rate) + "," + to_string(payload) + "\n";
            //config_file_common.close();

        }
        c++;
        //if(finished.load(std::memory_order_relaxed)){
        //    if(lockingQueue.size()==0)
        //        break;
        //}

        //if(lockingQueue.size()==0)
        //    break;
    }
    say = "SUB> thread is closing...";
    write_safely(&say);

}

// *** the crucial function. It takes the endpoint to connect, the topic to subscribe to , path csv to write on,
// name of the experiment to save the file,
// how much is the payload and what is the message rate ***
void
subscriber(const char *endpoint_custom, char *topic, const char *path_csv, const char *name_of_experiment,
           const int *payload, const int *msg_rate) {

    // ------------------------------------- STARTING THE SUB -------------------------------------------------------------
    BlockingQueue<Item> lockingQueue; // declare The Queue, is a blocked-locking queue, inspired by Java,
    // but in this case works like a common queue. It can be crucial if we create more than one consumer because the queue is thread safe.
    // It is controlled by a lock ( see Blocking Queue code, BlockingQueue.h).

    //finished = false; // say: "not finished" to all threads ( if there are ), deprecated

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
    int64_t end_to_end_delay; // this is the end to end delay
    string say; // string "say" just to print safely

    // ----------------- BEGINNING OF WHILE LOOP TO RECEIVE MESSAGES --------------------------------------
    int64_t starting_point = zclock_time();
    while (!zsys_interrupted and ((zclock_time()-starting_point)<=TIMEOUT) and c<NUM_MEX_MAX-1) {
        // function to receive. It is blocking. It takes 1 string, 1 int64, 1 message ( that contains other payload)
        succ = zsock_recv(sub, "s8m", &topic, &c, &msg);
        // the message is null, size message incorrect
        if (msg == nullptr) {
            cout << "SUB> exit, msg null" << endl;
            break;
        }
        // timestamps of receiving
        end = zclock_time();

        //cout<<"Recv on "<< topic<<endl;
        //cout<<"message Received: No. "<< c<<endl;
        // lets menage the payload... passing message, end timestamp, counter, queue
        int a = payloadManaging(&msg, &end, &c, &lockingQueue);

        cout << "managing payload exit code: " << a << endl;
        // if the last number received is Mex-1 or received function does not return 0, stop
        if (c == (NUM_MEX_MAX - 1) or succ == -1) {
            cout << "SUB> TERMINATING ABNORMALLY" << endl;
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
    config_file.open(name_path_csv, ios::app);
    config_file << "number,value,timestamp,message rate,payload size\n";
    config_file.close();
    // printing size of the Queue
    cout << "SUB> Size of the queue: " << lockingQueue.d_queue.size() << endl;
    config_file.open(name_path_csv, ios::app); // open the csv file and try to append metrics
    // ---------------------- STARTING CONSUMER THREAD --------------------------------------

    while (true) {
        if (verbose) {
            say = "SUB> trying to pop new item...";
            write_safely(&say);
        }
        // pop Item from the queue ( it is internally already thread safe)
        item = lockingQueue.pop();
        // now, compute the difference between two nodes
        end_to_end_delay = item.ts_end - item.ts_start;
        if (verbose) {
            say = "SUB> opening file csv...";
            write_safely(&say);
        }
        // try to write metrics on csv
        config_file << to_string(item.num) + "," + to_string(end_to_end_delay) + "," +
                       to_string(item.ts_end) +
                       "," + to_string(*msg_rate) + "," + to_string(*payload) + "\n";
        // when queue is empty, exit
        if (lockingQueue.d_queue.empty())
            break;

    }
    config_file.close(); // close the csv file. We do not need to write on it anymore
    cout << "all items dequeued" << endl;
    zsock_destroy(&sub); // destroy subscriber socket ( it is mandatory)
}

static void startNewSubThread(zsock_t *pipe, void *args) {

    zsock_signal(pipe, 0); // You must call this function when you work with z-actor
    const string *argv = (string *) args; // convert char array into string
    cout << "SUB> ARGS RECEIVED: " << *argv << endl;
    char *temp = (char *) argv->c_str();
    // ---------------USING A STRANGE/DUMB METHOD TO PARSE THE ARGUMENTS ---------------
    string str = temp;
    size_t pox = str.find(',');
    int pox2 = (int) str.find('&');
    int argc = (int) strlen(reinterpret_cast<const char *>(argv->c_str()));
    string substring = str.substr(pox2, str.length() - pox2);
    string csv = str.substr(pox + 1, pox2 - 1 - pox);
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
    string temp_str = str.substr(0, pox);
    const char *string_json_path = (const char *) temp_str.c_str(); // string that indicate the path of json file
    cout << "SUB> STRING OF JSON FILE IS: " << *string_json_path << endl; // file passed
    // from the bash script or manually from terminal
    // start deserialization
    json_object *PARAM; // json object, see library JSON-C
    const char *endpoint_inproc;
    string endpoint_customized;

    //int num_of_subs = NUM_SUBS;
    PARAM = json_object_from_file(string_json_path);
    char *topic = nullptr;
    char *type_connection;
    const char *port;
    const char *ip;
    const char *output_file;
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
            if (strcmp(key, "type_test") == 0)
                type_test = value;
            if (strcmp(key, "ip") == 0)
                ip = value;
            if (strcmp(key, "port") == 0) {
                port = value;
            }
            if (strcmp(key, "metrics_output_type") == 0)
                output_file = value;
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
        cout << "string for endpoint (from json file):\t" << endpoint_customized << endl;
    } else {
        cout << "FILE JSON NOT FOUND...EXIT" << endl;

    }
    int success = synchronizationService(ip, port);
    cout << "SUB> Synchronization success" << endl;
    subscriber(reinterpret_cast<const char *>(&endpoint_customized), topic,
               path_csv, name_of_experiment, &payload, &msg_rate);
    cout << "SUB> END OF SUBSCRIBER" << endl;
}

#endif //PROJECTPUBSUBZMQ_SUB_H
