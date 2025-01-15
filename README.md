# Installation and Usage
Clone the repository to your Raspberry Pi using:

`git clone https://github.com/jbacopulos/PiDashboard.git`

Install dependencies:

```
sudo apt update
sudo apt install -y build-essential cmake qt5-default qttools5-dev-tools libqt5network5 libqt5xml5
```

Create build directory:

```
cd PiDashboard
mkdir build
cd build
```

Generate the build files:

`cmake ..`

Build and install:

```
make -j$(nproc)
sudo make install
```

The executable will be placed in /usr/local/bin

To use, rename the .env.template file to .env and edit to have the following parameters (don't use any quotes):

```
ZIP=Your zip code followed by a comma and the country code - ex: 07008,US or H1A,CA
UNIT=metric or standard or imperial
OW_API_KEY=Get an openweathermap API key
W_API_KEY=Get a weatherapi API key
```

Then run the application with /usr/local/bin/PiDashboard .env
