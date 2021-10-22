import sys

import numpy as np
import pandas as pd
from matplotlib import pyplot as plt

number_of_messages = 5000
E_3 = 1000  # variable to specify the amount of payload
msg_rates = [200, 400, 600, 800, 1000]  # message rate, messages/second
range_payload = [64, 2000, 32000]  # amount of payload, # bytes
# msg_rates = [250.0, 10.0, 2250.0, 2500.0, 100.0]
# range_payload = [10, 2250, 2500, 100, 200, 25000, 1000]
num_experiments = 10  # number of directory of tests
num_sub = int(sys.argv[5])
dir_base = sys.argv[2]  # get the directory base
dad_path = sys.argv[1]
which_experiment = sys.argv[3]  # local or lan
where = sys.argv[4]  # get where the execution of tests was
# IF LAN is SET, We consider milliseconds, nanoseconds otherwise
lan = True
factor = 1
path_images = "/Images/image_"
dataframe_ = pd.DataFrame()  # create new pandas frame which will contain data from csv files
print("PATH CSV CHOSEN", dir_base)
millisecs_div = 1.0
if not lan:
    millsecs_div = 1000.0


def plotting_delays(x_lim, y_lim, weight_line, x_scrap, y_lim_2):
    fig, (ax1) = plt.subplots(1, 1, figsize=(15, 8))  # create new plot to show the delays
    j = 0  # counter to get the next scrap
    for PAYLOAD in range_payload:
        # creating an array for delays (mean) and one for error (standard deviation)

        delays = []
        error = []
        loss = []
        MSG_RATE = []  # for each payload plot and calculate  delay
        array_of_percent_high_delay = []
        print("\n\n******* PAYLOAD (bytes): " + str(PAYLOAD) + " *********\n\n")
        if PAYLOAD == 32000:
            continue
        for rate in msg_rates:

            array_of_values_high_variance = []

            print("\n With this payload we should have " + str(num_sub * num_experiments * number_of_messages) + "\n")

            msg_rate_ = dataframe_.loc[
                (dataframe_['message rate'] == rate) & (dataframe_['payload size'] == PAYLOAD)

                ]
            high_delay = dataframe_.loc[
                (dataframe_['message rate'] == rate) & (dataframe_['payload size'] == PAYLOAD) & (
                        (dataframe_['value']) >= 5.0)]
            array_of_percent_high_delay.append(((len(high_delay)) / (len(high_delay) + len(msg_rate_))) * 100)
            print(msg_rate_['payload size'])
            msg_rate_mean = (msg_rate_['value']).mean() / millisecs_div
            msg_rate_std = (msg_rate_['value']).std() / millisecs_div
            count = 0
            for a in msg_rate_['value']:
                if a > 1.0:
                    count += 1
                    array_of_values_high_variance.append(a)
            if len(array_of_values_high_variance) > 0:
                print(
                    f"MEAN OF VALUES >1.0 (with {rate} of message rate and {PAYLOAD} of payload : {np.array(array_of_values_high_variance).mean()}")
                print(f"COUNT OF VALUES >1.0 (with {rate} of message rate and {PAYLOAD} of payload : {count}")
            print("Len of dataframe: ", len(msg_rate_))
            print("mean for msg rate = " + str(msg_rate_) + " is : ", msg_rate_mean, " with std: ", msg_rate_std)
            print("LEN of array msg_rate  is: " + str(len(msg_rate_)), rate)
            packet_loss_ = 100.0 - ((len(msg_rate_) + len(high_delay)) / (
                    number_of_messages * num_experiments * num_sub)) * 100.0
            print("Packet loss (%): ", packet_loss_)
            delays.append(msg_rate_mean)
            error.append(msg_rate_std)
            loss.append(packet_loss_)
            MSG_RATE.append(rate * factor)
        msg_rates_temp = np.array(MSG_RATE) + x_scrap[j]
        # ax1.set_yscale('log')
        ax1.bar(msg_rates_temp, delays, yerr=error, align='center', alpha=0.5, ecolor='black', capsize=6,
                width=weight_line,
                label='payload: ' + str(PAYLOAD) + ' bytes')
        j += 1  # next scrap
        ax1.set_ylabel('Average of delays [milliseconds]')
        ax1.set_xticks(MSG_RATE)
        ax1.set_xticklabels(MSG_RATE)
        ax1.set_xlabel('Message rate [msg/sec]')

        ax1.set_title(
            'Average of end to end delays\n '
            'with standard deviation\ntest execution: ' + which_experiment + ' on ' + where + "\nwith "
            + str(num_sub) + " SUB")
        ax1.yaxis.grid(True)

        plt.tight_layout()
        ax1.autoscale(tight=True)
        ax1.set_ylim(y_lim)
        ax1.set_xlim(x_lim)
        # TODO save plot

        # ax2.bar(msg_rates_temp, array_of_percent_high_delay, align='center', alpha=0.5, capsize=6, width=weight_line,
        #        label='payload: ' + str(PAYLOAD) + ' bytes')
        # ax2.set_ylim(y_lim_2)

        # ax2.set_ylabel('Message with delay > 5.0 ms (%)')
        # ax2.set_xticks(MSG_RATE)
        # ax2.set_xticklabels(MSG_RATE)
        # ax2.set_xlabel('message rate [msg/sec]')
        # ax2.set_title(
        #    'Message with delay too high\n '
        #    + which_experiment + ' on ' + where)
        # ax2.yaxis.grid(True)
        # ax2.set_xlim(x_lim)

    plt.legend()
    # show everything
    plt.show()


def plotting_throughput(x_lim, y_lim, weight_line):
    fig, (ax1) = plt.subplots(1, 1, figsize=(15, 8))  # create new plot to show the delays

    array_throughput = []
    delays_throughput = []
    error_throughput = []

    for PAYLOAD in range_payload:
        for MSG_RATE in msg_rates:
            array_throughput.append((MSG_RATE * PAYLOAD * 8) / (1000.0 * 1000.0))
            msg_rate_ = dataframe_.loc[
                (dataframe_['message rate'] == MSG_RATE) & (dataframe_['payload size'] == PAYLOAD)]
            # & ((dataframe_['value']) < 5.0)
            mean = (msg_rate_['value']).mean()
            std = (msg_rate_['value']).std()
            delays_throughput.append(mean)
            error_throughput.append(std)

    from random import randint
    colors = []
    weights = []

    from_ = 0
    to = 7

    array_throughput[0] = 1.0
    delays_throughput[0] = np.array(delays_throughput[0:5]).mean()
    error_throughput[0] = np.array(error_throughput[0:5]).mean()
    del array_throughput[1:5]
    del delays_throughput[1:5]
    del error_throughput[1:5]
    for i in range(12):
        peso = 0.3
        weights.append(peso)
        colors.append('#%06X' % randint(0, 0xFFFFFF))
   # ax1.set_yscale('log')
    thr = ax1.bar(np.array((range(len(array_throughput[from_:to])))), np.array(delays_throughput[from_:to]),
                  yerr=np.array(error_throughput[from_:to]), ecolor='black', capsize=6,
                  width=weights[from_:to], color=colors[from_:to])

    print(np.array(array_throughput))
    print(np.array(delays_throughput))
    print(np.array(error_throughput))
    ax1.set_ylabel('Average of delays [milliseconds]')

    ax1.set_xlabel('Throughput [Mbps]')
    ax1.set_title(
        'Average of end to end delays ' +
        'with standard deviation\ntest execution: ' + which_experiment + ' on ' + where + "\nwith "
        + str(num_sub) + " SUB")
    ax1.yaxis.grid(True)

    ax1.autoscale(tight=True)

    labels = ['1.0', '3.2', '6.4', '9.6', '12.8', '16',
              '51.2', '102.4', '153.6', '204.8',
              '256']
    plt.xticks(range(len(array_throughput[from_:to])),
               labels[from_: to])
    plt.ylim(y_lim)
    # plt.xlim(x_lim)

    # show everything

    plt.legend(thr, array_throughput, loc='upper left')
    plt.setp(ax1.get_xticklabels(), rotation=45, horizontalalignment='right')
    # ax1.set_xlim(xmax=x_lim[1], xmin=x_lim[0])
    # ax1.set_xbound( upper=24)
    plt.tight_layout()
    plt.show()


# try to make it simpler: concat every csv(s)
for j in range(num_experiments):
    for i in range(num_sub):  # for each experiments
        dir_temp = dad_path + str(j) + dir_base + str(i)  # "ResultsPath"+"number"
        print("dir temp is : ", dir_temp)
        count = 0
        for payload in range_payload:  # for each payload, concat results
            for rate in msg_rates:  # same for each rate
                path = dir_temp + '/' + 'test_' + str(count) + '.csv'  # concat string to read the test
                print("PATH FINAL ANALYZED: ", path)
                count += 1  # increase counter
                try:
                    data_frame_temp = pd.read_csv(path, error_bad_lines=False)  # open csv file
                    # data_frame_temp = data_frame_temp.iloc[1:, :]
                except:
                    continue

                # real concat pandas arrays
                dataframe_ = pd.DataFrame(data=pd.concat([dataframe_, data_frame_temp]),
                                          columns=data_frame_temp.columns)
                # print(dataframe_)

print(dataframe_)  # print our dataframe

# x_scrap = [-120 * factor,-60*factor, 0, 60 * factor, 120*factor]  # array of craps, must be "x" dimension for "x" message rate
w = 20 * factor  # width of a bar
# plotting_delays((0, 1200 * factor), (-0.1, 1), w, x_scrap, (0, 100))
plotting_throughput((-1, 25.0 * factor), (-0.1, 10), w)
# plotting_delays((-1, 25.0 * factor), (-0.1, 1.0), w)
# x_scrap = [-200 * factor, 0, 200 * factor]
# w = 190 * factor
# plotting_delays_2((1500, 11000 * factor), (-1, 2), w, x_scrap, (0, 10))

x_scrap = [-75, 0, 75]  # array of craps, must be "x" dimension for "x" message rate
# w = 75  # width of a bar
plotting_delays((0, 1200), (-0.1, 8), w, x_scrap, (0, 15))
# x_scrap = [-200, 0, 200]
# w = 110
# plotting_delays((-200, 11000), (-50, 200), w, x_scrap, (0, 100))
