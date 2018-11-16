import paho.mqtt.client as mqtt
import time
import sys
import glob
import serial
import random
from tkinter import *
from tkinter import messagebox

from tkinter.filedialog import askopenfilename
from tkinter.filedialog import asksaveasfilename

class MQTTSource(object):

    def on_connect(self, client, userdata, flags, rc):
        if rc==0:
            print("connected OK Returned code=",rc)
        else:
            print("Bad connection Returned code=",rc)


    def on_message(self, client, userdata, message):
        time.sleep(1)
        print("received message =",str(message.payload.decode("utf-8")))

    strMqttBroker = "mqtt.connectedhumber.org"
    strMqttChannel = "sensor01/data"

    def __init__(self,root):
        '''
        Create a connection to consumer
        '''
        client = mqtt.Client()
        client.on_connect=on_connect
        client.username_pw_set('connectedhumber', '3fds8gssf6')
        client.connect(strMqttBroker, 1883, 60)
        client.loop_start()  #Start loop 
        time.sleep(4) # Wait for connection setup to complete
        print("tick")
        client.publish(strMqttChannel, "hello from python")
        client.on_message=on_message 
        client.subscribe("sensor01/data")
        input("Press return to stop the program")
        client.loop_stop()
        print("loop done")


class NodeConfig(object):
    '''
    Provides a configuration interface for the
    HullOS network connector. 
    '''
    def clear_output(self):
        self.output_Text.delete('0.0', END)
        self.root.update()

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

    def set_status(self,status):
        self.status_label.config(text=status)
        self.root.update()

    def set_serial_status_state(self, status):
        if status:
            self.serial_button_frame["bg"] = "green"
        else:
            self.serial_button_frame["bg"] = "red"
        self.root.update()

    def read_line_from_serial(self,ser):
        result = ""
        # Stop the ticker from reading the input
        while True:
            b=ser.read(size=1)
            if len(b)==0:
                # timeout
                return result
            c = chr(b[0])
            if c=='\n':
                return result
            result = result + c

    def send_text(self, text):
        
        if self.serial_port == None:
            self.set_status('Serial port not connected')
            return

        clean_text = ""
        for ch in text:
            if ch != '\n' and ch !='\r':
                clean_text = clean_text + ch
        
        text = clean_text.strip()
        text = text + '\r'
        return_text = new_numbers_lambda = map(lambda x : x if x != '\n' else '\r', text)

        byte_text = bytearray()
        byte_text.extend(map(ord,return_text))

        print("send line: " + text)
        for ch in byte_text:
            print(ch)

        self.serial_port.write(byte_text)

    def get_client_version(self, serial_port):
        return "kludge"
        try:
            serial_port.flushInput()
            serial_port.write(b'*IV\r')
            self.comms_active = True
            response = self.read_line_from_serial(serial_port)
            if response.startswith('Network Client '):
                self.comms_active = False
                return response
            else:
                self.comms_active = False
                return None
        except (OSError, serial.SerialException):
            self.comms_active = False
            return None

    def open_connection(self,port_name):
        try:
            port = serial.Serial(port_name, 9600, timeout=1)
        except (OSError, serial.SerialException):
            return None
        return port

    def try_to_connect(self,port_name):

        port = self.open_connection(port_name)

        if port == None:
            self.set_serial_status_state(False)
            return False

        self.set_status('Found port: ' + port_name)
        
        # give the robot time to settle after reset
        time.sleep(1)
        
        version = self.get_client_version(port)
        
        if version != None :
            self.serial_port = port
            self.set_status('Serial port ' + port_name + ' connected to Pixelbot Network Client ' + version)
            self.clear_output()
            self.set_serial_status_state(True)
            return True

        self.set_serial_status_state(False)
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

        self.set_status('No Devices found')

    def dump_string(self,title, string):
        print(title, string)
        print(title,end='')
        for ch in string:
            print(ord(ch),end='')
        print()

    def do_disconnect_serial(self):
        
        self.set_status('Serial port disconnected')
        
        if self.serial_port == None:
            return

        self.serial_port.close()

        self.serial_port = None
        self.force_disconnect = True
        self.set_serial_status_state(False)

    def update_output_text(self):
        if self.serial_port != None:
            try:
                while self.serial_port.in_waiting > 0:
                    b = self.serial_port.read()
                    c = chr(b[0])
                    self.output_Text.insert(END,c)
                    self.output_Text.see(END)
            except:
                self.serial_port.close()
                self.serial_port = None
                self.set_status('Serial port disconnected')
                self.set_serial_status_state(False)

    def do_tick(self):

        self.root.after(1000,self.do_tick)

        # stop the output tick display from absorbing
        # characters we want to see
        if self.comms_active:
            return
        
        if self.serial_port == None:
            if not self.force_disconnect:
                # Only try to reconnect if the user
                # hasn't forced a disconnection
                if self.last_working_port != None:
                    # We have been unplugged - try to reconnect
                    self.serial_port = self.open_connection(self.last_working_port)
                    if self.serial_port != None:
                        self.set_status('Serial port reconnected')
                        self.set_serial_status_state(True)
        
        if self.serial_port != None:
            try:
                while self.serial_port.in_waiting > 0:
                    b = self.serial_port.read()
                    c = chr(b[0])
                    self.output_Text.insert(END,c)
                    self.output_Text.see(END)
            except:
                self.serial_port.close()
                self.serial_port = None
                self.set_status('Serial port disconnected')
                self.set_serial_status_state(False)

    def do_save_settings(self):
        try:
            self.serial_port.flushInput()
            self.serial_port.write(b'*NS\r')
            self.comms_active = True
            for i in range(0, self.no_of_wifi_configs):
                self.send_text(self.accessPoints[i].get('1.0',END))
                self.send_text(self.passwords[i].get('1.0',END))

            self.send_text(self.devicename_text.get('1.0',END))
            self.send_text(self.azure_endpoint_text.get('1.0',END))
            self.send_text(self.key_text.get('1.0',END))

            self.set_status('Settings saved')

        except (OSError, serial.SerialException):
            self.set_status('Settings load failed')
        self.comms_active = False

    def do_load_settings(self):
        try:
            self.serial_port.flushInput()
            # Send the command to read the settings from the remote client
            self.serial_port.write(b'*NR\r')
            self.comms_active = True
            for i in range(0, self.no_of_wifi_configs):
                self.accessPoints[i].delete('0.0', END)
                self.accessPoints[i].insert('0.0', self.read_line_from_serial(self.serial_port))
                self.passwords[i].delete('0.0', END)
                self.passwords[i].insert('0.0', self.read_line_from_serial(self.serial_port))

            self.devicename_text.delete('0.0', END)
            self.devicename_text.insert('0.0', self.read_line_from_serial(self.serial_port))

            self.azure_endpoint_text.delete('0.0', END)
            self.azure_endpoint_text.insert('0.0', self.read_line_from_serial(self.serial_port))

            self.key_text.delete('0.0', END)
            self.key_text.insert('0.0', self.read_line_from_serial(self.serial_port))

            self.set_status('Settings loaded')

        except (OSError, serial.SerialException):
            self.set_status('Settings load failed')
        self.comms_active = False

    def do_scan_access_points(self):
        self.serial_port.write(b'*NA\r')

    def do_connect_to_network(self):
        self.serial_port.write(b'*NC\r')

    def do_list_settings(self):
        self.serial_port.write(b'*NR\r')

    def do_preset_settings(self):
        self.serial_port.write(b'*ND\r')

    def do_direct_connect(self):
        pass

    def show_mqtt_connected(self):
        self.mqtt_button_frame["bg"] = "green"
        self.root.update()

    def show_mqtt_disconnected(self):
        self.mqtt_button_frame["bg"] = "red"
        self.root.update()

    def node_message_handler(self, client, userdata, message):
        print("got", message.payload)
        text = message.payload.decode('ascii').strip()
        print(text)
        self.mqtt_output_Text.insert(END,text + '\n')
        self.mqtt_output_Text.see(END)

    def on_mqtt_connected(self,client, userdata, flags, rc):
        self.show_mqtt_connected()
        topic = self.mqtt_publish_text.get('1.0',END).strip()
        self.subscribe_to_MQTT_message(topic, self.node_message_handler)

    def MQTT_connect(self, host, username, password):
        self.client = mqtt.Client()
        self.client.on_connect=self.on_mqtt_connected
        self.client.username_pw_set(username, password)
        self.client.connect(host, 1883, 60)
        self.client.loop_start()  

    def do_MQTT_connect(self):
        user = self.mqtt_user_text.get('1.0',END).strip()
        password = self.mqtt_password_text.get('1.0',END).strip()
        host = self.mqtt_host_text.get('1.0',END).strip()
        self.MQTT_connect(host, user, password)
    
    def MQTT_disconnect(self):
        self.client.loop_stop()
        self.show_mqtt_disconnected()

    def do_MQTT_disconnect(self):
        self.MQTT_disconnect()

    def subscribe_to_MQTT_message(self, topic, message_handler):
        print("subscribing to:", topic)
        if self.mqtt_topic != None:
            client.unsbscribe(self.mqtt_topic)
            self.mqtt_topic = None

        self.client.on_message=message_handler 
        self.client.subscribe(topic)
        self.mqtt_topic = topic

    def do_MQTT_publish(self, topic, message_text):
        self.client.publish(topic, message_text)

    def reset_values(self):
        self.mqtt_device_text.insert('0.0', 'Sensor01')
        self.mqtt_host_text.insert('0.0', 'mqtt.connectedhumber.org')
        self.mqtt_user_text.insert('0.0', 'connectedhumber')
        self.mqtt_password_text.insert('0.0', '3fds8gssf6')
        self.mqtt_publish_text.insert('0.0', 'sensor01/data')
        self.mqtt_subscribe_text.insert('0.0', 'sensor01/commands')

    def MQTT_send_to_subscriber(self, topic, message):
        self.client.publish(topic,payload=message)

    def do_MQTT_send_to_subscriber(self):
        message = self.mqtt_subscribe_send_message_text.get('1.0',END).strip()
        topic = self.mqtt_subscribe_text.get('1.0',END).strip()
        self.MQTT_send_to_subscriber(topic,message)

    def ssid_set_button_pressed(self,button_number):
        print("button", button_number)

    def __init__(self,root):
        '''
        Create an instance of the editor. root provides
        the Tkinter root frame for the editor
        '''

        self.mqtt_topic = None

        self.root = root
        
        self.no_of_wifi_configs = 5

        Grid.rowconfigure(self.root, 0, weight=1)
        Grid.columnconfigure(self.root, 0, weight=1)

        self.root.title("HullOS Network Configuration 1.0 Rob Miles")

        self.serial_port = None
        self.last_working_port = None
        self.force_disconnect = False
        self.comms_active=False

        # used to detect when to save
        self.code_copy = ""
        
        self.frame = Frame(root,borderwidth=5)
        Grid.rowconfigure(self.frame, 0, weight=1)
        Grid.columnconfigure(self.frame, 1, weight=1)

        rowCount = 0
        
        self.frame.grid(row=rowCount,column=0, padx=5, pady=5,sticky='nsew')

        mqtt_frame = Frame(self.frame)
        mqtt_device_label = Label(mqtt_frame,text='MQTT Name: ');
        mqtt_device_label.grid(sticky=E+N+S, row=0, column=0, padx=5, pady=5)
        self.mqtt_device_text = Text(mqtt_frame, width=20, height=1)
        self.mqtt_device_text.grid(sticky=E+N+S+W, row=0, column=1, padx=5, pady=5)

        mqtt_host_label = Label(mqtt_frame,text='MQTT Host: ');
        mqtt_host_label.grid(sticky=E+N+S, row=0, column=2, padx=5, pady=5)
        self.mqtt_host_text = Text(mqtt_frame, width=20, height=1)
        self.mqtt_host_text.grid(sticky=E+N+S+W, row=0, column=3, padx=5, pady=5)

        mqtt_user_label = Label(mqtt_frame,text='MQTT User: ');
        mqtt_user_label.grid(sticky=E+N+S, row=0, column=4, padx=5, pady=5)
        self.mqtt_user_text = Text(mqtt_frame, width=20, height=1)
        self.mqtt_user_text.grid(sticky=E+N+S+W, row=0, column=5, padx=5, pady=5)

        mqtt_password_label = Label(mqtt_frame,text='MQTT Password: ');
        mqtt_password_label.grid(sticky=E+N+S, row=0, column=6, padx=5, pady=5)
        self.mqtt_password_text = Text(mqtt_frame, width=20, height=1)
        self.mqtt_password_text.grid(sticky=E+N+S+W, row=0, column=7, padx=5, pady=5)

        mqtt_frame.grid(row=rowCount, column=0, padx=5, pady=5, columnspan=2,sticky='nsew')

        mqtt_subscribe_label = Label(mqtt_frame,text='Node will subscribe to this topic: ');
        mqtt_subscribe_label.grid(sticky=E+N+S, row=1, column=0, padx=5, pady=5)
        self.mqtt_subscribe_text = Text(mqtt_frame, width=20, height=1)
        self.mqtt_subscribe_text.grid(sticky=E+N+S+W, row=1, column=1, padx=5, pady=5)

        mqtt_subscribe_send_message_label = Label(mqtt_frame,text='Message to send to node: ');
        mqtt_subscribe_send_message_label.grid(sticky=E+N+S, row=2, column=0, padx=5, pady=5)
        self.mqtt_subscribe_send_message_text = Text(mqtt_frame, width=20, height=1)
        self.mqtt_subscribe_send_message_text.grid(sticky=E+N+S+W, row=2, column=1, columnspan=6, padx=5, pady=5)
        mqtt_send_to_subscriber = Button(mqtt_frame, text='Send', command=self.do_MQTT_send_to_subscriber)
        mqtt_send_to_subscriber.grid(row=2, column=7,padx=5, pady=5, sticky='nsw')

        mqtt_publish_label = Label(mqtt_frame,text='Node will publish to this topic: ');
        mqtt_publish_label.grid(sticky=E+N+S, row=3, column=0, padx=5, pady=5)
        self.mqtt_publish_text = Text(mqtt_frame, width=20, height=1)
        self.mqtt_publish_text.grid(sticky=E+N+S+W, row=3, column=1, padx=5, pady=5)

        mqtt_output_label = Label(mqtt_frame,text='Output received from node:')
        mqtt_output_label.grid(sticky=E+N, row=4, column=0, padx=5, pady=5)

        self.mqtt_output_Text = Text(mqtt_frame, height=5)
        self.mqtt_output_Text.grid(row=4, column=1, columnspan=7,padx=5, pady=5, sticky='nsew')

        mqtt_output_Scrollbar = Scrollbar(mqtt_frame, command=self.mqtt_output_Text.yview)
        mqtt_output_Scrollbar.grid(row=4, column=8, sticky='nsw')
        self.mqtt_output_Text['yscrollcommand'] = mqtt_output_Scrollbar.set

        # MQTT quick commands
        self.mqtt_command_button_frame = Frame(mqtt_frame)
        


        # MQTT Connection management
        self.mqtt_button_frame = Frame(mqtt_frame)

        MQTT_connect_Button = Button(self.mqtt_button_frame, text='Connect MQTT', command=self.do_MQTT_connect)
        MQTT_connect_Button.pack(padx=5, pady=5, side=LEFT)

        MQTT_disconnect_Button = Button(self.mqtt_button_frame, text='Disonnect MQTT', command=self.do_MQTT_disconnect)
        MQTT_disconnect_Button.pack(padx=5, pady=5, side=RIGHT)

        self.mqtt_button_frame.grid(row=5, column=0, padx=5, pady=5, columnspan=8,sticky='nsew')

        rowCount = rowCount + 1

        # devicename_label = Label(self.frame,text='Device name: ');
        # devicename_label.grid(sticky=E+N+S, row=rowCount, column=0, padx=5, pady=5)
        # self.devicename_text = Text(self.frame, width=20, height=1)
        # self.devicename_text.grid(sticky=E+N+S+W, row=rowCount, column=1, padx=5, pady=5)

        # rowCount = rowCount + 1

        # self.accessPoints = []
        # self.passwords = []
        
        # wifi_frame = Frame(self.frame)

        # self.WiFi_SSID_buttons = []
        # self.WiFi_password_buttons = []
        
        # for wifi_count in range(0,self.no_of_wifi_configs):
        #     wifi_label = Label(wifi_frame,text='Access Point: ');
        #     wifi_label.grid(sticky=E+N+S+W, row=wifi_count, column=0, padx=5, pady=5)
        #     wifi_text = Text(wifi_frame, width=20, height=1)
        #     wifi_text.grid(sticky=E+N+S+W, row=wifi_count, column=1, padx=5, pady=5)
        #     c = wifi_count
        #     wifi_set_button = Button(wifi_frame, text='Update')
        #     wifi_set_button.bind('<Button-1>', self.ssid_set_button_pressed)
        #     wifi_set_button.grid(sticky=E+N+S+W, row=wifi_count, column=2, padx=5, pady=5)
        #     self.accessPoints.append(wifi_text)


        #     password_label = Label(wifi_frame,text='Password: ');
        #     password_label.grid(sticky=E+N+S+W, row=wifi_count, column=3, padx=5, pady=5)
        #     password_text = Text(wifi_frame, width=20, height=1)
        #     password_text.grid(sticky=E+N+S+W, row=wifi_count, column=4, padx=5, pady=5)
        #     self.passwords.append(password_text)

        # wifi_frame.grid(row=rowCount, column=0, padx=5, pady=5, columnspan=2,sticky='nsew')

        # rowCount = rowCount + 1

        self.reset_values()

        # azure_endpoint_label = Label(self.frame,text='Azure endpoint: ');
        # azure_endpoint_label.grid(sticky=E+N+S, row=rowCount, column=0, padx=5, pady=5)
        # self.azure_endpoint_text = Text(self.frame, width=20, height=1)
        # self.azure_endpoint_text.grid(sticky=E+N+S+W, row=rowCount, column=1, padx=5, pady=5)
        # rowCount = rowCount + 1
        
        # key_label = Label(self.frame,text='Key: ');
        # key_label.grid(sticky=E+N+S, row=rowCount, column=0, padx=5, pady=5)
        # self.key_text = Text(self.frame, width=20, height=1)
        # self.key_text.grid(sticky=E+N+S+W, row=rowCount, column=1, padx=5, pady=5)
        # rowCount = rowCount + 1
        
        # output_label = Label(self.frame,text='Output:')
        # output_label.grid(sticky=E+N, row=rowCount, column=0, padx=5, pady=5)

        # self.output_Text = Text(self.frame, height=5)
        # self.output_Text.grid(row=rowCount, column=1, padx=5, pady=5, sticky='nsew')

        # output_Scrollbar = Scrollbar(self.frame, command=self.output_Text.yview)
        # output_Scrollbar.grid(row=rowCount, column=2, sticky='nsew')
        # self.output_Text['yscrollcommand'] = output_Scrollbar.set

        # rowCount = rowCount + 1
       
        # program_button_frame = Frame(self.frame)

        # saveButton = Button(program_button_frame, text='Save Settings', command=self.do_save_settings)
        # saveButton.grid(sticky='nsew', row=0, column=0, padx=5, pady=5)

        # loadButton = Button(program_button_frame, text='Load Settings', command=self.do_load_settings)
        # loadButton.grid(sticky='nsew', row=0, column=1, padx=5, pady=5)

        # scanButton = Button(program_button_frame, text='List access points', command=self.do_scan_access_points)
        # scanButton.grid(sticky='nsew', row=0, column=2, padx=5, pady=5)

        # runProgramButton = Button(program_button_frame, text='Connect to network', command=self.do_connect_to_network)
        # runProgramButton.grid(sticky='nsew', row=0, column=3, padx=5, pady=5)

        # stopProgramButton = Button(program_button_frame, text='List settings', command=self.do_list_settings)
        # stopProgramButton.grid(sticky='nsew', row=0, column=4, padx=5, pady=5)

        # randomProgramButton = Button(program_button_frame, text='Preset settings', command=self.do_preset_settings)
        # randomProgramButton.grid(sticky='nsew', row=0, column=5, padx=5, pady=5)

        # newProgramButton = Button(program_button_frame, text='Connect serial', command=self.do_direct_connect)
        # newProgramButton.grid(sticky='nsew', row=0, column=6, padx=5, pady=5)

        # program_button_frame.grid(row=rowCount, column=0, padx=5, pady=5, columnspan=2)
        # rowCount = rowCount + 1

        # self.serial_button_frame = Frame(self.frame)

        # connectSerialButton = Button(self.serial_button_frame, text='Connect Controller', command=self.do_connect_serial)
        # connectSerialButton.pack(padx=5, pady=5, side=LEFT)
        
        # disconnectSerialButton = Button(self.serial_button_frame, text='Disconnect Controller', command=self.do_disconnect_serial)
        # disconnectSerialButton.pack(padx=5, pady=5, side=RIGHT)
        
        # self.serial_button_frame.grid(row=rowCount, column=0, padx=5, pady=5, columnspan=2,sticky='nsew')
        # self.set_serial_status_state(False)

        # rowCount = rowCount+1

        # self.status_label = Label(self.frame,text="Status")
        # self.status_label.grid(row=rowCount, column=0, columnspan=5,sticky='nsew')

        # root.update()
        # # now root.geometry() returns valid size/placement
        # root.minsize(root.winfo_width(), root.winfo_height())        

        # self.do_tick()

        #self.do_connect_serial()

root=Tk()
editor=NodeConfig(root)
root.mainloop()


