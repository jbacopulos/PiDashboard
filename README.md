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
