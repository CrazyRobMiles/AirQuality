import serial
import time
import sys
import struct
import datetime

class DustSensorException(Exception):
    pass

class DustSensorTester(object):

    verbose = False

    def serial_port_names(self):
        """ Gets a list of serial port names

            Returns:
                A list of the serial ports available on the system.
                Note that these may not have devices connected

            Raises:
                EnvironmentError: on unknown platforms
        """
        if sys.platform.startswith('win'):
            ports = ['COM%s' % (i + 1) for i in range(256)]
        elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
            # this excludes your current terminal "/dev/tty"
            ports = glob.glob('/dev/tty[A-Za-z]*')
        elif sys.platform.startswith('darwin'):
            ports = glob.glob('/dev/tty.*')
        else:
            raise EnvironmentError('Unsupported platform')

        return ports

    def active_serial_ports(self):
        """ Gets a list of active serial ports

            Returns:
                A list of the serial ports that have devices
                connected to them on the system

            Raises:
                EnvironmentError: on unknown platforms
        """

        results = []
        ports = self.serial_port_names()
        for port_name in ports:
            try:
                try_port = serial.Serial(port_name)
                results.append(port_name)
                try_port.close()
            except serial.SerialException:
                pass
        return results

    def contains_sequence (self,data, test):
        """ Checks to see if the data sequence contains the test sewquence

        Args:
            data: sequence of data items
            test: test sequence
        Returns:
            True if the test sequence is in the data sequence
        """

        if len(data)<len(test): return False

        for pos in range(0,len(data)-len(test)+1):
            if test == data[pos:pos+len(test)]: return True
        return False

    def find_devices(self, device_types, no_of_bytes_to_test=15):
            
        """ Finds active devices which generate a particular sequence of
            start bytes - as do the dust sensors
            
        Args:
            device_types: list of device types
            no_of_bytes_to_test: how many input bytes to read and test

        Returns:
                A list of dust sensors for testing

        Raises:
            SerialPortError: if the serial connection cannot be made
        """

        results = []
        
        active_ports = self.active_serial_ports()

        for device_type in device_types:
            device = None
            for port_name in active_ports:
                test_port = serial.Serial(port_name, device_type.baud_rate, timeout=2)
                test_bytes = []
                while len(test_bytes)<no_of_bytes_to_test:
                    b = test_port.read(size=1)
                    if len(b)==0:
                        break
                    test_bytes.append(int(b[0]))
                    if self.contains_sequence(test_bytes, device_type.start_sequence):
                        device = device_type(test_port)
                        results.append(device)
                        break
                if device == None:
                    test_port.close()
                else:
                    active_ports.remove(port_name)

        return results

class DustSensor(object):

    verbose = False

    AWAITING_START = 0
    READING_BLOCK = 1

    def pump_byte(self, b):
        """ Pump a byte into the block decode. Calls the decode method
            when the block is complete

        Args:
            b: byte to pump

        Raises:
            SerialPortError: if the serial connection cannot be made
        """
        if self.state==self.AWAITING_START:
            if self.verbose: print(self, "Awaiting start:", b)
            if b==self.start_sequence[self.start_pos]:
                # got a match - move to next byte in start sequence
                self.start_pos = self.start_pos+1
                if self.start_pos == len(self.start_sequence):
                    # matched the start sequence
                    self.block=self.start_sequence.copy()
                    self.state=self.READING_BLOCK
        elif self.state==self.READING_BLOCK:
            if self.verbose: print("Reading block:", b)
            if b==self.start_sequence[0]:
                # got the start of another block
                # in the middle of a block
                self.state=self.AWAITING_START
                self.start_pos=1
            else:
                self.block.append(b)
                if len(self.block) == self.block_size:
                    self.process_block()
                    self.state=self.AWAITING_START
                    self.start_pos=0

    def update(self):
        while self.port.in_waiting > 0:
            b = self.port.read(size=1)
            self.pump_byte(int(b[0]))

    def __init__(self, port):
        self.port = port
        self.state=self.AWAITING_START
        self.start_pos = 0
        self.times = []
        self.pm10_values = []
        self.pm2_5_values = []

class sds011_sensor(DustSensor):
    baud_rate = 9600
    start_sequence = [0xaa,0xc0]
    block_size = 10

    def process_block(self):
        if self.verbose: print("sds011 process block")
        if self.verbose: print([hex(x) for x in self.block])
        check_sum = 0
        for i in range(2,8):
            check_sum = check_sum + self.block[i]
        check_sum = check_sum & 0xff
        if self.verbose: print("Checksum:",hex(check_sum))
        if check_sum!=self.block[8]:
            raise DustSensorException(str(self)+" invalid checksum")
        ppm10 = (self.block[4]+256*self.block[5])/10
        ppm2_5 = (self.block[2]+256*self.block[3])/10

        self.times.append(datetime.datetime.now())
        self.pm10_values.append(ppm10)
        self.pm2_5_values.append(ppm2_5)

    def __repr__(self):
        return "SDS011 port:" + self.port.name
    
class xph01_sensor(DustSensor):
    baud_rate = 9600
    start_sequence = [0xff,0x18]
    block_size = 9
    
    def process_block(self):
        if self.verbose: print("xph01 process block")
        if self.verbose: print([hex(x) for x in self.block])
        check_sum = 0
        for i in range(1,8):
            check_sum = check_sum + self.block[i]
        if self.verbose: print("Checksum:",hex(check_sum))
        check_result = (check_sum+self.block[8])&255
        if self.verbose: print("Check result:", check_result)
        if check_result != 0:
            raise DustSensorException(str(self)+" invalid checksum")

        ppm2_5 = (self.block[3]+(self.block[4])/10)
        self.times.append(datetime.datetime.now())
        self.pm2_5_values.append(ppm2_5)
    
    def __repr__(self):
        return "xph01 port:" + self.port.name

