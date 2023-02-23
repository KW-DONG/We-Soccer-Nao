Based on robotics technology and artificial intelligence, RoboCup is a international robot soccer competition designed to stimulate public interest in them and bring about an advance in the study of them. There are three different kinds of robots size classes in Robocup competition: kidsize, teensize and adultsize. In the football competition, two teams advanced automatic robots compete with each other. This project researches kidsize soccer robot competition, which contains 4 players each team. This project focuses on the system of intelligent soccer robot in 3D and designs motion model and kicking strategy of Robocup. To be specific, the project designs a systematic RoboCup soccer robot system, including the inner communication subsystem, decision-making subsystem and motion control subsystem. 
As for inner communication subsystem, we build a 2D field of play with score, current game state and other competition information, which is based on OpenCV. And a robot called 'supervisor' has access to the location of every robot and the communication among each robot. Supervisor is a referee system based on Simspark. We develop the supervisor system by C++. 
When it comes to decision-making subsystem, different events (‘throw in’, ‘corner kick’, ‘free kick’, ‘penalty kick’, etc.) are reacted according to different states (‘before kick off’, ‘kick off’, ‘play on’ and ‘kick ball’). The relationship among them shows in the flow chart below. The system calls the control function through communication mechanism (receiver, emitter, etc.) in Webots to control the robots and send the information to the referee system. We develop the system by C++.
![avatar](/Users/az/Desktop)
In terms of motion control subsystem, a robot who react or anticipate on the movement and intention of opponents is designed in purpose of efficiently performing postures like ball shielding and interception. The default robot based on the basic data in Webots is static. We adjust the data of joint posture of the robot in 3D when it moves and kicks.











