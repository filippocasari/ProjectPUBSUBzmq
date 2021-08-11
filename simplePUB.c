
//  Espresso Pattern
//  This shows how to capture data using a pub-sub proxy
#include <czmq.h>
#include <json-c/json.h>
#include <math.h>

//default endpoint
const char *endpoint_tcp = "tcp://127.0.0.1:6000";
const char *endpoint_inprocess = "inproc://example";
const char *json_file_config;
zsock_t *pub; // new sock pub
#define ENDPOINT endpoint_tcp // it can be set by the developer
#define NUM_MEX_DEFAULT 10
#define SIGTERM_MSG "SIGTERM received.\n"

//thread of publisher
int
publisher_thread(const char **path) {
    //path of json file for configuration
    //json obj for deserialization
    json_object *PARAM = json_object_from_file(*path);
    int payload_size = 10; //payload, 10 default bytes
    int num_mex = NUM_MEX_DEFAULT; // maximum messages for the publisher, 10 default
    const char *topic; // name of the topic
    const char *endpoint_inproc;
    int msg_rate_sec = 1; //message rate, mex/sec
    // deserializing file

    const char *type_test;
    if (PARAM != NULL) { // file json found
        puts("PARAMETERS PUBLISHER: ");
        const char *type_connection;
        const char *port;
        const char *ip;
        const char *output_file;

        int int_value;
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
        endpoint[0]='\0';
        char *endpoint_customized = strcat(endpoint, type_connection);
        endpoint_customized = strcat(endpoint_customized, "://");

        //only for tcp, not for in process connection
        if (strcmp(type_connection, "tcp") == 0) {
            endpoint_customized = strcat(endpoint_customized, ip);
            endpoint_customized = strcat(endpoint_customized, ":");
            endpoint_customized = strcat(endpoint_customized, port);
        } else if (strcmp(type_connection, "inproc") == 0) {
            endpoint_customized = strcat(endpoint_customized, endpoint_inproc);
        }
        printf("string for endpoint (from json file): %s\t", endpoint_customized);

        pub = zsock_new_pub(endpoint_customized);

    } else {
        //default endpoint
        pub = zsock_new_pub(ENDPOINT);
    }

    int count = 0;
    puts("pub connected");
    //size_of_payload = (int) strtol(payload_size, NULL, 10);
    //max_mex = strtol(num_mex, NULL, 10);

    printf("PAYLOAD SIZE: %d\n", payload_size);
    printf("message rate: %d\n", msg_rate_sec);
    char string[14];

    while (count < num_mex && !zsys_interrupted) {
        //zmsg_t *mex_interrupt = zmsg_recv_nowait(pipe);
        //if (mex_interrupt)
        // break;
        long double milli_secs_of_sleeping = (1000.0 / msg_rate_sec);
        printf("millisecs of sleeping: %Lf\n", milli_secs_of_sleeping);
        //printf("millisecs of sleeping  (INT): %d\n", (int) milli_secs_of_sleeping);
        zclock_sleep((int) milli_secs_of_sleeping); //  Wait for x seconds

         // 12 byte for representation of timestamp in micro secs
        int64_t timestamp;
        if (strcmp(type_test, "LAN")==0){
            timestamp= zclock_time();
        }

        else if(strcmp(type_test, "LOCAL")==0){
            timestamp = zclock_usecs();
        }
            // catching timestamp
        else{
            timestamp=0000000000000;
        }


        int nDigits = floor(1 + log10(abs((int) timestamp)));


        sprintf(string, "%lld", timestamp); // fresh copy into the string
        printf("TIMESTAMP: %lld\n", timestamp);
        zmsg_t *msg = zmsg_new(); // creating new zmq message
        int rc = zmsg_pushstr(msg, string);
        assert(rc == 0);
        printf("SIZE OF RESIDUAL STRING (OF ZEROS) : %ld\n", payload_size - strlen(string));
        if (payload_size > (long) strlen(string)) {
            puts("PAYLOAD IS NOT NULL");
            char string_residual_payload[(abs(payload_size - (int)strlen(string)))];
            memset(string_residual_payload, '0', (abs(payload_size - (int)strlen(string))));
            string_residual_payload[payload_size - strlen(string)] = '\0';
            //printf("String of zeros: %s\n", string_residual_payload);

            if (zsock_send(pub, "ssss", topic, "TIMESTAMP", string, string_residual_payload) == -1) {
                puts("error to send");
                puts("packet loss");
            }

            //  Interrupted
        } else {

            //printf("String of zeros: %c\n", string_residual_payload);
            if (zsock_send(pub, "sss", &topic, "TIMESTAMP", string) == -1){
                puts("sending interrupted...");
                break;
            }

        }

        //char string_residual_payload[(payload_size - strlen(string))]; // string of zeros to complete payload sent

        zclock_log("Message No. %d", count);

        count++;
    }
    sleep(20);
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
        sleep(10);
        printf("exit code of publisher : %d", rc);
        //zactor_t *pub_actor = zactor_new(publisher_thread, cmdstring);
        //zstr_sendx (pub_actor, "BREAK", NULL);
        //puts("destroying zactor PUB");
        //zactor_destroy(&pub_actor);
    }
    zsock_destroy(&pub);
    return 0;
}