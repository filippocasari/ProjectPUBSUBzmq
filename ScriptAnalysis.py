import sys

import numpy as np
import pandas as pd
import scipy as scipy
from matplotlib import pyplot as plt

E_3 = 1000 # variable to specify the amount of payload
msg_rates = [250, 500, 1000, 10000] # message rate, messages/second
range_payload = [10 * E_3, 25 * E_3, 50 * E_3] # amount of payload, # bytes
# msg_rates = [250.0, 10.0, 2250.0, 2500.0, 100.0]
# range_payload = [10, 2250, 2500, 100, 200, 25000, 1000]
num_experiments = 2 # number of directory of tests
dir_base = sys.argv[1] # get the directory base
which_experiment = sys.argv[2] # local or lan
where = sys.argv[3] # get where the execution of tests was
# IF LAN is SET, We consider milliseconds, nanoseconds otherwise
lan = True

dataframe_ = pd.DataFrame() # create new pandas frame which will contain data from csv files
print("PATH CSV CHOSEN", dir_base)
millisecs_div = 1.0
if not lan:
    millsecs_div = 1000.0



# try to make it simpler: concat every csv(s)
for i in range(num_experiments):  # for each experiments
    dir_temp = dir_base + str(i)  # "ResultsPath"+"number"
    print("dir temp is : ", dir_temp)
    count = 0
    for payload in range_payload:  # for each payload, concat results
        for rate in msg_rates:  # same for each rate
            path = dir_temp + '/' + 'test_' + str(count) + '.csv'  # concat string to read the test
            print("PATH FINAL ANALYZED: ", path)
            data_frame_temp = pd.read_csv(path, error_bad_lines=False)  # open csv file
            count += 1  # increase counter
            # real concat pandas arrays
            dataframe_ = pd.DataFrame(data=pd.concat([dataframe_, data_frame_temp]), columns=data_frame_temp.columns)
            #print(dataframe_)

print(dataframe_) # print our dataframe
fig, ax = plt.subplots(1, 1, figsize=(13, 8)) # create new plot to show the delays
j = 0 # counter to get the next scrap
x_scrap = [-75, 0, 75] # array of craps, must be "x" dimension for "x" message rate

for payload in range_payload: # for each payload plot and calculate  delay
    print("PAYLOAD : ", payload)
    msg_rate_250 = dataframe_.loc[(dataframe_['message rate'] == 250) & (dataframe_['payload size'] == payload)]
    msg_rate_250_mean = msg_rate_250['value'].mean() / millisecs_div
    msg_rate_250_std = msg_rate_250['value'].std() / millisecs_div
    print("mean for msg rate = 250 is : ", msg_rate_250_mean, " with std: ", msg_rate_250_std)

    msg_rate_500 = dataframe_.loc[(dataframe_['message rate'] == 500) & (dataframe_['payload size'] == payload)]
    msg_rate_500_mean = msg_rate_500['value'].mean() / millisecs_div
    msg_rate_500_std = msg_rate_500['value'].std() / millisecs_div
    print("mean for msg rate = 500 is : ", msg_rate_500_mean, " with std: ", msg_rate_500_std)
    msg_rate_1000 = dataframe_.loc[(dataframe_['message rate'] == 1000) & (dataframe_['payload size'] == payload)]
    msg_rate_1000_mean = msg_rate_1000['value'].mean() / millisecs_div
    msg_rate_1000_std = msg_rate_1000['value'].std() / millisecs_div
    print("mean for msg rate = 1000 is : ", msg_rate_1000_mean, " with std: ", msg_rate_1000_std)

    msg_rate_10000 = dataframe_.loc[(dataframe_['message rate'] == 10000) & (dataframe_['payload size'] == payload)]
    msg_rate_10000_mean = msg_rate_10000['value'].mean() / millisecs_div
    msg_rate_10000_std = msg_rate_10000['value'].std() / millisecs_div
    print("mean for msg rate = 10000 is : ", msg_rate_10000_mean, " with std: ", msg_rate_10000_std)


    # Create lists for the plot

    # creating an array for delays (mean) and one for error (standard deviation)

    delays = [msg_rate_250_mean, msg_rate_500_mean, msg_rate_1000_mean, msg_rate_10000_mean]
    error = [msg_rate_250_std, msg_rate_500_std, msg_rate_1000_std, msg_rate_10000_std]

    msg_rates_temp = np.array(msg_rates) + x_scrap[j]
    w = 75 # width of a bar
    ax.bar(msg_rates_temp, delays, yerr=error, align='center', alpha=0.5, ecolor='black', capsize=6, width=w,
           label='payload: ' + str(payload) + ' bytes')
    j += 1  # next scrap
    ax.set_ylabel('Average of delays [milliseconds]')
    ax.set_xticks(msg_rates)
    ax.set_xticklabels(msg_rates)
    ax.set_xlabel('message rate [msg/sec]')
    ax.set_title(
        'Average of end to end delays\n with standard deviation\ntest execution: ' + which_experiment + ' on ' + where)
    ax.yaxis.grid(True)

    plt.tight_layout()
    ax.autoscale(tight=True)
    plt.ylim((-0.5, 2))
    plt.xlim((0, 1150))
    # TODO save plot
plt.legend()
# show everything
plt.show()

