# RIOT im Internet of Things, Sommersemester 2025

Welcome to the primary repository of the RIOT im Internet of Things project for the Summer 2025 semester at HAW Hamburg. The project was concerned with designing, implementing and presenting technologies to improve the city of Hamburg's methods of managing its harbor's floodgates.

## The Current Situation

Currently, the methods for ensuring floodgates are closed in the event of a flood are inefficient. Field workers receive orders, send updates and otherwise communicate with the central office via walkie-talkies. The central office relies on pen-and-paper record-keeping to log events, such as gates being opened or closed.

## Our Proposed Solution

The solution proposed and developed by our team is as follows: 
<ul>
  <li>Floodgates will be fitted with sensors, called "GateMates," which autonomously report the state of the gate (open or closed) to a server located in the central office.</li>
  <li>Field workers will be issued devices, called "SenseMates," which receive orders from the central office. SenseMates will also record the state of nearby gates by communicating with the GateMates, enabling workers to confirm or refute the state reported by the GateMate.</li>
  <li>The workers at the office will be given a web app with a user interface, allowing them to track the status of gates in real time, as well as providing a digital record of the status of gates and allowing them to issue orders without the need for walkie-talkie communication.</li>
</ul>

Our proposed solution replaces the inefficient methods currently used by the city of Hamburg with fast and reliable digital methods, saving crucial time in the event of a flood.

## GitFlow

See GitFlow here [GitFlow Document](/.github/GitFlow)

# Init git submodlues (RIOT) with
To init the submodules run submodules_linux.sh or submodules_windows.bat
You can also run these commands:
```
git submodule init
git submodule update
```
