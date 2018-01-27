# README #

Here is an example of my program for receiving Infra Red (IR) commands from an Infra Red (IR) remote controller,saving them and sending these commands through IR LED to destination device.
What was special in this project?
The device was made very little, and did not have much space for components, so here only microprocessor in SSOP package without any addition memory chip.
But, according to requirement, it was necessary to keep 100 very large commands. 
In general, this device should remote on an air conditioners, where IR commands are very large and totally different on different companies.
So for keeping so much information, and trying make the device very universal, I solved this not trivial problem that way:
Device receive command, identify it to group (according to type), packing it in short format (my own invention) and store it to microprocessor program memory.
This device is remoted by UART bus with a typical AT commands interface, and has external remoting through a radio channel and works into a smart home system.
Now it can identify and keep all type of IR command (at least for air conditioners)  

