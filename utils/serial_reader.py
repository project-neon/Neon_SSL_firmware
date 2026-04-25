#!pip install pyserial
# run as python serial_reader.py --port <PORT>
# This script reads data from a serial port, processes it to extract information 
# (e.g., ID, RSSI, battery), and prints the results or error messages in real-time.

import serial
import argparse

feedback_port = 7
baud_rate = 115200

ap = argparse.ArgumentParser()
ap.add_argument('-p', '--port', required=True, help='Enter feedback ESP32 port name e.g COM4')
args = vars(ap.parse_args())
port_name = args['port']


#port_name = 'COM'+str(feedback_port)

serial_port = serial.Serial(port_name, baud_rate)


def splitSerialPerId(serial_line: str):
    data_per_id = [i[1:] for i in serial_line.split('>')[0:3]]
    return data_per_id


def convertDataToDict(data_as_list: list):
    dict_list = []
    for data in data_as_list:
        data_split = data.split(',')
        robot_dict = {'id': data_split[0], 'rssi': data_split[1], 'battery': data_split[2]}
        dict_list.append(robot_dict) 
    return dict_list       


def parseFeedbackData(serial_line: str):
    #print(serial_line)
    data_per_id = splitSerialPerId(serial_line)
    if len(data_per_id) > 1:
        data_list = convertDataToDict(data_per_id)
        print(data_list)
    else:
        print("Invalid" )

def errorCheck(serial_line: str):
    if serial_line.count('>') < 3: return True
    else: return False

while True:
    line = str(serial_port.readline()).replace("\\r","").replace("\\n","").replace("'","").replace("b","")
    print(line)
    if errorCheck(line): print('serial error')
    else: parseFeedbackData(line)
    print()


