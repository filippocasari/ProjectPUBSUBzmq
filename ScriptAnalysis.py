import pandas as pd
import json

import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns


def change_in_millsec(i):
    i = i / 1000
    return i


df = pd.read_csv("ResultsCsv/test_1.csv")
with open('fileJson/test_1.json') as f:
    data_json = json.load(f)
string_for_legend = "TEST: " + str(data_json["experiment_name"]) + "\nnumber of messages: " + str(
    data_json["number_of_messages"]) + \
                    "\nmessage rate: " + str(data_json["msg_rate_sec"]) + "\npayload size: " + str(
    data_json["payload_size_bytes"]) + "\n"
print(string_for_legend)

print(df.info())
print(df.describe())
# sns.relplot(x="number", y="value", data=df, kind="line", ci="sd")
# plt.legend([string_for_legend], loc="upper left")
# plt.show()
df['value'] = df['value'].apply(change_in_millsec)
variance = df.var()['value']
sns.relplot(x="timestamp", y="value", data=df, kind="line", ci="sd", hue=variance)
plt.legend([string_for_legend], loc="upper left")
plt.show()
msg_rate = [1, 5, 10, 25, 50, 100]
range_payload = [10, 25, 50, 100, 200, 500, 1000]
array_msg_rate_mean = []
array_variances=[]
count = 0
fail_load_csv = False
for payload in range_payload:

    for i in msg_rate:

        try:
            df_temp = pd.read_csv("ResultsCsv/test_" + str(count) + ".csv")
            fail_load_csv = False
        except:
            fail_load_csv = True
            break
        with open('fileJson/test_' + str(count) + ".json") as f:
            json_data = json.load(f)
        print("With payload: ", json_data["payload_size_bytes"], " settled")
        mean = df_temp.mean()['value']/1000
        var=df_temp.var()['value']/1000
        array_msg_rate_mean.append(mean)
        array_variances.append(var)
        print("with message rate ", i, " ,mean ", df_temp.mean()['value'])
        count += 1
    if fail_load_csv == False:
        data=pd.DataFrame({'msg rate':msg_rate, 'mean':array_msg_rate_mean, 'variance':array_variances})
        sns.catplot(data=data, x='msg rate', y='mean', kind="bar", hue="variance")

        plt.show()
