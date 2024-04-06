# SolarRouterRMS

## Author
[F1ATB André](https://f1atb.fr/fr/)

This repository is just a technical tool to support the great work of [F1ATB André](https://f1atb.fr/fr/).

The original work can be found on [https://f1atb.fr](https://f1atb.fr/fr/routeur-photovoltaique-realisation-logicielle/).

## Description
SolarRouterRMS is a project that aims to create a solar-powered router monitoring system.  
It utilizes Arduino and other components to measure and monitor various parameters of the router, such as power consumption, temperature, and network status.

## Features
- Real-time monitoring of router power consumption/injection
- Solar power management for energy efficiency
- Power routing using phase cutting and/or relay

## Installation

### Linux

Intall Arduino Cli somewhere.

```bash
sudo -i
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/usr/local/bin/ sh
```

Make sure `arduino-cli` is in your `PATH`.

Next we will clone the repo and install the dependencies.
```bash
git clone https://github.com/quazardous/SolarRouterRMS.git`
cd SolarRouterRMS
make setup
```

Check and customize the file `.env.local`.

Plug in your ESP and run `arduino-cli board list` to list the available devices and update the `BOARD` env.

You may need to add your unix user in the `dialout` group.

NB: the default `CONFIG_FILE` assume your using Arduino IDE 2.x.

## Compile

```bash
make compile
```

## Upload

```bash
make upload
```

## License
This project is licensed under the [GPL 3.0]([LICENSE](https://www.gnu.org/licenses/gpl-3.0.html)).

## Responsibility
This project is provided as-is, without any warranties or guarantees of any kind.  
The project owner and contributors shall not be held responsible for any damages or liabilities arising from the use of this software.  
Users are solely responsible for their own actions and usage of the software.  
