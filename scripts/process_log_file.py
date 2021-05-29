import datetime
import os

import matplotlib.pyplot as plt


# def get_timestamps(ms_list):
# 	"""Convert timestamps in milliseconds to datetime format"""
#
# 	base_datetime = datetime.datetime(1970, 1, 1)
# 	target_list = []
# 	for stamp in ms_list:
# 		time_date = datetime.datetime.fromtimestamp(stamp)
# 		target_list.append(time_date)
# 		# delta = datetime.timedelta(0, 0, 0, stamp)
# 		# target_list.append(base_datetime + delta)
#
# 	return target_list

camera_process = {}
sender_process = {}
led_process = {}

with open('log-2021-05-29-23-07.txt', 'r') as in_log:
	# extract data from log file
	file_content = [line.rstrip('\n') for line in in_log]
	for line in file_content:
		line = line.split(': ')[-1].rstrip('.') # cut prefix of message and dot at the end
		if 'camera-capture' in line:
			# for each message type in process create new list
			words = line.split(' ')
			key = " ".join(words[1:-1])
			if key not in camera_process.keys():
				camera_process[key] = []
			try:
				camera_process[key].append(int(words[-1]))
			except ValueError:
				# no time data at the end of line
				continue

		elif 'send-image' in line:
			words = line.split(' ')
			key = " ".join(words[1:-1])
			if key not in sender_process.keys():
				sender_process[key] = []
			try:
				sender_process[key].append(int(words[-1]))
			except ValueError:
				# no time data at the end of line
				continue

		elif 'execute-command' in line:
			words = line.split(' ')
			key = " ".join(words[1:-1])
			if key not in led_process.keys():
				led_process[key] = []
			try:
				led_process[key].append(int(words[-1]))
			except ValueError:
				# no time data at the end of line
				continue

	# place all points on one plot

	print("did sth")
	i = 1
	plt.figure(1)

	# plt.plot(camera_process['captured at'], [1 for stamp in camera_process['captured at']], '*', label='captured at')
	for key in camera_process.keys():
		# timestamps = get_timestamps(camera_process[key])
		plt.plot(camera_process[key], [i for stamp in camera_process[key]], '.', label=key)
		# i = i + 1

	# plt.legend(loc='best')

	# plt.figure(2)
	for key in sender_process.keys():
		plt.plot(sender_process[key], [i for stamp in sender_process[key]], '.', label=key)
	# i = i + 1
	plt.legend(loc='best')
	plt.show()
	# plot intervals between messages for each message type

	# calculate histograms for intervals from each message type



