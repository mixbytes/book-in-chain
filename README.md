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
  add ```export PATH=$PATH:$HOME/eos/build/install/bin``` to .bashrc and run ``` sourse $HOME/.bashrc ```
  

## build
  clone repo:
  ``` bash
  git clone https://github.com/mixbytes/book-in-chain
  ```
  generate booking.abi file (if booking.hpp changed):
  ``` bash
  eoscpp -g booking.abi booking.hpp
  ```  
  build .wast file:
  ``` bash 
  eoscpp -o booking.wast booking.cpp
  ```
  
## start eos node
  ``` bash
  cd ~
  sudo mkdir eos_data-dir
  sudo cp book-in-chain/eos/config.ini eos_data_dir/
  sudo cp eos/genesis.json eos_data_dir/
  eosd -d eos_data_dir/ --skip-transaction-signatures
  ```
 
## deploy
### create a wallet:
  ``` bash
  eosc wallet create
  ```
  save the wallet password
  
### generete key pair for booking account:
  ``` bash
  eosc create key
  ```
  save key pair
 
### create booking account:
  ``` bash
  eosc wallet open
  eosc wallet unlock --password {wallet_password}
  eosc wallet import 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3
  eosc create account inita booking {booking_public_key} {booking_public_key}
  ```
  
### deploy
  ``` bash
  cd ~/book-in-chain
  eosc wallet open
  eosc wallet unlock --password {wallet_password}
  eosc wallet import {booking_private_key}
  eosc set contract booking booking.wast booking.abi
  ```
  
  example: https://github.com/EOSIO/eos/wiki/Smart%20Contract#6-deploy-and-update-smart-contract
