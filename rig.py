import serial
import time
import os
from datetime import datetime
import subprocess

from motor_controller import StepperMotorController
motor = StepperMotorController()


#arduino serial
if os.path.exists('/dev/ttyACM0'):
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
else:
    ser = serial.Serial('/dev/ttyACM1', 9600, timeout=1)

time.sleep(2)

today = datetime.now().strftime("%m%d%Y") #main folder is date of experiments
base_dir = os.path.join(os.getcwd(), today)
os.makedirs(base_dir, exist_ok=True)

#--------------------------------------------------------
#VARIABLES

TRIALS = 3
ODORS = 8
NUM_LOCUSTS = 6 #adjust if adding/removing locusts
PAIRS = int(NUM_LOCUSTS/2)


#adjust if adding/removing locusts
#left camera always has odd, right has even (just for naming, doesn't affect logic)
LEFT_CAM_LOCUSTS = [1, 3, 5]
RIGHT_CAM_LOCUSTS = [2, 4, 6]

#timing variables
PRE_ODOR = 2
ODOR_ON = 4
POST_ODOR = 2
VIDEO_DURATION = 1 + PRE_ODOR + ODOR_ON + POST_ODOR


#rig movement variables
total_steps = 184245 #this was premeasured
endBuffer = 150
usableSteps = int(total_steps - (2*endBuffer))
left_limit = endBuffer
right_limit = total_steps - endBuffer
middle = int(usableSteps/2)

move_interval = int(usableSteps/(PAIRS - 1))


#----------------------------------------------------
#FUNCTIONS

def record_trial(odor, trial, pair):
    odor_dir = os.path.join(base_dir, f"odor_{odor+1}")
    os.makedirs(odor_dir, exist_ok=True)
    
    #current locusts
    left_locust = f"locust_{LEFT_CAM_LOCUSTS[pair]}"
    right_locust = f"locust_{RIGHT_CAM_LOCUSTS[pair]}"

    #make locust dirs - separate folder for each locust
    left_locust_dir = os.path.join(odor_dir, left_locust)
    os. makedirs(left_locust_dir, exist_ok=True)
    right_locust_dir = os.path.join(odor_dir, right_locust)
    os. makedirs(right_locust_dir, exist_ok=True)
    
    #video names for each locusts' folders
    #format ex: locust_2_trial_5
    filename0 = os.path.join(left_locust_dir, f"{left_locust}_trial_{trial+1}.mp4")
    filename1 = os.path.join(right_locust_dir, f"{right_locust}_trial_{trial+1}.mp4")
    print(f"Odor {odor}, Trial {trial+1}")
    
    
    #commands for each camera to record
    cmd0 = [ #left camera
        "rpicam-vid",
        "--camera", "0",
        "-t", str(VIDEO_DURATION * 1000),
        "-o", filename0
#         "--nopreview" #stops view of camera from popping up in terminal
    ]
    
    cmd1 = [ #right camera
        "rpicam-vid",
        "--camera", "1",
        "-t", str(VIDEO_DURATION * 1000),
        "-o", filename1
#       "--nopreview"

    ]

    
    proc0 = subprocess.Popen(cmd0) #commands to start videos
    proc1 = subprocess.Popen(cmd1)
    
    time.sleep(1+PRE_ODOR) #adjust timing for waiting before odor turns on
    
    ser.write(f"ODOR_ON {odor}\n".encode())
    time.sleep(ODOR_ON) #duration of odor
    ser.write(f"ODOR_OFF {odor}\n".encode())
    
    proc0.wait() #make sure videos finish and save
    proc1.wait()
    
    print(f"Saved videos {filename0} & {filename1}")


#move check for going to next pair of locusts
#only move left if it's not already at the left limit
def move_rig():
    current_position = int(motor.get_position())
    if (abs(int(current_position - left_limit)) >= 500):
        motor.move_left(move_interval) 


#-----------------------------------------------------
    
#MAIN LOOP

def main():    
    motor.home_to_center() #home rig position
    time.sleep(1)
    
    for odor in range(ODORS):
        for trial in range(TRIALS):
            motor.move_to(right_limit) #move to the first pair of locusts
            for pair in range(PAIRS):
                time.sleep(0.5)
                record_trial(odor, trial, pair)
    
                time.sleep(0.5)
                move_rig() #move to next pair
                
            motor.move_to(right_limit) #return to original locust pair - offset for rest time
            #this is for making sure the wait time isn't hovering on the original locust pairs - it'll be in between the two locusts so there is more consistency between trials
            

    motor.move_to(middle) #reset to middle after done (so rail isn't extended)
    print("All trials done")
    motor.close()


if __name__ == "__main__":
    main()
    
    
    