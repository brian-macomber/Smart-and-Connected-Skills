### Skill 28 - Leader Election

##### Brian Macomber
For the Leader election, I followed the bully algorithm and defined three states: Leader, Follower, Election. The Leader state is represented by green, the Follower state is represented by Red, and the Election state is represented by Blue. Between each ESP that is a part of the system, there are four parameters sent over UDP to ensure the leader election algorithm keeps running: 
- Status (either Leader or No Leader)
- ID (the ID of the ESP the message was sent from)
- Age (New if the ESP hasn't been recognized by any other ESPs in the system, Old if in the system) 
- Leader Heartbeat (Dead if the current ESP has not recieved message from the leader, Alive if it has recieved message from an ESP in the leader state)

Each ESP has the same code running on it, with the exception of the ESP ID that is unique to each ESP. First it connects to Wifi, then it startst o ping the other ESPs and wait for a response to go into the election state.

Below is the diagram for the finite state machine:
![state_model](/skills/cluster-4/28/images/State_Diagram.jpg)


#### Demo Video for Leader Election
<a href="https://www.youtube.com/watch?feature=player_embedded&v=geBb9610IHs" target="_blank">
<img src="https://img.youtube.com/vi/geBb9610IHs/0.jpg" 
    alt="Image Text" 
    width="240" height="180" 
    />
</a>