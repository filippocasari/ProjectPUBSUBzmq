import sys
import json
import os

# arguments passed:
# 1) path where create json files
# 2) if tests will be on lan

E_3 = 1000  # number * E_3
# msg_rate = [5, 10, 25, 50, 100]
msg_rate = [250, 500, 1000, 10000]  # message rate, unit: messages/sec
# range_payload = [10, 25, 50, 100, 200, 500, 1000]
range_payload = [10 * E_3, 25 * E_3, 50 * E_3]  # payload (bytes)
topic = "CAR"  # topic for the publisher and subscribers
connection_type = "tcp"  # can be tcp or inproc
endpoint_inproc = "CAR"  # where inproc is set, this is its endpoint
ip = "127.0.0.1"  # ip of PUB and SUB (if local)
port = "6000"  # port
metrics_output_type = "csv"  # can be csv or console (=only stdout)
num_of_subs = 1  # number of subs, can not be more than one anymore (deprecated). It used zactor to create more thread
# as subs
num_consumer_threads = 1  # number of threads who want to eat items of linked-blocking queue
number_of_messages = 10000  # number of messages that PUB must send

def create_dir(path):
    num_test = 0  # just a counter, increase during the program
    for i in range_payload:
        for j in msg_rate:
            string_name = "test_" + str(num_test)
            num_test += 1
            data = {"msg_rate_sec": j, "number_of_messages": number_of_messages, "topic": topic,
                    "connection_type": connection_type, "endpoint_inproc": endpoint_inproc, "payload_size_bytes": i,
                    "ip": ip,
                    "port": port, "metrics_output_type": metrics_output_type,
                    "experiment_name": string_name, "num_of_subs": num_of_subs,
                    "type_test": type_test,
                    "num_consumer_threads": num_consumer_threads}
            try:
                with open(path + string_name + ".json", 'w') as outfile:
                    json.dump(data, outfile, indent=2)
                print("json file created, name = " + string_name)
            except:
                print("Error, we can not write/open the file " + string_name, sys.stderr)

if (sys.argv[2].upper() == "LAN"):
    is_on_LAN = True  # if tests are on LAN, path and configurations are different
else:
    is_on_LAN = False
print("IS on LAN: ", str(is_on_LAN))
try:
    dir_name = sys.argv[1]  # get first argv
except:
    print("error, no input string")
    exit(1)
type_test = "LAN"
cwd = os.getcwd()
string_temp = ""
print("We are in ", os.getcwd())
if not os.path.exists(str(cwd + dir_name)):  # if path does not exist yet
    os.chdir(cwd)
    print("We are in ", os.getcwd())
    string_path = dir_name.split("/")
    string_path = string_path[1:]
    print(string_path)

    for i in (string_path):
        print("creating path ", cwd + i)
        try:
            string_temp += "/" + i
            os.mkdir(cwd + string_temp)

        except:
            print("this path already exists")
    if is_on_LAN:

        path_of_PUB = cwd + string_temp + "/" + "PUB"
        print("PATH PUB: ", path_of_PUB)
        path_of_SUB = cwd + string_temp + "/" + "SUB"
        print("PATH SUB: ", path_of_SUB)
        os.mkdir(path_of_PUB)
        os.mkdir(path_of_SUB)

        print("###################### STARTING TO CREATE NEW CONFIGURATIONS ####################################\n\
        ################################################################################################")


        try:
            create_dir(str(path_of_PUB+"/"))
            ip=sys.argv[3] # ip of the publisher
            create_dir(str(path_of_SUB+"/"))
        except:
            print(path_of_PUB + " does not exist")
    else:
        create_dir(cwd+dir_name)











