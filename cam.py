import RPi.GPIO as GPIO
import os
import time
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.IN, pull_up_down = GPIO.PUD_DOWN)
while True:
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; GPIO.wait_for_edge(18, GPIO.RISING)
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; print("RecordingVideo")
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; filename1 = time.strftime("%Y%m%d-%H%M%S") + '.h264'
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; os.system('raspivid -t 99999999 -o /home/pi/vids/' + filename1 + '&' )
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; time.sleep(1)
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; GPIO.wait_for_edge(18, GPIO.RISING)
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; print("Stopped")
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; os.system('pkill raspivid')
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; time.sleep(1)
GPIO.cleanup()




