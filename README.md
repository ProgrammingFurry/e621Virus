This program is made as a way to work as many computer viruses are depicted, 
opening e621 images at random intervals on your computer screen. While there 
is randomization at work, most options can easily be changed in the config 
file made after opening the program for the first time. 

This is NOT meant to be 
an actual virus nor is it meant to be used maliciously. The intent is either 
harmless fun and the basis for creating it was to experiment using JSON files, configs
and APIs in C++.

How to build on linux (tested on windows subsystem for linux, still WIP):
1. Pull the project from github
2. Use "git submodule update --init --recursive" to get all submodules
3. Create a directory inside the main folder named "build
4. Inside build run "cmake ../" to create the make file
5. Run "make"
6. It will create an executable called "Virus.exe"

For building on Windows it is still WIP (If you find a way to please document and send a commit to explain how)

Config.json:
After running the program for the first time it will exit the program and you must open the config.json 
file created. Create an api key on e621 and insert the key and your username into the config file.
Additionally edit the tags and other values as much as you'd like. All time variables are in 
seconds.

This project has barely begun and it is a passion project I started as a way to learn programming
better. Any modifications or comments will be taken into consideration and I wish to improve on the code.

All licences for submodules are written in their respective github links found in .gitmodules
All images are from e621 and while the program runs the links to the images are printed in the console
