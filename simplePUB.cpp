
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
const char *endpoint_tcp = "tcp://127.0.0.1:6000";
//const char *json_file_config;

#define ENDPOINT endpoint_tcp // it can be set by the developer
#define NUM_MEX_DEFAULT 10
#define SUBSCRIBERS_EXPECTED 7
using namespace std;
//thread of publisher
bool g_time_nano_sec=true;
char* get_ip(){
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
publisher_thread(const char **path) {
    zsock_t *pub; // new sock pub
    //path of json file for configuration
    //json obj for deserialization

    json_object *PARAM = json_object_from_file(*path);
    int payload_size = 10; //payload, 10 default bytes
    int num_mex = NUM_MEX_DEFAULT; // maximum messages for the publisher, 10 default
    const char *topic; // name of the topic
    const char *endpoint_inproc;
    int msg_rate_sec = 1; //message rate, mex/sec
    // deserializing file
    const char *type_connection;
    const char *port;
    const char *ip;
    const char *output_file;
    //int int_value;

    const char *type_test;
    if (PARAM != nullptr) { // file json found
        puts("PARAMETERS PUBLISHER: ");
        const char *value;
        // starting a new for each for the couple key, value

        json_object_object_foreach(PARAM, key, val) {
            value = json_object_get_string(val);
            //check if the value is an int
            //int_value = (int) json_object_get_int64(val);
            if (strcmp(key, "msg_rate_sec") == 0) {
                msg_rate_sec = (int) json_object_get_int64(val);
            }
            if (strcmp(key, "type_test") == 0)
                type_test = value;
            if (strcmp(key, "number_of_messages") == 0)
                num_mex = (int) json_object_get_int64(val);
            if (strcmp(key, "payload_size_bytes") == 0)
                payload_size = (int) json_object_get_int64(val);
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
            if (strcmp(key, "metric output file") == 0) {
                output_file = value;
                printf("output file found: %s\n", output_file);
            }
            if (strcmp(key, "topic") == 0) {
                topic = value;
            }
            if (strcmp(key, "endpoint_inproc") == 0) {
                endpoint_inproc = value;
            }

            //printf("key: %s, value: %s\n", key,value);

        }
        // create a new endpoint composed of the items inside the json file
        char endpoint[20];
        endpoint[0] = '\0';
        char *endpoint_customized = strcat(endpoint, type_connection);
        endpoint_customized = strcat(endpoint_customized, "://");

        //only for tcp, not for in process connection
        if (strcmp(type_connection, "tcp") == 0) {
            endpoint_customized = strcat(endpoint_customized, "169.254.157.194");
            endpoint_customized = strcat(endpoint_customized, ":");
            endpoint_customized = strcat(endpoint_customized, "5600");
        } else if (strcmp(type_connection, "inproc") == 0) {
            endpoint_customized = strcat(endpoint_customized, endpoint_inproc);
        }
        printf("string for endpoint (from json file): %s\t", endpoint_customized);
        //pub= zsock_new(ZMQ_PUB);
        //zsock_bind(pub,"%s",endpoint_customized);
        pub = zsock_new_pub(endpoint_customized);

    } else {
        //default endpoint
        pub = zsock_new_pub(ENDPOINT);
    }

    int64_t count = 0;
    puts("pub connected");
    //size_of_payload = (int) strtol(payload_size, NULL, 10);
    //max_mex = strtol(num_mex, NULL, 10);

    printf("PAYLOAD SIZE: %d\n", payload_size);
    printf("message rate: %d\n", msg_rate_sec);
    int64_t timestamp;
    std::string time_string;
    long double milli_secs_of_sleeping = (1000.0 / msg_rate_sec);
    zclock_sleep(4000);
    string endpoint_sync = "tcp";
    endpoint_sync.append("://");

    //only for tcp, not for in process connection

    endpoint_sync.append(ip);
    endpoint_sync.append(":");
    endpoint_sync.append(to_string(5601));

    auto *syncservice = zsock_new_rep(endpoint_sync.c_str());
    printf("Waiting for subscribers\n");
    int subscribers = 0;
    zclock_sleep(3000);
    cout << "PUB>Endpoint for sync service: " << endpoint_sync << endl;
    while (subscribers < SUBSCRIBERS_EXPECTED) {
        //  - wait for synchronization request
        char *stringa;
        zsock_recv(syncservice, "s", &stringa);
        free(stringa);
        //  - send synchronization reply
        zsock_send(syncservice, "s", "END");
        cout << "PUB " << subscribers << " connected" << endl;
        subscribers++;
    }
    zclock_sleep(4000);
    zsock_destroy(&syncservice);
    for (; count < num_mex; count++) {
        //zmsg_t *mex_interrupt = zmsg_recv_nowait(pipe);
        //if (mex_interrupt)
        // break;
        if (count == num_mex) {
            break;
        }
        printf("millisecs of sleeping: %d\n", (int)milli_secs_of_sleeping);
        //printf("millisecs of sleeping  (INT): %d\n", (int) milli_secs_of_sleeping);
        zclock_sleep((int) milli_secs_of_sleeping); //  Wait for x milliseconds

        if(g_time_nano_sec)
            timestamp=zclock_usecs();
        else
            timestamp=zclock_time();
        // catching timestamp

        //int nDigits = floor(1 + log10(abs((int) timestamp)));
        time_string = to_string(timestamp);
        printf("TIMESTAMP: %lld\n", timestamp);

        printf("SIZE OF RESIDUAL STRING (OF ZEROS) : %ld\n", payload_size - (long) (time_string.length()));
        string string_residual_payload;

        string_residual_payload = string(payload_size - (long) time_string.length(), '0');
        //printf("String of zeros: %s\n", string_residual_payload);
        zchunk_t *chunk = zchunk_new(string_residual_payload.c_str(),
                                     abs(payload_size - (long) (time_string.length())));
        if (zsock_send(pub, "s8sscii", topic, count, "TIMESTAMP", time_string.c_str(), chunk, num_mex, msg_rate_sec) ==
            -1) {
            puts("error to send,packet loss");
        }
        cout << "size of chunk is: " << zchunk_size(chunk) << endl;
        zchunk_destroy(&chunk);

        //  Interrupted

    //char string_residual_payload[(payload_size - strlen(string))]; // string of zeros to complete payload sent

        zclock_log("Message No. %llu", count);
    }
    zclock_sleep(2000);
    zsock_destroy(&pub);
    return 0;
}

int main(int argc, char **argv) {

    if (argc < 1) {
        printf("NO INPUT JSON FILE...EXIT\n");
        return -1;
    } else {

        const char *cmdstring;
        cmdstring = strdup(argv[1]);

        printf("INPUT FILE JSON (NAME): %s\n", cmdstring);
        int rc =publisher_thread(&cmdstring);
        printf("exit code of publisher : %d", rc);
        //zactor_t *pub_actor = zactor_new(publisher_thread, cmdstring);
        //zstr_sendx (pub_actor, "BREAK", NULL);
        //puts("destroying zactor SUB");
        //zactor_destroy(&pub_actor);

    }
    return 0;
}