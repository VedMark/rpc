# prc
Application that implements simple rpc communication.

## Getting Started

### Prerequisites
  - C++ compiler supporting the C++11 standard
  - rpcbind
  
### Installing

1. Clone project.
2. Make stubgen.
3. Run stubgen
4. Make server
5. Make client

```
git clone https://github.com/VedMark/rpc.git
cd rpc/stubgen
cmake CMakeLists.txt
make
./stubgen maths.x
cp maths.h maths_client.cpp ../client
cp maths.h maths_server.cpp ../server
cp ../server
[edit maths_server.cpp if needed]
cmake CMakeLists.txt
make
cp ../client
cmake CMakeLists.txt
make
```

### Running

```
[console 1]:
cd rpc/server
./server

[console 2]:
cd rpc/client
./client <hostmane>
```
