NewsComet (NC) is an information hub simulator that allows journalists to submit pieces of news that are then distributed to users who have subscribed to their respective topics. This simulation also explores network protocols such as TCP and UDP.

To demonstrate how NewsComet works, a demo GNS3 project file is included in this repository, which uses the Cisco 2691 router image. Additionally, a Dockerfile is provided to build the necessary containers used for running the program inside the GNS3 simulation.

It's important to note that the demo project file and Dockerfile are optional components of the project, but they can be useful for anyone who wants to test and run the simulation locally.

## 🛠️ Installation

On Linux:

Using GCC to compile the C source file, simply run the ```$ make``` command and let the Makefile handle everything:

```
$ make
gcc -c news_server.c -lpthread -D_REENTRANT -Wall -Wextra -Wundef -Wshadow -Wswitch-default
gcc -o news_server.exe news_server.o -lpthread -D_REENTRANT -Wall -Wextra -Wundef -Wshadow -Wswitch-default
gcc -c news_client.c -lpthread -D_REENTRANT -Wall -Wextra -Wundef -Wshadow -Wswitch-default
gcc -o news_client.exe news_client.o -lpthread -D_REENTRANT -Wall -Wextra -Wundef -Wshadow -Wswitch-default
```

You can also run it to clean the compiled .o files with ```$ make clean```, if you no longer need them:

```
$ make clean
rm -f *.o
```
