
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
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>


#define NUM_CONSUMERS 1
#define ENDPOINT tcp_endpoint_default
#define NUM_MEX_MAX 10000

using namespace std;

const char *tcp_endpoint_default = "tcp://127.0.0.1:6000";
char *g_path_csv = nullptr;
bool g_verbose=false;
const char *g_type_test;
mutex cout_mutex;
mutex access_to_file_csv;
atomic<bool> g_finished;
void writeSafely(string *what_i_said){
    cout_mutex.lock();
    cout<<*what_i_said<<endl;
    cout_mutex.unlock();
}
char* getIp(){
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    /* display result */
    return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}
int pullPayload(zmsg_t **msg, const int64_t
*end, const int64_t *c, BlockingQueue<Item> *lockingQueue) {
    char *end_pointer;
    try {
        string say;
        char *frame;
        if(g_verbose){
            say = "size of msg: " +to_string(zmsg_size(*msg));
            writeSafely(&say);
        }
        frame = zmsg_popstr(*msg);
        if(strcmp(frame, "$TERM")==0){
            cout<<"Message received :"<<frame<<endl;
            cout<<"exit"<<endl;
            return 0;
        }

        if (strcmp(frame, "TIMESTAMP") == 0) {
            frame = zmsg_popstr(*msg);
            if(g_verbose){
                say ="frame: "+ string(frame);
                writeSafely(&say);
            }
            int64_t start_time = strtoll(frame, &end_pointer, 10);
            Item item(start_time, end, "end_to_end_delay", c) ;
            lockingQueue->push(item);
        }
        if (zmsg_size(*msg) == 0) {
            if(g_verbose){
                say = "NO MORE MESSAGES";
                writeSafely(&say);
            }

            return -2;
        }
        while (zmsg_size(*msg) > 0) {
            //puts("estrapolating other payload...");
            frame = zmsg_popstr(*msg);
            if(g_verbose){
                say = "size of payload (byte): "+ to_string((strlen(frame) * sizeof(char)));
                writeSafely(&say);
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

int createNewConsumers(BlockingQueue<Item> *lockingQueue, ofstream *config_file, char *string_json_path) {
    bool console = false;
    int num_consumers = NUM_CONSUMERS;
    string name_of_experiment;
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
            name_of_experiment = json_object_get_string(val);
        if (strcmp(key, "num_consumer_threads") == 0)
            num_consumers = (int) strtol(json_object_get_string(val), nullptr, 10);
        if (strcmp(key, "msg_rate_sec") == 0)
            msg_rate = (int) strtol(json_object_get_string(val), nullptr, 10);
        if (strcmp(key, "payload_size_bytes") == 0)
            payload = (int) strtol(json_object_get_string(val), nullptr, 10);
        if(strcmp(key, "number_of_messages")==0)
            number_of_messages=(int) strtol(json_object_get_string(val), nullptr, 10);
    }
    std::vector<thread> consumers; // create a vector of consumers
    consumers.reserve(num_consumers);
    string name_of_csv_file = name_of_experiment /*+ '_' + std::to_string(zclock_time()) */ + ".csv";
    printf("Num of consumer threads: %d\n", num_consumers);
    string name_g_path_csv = g_path_csv + name_of_csv_file;

    access_to_file_csv.lock();
    config_file->open(name_g_path_csv, ios::app);
    *config_file << "number,value,timestamp,message rate,payload size\n";
    config_file->close();

    access_to_file_csv.unlock();

    sleep(1);

    //int c = 1;
    for (int i = 0; i < num_consumers; i++) { //same as producers
        if(g_verbose){
            cout<<"launch consumer No. " <<i<<endl;
        }
        consumers.emplace_back(
                [ &console, &msg_rate, &payload, &name_g_path_csv, &num_consumers, &number_of_messages, &lockingQueue, &config_file]() {
                    auto name = this_thread::get_id();
                    stringstream id;
                    id <<name;
                    string say = "new consumer thread created with ID: "+id.str();
                    writeSafely(&say);
                    int64_t end_to_end_delay;
                    int c = 0;
                    Item item;
                    int number_of_iterations=(int)number_of_messages/num_consumers;
                    int64_t start;
                    while (c<number_of_iterations && !zsys_interrupted ) {

                        item=lockingQueue->pop();


                        say ="--------------THREAD No. "+id.str()+" IS WORKING--------";
                        writeSafely(&say);
                        char *end_pointer;
                         start= item.ts_start;
                        if(g_verbose){
                            say =  "end : " + to_string(item.ts_end) + " start: " + to_string(start) + "\nmanaging payload";
                            writeSafely(&say);
                        }

                        end_to_end_delay = item.ts_end - start;

                        if (console) {
                            say = "Metric name: " + item.name_metric +"\nvalue: "
                                    + to_string(end_to_end_delay);
                            writeSafely(&say);
                        } else {
                            //cout << "lockingQueue empty? " << lockingQueue->empty() << endl;
                            if(g_verbose){
                                say ="opening file csv...";
                                writeSafely(&say);
                            }
                            if(num_consumers>1)
                                access_to_file_csv.lock();

                            config_file->open(name_g_path_csv, ios::app);
                            *config_file << to_string(item.num) + "," + to_string(end_to_end_delay) + "," +
                            to_string(item.ts_end) +
                            "," + to_string(msg_rate) + "," + to_string(payload) + "\n";
                            config_file->close();
                            if(num_consumers>1)
                                access_to_file_csv.unlock();
                            say = "--------------THREAD No. " +id.str()+" FINISHED ITS JOB----------\nValue of c: "+to_string(c) ;
                            writeSafely(&say);
                        }
                        c++;
                        if(g_finished.load(std::memory_order_relaxed)){
                            if(lockingQueue->size()==0)
                                break;
                        }


                    }
                    say = "thread is closing...";
                    writeSafely(&say);
                });

        this_thread::sleep_for(chrono::milliseconds(100) );

    }
    for (auto& th : consumers)
        th.join();

    return 0;
}

void
launchSubscriber(string *endpoint_custom, char *topic, BlockingQueue<Item> *lockingQueue) {

    // -----------------------------------------------------------------------------------------------------

    // -----------------------------------------CREATING CONSUMERS----------------------------------------------------
    g_finished=false;
    //thread thread_start_consumers([]() {
    //    int succ = createNewConsumers();
    //    cout<<"consumers terminate? "<< succ<<endl;
//
    //});// create new thread to manage payload
    //--------------------------------------------------------------------------------------------------------
    //auto *sub = static_cast<zsock_t *>(args); // create new sub socket
    cout<<"topic is "<<topic<<endl;
    zsock_t *sub = zsock_new_sub(endpoint_custom->c_str(), topic);

    //long time_of_waiting = 0;
    int64_t c =0;
    //bool terminated = false;

    int64_t end;
    string metric = "end_to_end_delay";
    zmsg_t *msg = zmsg_new();
    int succ;
    bool terminated=false;
    string say;
    while(!terminated) {
        succ=zsock_recv(sub, "s8m", &topic, &c, &msg);

        if (msg == nullptr) {
            cout<<"exit, msg null"<<endl;
            break;
        }
        if (strcmp(g_type_test, "LAN") == 0)
            end = zclock_time();
        else if (strcmp(g_type_test, "LOCAL") == 0)
            end = zclock_usecs();
        else
            end = 0;
        say="message Received: No. ";
        say.append(to_string(c));
        writeSafely(&say);
        //increment_counter.lock();
        //increment_counter.unlock();
        thread managing([&msg, &end, &c, &lockingQueue, &terminated]{
            terminated = pullPayload(&msg, &end, &c, lockingQueue);
        });
        managing.join();
        //cout << "managing payload exit code: " << a << endl;
        if(c==(NUM_MEX_MAX-1) or succ==-1){
            cout<<"TERMINATING"<<endl;
            zclock_sleep(1000);
            g_finished=true;
            break;
        }
        //zmsg_destroy(&msg);
    }
    sleep(3);
    //thread_start_consumers.join();
    zsock_destroy(&sub);

    //terminate();
}


int launchSynchronizationService( const char* ip, const char* port, const char *type_connection) {
    string endpoint_sync = "tcp";
    endpoint_sync.append("://");

    //only for tcp, not for in process connection
    if(strcmp(type_connection, "inproc")==0)
        endpoint_sync.append(getIp());
    else
        endpoint_sync.append(ip);
    endpoint_sync.append( ":");
    endpoint_sync.append(to_string(atoi(port)+1));

    cout<<"Endpoint for Sync service: "<<endpoint_sync<<endl;
    zsock_t *syncservice = zsock_new_req(endpoint_sync.c_str());

    zsock_send(syncservice, "s", "INIT");
    char *string;
    zsock_recv(syncservice, "s",&string);

    zsock_destroy(&syncservice);
    return 0;


}

int main(int argc, char **argv) {
    for(int i=1; i<argc; i++){
        cout<<"ARGV["<<i<<"]: "<<argv[i]<<endl;
    }
    char *string_json_path; // string of args
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
        g_path_csv = argv[2];
        DIR *dir = opendir(g_path_csv);
        if (dir) {
            cout << "path csv already exists" << endl;
            closedir(dir);

        } else if (ENOENT == errno) {
            int a = mkdir(g_path_csv, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (a != 0)
            {
                cout<<"Error to create a directory"<<endl;
                return 3;
            }
        } else {
            cout<<"Error unknown, i could not be possible to open or create a directory for csv tests"<<endl;
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
    const char *v =  (char *) argv[3];
    if(strcmp(v, "-v") == 0)
        g_verbose=true;
    else
        g_verbose=false;

    if (PARAM != nullptr) {
        puts("READING PARAMETERS OF SUBSCRIBER... ");
        int int_value;
        char *value;

        json_object_object_foreach(PARAM, key, val) {
            value = (char *) json_object_get_string(val);
            cout<<value<<endl;
            if (json_object_is_type(val, json_type_int)) {
                int_value = (int) json_object_get_int64(val);
                if(strcmp(key, "number_of_messages")==0)
                    num_mex=int_value;
                else if (strcmp(key, "msg_rate_sec") == 0)
                    msg_rate = (int) strtol(value, nullptr, 10);
                else if (strcmp(key, "payload_size_bytes") == 0)
                    payload = (int) strtol(value, nullptr, 10);
            }
            else if (strcmp(key, "connection_type") == 0){
                type_connection = (char *) value;
                endpoint_customized = type_connection;
                endpoint_customized.append("://");
            }

            else if (strcmp(key, "g_type_test") == 0)
                g_type_test = value;
            else if (strcmp(key, "ip") == 0)
                ip = value;
            else if (strcmp(key, "port") == 0)
                port = value;
            else if (strcmp(key, "metrics_output_type") == 0)
                cout<<"deprecated output file "<<endl;
            else if (strcmp(key, "topic") == 0)
                topic = value;
            else if (strcmp(key, "endpoint_inproc") == 0)
                endpoint_inproc = value;
            else if (strcmp(key, "experiment_name") == 0)
                name_of_experiment = value;
            if(g_verbose)
                printf("\t%s: %s\n", key, value);
        }
        if (strcmp(type_connection, "tcp") == 0)
            endpoint_customized.append(ip).append(":").append(port);
        if (strcmp(type_connection, "inproc") == 0)
            endpoint_customized.append(endpoint_inproc);

    } else {
        cout<<"FILE JSON NOT FOUND...EXIT"<<endl;
        return 2;
    }
    cout<<"endpoint (final) : "<< endpoint_customized<<endl;
    int success=launchSynchronizationService( ip, port, type_connection);
    assert(success==0);
    cout<<"Syncronization success"<<endl;
    BlockingQueue<Item> lockingQueue; //initialize lockingQueue
    lockingQueue.d_queue.resize(num_mex);
    launchSubscriber(&endpoint_customized, topic, &lockingQueue);

    string name_of_csv_file = name_of_experiment /*+ '_' + std::to_string(zclock_time()) */  ;
    name_of_csv_file.append(".csv");
    string name_g_path_csv = g_path_csv ;
    name_g_path_csv.append("/" +name_of_csv_file);
    cout<<"OPENING FILE: "<<name_g_path_csv<<endl;
    ofstream config_file;
    sleep(3);
    config_file.open(name_g_path_csv, ios::app);
    config_file << "number,value,timestamp,message rate,payload size\n";
    config_file.close();
    Item item;
    int64_t end_to_end_delay;
    string say;
    cout<<"Size of the queue: "<<lockingQueue.size()<<endl;
    config_file.open(name_g_path_csv, ios::app);
    int64_t start;
    while(true){
        if(g_verbose){
            say ="trying to pop new item...";
            writeSafely(&say);
        }
        item=lockingQueue.pop();
        char *end_pointer;
        start = item.ts_start;
        end_to_end_delay = item.ts_end - start;
        //access_to_file_csv.lock();
        config_file << to_string(item.num) + "," + to_string(end_to_end_delay) + "," +
                       to_string(item.ts_end) +
                       "," + to_string( msg_rate) + "," + to_string(payload) + "\n";
        //access_to_file_csv.unlock();
        if(lockingQueue.size()==0){
            g_finished=true;
            break;
        }

    }

    config_file.close();
    cout<<"all items dequeued"<<endl;
    cout << "END OF SUBSCRIBER" << endl;
    return 0;
}

