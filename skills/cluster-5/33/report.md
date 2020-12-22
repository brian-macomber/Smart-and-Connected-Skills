### Skill 33 - PID

##### Brian Macomber
For the PID skill, I wired up 3 LEDs to show the error: the red LED is on when error < 0, the green LED is on when the error = 0 (we are exactly at the setpoint), and the blue LED is on when error > 0. This means that when the Lidar is measuring distance with the setpoint at 50cm, it will be green at 50cm, red when greater than 50cm, and blue when less than 50cm.


PID/LED circuit
![pic1](/skills/cluster-5/33/images/pid_circuit.jpg)


#### Demo Video for PID
<a href="https://www.youtube.com/watch?feature=player_embedded&v=yWI2UXbnJ8k" target="_blank">
<img src="https://img.youtube.com/vi/yWI2UXbnJ8k/0.jpg" 
    alt="Image Text" 
    width="240" height="180" 
    />
</a>