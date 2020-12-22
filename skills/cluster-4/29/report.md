### Skill 29 - Security in Connected Systems

##### Brian Macomber

Sketch of the flow of information:
![info_flow](/skills/cluster-4/29/images/info_flow.jpg)

What ways can a bad guy attack your system? 5 specific ways:
- Spoof and Denial of Service attacks between the ESP and the node server could happen. A random person could send an overwhelming amount of "forward" messages to the ESP to make the car not able to stop. This could be solved by implementing something on the server side to look at all of the incoming data and set some threshold of data it wil accept at a given time to protect against a DoS attack.

- Since our UDP communication isn't secure, someone could gain access to the information sent from the ESP to the server, something known as packet sniffing. To mitigate this issue, we could use a more reliable form of communication, such as TCP which requires some sort of acknowledgement between the server and the client to ensure it is more secure.

- The system is run on a local subnet, so anyone who can access the subnet could control the car from the web client. To make this more secure, we could implement some sort of SSO login on the front end that only allows certain users to control the car.

- The server is running on a raspberry pi, so there is the chance that someone could access the pi remotely (through ssh) and take data from the database or control the car. To mitigate this we could use a strong username and password for the pi, or set up more security on the local network. This also coincides with someone gaining access to the local subnet, so a different Wifi security protocol such as WPA2 could be used.

- The location of the system is also rather unreliable, someone could replace the system's ESP with their own ESP and falisfy data and always have access to the system. This could be solved by using TCP communication and confirming that the ESP being used is the one known to the system, and any other hardware would not be able to communicate with the server.


