## 1 Introduction
### 1.1 Project statement
Based on robotics technology and artificial intelligence, RoboCup is a international robot soccer competition designed to stimulate public interest in them and bring about an advance in the study of them. There are three different kinds of robots size classes in Robocup competition: kidsize, teensize and adultsize. In the football competition, two teams advanced automatic robots compete with each other. This project researches kidsize soccer robot competition, which contains 4 players each team. This project focuses on the system of intelligent soccer robot in 3D and designs motion model and kicking strategy of Robocup. 

### 1.2 System Characteristics
The project designs a systematic RoboCup soccer robot system, including the inner communication subsystem, decision-making subsystem and motion control subsystem. As for inner communication subsystem, we build a 2D field of play with score, current game state and other competition information, which is based on OpenCV. And a robot called 'supervisor' has access to the location of every robot and the communication among each robot. Supervisor is a referee system based on Simspark. We develop the supervisor system by C++. 
### 
When it comes to decision-making subsystem, different events (‘throw in’, ‘corner kick’, ‘free kick’, ‘penalty kick’, etc.) are reacted according to different states (‘before kick off’, ‘kick off’, ‘play on’ and ‘kick ball’). The relationship among them shows in the flow chart below. The system calls the control function through communication mechanism (receiver, emitter, etc.) in Webots to control the robots and send the information to the referee system. We develop the system by C++.
### 
In terms of motion control subsystem, a robot who react or anticipate on the movement and intention of opponents is designed in purpose of efficiently performing postures like ball shielding and interception. The default robot based on the basic data in Webots is static. We adjust the data of joint posture of the robot in 3D when it moves and kicks.

### 1.3 Operational Costs
According to the creteria, the staff costs have been set at £100 per hour for this project. In addition, the cost of additional expert help have been set at £1000 per hour (this does not include consultation at weekly meetings).

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
Wihthin first 7 weeks, we scheduled our team meeting between 3pm and 5pm on Monday and 5pm-6pm on Thursday. After communicating with the supervisor, we confirmed that the time of lab meeting was 5 to 6 in the afternoon every Friday. In the following 4 weeks, we arranged our team meeting from 5pm to 6pm every Monday and lab meeting with the supervisor 4:00-5:00 Tuesday.

We divided our project into seven stages: specification, requirements, modelling, control & guidance, algorithm development, environment development and operation. 

(Time of every stage !)

In the first stage specification, we broke the work down into for parts: project statement, system characteristics, operational costs and summary. The fisrt part is project statement, during which we analysed mark scheme and analysed desired functions and scheduled timetable crash. As for mark scheme analysis, We solved the question of key stages that affect the total score. When it comes to desired functions analysis, we worked on what is the desired outcome from the document. And we confirmed that the solution is player movements (run, fall down, get up, shoot, pass), football information (size, direction, speed), football pitch information (size, colour, goal), display 1, 2, 3 points in 3D (2D) and play autonomously according to the rules of football. In terms of timetable crash, we looked into whether the project schedule conflict with other courses. The second part is system characteristics. In this part we studied the Robocup rule, robot characteristics and software & libraries. When studying the robocup rule, we searched the rule of the football game and the official Robocup rule in support of the rule of our project. While looking into robot characteristics, we discussed which sensors and actuators can be used and what the specifications and limits are. As regards software & libraries, we confirmed how many open source libraries and demos can be used. The following part is operational costs, containing additional hardware and additional software. We researched if there is any additional hardware and any additional payied software required. The final part is summary, which consists of critical design reviews and lab meeting summary. Critical design reviews are reviews of previous work in the team meeting, while lab meeting summary is the minutes of the lab meeting.

The second stage is requirements, including system inputs &outputs defining, project planning, time & cost management and summary. System inputs &outputs involves simulation inputs, simulation outputs and visualiser. Simulation inputs are the inputs of the simulator. Simulation outputs are composed of server I&O and robot I&O, explaining the information that the robot need to get. Visualiser is the information which should be visualised. Project planning consists of version control and gantt chart. Version control is to create and manage a github repository( https://github.com/KW-DONG/We-Soccer-Nao/tree/develop). Gantt chart shows below. Time & cost management includes personal time, team time and cost. We solved the time per person cost, the team time cost and the total cost.

The third satge is modelling, which comprises mathematical model, simulation development and summary. Mathematical model is made up of humanoid robot control algorithm, robot sensor model and kicking strategy. Regarding humanoid robot control algorithm, we tried to solve the problem that how the robots walk, stand up and kick. When modelling robot sensorow, we determined the state and location of robots with sensors. Kicking strategy is the ways to design the kicking strategy and the robot functions needed. Simulation development is devided into three sections: setup Simspark simulator, choregraphy setup and soccer field modifing.

The following stage is control & guidance, consisting of control system designing, navigation sensor representation, guidance algorithm and summary. Open loop walking skill, close loop walking skill, get up skill,dive skill and kick ball skill are all contents of control system designing. While navigation snsor representation demonstrete referee-player communication, player position tracking and referee system. Guidance algorithm contains trajectory planning and robot steering. 

Next two stages are algorithm development and environment development. Algorithm development mainly involves behavioural algorithm development, wheras environment development with environmental conditions and visualisation included.

The last but not the least, in operation stage, we complete final product, integrate deliverables, peer review, an oral presentation and a written report. 

## 3 Modelling
### 3.1 Mathematical Model
We tried to derive the movement operators directly from the vision data, estimate the ball distance and generate a movement command. This paper describes the robot motion control model and establishes the kinematic model and dynamic model. According to a variety of situations may occur in the competition, we divided it into offensive and defensive strategies. The strategies in offensive situation including kick-off attacking tactics, free kick attacking tactics, corner kick attacking tactics and penalty kick attacking tactics.

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

