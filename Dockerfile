FROM ubuntu:20.04

WORKDIR /webserver

RUN apt-get update

RUN DEBIAN_FRONTEND=noninteractive\
  TZ=America/Sao_Paulo \
  apt-get install -y tzdata build-essential clang-12 valgrind php

RUN mv /usr/bin/clang++-12 /usr/bin/clang++ && \
  rm /usr/bin/c++ && \
  ln -sf /usr/bin/clang++ /usr/bin/c++ && \
  adduser --disabled-password webserver && \
  chown -R webserver:webserver /webserver


COPY --chown=webserver:webserver . .

USER webserver
