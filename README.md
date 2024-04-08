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

### Windows

Install [Arduino IDE 2.0](https://www.arduino.cc/en/software) (so you can share the `arduino-cli.yaml`).

Install [Git-Bash](https://gitforwindows.org/).

Install [Chocolatey](https://chocolatey.org/).
Open an admin PowerShell/Terminal and type the following command:
```
Set-ExecutionPolicy Bypass -Scope Process -Force; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
```

Install miscellaneous tools. Open an admin PowerShell/Terminal and type the following commands:
```bash
choco install make
choco install curl
choco install arduino-cli
choco install python3 --pre
```

Next we will clone the repo. Open Git-Bash and type the following commands:
```bash
cd
mkdir -p Arduino
cd Arduino
git clone https://github.com/quazardous/SolarRouterRMS.git`
cd SolarRouterRMS
make env
```

Check and customize the file `.env.local`.  
Plug in your ESP, open the device manager and check if drivers are OK.
If not OK install either [WCH340](http://www.wch-ic.com/downloads/CH341SER_EXE.html) or [CP21XX](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads).  
Run `arduino-cli board list` to list the available devices and update the `BOARD` env.

Finally we need to setup Arduino Cli dependencies.
```bash
make setup
```

#### Optionally

Use `git` with `ssh`: https://stackoverflow.com/questions/2643502/git-how-to-solve-permission-denied-publickey-error-when-using-git

Use **Git-Bash** from [Windows Terminal](https://learn.microsoft.com/windows/terminal/install): https://blog.shevarezo.fr/post/2022/02/23/ajouter-git-bash-windows-terminal  
Update the Terminal profiles (JSON), and add this in `profiles.list[]`:  
```json
{
    "acrylicOpacity": 0.75,
    "closeOnExit": "graceful",
    "colorScheme": "Campbell",
    "commandline": "C:/Program Files/Git/bin/bash.exe --login",
    "cursorColor": "#FFFFFF",
    "cursorShape": "bar",
    "experimental.retroTerminalEffect": false,
    "font": 
    {
        "face": "Consolas",
        "size": 10
    },
    "guid": "{14ad203f-52cc-4110-90d6-d96e0f41b64d}",
    "hidden": false,
    "historySize": 9001,
    "icon": "C:\\Program Files\\Git\\mingw64\\share\\git\\git-for-windows.ico",
    "name": "Git Bash",
    "padding": "0, 0, 0, 0",
    "snapOnInput": true,
    "tabTitle": "Git Bash",
    "useAcrylic": true
}
```

### Linux

Install [Arduino IDE 2.0](https://www.arduino.cc/en/software) (so you can share the `arduino-cli.yaml`).

Intall Arduino Cli somewhere.

```bash
sudo -i
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/usr/local/bin/ sh
```

Make sure `arduino-cli` is in your `PATH`.

Install Python3.

Next we will clone the repo.
```bash
git clone https://github.com/quazardous/SolarRouterRMS.git`
cd SolarRouterRMS
make env
```

Check and customize the file `.env.local`.  
Plug in your ESP and run `arduino-cli board list` to list the available devices and update the `BOARD` env.

You may need to add your unix user in the `dialout` group.

NB: the default `CONFIG_FILE` assume your using Arduino IDE 2.0.

Finally we need to setup Arduino Cli dependencies.
```bash
make setup
```

## Compile

```bash
make compile
```

## Upload

```bash
make upload
```

## IDE

Using Arduino IDE 2.0 should be straightforward. More on the original [post](https://f1atb.fr/fr/programmation-de-lesp32-application-au-routeur-photovoltaique/).

Arduino Cli allows us to use [Visual Studio Code](https://code.visualstudio.com/).  
You will need to install [Arduino Extension](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino).  
You may have to install a built in example (Ctrl-p + `>Arduino: example`) and/or toy with your `.vscode/c_cpp_properties.json` (Google it).

## License
This project is licensed under the [GPL 3.0]([LICENSE](https://www.gnu.org/licenses/gpl-3.0.html)).

## Responsibility
This project is provided as-is, without any warranties or guarantees of any kind.  
The project owner and contributors shall not be held responsible for any damages or liabilities arising from the use of this software.  
Users are solely responsible for their own actions and usage of the software.  
