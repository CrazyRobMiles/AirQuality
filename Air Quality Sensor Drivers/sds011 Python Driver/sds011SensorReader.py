# Connects to a ZPH01 sensor and displays the readings
# Version 1.0
# Rob Miles

import serial
import time
import sys
import struct

class DustSensorException(Exception):
    pass

class DustSensor(object):

    verbose = False
    
    def serial_port_names(self):
        """ Lists serial port names

            :raises EnvironmentError:
                On unknown platforms
            :returns:
                A list of the serial ports available on the system
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

    def active_serialPorts(self):
        """ Lists active serial ports
            :raises EnvironmentError:
                On unknown platforms
            :returns:
                A list of the active serial ports available on the system
        """
        results = []
        ports = self.serial_port_names()
        for port_name in ports:
            try:
                try_port = self.open_serial(port_name)
                results.append(port_name)
                try_port.close()
            except serial.SerialException:
                pass
        return results
                
    def open_serial(self,port_name):
        """ Opens a connetion to the named serial port
            :raises serial.SerialException:
                When port cannot be opened
            :returns:
                A serial port instance
        """
        port = serial.Serial(port_name, 9600, timeout=1)
        return port

    def get_byte(self):
        """ Reads a single byte from the serial port
            :raises DustSensorException:
                When byte not read 
            :returns:
                Byte that was read
        """
        b=self.port.read(size=1)
        if len(b)==0:
            self.port.close()
            raise DustSensorException('No data received')
        return b[0]

    def get_block(self):
        """ Reads a block of data from the serial port
            :raises DustSensorException:
                When block checksum incorrect
            :returns:
                Byte block that was read
        """
        byte=b'\x00'

        while True:
            firstbyte = byte
            print(byte)
            byte = self.port.read(size=1)
            if firstbyte == b'\xAA' and byte == b'\xC0':
                break;
            
        print('got header')

        block = self.port.read(size=8)
        readings = struct.unpack('<hhxxcc',block) # Decode the packet - big endian, 2 shorts for pm2.5 and pm10, 2 reserved bytes, chec

        print(readings)

        pm_25 = readings[0]/10.0

        pm_10 = readings[1]/10.0
        
        result =[]
        while self.get_byte() != 0xAA:
            pass
        
        result.append(0xAA)
        check=0
        for i in range(9):
            b = self.get_byte()
            check += b
            result.append(b)

        print(result)

        check_read = self.get_byte()
        check_result = (check+check_read)&255
        if self.verbose:
            print('Check:', check, 'Check read:', check_read,'result:', check_result)        
        if check_result!=0:
            raise DustSensorException('Invalid block checksum')
        return result
                    
    def get_reading(self):
        """ Gets a reading from the sensor
            :raises DustSensorException:
                When block id invalid
            :returns:
                Float value that was read
        """
        if self.verbose:
            print('getting reading')
        block = self.get_block()
        if self.verbose:
            print('type:',block[1])
        if block[1] != 0x18:
            raise DustSensorException('Invalid block type')
            return None
        rate = block[3] + block[4]/10.0
        return rate

    def open_connection(self,port_name):
        """ Opens a connection to the sensor
            :raises EnvironmentError:
                On unknown platforms
            :returns:
                True if connection succeeds
                False if connection failed
        """
        try:
            self.port = self.open_serial(port_name)
        except serial.SerialException:
            print('Port:', port_name,'could not be opened.')
            print('Available ports are:')
            ports = self.active_serialPorts()
            for port_name in ports:
                print('    ',port_name)
            return False
        if self.verbose:
            print('Port open')
        return True
        
d = DustSensor()
d.verbose = True
if d.open_connection('com4'):
    while True:
        print ('Rate:', d.get_reading())
        time.sleep(0.1)
            
        

    
