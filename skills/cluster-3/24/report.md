### Skill 24 - PWM LED

##### Brian Macomber

For this skill, I used a regular LED wired up to GPIO 12, since it has an internal pullup resistor. I used the espressif/esp-idf starter code for ledc (led control) and modified it to allow the brightness to change based on user input or a cycle from 0% to 90%.

PWM LED circuit:
![pic1](/skills/cluster-3/24/images/control_led_circuit.jpg)

#### Demo Video for user input LED

<a href="https://www.youtube.com/watch?feature=player_embedded&v=HnHJPFHOS8g" target="_blank">
<img src="https://img.youtube.com/vi/HnHJPFHOS8g/0.jpg" 
    alt="Image Text" 
    width="240" height="180" 
    />
</a>

#### Demo Video for auto dimming LED

<a href="https://www.youtube.com/watch?feature=player_embedded&v=u6lxwSw9X9E" target="_blank">
<img src="https://img.youtube.com/vi/u6lxwSw9X9E/0.jpg" 
    alt="Image Text" 
    width="240" height="180" 
    />
</a>
