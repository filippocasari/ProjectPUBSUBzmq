import numpy as np
import pandas as pd
from matplotlib import pyplot as plt

msg_rates = [5.0, 10.0, 25.0, 50.0, 100.0]
range_payload = [10, 25, 50, 100, 200, 500, 1000]
num_experiments = 2
dir_base = "./ResultsCsv_"

dataframe_ = pd.DataFrame()
for i in range(num_experiments):
    dir_temp = dir_base + str(i)
    count = 0
    for payload in range_payload:
        for rate in msg_rates:
            data_frame_temp = pd.read_csv(dir_temp + "/" + "test_" + str(count) + ".csv")
            count += 1
            dataframe_ = pd.DataFrame(data=pd.concat([dataframe_, data_frame_temp]), columns=data_frame_temp.columns)

print(dataframe_)
fig, ax = plt.subplots(1, 1, figsize=(13, 8))
j = 0
x_scarti = [-1, 0.0, 1, 2]
range_payloads = [10, 50, 200, 1000]
for payload in range_payloads:
    msg_rate_5 = dataframe_.loc[(dataframe_['message rate'] == 5) & (dataframe_['payload size'] == payload)]
    msg_rate_5_mean = msg_rate_5['value'].mean()
    msg_rate_5_std = msg_rate_5['value'].std()
    print("mean for msg rate = 5 is : ", msg_rate_5_mean, " with std: ", msg_rate_5_std)

    msg_rate_10 = dataframe_.loc[(dataframe_['message rate'] == 10) & (dataframe_['payload size'] == payload)]
    msg_rate_10_mean = msg_rate_10['value'].mean()
    msg_rate_10_std = msg_rate_10['value'].std()
    print("mean for msg rate = 10 is : ", msg_rate_10_mean, " with std: ", msg_rate_10_std)
    msg_rate_25 = dataframe_.loc[(dataframe_['message rate'] == 25) & (dataframe_['payload size'] == payload)]
    msg_rate_25_mean = msg_rate_25['value'].mean()
    msg_rate_25_std = msg_rate_25['value'].std()
    print("mean for msg rate = 25 is : ", msg_rate_25_mean, " with std: ", msg_rate_25_std)

    msg_rate_50 = dataframe_.loc[(dataframe_['message rate'] == 50) & (dataframe_['payload size'] == payload)]
    msg_rate_50_mean = msg_rate_50['value'].mean()
    msg_rate_50_std = msg_rate_50['value'].std()
    print("mean for msg rate = 50 is : ", msg_rate_50_mean, " with std: ", msg_rate_50_std)

    msg_rate_100 = dataframe_.loc[(dataframe_['message rate'] == 100) & (dataframe_['payload size'] == payload)]
    msg_rate_100_mean = msg_rate_100['value'].mean()
    msg_rate_100_std = msg_rate_100['value'].std()
    print("mean for msg rate = 100 is : ", msg_rate_100_mean, " with std: ", msg_rate_100_std)
    # print(msg_rate_5)

    # Create lists for the plot

    delays = [msg_rate_5_mean, msg_rate_10_mean, msg_rate_25_mean, msg_rate_50_mean, msg_rate_100_mean]
    error = [msg_rate_5_std, msg_rate_10_std, msg_rate_25_std, msg_rate_50_std, msg_rate_100_std]

    msg_rates_temp = np.array(msg_rates) + x_scarti[j]
    ax.bar(msg_rates_temp, delays, yerr=error, align='center', alpha=0.5, ecolor='black', capsize=6, width=1)
    j += 1
    ax.set_ylabel('mean of delay [microseconds]')
    ax.set_xticks(msg_rates)
    ax.set_xticklabels(msg_rates)
    ax.set_xlabel('message rate [msg/sec]')
    ax.set_title('Mean of delays with standard deviation')
    ax.yaxis.grid(True)

    plt.tight_layout()
    ax.autoscale(tight=True)
    plt.xlim((-3, 120))
    # Save the figure and show
plt.legend()
plt.show()
