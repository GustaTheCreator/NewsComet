<h1 align="center">
  Get the latest scoops from across the globe! 📰🌍
</h1>

<p align="center">
  <picture>
    <img src="res/logo.png" alt="NewsComet logo" width="200px">
  </picture>
</p>

<p align="center">
  NewsComet (NC) is an information hub simulator that allows journalists to submit pieces of news that are then distributed to users who have subscribed to their respective     topics. Network protocols such as TCP and UDP are explored.
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
  
    * The `.o` files will be found in the `bin` folder and the `.exe` in the root, you can also use the `Makefile` to remove them
      <br>
      
      ```
      make clean
      ```
 
* (Optional) Setup the GNS3 network
  ```
    To demonstrate how NewsComet works, a demo GNS3 project file is included in this repository, which uses the Cisco 2691 router image. Additionally, a Dockerfile is provided     to build the necessary containers used for running the program inside the GNS3 simulation.
  ```
  
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

  * Start one or more `news_admin.exe` to access admin actions
    ```
    ./news_admin.exe <ip> 9876
    ```

* Replace `<ip>` with the IP address of the device running the `news_server.exe` or use `localhost` if testing locally

