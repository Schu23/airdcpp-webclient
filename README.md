# AirDC++ Web Client

AirDC++ Web Client is a cross-platform file sharing client for Advanced Direct Connect network. It has a web-based user interface that can be accessed with a web browser.

**The application isn't suitable for regular usage yet as there is no interface for configuring settings.**

## Compiling on Ubuntu

Ubuntu 14.04 or newer is required for installing the client.

### Install tools

`sudo apt-get install gcc g++ git cmake npm nodejs`

### Install libraries

`sudo apt-get install libbz2-dev zlib1g-dev libssl-dev libstdc++6 libminiupnpc-dev libnatpmp-dev libtbb-dev libgeoip-dev libboost1.5*-dev libboost-regex1.5*-dev libboost-thread1.5*-dev libboost-system1.5*-dev libleveldb-dev`

### Additional libraries

The websocketpp package is available only on Ubuntu 15.10 or newer.

`sudo apt-get libwebsocketpp-dev`

If the package isn't available for you, the following steps can be used to install it manually:

```
git clone git://github.com/zaphoyd/websocketpp.git
mkdir websocketpp/build && cd websocketpp/build
cmake ..
make
sudo make install
cd..
cd..
```

# Downloading the client

`git clone https://github.com/maksis/airdcpp-webui.git`

# Compiling

```
cd airdcpp-webui
cmake .
make -j4
```
This will compile the client with 4 simultaneous threads.
