How to build on linux (tested on windows subsystem for linux:
Pull the project from github
Use "git submodule update --init --recursive" to get all submodules
Create a directory inside the main folder named "build
Inside build run "cmake ../" to create the make file
Run "make"
It will create an executable called "Virus.exe"

For building on Windows it is still WIP (If you find a way to please document and send a commit to explain how)

Config.json:
After running the program for the first time it will exit the program and you must open the config.json 
file created. Create an api key on e621 and insert the key and your username into the config file.
Additionally edit the tags and other values as much as you'd like. All time variables are in 
seconds.

This project has barely begun being made and it is a passion project I started as a way to learn programming
better. Any modifications or comments will be taken into consideration and I wish to improve on the code.

