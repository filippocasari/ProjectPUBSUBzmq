//
// Created by Filippo Casari on 03/09/21.
//

#ifndef PROJECTPUBSUBZMQ_PUB_H
#define PROJECTPUBSUBZMQ_PUB_H



//  Espresso Pattern
//  This shows how to capture data using a pub-sub proxy
#include <czmq.h>
#include <json-c/json.h>
#include <cmath>
#include <thread>
#include <iostream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
//default endpoint
const char *endpoint_inprocess = "inproc://example";
//const char *json_file_config;
#define SUBSCRIBERS_EXPECTED 7
#define ENDPOINT endpoint_tcp // it can be set by the developer
#define NUM_MEX_DEFAULT 10
bool verbose = true;
using namespace std;
//thread of publisher
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

int
publisher(const char *path) {
    char *endpoint_customized;
    zsock_t *pub; // new sock pub
    //path of json file for configuration
    //json obj for deserialization

    json_object *PARAM = json_object_from_file(path);
    int payload_size = 10; //payload, 10 default bytes
    int num_mex = NUM_MEX_DEFAULT; // maximum messages for the publisher, 10 default
    const char *topic; // name of the topic
    const char *endpoint_inproc;
    int msg_rate_sec = 1; //message rate, mex/sec
    // deserializing file

    const char *type_test;
    const char *type_connection;
    const char *port;
    const char *ip;
    const char *output_file;
    if (PARAM != nullptr) { // file json found
        if(verbose)
            puts("PUB> PARAMETERS PUBLISHER: ");
        //int int_value;
        const char *value;
        // starting a new for each for the couple key, value

        json_object_object_foreach(PARAM, key, val) {
            value = json_object_get_string(val);
            //check if the value is an int
            //int_value = (int) json_object_get_int64(val);
            if (strcmp(key, "msg_rate_sec") == 0)
                msg_rate_sec = (int) json_object_get_int64(val);
            if (strcmp(key, "type_test") == 0)
                type_test = value;
            if (strcmp(key, "number_of_messages") == 0)
                num_mex = (int) json_object_get_int64(val);
            if (strcmp(key, "payload_size_bytes") == 0)
                payload_size = (int) json_object_get_int64(val);
            if (strcmp(key, "connection_type") == 0)
                type_connection = value;
            if (strcmp(key, "ip") == 0)
                ip = value;
            if (strcmp(key, "port") == 0)
                port = value;
            if (strcmp(key, "topic") == 0)
                topic = value;

            if (strcmp(key, "endpoint_inproc") == 0)
                endpoint_inproc = value;
            if(strcmp(key, "number_of_messages")==0)
                num_mex=(int) strtol(json_object_get_string(val), nullptr, 10);
            // printing...
            if(verbose){
                cout<<key<<" : "<<value<<endl;
            }

        }

        // create a new endpoint composed of the items inside the json file
        string endpoint=type_connection;
        endpoint.append("://");

        //only for tcp, not for in process connection
        if (strcmp(type_connection, "tcp") == 0) {
            endpoint.append(ip);
            endpoint.append(":");
            endpoint.append(port);
        }else{
            endpoint.append(topic);
        }
        if(verbose)
            cout<<"PUB> string for endpoint (from json file): "<< endpoint<<endl;

        pub = zsock_new_pub(endpoint.c_str());

    } else {
        puts("PUB> error");
        return 2;
    }
    int64_t count = 0;
    puts("pub connected");
    //size_of_payload = (int) strtol(payload_size, NULL, 10);
    //max_mex = strtol(num_mex, NULL, 10);
    if(verbose){
        printf("PUB> PAYLOAD SIZE: %d\n", payload_size);
        printf("PUB> message rate: %d\n", msg_rate_sec);
    }

    int64_t timestamp;
    std::string time_string;
    long double milli_secs_of_sleeping = (1000.0 / msg_rate_sec);
    zclock_sleep(4000);

    string endpoint_sync = "tcp";
    endpoint_sync.append("://");

    //only for tcp, not for in process connection

    endpoint_sync.append( "0.0.0.0");
    endpoint_sync.append( ":");
    endpoint_sync.append(to_string(atoi(port)+1));

    auto *syncservice = zsock_new_rep(endpoint_sync.c_str());
    printf ("PUB> Waiting for subscribers\n");
    int subscribers = 0;

    cout<<"PUB> Endpoint for sync service: "<<endpoint_sync<<endl;
    while (subscribers < SUBSCRIBERS_EXPECTED) {
        //  - wait for synchronization request
        char *stringa;
        zsock_recv(syncservice, "s",&stringa );
        //  - send synchronization reply
        zsock_send(syncservice, "s", "END");
        subscribers++;
        cout<<"SUB "<<subscribers<<" SYNCHRONIZED"<<endl;
    }
    sleep(1);
    zsock_destroy(&syncservice);
    sleep(10);
    for(;count<num_mex; count++){

        if(verbose)
            printf("millisecs of sleeping: %Lf\n", milli_secs_of_sleeping);
        //printf("millisecs of sleeping  (INT): %d\n", (int) milli_secs_of_sleeping);
        zclock_sleep((int) milli_secs_of_sleeping); //  Wait for x milliseconds

        if (strcmp(type_test, "LAN")==0)
        {
            timestamp= zclock_time();
        }
        else if(strcmp(type_test, "LOCAL")==0){
            timestamp = zclock_usecs();
        }
        // catching timestamp
        else{
            timestamp=0;
        }
        time_string =to_string(timestamp);
        if(verbose)
            printf("TIMESTAMP: %lld\n", timestamp);
        zmsg_t *msg = zmsg_new(); // creating new zmq message
        int rc = zmsg_pushstr(msg, time_string.c_str());
        assert(rc == 0);
        if(verbose)
            printf("SIZE OF RESIDUAL STRING (OF ZEROS) : %ld\n", payload_size - (long)(time_string.length()));
        std::string string_residual_payload;
        if (payload_size > (long) time_string.length()) {

            string_residual_payload = string(payload_size-(long) time_string.length(), '0');
            //printf("String of zeros: %s\n", string_residual_payload);
            zchunk_t *chunk= zchunk_new(string_residual_payload.c_str(),abs(payload_size - (long)(time_string.length())) );
            if (zsock_send(pub, "s8ssc", topic,count, "TIMESTAMP", time_string.c_str(), chunk) == -1) {
                puts("error to send,packet loss");
            }

            zchunk_destroy(&chunk);
            zmsg_destroy(&msg);
            //  Interrupted
        } else {

            //printf("String of zeros: %c\n", string_residual_payload);
            if (zsock_bsend(pub, "sss", &topic, "TIMESTAMP", time_string.c_str()) == -1){
                puts("sending interrupted...");
                return 1;
            }

        }
        if(verbose)
            zclock_log("Message No. %llu", count);
    }
    zclock_sleep(4000);
    zsock_send(pub, "s1s", &topic, -1, "TERMINATE");
    zclock_sleep(2000);
    //zsock_disconnect(pub, "%s", endpoint_customized);
    zsock_destroy(&pub);
    return 0;
}

static void startPubThread(zsock_t *pipe, void *args) {
    zsock_signal(pipe, 0);
    char **args_new = (char**)args;
    const char *file_json=strdup((char*)args_new[1]);
    const char *v =  strdup( (char*)args_new[3]);
    if(strcmp(v, "-v") == 0)
        verbose=true;
    else
        verbose=false;
    printf("INPUT FILE JSON (NAME): %s\n", file_json);
    int rc =publisher(file_json);
    cout<<"exit code of publisher : "<< rc<<endl;
    //zactor_t *pub_actor = zactor_new(publisher, file_json);
    //zstr_sendx (pub_actor, "BREAK", NULL);
    //puts("destroying zactor PUB");
    //zactor_destroy(&pub_actor);
}
#endif //PROJECTPUBSUBZMQ_PUB_H
