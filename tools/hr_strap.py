"""
Read heart rate data from a heart rate peripheral using the standard BLE
Heart Rate service.
"""

import time

import adafruit_ble
from adafruit_ble.advertising.standard import ProvideServicesAdvertisement
from adafruit_ble.services.standard.device_info import DeviceInfoService
from adafruit_ble_heart_rate import HeartRateService

# PyLint can't find BLERadio for some reason so special case it here.
ble = adafruit_ble.BLERadio()  # pylint: disable=no-member

hr_connection = None

while True:
    print("Scanning...")
    for adv in ble.start_scan(ProvideServicesAdvertisement, timeout=5):
        if HeartRateService in adv.services:
            print("found a HeartRateService advertisement")
            hr_connection = ble.connect(adv)
            print("Connected")
            break

    # Stop scanning whether or not we are connected.
    ble.stop_scan()
    print("Stopped scan")

    if hr_connection and hr_connection.connected:
        print("Fetch connection")
        if DeviceInfoService in hr_connection:
            dis = hr_connection[DeviceInfoService]
            try:
                manufacturer = dis.manufacturer
            except AttributeError:
                manufacturer = "(Manufacturer Not specified)"
            try:
                model_number = dis.model_number
            except AttributeError:
                model_number = "(Model number not specified)"
            print("Device:", manufacturer, model_number)
        else:
            print("No device information")
        hr_service = hr_connection[HeartRateService]
        print("Location:", hr_service.location)
        while hr_connection.connected:
            print(hr_service.measurement_values)
            time.sleep(1)



"""
/home/jan/projects/stages-monitor/venv/bin/python /home/jan/projects/stages-monitor/test/hr_strap.py
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
found a HeartRateService advertisement
Connected
Stopped scan
Fetch connection
Device: Wahoo Fitness (Model number not specified)
Location: Chest
None
HeartRateMeasurementValues(heart_rate=85, contact=True, energy_expended=None, rr_intervals=[796])
HeartRateMeasurementValues(heart_rate=85, contact=True, energy_expended=None, rr_intervals=[796])
HeartRateMeasurementValues(heart_rate=81, contact=True, energy_expended=None, rr_intervals=[804, 816])
HeartRateMeasurementValues(heart_rate=78, contact=True, energy_expended=None, rr_intervals=[812])
HeartRateMeasurementValues(heart_rate=77, contact=True, energy_expended=None, rr_intervals=[860, 348])
HeartRateMeasurementValues(heart_rate=77, contact=True, energy_expended=None, rr_intervals=[692])
HeartRateMeasurementValues(heart_rate=76, contact=True, energy_expended=None, rr_intervals=[1060])
HeartRateMeasurementValues(heart_rate=76, contact=True, energy_expended=None, rr_intervals=[1004])
HeartRateMeasurementValues(heart_rate=76, contact=True, energy_expended=None, rr_intervals=[920])
HeartRateMeasurementValues(heart_rate=76, contact=True, energy_expended=None, rr_intervals=[652])
None
HeartRateMeasurementValues(heart_rate=71, contact=True, energy_expended=None, rr_intervals=[1056, 892])
None
HeartRateMeasurementValues(heart_rate=69, contact=True, energy_expended=None, rr_intervals=[868])
None
None
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
Stopped scan
Scanning...
found a HeartRateService advertisement
Connected
Stopped scan
Fetch connection
Device: Wahoo Fitness (Model number not specified)
Location: Chest
None
HeartRateMeasurementValues(heart_rate=62, contact=True, energy_expended=None, rr_intervals=[988])
HeartRateMeasurementValues(heart_rate=62, contact=True, energy_expended=None, rr_intervals=[1056])
HeartRateMeasurementValues(heart_rate=62, contact=True, energy_expended=None, rr_intervals=[1108])
HeartRateMeasurementValues(heart_rate=61, contact=True, energy_expended=None, rr_intervals=[1104])
HeartRateMeasurementValues(heart_rate=61, contact=True, energy_expended=None, rr_intervals=[1024])
HeartRateMeasurementValues(heart_rate=61, contact=True, energy_expended=None, rr_intervals=[1036])
HeartRateMeasurementValues(heart_rate=61, contact=True, energy_expended=None, rr_intervals=[1072])
HeartRateMeasurementValues(heart_rate=59, contact=True, energy_expended=None, rr_intervals=[1068])
HeartRateMeasurementValues(heart_rate=58, contact=True, energy_expended=None, rr_intervals=[1080])
HeartRateMeasurementValues(heart_rate=58, contact=True, energy_expended=None, rr_intervals=[1088])
HeartRateMeasurementValues(heart_rate=58, contact=True, energy_expended=None, rr_intervals=[1068])
HeartRateMeasurementValues(heart_rate=58, contact=True, energy_expended=None, rr_intervals=[1091])
HeartRateMeasurementValues(heart_rate=58, contact=True, energy_expended=None, rr_intervals=[1056])
HeartRateMeasurementValues(heart_rate=57, contact=True, energy_expended=None, rr_intervals=[1044])
HeartRateMeasurementValues(heart_rate=57, contact=True, energy_expended=None, rr_intervals=[1056])
HeartRateMeasurementValues(heart_rate=58, contact=True, energy_expended=None, rr_intervals=[1032])
HeartRateMeasurementValues(heart_rate=58, contact=True, energy_expended=None, rr_intervals=[1028])
HeartRateMeasurementValues(heart_rate=58, contact=True, energy_expended=None, rr_intervals=[1036])
HeartRateMeasurementValues(heart_rate=58, contact=True, energy_expended=None, rr_intervals=[980])
HeartRateMeasurementValues(heart_rate=59, contact=True, energy_expended=None, rr_intervals=[1016])
HeartRateMeasurementValues(heart_rate=59, contact=True, energy_expended=None, rr_intervals=[1100])
HeartRateMeasurementValues(heart_rate=59, contact=True, energy_expended=None, rr_intervals=[])
HeartRateMeasurementValues(heart_rate=59, contact=True, energy_expended=None, rr_intervals=[1052])
HeartRateMeasurementValues(heart_rate=59, contact=True, energy_expended=None, rr_intervals=[1100])
HeartRateMeasurementValues(heart_rate=59, contact=True, energy_expended=None, rr_intervals=[1120])
HeartRateMeasurementValues(heart_rate=59, contact=True, energy_expended=None, rr_intervals=[1148])
HeartRateMeasurementValues(heart_rate=58, contact=True, energy_expended=None, rr_intervals=[1144])
None
HeartRateMeasurementValues(heart_rate=57, contact=True, energy_expended=None, rr_intervals=[1292])
HeartRateMeasurementValues(heart_rate=57, contact=True, energy_expended=None, rr_intervals=[])
HeartRateMeasurementValues(heart_rate=56, contact=True, energy_expended=None, rr_intervals=[1276])
HeartRateMeasurementValues(heart_rate=55, contact=True, energy_expended=None, rr_intervals=[1307])
HeartRateMeasurementValues(heart_rate=54, contact=True, energy_expended=None, rr_intervals=[1296])
HeartRateMeasurementValues(heart_rate=53, contact=True, energy_expended=None, rr_intervals=[1288])
HeartRateMeasurementValues(heart_rate=53, contact=True, energy_expended=None, rr_intervals=[])
HeartRateMeasurementValues(heart_rate=52, contact=True, energy_expended=None, rr_intervals=[1360])
HeartRateMeasurementValues(heart_rate=50, contact=True, energy_expended=None, rr_intervals=[1424])
None
HeartRateMeasurementValues(heart_rate=48, contact=True, energy_expended=None, rr_intervals=[])
HeartRateMeasurementValues(heart_rate=48, contact=True, energy_expended=None, rr_intervals=[1236])
HeartRateMeasurementValues(heart_rate=48, contact=True, energy_expended=None, rr_intervals=[1412])
HeartRateMeasurementValues(heart_rate=47, contact=True, energy_expended=None, rr_intervals=[1244])
HeartRateMeasurementValues(heart_rate=47, contact=True, energy_expended=None, rr_intervals=[])
HeartRateMeasurementValues(heart_rate=47, contact=True, energy_expended=None, rr_intervals=[1628])
HeartRateMeasurementValues(heart_rate=47, contact=True, energy_expended=None, rr_intervals=[1424])
HeartRateMeasurementValues(heart_rate=47, contact=True, energy_expended=None, rr_intervals=[])
HeartRateMeasurementValues(heart_rate=47, contact=True, energy_expended=None, rr_intervals=[1404])
HeartRateMeasurementValues(heart_rate=47, contact=True, energy_expended=None, rr_intervals=[1280])
HeartRateMeasurementValues(heart_rate=47, contact=True, energy_expended=None, rr_intervals=[1296])
HeartRateMeasurementValues(heart_rate=47, contact=True, energy_expended=None, rr_intervals=[])
HeartRateMeasurementValues(heart_rate=46, contact=True, energy_expended=None, rr_intervals=[1468])
HeartRateMeasurementValues(heart_rate=46, contact=True, energy_expended=None, rr_intervals=[1463])
HeartRateMeasurementValues(heart_rate=46, contact=True, energy_expended=None, rr_intervals=[1436])
HeartRateMeasurementValues(heart_rate=46, contact=True, energy_expended=None, rr_intervals=[])
HeartRateMeasurementValues(heart_rate=45, contact=True, energy_expended=None, rr_intervals=[1460])
HeartRateMeasurementValues(heart_rate=44, contact=True, energy_expended=None, rr_intervals=[1416])
HeartRateMeasurementValues(heart_rate=44, contact=True, energy_expended=None, rr_intervals=[1320])
HeartRateMeasurementValues(heart_rate=44, contact=True, energy_expended=None, rr_intervals=[])
None
None
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[])
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[1448])
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[1616])
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[])
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[1540])
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[1516])
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[])
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[1352])
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[1384])
None
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[1327])
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[])
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[1564])
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[1448])
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[])
HeartRateMeasurementValues(heart_rate=43, contact=True, energy_expended=None, rr_intervals=[1228])
HeartRateMeasurementValues(heart_rate=44, contact=True, energy_expended=None, rr_intervals=[1252])
HeartRateMeasurementValues(heart_rate=44, contact=True, energy_expended=None, rr_intervals=[1364])
HeartRateMeasurementValues(heart_rate=45, contact=True, energy_expended=None, rr_intervals=[1204])
None
HeartRateMeasurementValues(heart_rate=45, contact=True, energy_expended=None, rr_intervals=[1316])
HeartRateMeasurementValues(heart_rate=46, contact=True, energy_expended=None, rr_intervals=[1260])
HeartRateMeasurementValues(heart_rate=46, contact=True, energy_expended=None, rr_intervals=[1264])
Traceback (most recent call last):
  File "/home/jan/projects/stages-monitor/test/hr_strap.py", line 50, in <module>
    time.sleep(1)
KeyboardInterrupt

Process finished with exit code 130 (interrupted by signal 2: SIGINT)
"""