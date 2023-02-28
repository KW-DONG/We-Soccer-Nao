## 1 Introduction
### 1.1 Project statement
Based on robotics technology and artificial intelligence, RoboCup is a international robot soccer competition designed to stimulate public interest in them and bring about an advance in the study of them. There are three different kinds of robots size classes in Robocup competition: kidsize, teensize and adultsize. In the football competition, two teams advanced automatic robots compete with each other. This project researches kidsize soccer robot competition, which contains 4 players each team. This project focuses on the system of intelligent soccer robot in 3D and designs motion model and kicking strategy of Robocup. 

### 1.2 System Characteristics
The project designs a systematic RoboCup soccer robot system, including the inner communication subsystem, decision-making subsystem and motion control subsystem. As for inner communication subsystem, we build a 2D field of play with score, current game state and other competition information, which is based on OpenCV. And a robot called 'supervisor' has access to the location of every robot and the communication among each robot. Supervisor is a referee system based on Simspark. We develop the supervisor system by C++. 
### 
When it comes to decision-making subsystem, different events (‘throw in’, ‘corner kick’, ‘free kick’, ‘penalty kick’, etc.) are reacted according to different states (‘before kick off’, ‘kick off’, ‘play on’ and ‘kick ball’). The relationship among them shows in the flow chart below. The system calls the control function through communication mechanism (receiver, emitter, etc.) in Webots to control the robots and send the information to the referee system. We develop the system by C++.
### 
In terms of motion control subsystem, a robot who react or anticipate on the movement and intention of opponents is designed in purpose of efficiently performing postures like ball shielding and interception. The default robot based on the basic data in Webots is static. We adjust the data of joint posture of the robot in 3D when it moves and kicks.


## 2 Background
### 2.1 Define System Inputs & Outputs

###
2.1.1 

2.1.2 Related work (?)
We searched UT Austin Villa’s work. They focus on the joint posture of soccer robot kicking ball with the background of task-based control. The physical multiagent system simulator provided by SimSpark is empoyed and the visualization principle of the homogeneous Nao robot in soccer matches through joint perceptors and effectors is realized.

While after communicating with Euen, off-the-shelf physical engine in SimSpark is not permitted. So we chose to use Webots as our simulator.

The following five chapters respectively explain the agent moves its head in the perception system to monitor objects and records objects by the combination of the WorldObject array and visual memory; Use particle filter to solve the poser of localization; After transmitting different amount of bits through the 3D simulator, the corresponding state information was received by the agent; How to adjust the joint position through key frames to achieve open-loop skills, like getting up, goalie-diving and kicking; The robot falls are detected by the thresholding rule and the appropriate sequence of keyframes are used to move the joint back to its original position.

Chapter 9 discusses the goalie's two states in the game: assuming the role of onBall in front of the goal, and using Kalman Filter to react to the ball moving towards the goal. 
Chapter 10 introduces the design of the Walk Engine, which uses sigmoid function, sinusoidal functions, inverse kinematics and the PID controllers to control the joint position, and then uses CMA-ES algorithm to optimize the parameters.

In Chapter 11, the team formation and game mode are further studied and the optimal solution of dynamic programming for role assignment function is found.

In the following four chapters, we research the situation of the agent holding the ball, avoiding collision, as well as their movement and actions; Create a kicking engine based on OpenRAVE’s analytic inverse kinematics; Analyze the data difference of penalty kicks; Get the result of the game.


### 2.2 Project Planning
Everyone’s responsibility

### 2.3 Time & Cost Management



## 3 Modelling
### 3.1 Mathematical Model
### 3.2 Simulation Development
###

## 4 Control & Guidance
### 4.1 Control System Design
### 4.2 Navigation Sensor Representation
### 4.3 Guidance Algorithm
###

## 5 Algorithm Development
### 5.1 Behavioural Algorithm Development
###

## 6 Environment Development
### 6.1 Environmental Conditions
### 6.2 Visualisation
###

## 7 Summary
### 7.1 Final Product
### 7.2 Inadequacy of Proposed Project
### 7.3 Future Prospects

