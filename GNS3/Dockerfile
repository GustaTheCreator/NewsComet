FROM ubuntu:20.04
RUN apt-get update && apt-get install -y nmap iputils-ping gcc gdb make net-tools traceroute netcat
RUN chmod 777 /home
WORKDIR /gns3volumes/home

# docker build -t diogu/news_container:1.0 .

# outros comandos úteis:

# docker --version                  - mostrar a versão instalada do Docker
# docker run -it -d {image name}    - criar um container a partir de uma imagem
# docker ps -a                      - mostrar todos os containers
# docker stop {container id}        - parar a execução de um container
# docker kill {container id}        - terminar a execução de um container imediatamente
# docker images                     - listar todas as Docker images existentes na máquina
# docker rm {container id}          - remover um container
# ocker rmi {image id}              - remover uma imagem da storage local
# docker build {path to dockerfile} - fazer o build de uma imagem a partir do Dockerfile