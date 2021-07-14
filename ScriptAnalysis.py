import pandas as pd
import json

import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns


def barplot_err(x, y, xerr=None, yerr=None, data=None, **kwargs):
    _data = []
    for _i in data.index:

        _data_i = pd.concat([data.loc[_i:_i]] * 3, ignore_index=True, sort=False)
        _row = data.loc[_i]
        if xerr is not None:
            _data_i[x] = [_row[x] - _row[xerr], _row[x], _row[x] + _row[xerr]]
        if yerr is not None:
            _data_i[y] = [_row[y] - _row[yerr], _row[y], _row[y] + _row[yerr]]
        _data.append(_data_i)

    _data = pd.concat(_data, ignore_index=True, sort=False)

    _ax = sns.barplot(x=x, y=y, data=_data, ci='sd', **kwargs)

    return _ax


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

count = 0
fail_load_csv = False
for payload in range_payload:
    array_msg_rate_mean = []
    array_variances = []
    for i in msg_rate:

        try:
            df_temp = pd.read_csv("ResultsCsv/test_" + str(count) + ".csv")
            fail_load_csv = False
            print("Lenght of csv: ", len(df_temp.index))
            print("packets lost :", 1000-len(df_temp.index))
        except Exception as inst:
            fail_load_csv = True
            break
        with open('fileJson/test_' + str(count) + ".json") as f:
            json_data = json.load(f)
        print("With payload: ", json_data["payload_size_bytes"], " settled")
        mean = df_temp.mean()['value'] / 1000
        var = df_temp.var()['value'] / 1000
        array_msg_rate_mean.append(mean)
        array_variances.append(var)
        print("with message rate ", i, " ,mean ", df_temp.mean()['value'])
        count += 1

    data = pd.DataFrame({'msg rate': msg_rate, 'mean': array_msg_rate_mean, 'variance': array_variances})
    sns.barplot(y="mean", x="msg rate", data=data)
    plt.title('with bytes payload: ' + str(payload))
    plt.show()
        # barplot_err(y="mean", x="msg rate", yerr="variance",
        #          capsize=.2, data=data)

