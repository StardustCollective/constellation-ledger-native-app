# Constellation DAG Ledger Nano S Guide

## Introduction

Constellation DAG : A Different Approach To Blockchain

## Requirements

To use Constellation with a Ledger Nano S Device or a Ledger Nano X Device, you need a Ledger Nano S Device or a Ledger Nano X Device, and need access to the internet.

We recommend using the Constellation Javascript CLI:
[Constellation Javascript CLI](https://github.com/StardustCollective/constellationjs)

## Installation and Setup Instructions

Initial setup:

If you haven’t already, create a PIN for your Ledger and install Ledger Live Desktop from http://ledger.com or https://github.com/LedgerHQ/ledger-live-desktop/releases.
Don’t forget to back up your recovery phrase!

### Connect your Ledger device to your computer via USB and unlock it with your PIN.
Open Ledger Live Desktop, select “Manager” in the left panel, and install the Elastos application.

![Ledger App Manager](https://i.imgur.com/6IZJVE8.png)

### On your Ledger device, select the newly installed Constellation application.

![Constellation App Icon](https://i.imgur.com/b8mj3jY.jpg)

You should now see a home screen that looks like this:

![Constellation App Home Screen](https://i.imgur.com/MtaSbk3.jpg)

## Connect your Ledger to Constellation Javascript CLI.

### With your Ledger connected to your computer, pull up a command line interface that can run constellationjs.

## View Account Balance
### To view the account balance, run the below command.  

    npm start getlbalance

### the response should look like this:

    constellationjs
    address DAG4EqbfJNSYZDDfs7AUzofotJzZXeRYgHaGZ6jQ
    balance 1000000000000

## Receive Crypto
### To receive crypto, look on the device for the receiving address.

## Send Crypto
### To send crypto, run the below command.   

    npm start lsend 1 DAG6xXrv67rLAaGoYCaUe2ppBJMKsriUiNVzkJvv

### the response should look like this:

    constellationjs
    sendAmountUsingLedger 1 DAG6xXrv67rLAaGoYCaUe2ppBJMKsriUiNVzkJvv
    send success "17dd624b7e2c16587ecf50e9e1e8d6f14bc5ff3f0b345f5df9ba21d79ea79f0b"

### note
  Be sure to verify the the transaction details on the device!

## Support
If you have trouble, the best way to get help is to contact coranos2 on reddit.
[Coranos2 on Reddit](https://www.reddit.com/user/coranos2)

## How It was made
This guide was made according to the ledger third party application design guidelines.

### Ledger Third Party Application Design Guidelines
[Ledger Third Party Application Design Guidelines](https://ledger.readthedocs.io/en/latest/additional/publishing_an_app.html#design-guidelines)
