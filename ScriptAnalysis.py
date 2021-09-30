
import sys


import numpy as np
import pandas as pd
from matplotlib import pyplot as plt

number_of_messages = 10000
E_3 = 1000  # variable to specify the amount of payload
msg_rates = [250, 500, 1000, 5000, 10000]  # message rate, messages/second
range_payload = [10 * E_3, 25 * E_3, 50 * E_3]  # amount of payload, # bytes
# msg_rates = [250.0, 10.0, 2250.0, 2500.0, 100.0]
# range_payload = [10, 2250, 2500, 100, 200, 25000, 1000]
num_experiments = 3  # number of directory of tests
num_sub = 3
dir_base = sys.argv[2]  # get the directory base
dad_path = sys.argv[1]
which_experiment = sys.argv[3]  # local or lan
where = sys.argv[4]  # get where the execution of tests was
# IF LAN is SET, We consider milliseconds, nanoseconds otherwise
lan = True
factor=1
path_images = "/Images/image_"
dataframe_ = pd.DataFrame()  # create new pandas frame which will contain data from csv files
print("PATH CSV CHOSEN", dir_base)
millisecs_div = 1.0
if not lan:
    millsecs_div = 1000.0


def plotting_delays(xlim, ylim, w, x_scrap, ylim_2):
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 8))  # create new plot to show the delays
    j = 0  # counter to get the next scrap

    for payload in range_payload:  # for each payload plot and calculate  delay
        print("\n\n******* PAYLOAD (bytes): " + str(payload) + " *********\n\n")
        print("\n With this payload we should have " + str(num_sub * num_experiments * number_of_messages) + "\n")

        msg_rate_250 = dataframe_.loc[(dataframe_['message rate'] == 250) & (dataframe_['payload size'] == payload)]
        msg_rate_250_mean = msg_rate_250['value'].mean() / millisecs_div
        msg_rate_250_std = msg_rate_250['value'].std() / millisecs_div
        print("mean for msg rate = 250 is : ", msg_rate_250_mean, " with std: ", msg_rate_250_std)
        print("LEN of array msg_rate 250 is: " + str(len(msg_rate_250)))
        packet_loss_250 = 100.0 - ((len(msg_rate_250)) / (number_of_messages * num_experiments * num_sub)) * 100.0
        print("Packet loss (%): ", packet_loss_250)

        msg_rate_500 = dataframe_.loc[(dataframe_['message rate'] == 500) & (dataframe_['payload size'] == payload)]
        msg_rate_500_mean = msg_rate_500['value'].mean() / millisecs_div
        msg_rate_500_std = msg_rate_500['value'].std() / millisecs_div
        print("mean for msg rate = 500 is : ", msg_rate_500_mean, " with std: ", msg_rate_500_std)
        packet_loss_500 = 100.0 - (
                ((len(msg_rate_500)) * 100.0) / (
                number_of_messages * num_experiments * num_sub))
        print("LEN of array msg_rate 500 is: " + str(len(msg_rate_500)))
        print("Packet loss (%): ", packet_loss_500)

        msg_rate_1000 = dataframe_.loc[(dataframe_['message rate'] == 1000) & (dataframe_['payload size'] == payload)]
        msg_rate_1000_mean = msg_rate_1000['value'].mean() / millisecs_div
        msg_rate_1000_std = msg_rate_1000['value'].std() / millisecs_div
        print("mean for msg rate = 1000 is : ", msg_rate_1000_mean, " with std: ", msg_rate_1000_std)
        packet_loss_1000 = 100.0 - (
                ((len(msg_rate_1000)) * 100.0) / (
                number_of_messages * num_experiments * num_sub))
        print("LEN of array msg_rate 1000 is: " + str(len(msg_rate_1000)))
        print("Packet loss (%): ", packet_loss_1000)

        msg_rate_5000 = dataframe_.loc[(dataframe_['message rate'] == 5000) & (dataframe_['payload size'] == payload)]
        msg_rate_5000_mean = msg_rate_5000['value'].mean() / millisecs_div
        msg_rate_5000_std = msg_rate_5000['value'].std() / millisecs_div
        print("mean for msg rate = 5000 is : ", msg_rate_5000_mean, " with std: ", msg_rate_5000_std)
        packet_loss_5000 = 100.0 - (
                ((len(msg_rate_5000)) * 100.0) / (
                number_of_messages * num_experiments * num_sub))
        print("LEN of array msg_rate 5000 is: " + str(len(msg_rate_5000)))
        print("Packet loss (%): ", packet_loss_5000)

        msg_rate_10000 = dataframe_.loc[(dataframe_['message rate'] == 10000) & (dataframe_['payload size'] == payload)]
        msg_rate_10000_mean = msg_rate_10000['value'].mean() / millisecs_div
        msg_rate_10000_std = msg_rate_10000['value'].std() / millisecs_div
        print("mean for msg rate = 10000 is : ", msg_rate_10000_mean, " with std: ", msg_rate_10000_std)
        packet_loss_10000 = 100.0 - (
                ((len(msg_rate_10000)) * 100.0) / (
                number_of_messages * num_experiments * num_sub))
        print("LEN of array msg_rate 10000 is: " + str(len(msg_rate_10000)))
        print("Packet loss (%): ", packet_loss_10000)
        # Creating a lists for the plots

        # creating an array for delays (mean) and one for error (standard deviation)

        delays = [msg_rate_250_mean, msg_rate_500_mean, msg_rate_1000_mean, msg_rate_5000_mean, msg_rate_10000_mean]
        error = [msg_rate_250_std, msg_rate_500_std, msg_rate_1000_std, msg_rate_5000_std, msg_rate_10000_std]
        loss = [packet_loss_250, packet_loss_500, packet_loss_1000, packet_loss_5000, packet_loss_10000]
        msg_rates = [250*factor, 500*factor, 1000*factor, 5000*factor, 10000*factor]
        msg_rates_temp = np.array(msg_rates) + x_scrap[j]

        ax1.bar(msg_rates_temp, delays, yerr=error, align='center', alpha=0.5, ecolor='black', capsize=6, width=w,
                label='payload: ' + str(payload) + ' bytes')
        j += 1  # next scrap
        ax1.set_ylabel('Average of delays [milliseconds]')
        ax1.set_xticks(msg_rates)
        ax1.set_xticklabels(msg_rates)
        ax1.set_xlabel('message rate [msg/sec]')
        ax1.set_title(
            'Average of end to end delays\n '
            'with standard deviation\ntest execution: ' + which_experiment + ' on ' + where + "\nwith "
            + str(num_sub) + " SUB")
        ax1.yaxis.grid(True)

        plt.tight_layout()
        ax1.autoscale(tight=True)
        ax1.set_ylim(ylim)
        ax1.set_xlim(xlim)
        # TODO save plot

        ax2.bar(msg_rates_temp, loss, align='center', alpha=0.5, capsize=6, width=w,
                label='payload: ' + str(payload) + ' bytes')
        ax2.set_ylim(ylim_2)

        ax2.set_ylabel('Packet loss (%)')
        ax2.set_xticks(msg_rates)
        ax2.set_xticklabels(msg_rates)
        ax2.set_xlabel('message rate [msg/sec]')
        ax2.set_title(
            'Packet Loss\n '
            + which_experiment + ' on ' + where)
        ax2.yaxis.grid(True)
        ax2.set_xlim(xlim)

    plt.legend()
    # show everything
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
                except:
                    continue

                # real concat pandas arrays
                dataframe_ = pd.DataFrame(data=pd.concat([dataframe_, data_frame_temp]),
                                          columns=data_frame_temp.columns)
                # print(dataframe_)

print(dataframe_)  # print our dataframe

x_scrap = [-80*factor, 0, 80*factor]  # array of craps, must be "x" dimension for "x" message rate
w = 80*factor  # width of a bar
plotting_delays((0, 1200*factor), (-1, 2), w, x_scrap, (0, 0.2))
x_scrap = [-200*factor, 0, 200*factor]
w = 190*factor
plotting_delays((1500, 11000*factor), (-1, 2), w, x_scrap, (0, 10))

#x_scrap = [-75, 0, 75]  # array of craps, must be "x" dimension for "x" message rate
#w = 75  # width of a bar
#plotting_delays((0, 1200), (-1, 2), w, x_scrap, (0, 0.2))
#x_scrap = [-200, 0, 200]
#w = 110
#plotting_delays((-200, 11000), (-50, 200), w, x_scrap, (0, 100))
