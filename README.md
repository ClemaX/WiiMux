# WiiMux
Wiimote IR Multiplexer
Reports the position of a tracked point via UDP sockets.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

At this point of time, this software only runs on Linux, using the official BlueZ bluetooth stack and the CWIID library.
You also need make and gcc to be able to build this project.
To make use of this software you'll also need at least two Wiimote controllers.

```
apt-get install bluez libbluetooth-dev
apt-get install libcwiid-dev
apt-get install gcc make
```

### Installing

Once you have installed all the prerequisites, you can proceed with cloning.

```
git clone https://github.com/ClemaX/WiiMux.git
```

Then you just need to build the project with make.

```
make
```
The generated binaries will appear in the "bin" directory.

You can now execute the binaries.
```
cd bin
./wmclient 127.0.0.1 11000
```

You can log the connection with the "-v" switch or using netcat.
```
./wmclient 127.0.0.1 11000 -v
```
or
```
nc -ul 11000
```

## Running the tests

There is no test functionality yet.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Thanks to abstrakraft for providing the [CWiid library](https://github.com/abstrakraft/cwiid)
* Thanks to [Johnny Chung Lee](http://www.johnnylee.net/academic/)  for the [inspriration](http://www.johnnylee.net/projects/wii/)
* Thanks to PurpleBooth for providing this [readme template](https://gist.github.com/PurpleBooth/109311bb0361f32d87a2)
