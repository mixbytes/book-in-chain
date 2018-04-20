#!/bin/bash


# WARNING !!! this script runs in fresh test eos installation
# it uses $EOS_DATA_DIR for wallet file, BE CAREFUL - IT CAN BE DELETED
# set -x

if [ -z ${EOS_DATA_DIR+x} ]; then
	echo "EOS_DATA_DIR variable is unset, I want to create new wallet file there";
	exit 10; 
else 
	echo "var is set to '$var'";
fi

cd $EOS_DATA_DIR

# start-kill
# if pgrep eosd; then
# 	pkill eosd;
# fi
# 
# bash -e \ 
# 	eosd 	--data-dir $EOS_DATA_DIR \
# 			--genesis-json=$EOS_DATA_DIR/genesis.json \
# 			--config=$EOS_DATA_DIR/config.ini \
# 			--skip-transaction-signatures &

# generate new temp_wallet.wallet file in $EOS_DATA_DIR and save password into $EOS_TEST_WALLET_PASSWORD

rm -f $EOS_DATA_DIR/temp_wallet.wallet
EOS_TEST_WALLET_PASSWORD=$(eosc wallet create -n temp_wallet | perl -wne '/\"([a-zA-Z0-9]{53})\"/ && print $1')
if [ -z ${EOS_TEST_WALLET_PASSWORD+x} ]; then
	echo "EOS_TEST_WALLET_PASSWORD variable is unset, wallet file creation in $EOS_DATA_DIR was unsuccessful.";
	exit 20;
else 
	echo "EOS_TEST_WALLET_PASSWORD is set to $EOS_TEST_WALLET_PASSWORD";
fi

# Creating and saving private and public keys into shell variables (to use in next scripts)

# EOS_TEST_TMP_OUTPUT=$(eosc create key)
# EOS_TEST_WALLET_PRIVATE_KEY=$(echo $EOS_TEST_TMP_OUTPUT | perl -wne 'print $1 if /Private\ key\:\s(\S+)/')
# EOS_TEST_WALLET_PUBLIC_KEY=$(echo $EOS_TEST_TMP_OUTPUT | perl -wne 'print $1 if /Public\ key\:\s(\S+)/')

# if [ -z ${EOS_TEST_WALLET_PRIVATE_KEY+x} ]; then
# 	echo "EOS_TEST_WALLET_PRIVATE_KEY variable is unset, wallet create key in $EOS_DATA_DIR was unsuccessful.";
# else 
# 	echo "EOS_TEST_WALLET_PRIVATE_KEY is set to $EOS_TEST_WALLET_PRIVATE_KEY";
# fi

# if [ -z ${EOS_TEST_WALLET_PUBLIC_KEY+x} ]; then
# 	echo "EOS_TEST_WALLET_PUBLIC_KEY variable is unset, wallet create key in $EOS_DATA_DIR was unsuccessful.";
# else 
# 	echo "EOS_TEST_WALLET_PUBLIC_KEY is set to $EOS_TEST_WALLET_PUBLIC_KEY";
# fi

# Creating booking account:
# "bash -e" exits if command did not returned exit status 0 (ok)

# Keys of 'inita' inside config.ini
EOS_TEST_WALLET_PUBLIC_KEY=EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
EOS_TEST_WALLET_PRIVATE_KEY=5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3

eosc wallet create -n temp_wallet $EOS_TEST_WALLET_PRIVATE_KEY
eosc wallet open -n temp_wallet || { echo 'eosc wallet open failed' ; exit 30; }
eosc wallet unlock -n temp_wallet --password $EOS_TEST_WALLET_PASSWORD || { echo 'eosc wallet unlock failed' ; exit 40; }
unset EOS_TEST_WALLET_PASSWORD

eosc wallet import -n temp_wallet $EOS_TEST_WALLET_PRIVATE_KEY || { echo 'eosc wallet import failed' ; exit 50; }
unset EOS_TEST_WALLET_PRIVATE_KEY





EOS_TEST_ACCOUNT_NAME=booking

if eosc get account $EOS_TEST_ACCOUNT_NAME; then
	echo "Account $EOS_TEST_ACCOUNT_NAME already exists";
else
	# account "inita" creates $EOS_TEST_ACCOUNT_NAME account ("booking")
	eosc create account inita $EOS_TEST_ACCOUNT_NAME $EOS_TEST_WALLET_PUBLIC_KEY $EOS_TEST_WALLET_PUBLIC_KEY || { echo 'eosc create account failed' ; exit 60; };
fi


cd $EOS_DATA_DIR/contracts
eosc set contract booking booking.wast booking.abi || { echo 'eosc set contract failed' ; exit 70; }
cd $EOS_DATA_DIR

echo "Contract successfully set"
exit 0;



# if pgrep eosd; then
# 	pkill eosd;
# fi




#deploy
#cd ~/book-in-chain
#eosc wallet open
#eosc wallet unlock --password {wallet_password}
#eosc wallet import {booking_private_key}


