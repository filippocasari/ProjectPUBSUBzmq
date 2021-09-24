//
// Created by Filippo Casari on 04/09/21.
//

#ifndef PROJECTPUBSUBZMQ_CONFIG_H
#define PROJECTPUBSUBZMQ_CONFIG_H

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

#define PATH_CSV "./ResultsCsv_1/"
#define NUM_PRODUCERS 1
#define NUM_CONSUMERS 4

#define NUM_SUBS 1
#define ENDPOINT endpoint_tcp
#define NUM_MEX_MAX 10000
#define FOLDER_EXPERIMENT "./Results/"
#define SIGTERM_MSG "SIGTERM received.\n"
using namespace std;
//#define MSECS_MAX_WAITING 10000
//const char *endpoint_inprocess = "inproc://example";
const char *string_json_path;
char *path_csv = nullptr;

const char *type_test;
#include <czmq.h>
#include <json-c/json.h>
#include <cmath>
#include <thread>
//default endpoint
const char *endpoint_tcp = "tcp://127.0.0.1:6000";
const char *endpoint_inprocess = "inproc://example";
//const char *json_file_config;

#define ENDPOINT endpoint_tcp // it can be set by the developer
#define NUM_MEX_DEFAULT 10

using namespace std;


#endif //PROJECTPUBSUBZMQ_CONFIG_H
