# python -m pip install numpy
# python -m pip install matplotlib

import matplotlib
import numpy as np
import time 

import matplotlib.pyplot as plt


from DustSensorReader import *

d = DustSensorTester()
print("Finding sensors")
sensors = d.find_devices([sds011_sensor,xph01_sensor])
print("Sensors located")

if len(sensors)==0:
    print("No sensors found")
else:
    for sensor in sensors:
        print(sensor)

    count = 0

    while True:
        for sensor in sensors:
            sensor.update()
            if len(sensor.pm2_5_values)>0:
                dates = matplotlib.dates.date2num(sensor.times)
                matplotlib.pyplot.plot_date(dates, sensor.pm2_5_values)
            if len(sensor.pm10_values)>0:
                dates = matplotlib.dates.date2num(sensor.times)
                matplotlib.pyplot.plot_date(dates, sensor.pm10_values)
        plt.draw()
        plt.pause(0.1)
        plt.clf()
