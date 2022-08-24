# Build the application

Ledger App Builder is a container image which holds the all dependencies to compile an application for Nano hardware wallets.

You need to clone it with:

```
git clone https://github.com/LedgerHQ/ledger-app-builder 
```

## 1. Build the container

The container can be build using standard tools. Navigate to the ledger-app-builder directory and run the following command:

```
sudo docker build -t ledger-app-builder:latest .
```

## 2. Compile your app in the container

Compile binaries for each of the three Ledger device types: Nano S, Nano X, and Nano S Plus. Move each binary to a `builds` directory in order to make it available to 
your local environment.

In the source folder of the application run the following. Replace ‘realpath’ with your app’s local path (ex: /Users/john/constellation-ledger-native-app:/app)
```
$ sudo docker run --rm -ti -v "$(realpath .):/app" ledger-app-builder:latest
root@656be163fe84:/app# mkdir builds
```

### For the Nano S

```
root@656be163fe84:/app# make clean
root@656be163fe84:/app# make
root@656be163fe84:/app# mv bin/app.elf builds/app_s.elf
```

### For the Nano X and Nano S Plus

For Nano X and S Plus, specify the BOLOS_SDK environment variable before building your app, in the source folder of the app:

Nano X
```
root@656be163fe84:/app# make clean
root@656be163fe84:/app# BOLOS_SDK=$NANOX_SDK make
root@656be163fe84:/app# mv bin/app.elf builds/app_x.elf
```

Nano S Plus
```
root@656be163fe84:/app# make clean
root@656be163fe84:/app# BOLOS_SDK=$NANOSP_SDK make
root@656be163fe84:/app# mv bin/app.elf builds/app_sp.elf
```

You can now exit the build container. Your builds should be available locally in the `builds` directory. 

## Run the tests

1. Pre-requisite: Docker must be installed on local environment
2. ledger-app-builder container must be built.

### Functional Tests

1. Exit the build container and in your local environment navigate to the `tests/functional` and run `npm i` to install node modules.

2. Run `npm test`.

### Unit Tests

Run the following (replace ‘realpath’ with your app’s local path):

```
$ sudo docker run --rm -ti -v "$(realpath .):/app" ledger-app-builder:latest
root@656be163fe84:/app# cd tests/unit
root@656be163fe84:/app# cmake -Bbuild -H.
root@656be163fe84:/app# make -C build 
root@656be163fe84:/app# cd build/ && ctest -V
```

