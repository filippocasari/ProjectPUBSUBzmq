
//  Espresso Pattern
//  This shows how to capture data using a pub-sub proxy
#include <czmq.h>
#include <json-c/json.h>
#include <math.h>

//default endpoint
const char *endpoint_tcp = "tcp://127.0.0.1:6000";
const char *endpoint_inprocess = "inproc://example";
const char *json_file_config;
#define ENDPOINT endpoint_tcp // it can be set by the developer
#define NUM_MEX_DEFAULT 10
#define SIGTERM_MSG "SIGTERM received.\n"

void sig_term_handler(int signum, siginfo_t *info, void *ptr) {
    write(STDERR_FILENO, SIGTERM_MSG, sizeof(SIGTERM_MSG));
}

void catch_sigterm() {
    static struct sigaction _sigact;

    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = sig_term_handler;
    _sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &_sigact, NULL);
}

//thread of publisher
void
publisher_thread(const char *path) {
    //path of json file for configuration


    //json obj for deserialization
    json_object *PARAM = json_object_from_file(path);
    int payload_size = 10; //payload, 10 default bytes
    int num_mex = NUM_MEX_DEFAULT; // maximum messages for the publisher, 10 default
    const char *topic; // name of the topic
    const char *endpoint_inproc;
    int msg_rate_sec = 1; //message rate, mex/sec
    // deserializing file
    zsock_t *pub; // new sock pub
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
    while (!zctx_interrupted && count < num_mex) {
        //zmsg_t *mex_interrupt = zmsg_recv_nowait(pipe);
        //if (mex_interrupt)
        // break;
        long double milli_secs_of_sleeping = (1000.0 / msg_rate_sec);
        printf("millisecs of sleeping: %Lf\n", milli_secs_of_sleeping);
        //printf("millisecs of sleeping  (INT): %d\n", (int) milli_secs_of_sleeping);
        zclock_sleep((int) milli_secs_of_sleeping); //  Wait for x seconds

        char *string; // 12 byte for representation of timestamp in micro secs
        long timestamp = zclock_usecs(); // catching timestamp
        int nDigits = floor(1 + log10(abs((int) timestamp)));
        string = (char *) malloc((nDigits + 1) * sizeof(char));
        sprintf(string, "%ld", timestamp); // fresh copy into the string
        printf("TIMESTAMP: %ld\n", timestamp);
        zmsg_t *msg = zmsg_new(); // creating new zmq message
        int rc = zmsg_pushstr(msg, string);
        assert(rc == 0);
        printf("SIZE OF RESIDUAL STRING (OF ZEROS) : %ld\n", payload_size - strlen(string));
        if (payload_size > (long) strlen(string)) {
            puts("PAYLOAD IS NOT NULL");
            char string_residual_payload[(payload_size - strlen(string))];
            for (int i = 0; i < (payload_size - strlen(string)); i++) {
                string_residual_payload[i] = '0';
            }
            string_residual_payload[payload_size - strlen(string)] = '\0';
            //printf("String of zeros: %s\n", string_residual_payload);

            if (zsock_send(pub, "ssss", topic, "TIMESTAMP", string, string_residual_payload) == -1)
                break;              //  Interrupted
        } else {
            char string_residual_payload = '\0';
            //printf("String of zeros: %c\n", string_residual_payload);
            if (zsock_send(pub, "sss", topic, "TIMESTAMP", string) == -1)
                break;
        }
        //char string_residual_payload[(payload_size - strlen(string))]; // string of zeros to complete payload sent

        zclock_log("Message No. %d", count);
        free(string);
        count++;
    }
    zsock_destroy(&pub);
}

int main(int argc, char *argv[]) {
    catch_sigterm();
    if (argc < 1) {
        printf("NO INPUT JSON FILE...EXIT\n");
        return -1;
    } else {
        int i;
        size_t str_size = 0;
        for (i = 1; i < argc; i++) {
            str_size += strlen(argv[i]);
            if (argc > i + 1)
                str_size++;
        }
        str_size = (int) str_size;
        char *cmdstring;
        cmdstring = (char *) malloc(str_size * sizeof(char));
        cmdstring[0] = '\0';

        for (i = 1; i < argc; i++) {
            strcat(cmdstring, argv[i]);
            if (argc > i + 1)
                strcat(cmdstring, " ");
        }
        printf("INPUT FILE JSON (NAME): %s\n", cmdstring);
        publisher_thread(cmdstring);
        //zactor_t *pub_actor = zactor_new(publisher_thread, cmdstring);
        free(cmdstring);
        //zstr_sendx (pub_actor, "BREAK", NULL);
        //puts("destroying zactor PUB");
        //zactor_destroy(&pub_actor);
    }

    return 0;
}