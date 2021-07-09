import pandas as pd
import json

import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

df = pd.read_csv("ResultsCsv/test_0.csv")
with open('fileJson/test_0.json') as f:
    data_json = json.load(f)
string_for_legend="TEST: "+str( data_json["experiment_name"])+"\nnumber of messages: " + str(data_json["number_of_messages"]) +\
"\nmessage rate: " + str(data_json["msg_rate_sec"])+"\npayload size: "+str(data_json["payload_size_bytes"])+"\n"
print(string_for_legend)

print(df.info())
print(df.describe())
sns.relplot(x="number", y="value", data=df, kind="line", ci="sd")
plt.legend([string_for_legend], loc="upper left")

plt.show()
