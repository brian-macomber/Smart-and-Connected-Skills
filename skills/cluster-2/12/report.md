### Skill 12 - Battery Monitor

##### Brian Macomber

For this skill I used code from the I2C display where I created a task that will display the global variable voltage. In
app main where I sample the voltage from the ADC input, the code loops 10 times with 100ms delay in each iteration (and prints the raw value), sums those sampled values and averages it. It is then converted to voltage and printed to the console. When this happens, the global variable called globalVoltage is given the value of voltage after the conversion, and the I2C displays that value continously. After all of this happens there is a 1 second delay to adhere to the requirements of the skill.

#### Schematic

![pic1](/skills/cluster-2/12/images/schematic.jpg)

#### Demo Video for Battery Monitor

<a href="https://www.youtube.com/watch?feature=player_embedded&v=NqzxDheR2n4" target="_blank">
<img src="https://img.youtube.com/vi/NqzxDheR2n4/0.jpg" 
    alt="Image Text" 
    width="240" height="180" 
    />
</a>
