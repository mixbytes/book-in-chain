# book-in-chain
Open research prototype of booking system, based on EOS blockchain engine

## build preparations
  Install EOSIO dev tools&&programs (for ubuntu 16.xx, install in $HOME):
  ``` bash
  cd $HOME
  git clone https://github.com/eosio/eos -b dawn-2.x --recursive
  cd eos
  ./build.sh ubuntu
  cd build
  sudo make install
  ```
  manually add line ```export PATH=$PATH:$HOME/eos/build/install/bin``` to ```.bashrc``` file
  or execute 
  ``` bash
  # seeks string 'export PATH=$PATH:$HOME/eos/build/install/bin' in ~/.bashrc and adds it to the end of file if it's not found
  grep -q -F 'export PATH=$PATH:$HOME/eos/build/install/bin' $HOME/.bashrc || echo 'export PATH=$PATH:$HOME/eos/build/install/bin' >> $HOME/.bashrc
  ```
  Then apply ~/.bashrc changes by executing 
  ```
  source $HOME/.bashrc
  ```
  

## build booking contract
  clone repo:
  ``` bash
  git clone https://github.com/mixbytes/book-in-chain
  cd book-in-chain
  ```
  generate booking.abi file (if booking.hpp changed):
  ``` bash
  eoscpp -g booking.abi booking.hpp
  ```  
  build .wast file:
  ``` bash 
  eoscpp -o booking.wast booking.cpp
  ```
  As result you should get ```booking.wast``` file
  
## start eos node
  Set (and create if needed) the directory for EOS blockchain data. You can change it to suitable location, if needed
  ```
  EOS_DATA_DIR=/var/log/eos-data-dir
  sudo mkdir -p $EOS_DATA_DIR
  sudo chown ${USER:=$(/usr/bin/id -run)} $EOS_DATA_DIR
  ```
  Copying configuration file and genesis block
  ```
  cd $HOME
  cp book-in-chain/config.ini $EOS_DATA_DIR/config.ini
  cp eos/genesis.json $EOS_DATA_DIR/genesis.json
  ```
  run ```eosd``` daemon
  ```
  eosd -d $EOS_DATA_DIR --genesis-json=$EOS_DATA_DIR/genesis.json --skip-transaction-signatures
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
