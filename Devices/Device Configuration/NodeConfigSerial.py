import sys
import glob
import serial
import time
import random
import json
from tkinter import *
from tkinter import messagebox

from tkinter.filedialog import askopenfilename
from tkinter.filedialog import asksaveasfilename

class NodeConfigSerial():

    def serial_port_names(self):
        """ Lists serial port names

            :raises EnvironmentError:
                On unsupported or unknown platforms
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

    def send_text_to_port(self, text, serial_port):
        return_text = new_numbers_lambda = map(lambda x : x if x != '\n' else '\r', text)

        byte_text = bytearray()
        byte_text.extend(map(ord,return_text))

        serial_port.write(byte_text)
        serial_port.write(ord('\n'))
        
        self.set_status('Sent to node')

    def send_text(self, text):
        
        if self.serial_port == None:
            self.set_status('Serial port not connected')
            return

        self.send_text_to_port(text, self.serial_port)
        
    def read_line_from_serial(self,ser):
        result = ""
        while True:
            b=ser.read(size=1)
            if len(b)==0:
                # timeout
                return result
            c = chr(b[0])
            if c=='\n':
                return result
            result = result + c

    def assemble_command (self,version, command, option,  seq=None, target=None, value=None):
        result = {}
        result['v'] = version
        result['c'] = command
        result['o'] = option
        if target != None:
            result['t'] = target
        if value != None:
            result['val'] = value
        if seq != None:
            result['seq'] = seq
        return json.dumps(result)

    def issue_comand(self, command,serial_port):
        try:
            serial_port.reset_input_buffer()
            self.send_text_to_port(command, serial_port)
            response = self.read_line_from_serial(serial_port)
            return response
        except (OSError, serial.SerialException):
            return None


    def get_node_version(self, serial_port):
        try:
            reset_cmd = self.assemble_command(1, 'node', 'ver', seq=1)
            print(reset_cmd)
            response = self.issue_comand(reset_cmd,serial_port)
            print("Response: ", response)
            version_dict = json.loads(response)
            version_number = version_dict['version']
            return version_number
        except (OSError, serial.SerialException, KeyError, json.decoder.JSONDecodeError):
            return None

    def open_connection(self,port_name):
        try:
            port = serial.Serial(port_name, 115200, timeout=1)
        except (OSError, serial.SerialException):
            return None
        return port

    def set_status(self,status):
        self.status_label.config(text=status)
        self.root.update()

    def set_serial_status_state(self, status):
        if status:
            self.serial_button_frame["bg"] = "green"
        else:
            self.serial_button_frame["bg"] = "red"
        self.root.update()
    
    def try_to_connect(self,port_name):
        if self.trying_to_connect:
            print("re-entrant call of try to connect")
            return

        self.trying_to_connect = True
        
        port = self.open_connection(port_name)

        if port == None:
            self.set_serial_status_state(False)
            self.trying_to_connect = False
            return False

        self.set_status('Found port: ' + port_name)

        time.sleep(1)
        
        version = self.get_node_version(port)
        
        if version != None :
            self.serial_port = port
            self.set_status('Serial port ' + port_name + ' connected to sensor version' + str(version))
            self.set_serial_status_state(True)
            self.trying_to_connect = False
            return True

        self.set_serial_status_state(False)
        self.trying_to_connect = False
        return False
    
    def do_connect_serial(self):
        
        if self.serial_port != None:
            self.set_status('Serial port already connected')
            return
        
        self.set_status('Connecting..')

        if self.last_working_port != None:
            if self.try_to_connect(self.last_working_port):
                return

        port_names = self.serial_port_names()

        for port_name in port_names:
            if self.try_to_connect(port_name):
                # We found a port - remember it
                self.last_working_port = port_name
                self.force_disconnect = False
                return

        self.set_status('No Sensors found')

    def do_disconnect_serial(self):
        
        self.set_status('Serial port disconnected')
        
        if self.serial_port == None:
            return

        self.serial_port.close()

        self.serial_port = None
        self.force_disconnect = True
        self.set_serial_status_state(False)

    def dump_string(self,title, string):
        print(title, string)
        print(title,end='')
        for ch in string:
            print(ord(ch),end='')
        print()

    def __init__(self,root):
        '''
        Create an instance of the editor. root provides
        the Tkinter root frame for the editor
        '''

        self.root = root

        Grid.rowconfigure(self.root, 0, weight=1)
        Grid.columnconfigure(self.root, 0, weight=1)

        self.root.title("Node Configurator")

        self.serial_port = None
        self.last_working_port = None
        self.force_disconnect = False
        self.trying_to_connect = False

        self.frame = Frame(root,borderwidth=5)
        Grid.rowconfigure(self.frame, 0, weight=1)
        Grid.columnconfigure(self.frame, 1, weight=1)
        self.frame.grid(row=0,column=0, padx=5, pady=5,sticky='nsew')

        rowCount=0
        colCount=0

        devicename_label = Label(self.frame,text='Device name: ')
        devicename_label.grid(sticky=E+N+S, row=rowCount, column=0, padx=5, pady=5)
        self.devicename_text = Text(self.frame, width=20, height=1)
        self.devicename_text.grid(sticky=E+N+S+W, row=rowCount, column=1, padx=5, pady=5)
        rowCount = rowCount + 1

        self.serial_button_frame = Frame(self.frame)

        connectSerialButton = Button(self.serial_button_frame, text='Connect Sensor', command=self.do_connect_serial)
        connectSerialButton.pack(padx=5, pady=5, side=LEFT)
        
        disconnectSerialButton = Button(self.serial_button_frame, text='Disconnect Sensor', command=self.do_disconnect_serial)
        disconnectSerialButton.pack(padx=5, pady=5, side=RIGHT)
        
        self.serial_button_frame.grid(row=rowCount, column=colCount, padx=5, pady=5, columnspan=2,sticky='nsew')
        self.set_serial_status_state(False)

        rowCount = rowCount+1

        self.status_label = Label(self.frame,text="Status")
        self.status_label.grid(row=rowCount, column=0, columnspan=5,sticky='nsew')

root=Tk()
editor=NodeConfigSerial(root)
root.mainloop()  

