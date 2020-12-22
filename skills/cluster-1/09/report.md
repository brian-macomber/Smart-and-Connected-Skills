### Skill 09 - Servos

##### Brian Macomber

To find the min and max pulse width, I started at around 1000us min and 2000us max, and slowly increased each one until the servo would not move any farther, or it was already rotating the full 180.

Eventually I found the min to be 450us and the max to be 2200us.

Wiring the servo to the ESP32:
![!pic1](/skills/cluster-1/09/images/servo_circuit.jpg)
