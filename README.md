
# Node Mcu SoundBoard

A c programme that enables user to use NodeMcu(ESP8266)/any similar microcontroller with serial io capabilities to be used as a custom macropad/soundboard to inject audio input in other applications like- Discord or Google meet etc. This project aims to use Linux's pipewire audio server's graph based audio processing capabilities to integrate other applications. 




## Motivation

I was intrigued by the capabilities of streamdeck and how it can be used to inject custom sound bytes when paired with OBS and similar streaming software that accepts multiple sources. Applications like Discord provide userspace apis to be able to configure custom soundbytes to use during calls. But those are limited to a few selected inputs and limited to particular server. So i wanted  to have similar functionalities but with any programme that can accept audio input and with pipewire's graph based audio processing i can acheive that. 
## Using Pipewire

Because pipewire's graph and node based audio processing it enables users to link this programmes output to any other applications' audio input hence being able to use it outside of applications like OBS. 


## Current Project Status 

Currently this project is in development and  still requires some work to reach the first fully functional stage. Currently i am working on the programme's logic to integrate it with the pipewire deamon with the native c api. 
