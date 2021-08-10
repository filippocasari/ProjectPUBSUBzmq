import sys
import json
import os

E_3 = 1000
# msg_rate = [5, 10, 25, 50, 100]
msg_rate = [250, 500, 1000, 10000]
# range_payload = [10, 25, 50, 100, 200, 500, 1000]
range_payload = [10 * E_3, 25 * E_3, 50 * E_3]
topic = "CAR"
connection_type = "tcp"
endpoint_inproc = "example"
ip = "127.0.0.1"
port = "6000"
metrics_output_type = "csv"
num_of_subs = 1
num_consumer_threads = 4
number_of_messages = 10000
num_test = 0
dir_name = "./fileJson/"
type_test="LAN"
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
                "type_test": "LAN",
                "num_consumer_threads": num_consumer_threads}
        try:
            with open(dir_name + string_name + ".json", 'w') as outfile:
                json.dump(data, outfile, indent=2)
            print("file json created, name = " + string_name)
        except:
            print("Error, we can not write", sys.stderr)
