import serial
import time
import os

if os.path.exists('/dev/ttyACM0'):
    PORT = '/dev/ttyACM0'
else:
    PORT = '/dev/ttyACM1'

total_steps = 184245
endBuffer = 150
usableSteps = int(total_steps - (2*endBuffer))
left_limit = endBuffer
right_limit = total_steps - endBuffer
middle = int(usableSteps/2)
PAIRS = 3

move_interval = int(usableSteps/(PAIRS - 1))

class StepperMotorController:
    def __init__(self, port=PORT, baudrate=115200, timeout=1):
        self.ser = serial.Serial(port, baudrate, timeout=timeout)
        time.sleep(2)
        
    def send_command(self, command):
        """Send command to arduino"""
        self.ser.write(f"{command}\n".encode())
        while True:
            if self.ser.in_waiting > 0:
                response = self.ser.readline().decode().strip()
                if response.startswith("ERROR"):
                    raise Exception(response)
                return response
    def emergency_stop(self):
        self.ser.write(b"EMERGENCY_STOP\n")
        time.sleep(0.1)
        
    def _wait_for_completion(self, completion_msg):
        while True:
            if self.ser.in_waiting > 0:
                response = self.ser.readline().decode().strip()
                if response.startswith("ERROR"):
                    raise Exception(response)
                if response == completion_msg:
                    return True            
            
    def move_to(self, position):
        self.ser.write(f"MOVE_TO {position}\n".encode())
        return self._wait_for_completion("MOVEMENT_COMPLETE")
    
    def move_left(self, steps):
        self.ser.write(f"MOVE_LEFT {steps}\n".encode())
        return self._wait_for_completion("MOVEMENT_COMPLETE")
    
    def move_right(self, steps):
        self.ser.write(f"MOVE_RIGHT {steps}\n".encode())
        return self._wait_for_completion("MOVEMENT_COMPLETE")
    
    def home_to_center(self):
        self.ser.write("HOME_TO_CENTER\n".encode())
        return self._wait_for_completion("MOVEMENT_COMPLETE")

    def seek_left_limit(self):
        self.ser.write("SEEK_LEFT_LIMIT\n".encode())
        return self._wait_for_completion("MOVEMENT_COMPLETE")
    
    def seek_right_limit(self):
        self.ser.write("SEEK_RIGHT_LIMIT\n".encode())
        return self._wait_for_completion("MOVEMENT_COMPLETE")
    
    def set_position(self, position):
        return self.send_command(f"SET_POS {position}")
    
    def get_position(self):
        response = self.send_command("GET_POS")
        if response.startswith("POS "):
            return int(response[4:])
        return response
    
    def close(self):
        self.ser.close()

