<h1 align="center">
  Get the latest scoops from across the globe! 📰🌍
</h1>

<p align="center">
  <img src="res/logo_transparent_zoomed.png" alt="NewsComet logo" width="200px">
</p>

<p align="center">
  NewsComet (NC) is an information hub simulator that allows journalists to submit pieces of news that are then distributed to users who have subscribed to their respective     topics. This simulation explores network protocols such as TCP and UDP.
</p>
<p align="center">
  <i>Part of the final assignment for my Networks and Communications course in university.</i>
</p>

<p align="center">
  <picture>
    <img src="https://img.shields.io/badge/platform-linux-blue" alt="Supported platform">
  </picture>
</p>

<h2>
🛠️ Installation
</h2>

* Clone the repository
```
git clone https://github.com/Diogu-Simoes/NewsComet.git
```

* Open a terminal in the root directory of the repository

  * On Linux, run the `Makefile` to handle all the compiling with GCC
  ```
  make
  ```
  
* The object files can be found in the `bin` folder and the executables in the root
 
* (Optional) Setup the GNS3 network
<p>
  To demonstrate how NewsComet works, a demo GNS3 project file is included in this repository, which uses the Cisco 2691 router image. Additionally, a Dockerfile is provided     to build the necessary containers used for running the program inside the GNS3 simulation.
</p>
  
<h2>
🚩 Usage
</h2>

* Open a terminal in the root directory of the repository

  * Run the `news_server.exe`
  ```
   ./news_server.exe
  ```

  * Start one or more `news_client.exe` to interact with the system as a user
  ```
  ./news_client.exe <ip> 9000
  ```

  * Start one or more `news_config.exe` to access and modify admin settings
  ```
  ./news_config.exe <ip> 9876
  ```

* Replace `<ip>` with the IP address of the device running the `news_server.exe` or use `127.0.0.1` if testing locally

