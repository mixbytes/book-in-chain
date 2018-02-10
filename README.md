# book-in-chain
Open research prototype of booking system, based on EOS blockchain engine

## build preparations
  install EOSIO dev tools&&programs (for ubuntu 16.xx):
  ``` bash
  git clone https://github.com/eosio/eos -b dawn-2.x --recursive
  cd eos
  ./build.sh ubuntu
  cd build
  sudo make install
  ```

## build
  generate booking.abi file (if booking.hpp changed):
  ``` bash
  eoscpp -g booking.abi booking.hpp
  ```  
  build .wast file:
  ``` bash 
  eoscpp -o booking.wast booking.cpp
  ```
 ## deploy 
 https://github.com/EOSIO/eos/wiki/Smart%20Contract#6-deploy-and-update-smart-contract
