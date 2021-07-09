import sys
import json
import os
msg_rate = [1, 5, 10, 25, 50, 100]
range_payload = [10, 25, 50, 100, 200, 500, 1000]
topic = "FRIDGE"
connection_type = "tcp"
endpoint_inproc = "example"
ip = "127.0.0.1"
port = "6000"
metrics_output_type = "csv"
num_of_subs = 1
num_consumer_threads = 5
number_of_messages = 1000
num_test = 0
dir_name="./fileJson/"
if not os.path.exists(dir_name):
    os.mkdir(dir_name)
for i in range_payload:
    for j in msg_rate:
        string_name = "test_" + str(num_test)
        num_test += 1

        data = {"msg_rate_sec": j, "number_of_messages": number_of_messages, "topic": topic,
                "connection_type": connection_type, "endpoint_inproc": endpoint_inproc, "payload_size_bytes": i,
                "ip": ip,
                "port": port, "metrics_output_type": metrics_output_type,
                "experiment_name": string_name, "num_of_subs": num_of_subs,
                "num_consumer_threads": num_consumer_threads}
        try:
            with open(dir_name+string_name+".json", 'w') as outfile:
                json.dump(data, outfile, indent=2)
            print("file json created, name = " + string_name)
        except:
            print("Error, we can not write", sys.stderr)
