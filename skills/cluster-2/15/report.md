### Skill 15 - Long Range IR Sensor

##### Brian Macomber

For this skill, I wired the IR sensor through an ADC input and used code from before to sample the input. I then referenced the plots on the data sheet for the sensor, and took some data points of my own to plot in Matlab. I then fit a line to the curve of the data and found an equation to solve for the distance (cm) from voltage (mV).

Note: It can be seen on the graph (in images folder) that the high and low ranges of the sensor are calculated with some error, so a parabola would be a better fit to this graph. I had some trouble implenting the equation of the parabola to solve to the distance, so I went with linear.

#### Schematic

![pic1](/skills/cluster-2/15/images/IR_rangefinder_circuit.jpg)

#### Demo Video for IR sensor

<a href="https://www.youtube.com/watch?feature=player_embedded&v=KPDJglq4Lmo" target="_blank">
<img src="https://img.youtube.com/vi/KPDJglq4Lmo/0.jpg" 
    alt="Image Text" 
    width="240" height="180" 
    />
</a>
