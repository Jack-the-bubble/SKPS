import datetime
import os
from typing import List

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
camera_time_stamp = []
sender_time_stamp = []
led_timestamp = []

with open('log-latest.txt', 'r') as in_log:
	# extract data from log file
	file_content = [line.rstrip('\n') for line in in_log]
	for line in file_content:
		line = line.split(': ')[-1].rstrip('.') # cut prefix of message and dot at the end
		if 'camera-capture' in line:
			# for each message type in process create new list
			words = line.split(' ')
			key = " ".join(words[1:-1])
			number = [int(s) for s in line.split() if s.isdigit()]
			camera_time_stamp.append(number[0])

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
			number = [int(s) for s in line.split() if s.isdigit()]
			sender_time_stamp.append(number[0])

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
			number = int(words[-1]) if words[-1].isdigit() else None
			if number is None:
				continue

			led_timestamp.append(number)
			if key not in led_process.keys():
				led_process[key] = []
			try:
				led_process[key].append(int(words[-1]))
			except ValueError:
				# no time data at the end of line
				continue

	# place all points on one plot
	i=1
	plt.figure(1)

	# plt.plot(camera_process['captured at'], [1 for stamp in camera_process['captured at']], '*', label='captured at')
	for key in camera_process.keys():
		# timestamps = get_timestamps(camera_process[key])
		plt.plot(camera_process[key], [i for stamp in camera_process[key]], '.', label=key)


	for key in sender_process.keys():
		plt.plot(sender_process[key], [i for stamp in sender_process[key]], '.', label=key)

	for key in led_process.keys():
		plt.plot(led_process[key], [i for stamp in led_process[key]], '.', label=key)


	plt.legend(loc='best')
	# plt.show()

	# plot histogram of intervals between messages for each message type

	# camera capture
	plt.figure(2)
	camera_time_stamp = [element/1000000 for element in camera_time_stamp]
	delta_camera = []
	for i in range(int(len(camera_time_stamp)/2)):
		delta_camera.append(camera_time_stamp[2*i+1] - camera_time_stamp[2*i])

	plt.hist(delta_camera)
	plt.xlabel("różnica czasu [ms]")
	plt.ylabel("ilość próbek")
	# plt.show()

	# sender
	plt.figure(3)
	sender_time_stamp = [element/1000000 for element in sender_time_stamp]
	delta_sender = []
	for i in range(int(len(sender_time_stamp) / 2)):
		delta_sender.append(sender_time_stamp[2 * i + 1] - sender_time_stamp[2 * i])

	plt.hist(delta_sender)
	plt.xlabel("różnica czasu [ms]")
	plt.ylabel("ilość próbek")

	# commands histogram
	led_timestamp = [stamp/1000000 for stamp in led_timestamp]
	delta_led = []
	for i in range(int(len(led_timestamp)/2)):
		delta_led.append(led_timestamp[2*i+1] - led_timestamp[2*i])

	plt.figure(4)
	plt.hist(delta_led)
	plt.xlabel("różnica czasu [ms]")
	plt.ylabel("ilość próbek")
	plt.show()
