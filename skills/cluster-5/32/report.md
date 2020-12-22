### Skill 32 - Wheel Speed

##### Brian Macomber

For the wheel speed skill, I followed the circuit diagram from a refernce given on whizzer and wired up the sensor. I used ADC to sample the voltage and used conditionals to check when the voltage decreased, or when there was a change from white to black on the encoder pattern. Knowing the circumference of the wheel and how many black/white stripes were in the encoder pattern, I could count the number of pulses seen in a given time and calculate the velocity in m/s of the car.


Sensor Mounting
![pic1](/skills/cluster-5/32/images/wheel_speed_1.jpg)

Sensor Circuit
![pic2](/skills/cluster-5/32/images/wheel_speed_2.jpg)


#### Demo Video for Wheel Speed on the Crawler
<a href="https://www.youtube.com/watch?feature=player_embedded&v=6CIlQYsy6M4" target="_blank">
<img src="https://img.youtube.com/vi/6CIlQYsy6M4/0.jpg" 
    alt="Image Text" 
    width="240" height="180" 
    />
</a>

