# Node Mcu SoundBoard

A c programme that lets user to use NodeMcu(ESP8266)/any similar microcontroller with serial io capabilities to be used as a custom macropad/soundboard to inject audio input in other applications like- Discord or Google meet etc. This project aims to use Linux's pipewire audio server's graph based audio processing capabilities to implement this universal functionality that can work with any application. 


## Motivation

I was intrigued by the capabilities of streamdeck and how it can be used to inject custom sound bytes when paired with OBS and similar broadcasting software that accepts multiple sources of audio and video data. Applications like Discord provide special apis to be able to configure custom similar functionalities but those are limited to a few selected inputs and limited to particular server configuration and Discord's api limitations. So i wanted  to have similar functionalities but with any programme that can accept audio input and hence the beginning of this project.

## Using Pipewire

Because pipewire's graph and node based audio processing it enables us to pipe the ouptput and input of different application whichever way we like. This used to require the use of special sound server implementation like JACK on linux. Thus this project heavily uses the Pipewire deamons's c apis for creating custom audio producer nodes. 

## FFMPEG For Audio

This project need to be able to play/decode any kind of audio file(container) and be able to do that efficiently. FFMPEG's wide adoption and extensive api's makes it the perfect choice for audio decoding. FFMPEG having native c apis also helps. 

## Current Project 

Currently the first working functional prototype is ready ot be used with any application. Though in the current state it still requires a bit of manual configuration to able to connect with output application(to send the audio data). We need to provide the dev port path to the microcontroller. Though the main core functionalities and configuration parser is working perfectly to inject audio bytes in the target application. 

Current goals- 
1. Configure a Bash script to setup the udev rules to assign a specific path when connnecting the microcontroller over USB. 
2. Implementing the logic for auto linking with target applications using the pipewire apis.
3. A second confiugration parser to specify the target application along with the existing audio file mappings configuration. 
4. Analysis with Valigrind to detect to potential memoery leaks in some parts of the applicaiton. 5. Implementing the interupt to cancel a playback midway when receiving a new input from the microcontroller.

## Use of AI

For the most part this project is written by me. Though for certain scenarios where i needed help to research certain topics while developing the project or i needed help with debugging some bugs i have used llms like Gemini to help me out.
