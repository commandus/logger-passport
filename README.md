# logger-passport

logger-passport is a library and command line tools intended for read passport of temperature
logger.

This library is a part of [lorawan-network-server](https://github.com/commandus/lorawan-network-server).

See also [logger-huffman](https://github.com/commandus/logger-huffman) repository:


```
git clone git@github.com:commandus/logger-passport.git
```
or
```
git clone https://github.com/commandus/logger-passport.git
```

## Usage

### Library

Static library liblogger-passport.a (-l logger-passport).

There are two way to use a library:

- include headers with types (first way)
- include only one "void *" type header (second way)

#### First way

Header file: logger-sql-clause.h

Optional headers:

- logger-collection.h
- logger-passport.h
- util-compress.h
- util-time-fmt.h

Source code example:

```
#include "logger-sql-clause.h"
...
// create table clause 
std::string r = createTableSQLClause(OUTPUT_FORMAT_SQL, dialect);
...
// load packet
LoggerKosaCollection c;
std::vector<std::string> s;
    s.push_back(hex2binString(packet0));
    LOGGER_PACKET_TYPE t = c.put(s);

...
// insert into clause
r = parsePacketsToSQLClause(OUTPUT_FORMAT_SQL, dialect, *c.koses.begin());

```

#### Second way

Header file: logger-parse.h

Source code example:

```
#include "logger-parse.h"
...
void *env = initLoggerParser();

sqlCreateTable(SQL_POSTGRESQL); // SQL_POSTGRESQL - 0
parsePacket(env, binaryDataString));

std::vector <std::string> clauses;
sqlInsertPackets(env, clauses, dialect);
for (auto it(clauses.begin()); it != clauses.end(); it++) {
    std::cout << *it << std::endl << std::endl;
}

flushLoggerParser(env);
doneLoggerParser(env);
```

##### Calls

First you need create a descriptor by initLoggerParser() call. It allocates
memory to store received packets;

Call parsePacket() save packet in the memory.

Lora device send 5-10 packets in about 100-300 seconds.

Packets are collected in a memory. When last packet has been
received, it can be stored in the database and removed from the memory.

When some packets are missed especially first ones, it is impossible to
assembly them in the database record.

In this case, memory must be free implicitly by sqlInsertPackets() or parsePacket() calls
or explicitly by flushLoggerParser() call. These calls check received time and if it
is later than 5 minutes, packets trying to store in the database(if sqlInsertPackets()
called) then it removes from the memory.

- initLoggerParser() return an descriptor
- sqlCreateTable() return "CREATE TABLE .." SQL statement string
- parsePacket() trying to parse received packet. Packets stored in memory.  
- sqlInsertPackets() find completed packets
- flushLoggerParser()
- doneLoggerParser()

### Command line tools

- logger-passport-print get packets in hex and print out temperature values

#### logger-passport-print

logger-passport-print utility prints packet data tos stdout.

Pass packet as hex string in command line:
```
./logger-passport-print <file>
```

or pass binary data:

```
cat packet-data.bin | ./logger-passport-print
```

## Build

You can use

- Automake
- CMake

build system.

Make sure you have automake installed:
```
apt install autoconf build-essential libtool
```

### Automake/Autotools

Generate automake files, configure and make:
```
cd logger-passport
autogen.sh
```

Configure and make project using Autotools:
```
cd logger-passport
./configure
make
```

If you prefer use clang instead of gcc:

```
./configure CC=clang CXX=clang++
```

You can install library and utilities in the system:
```
sudo make install
```

### Cmake

On Windows, you need install vcpkg. Do not forget integrate vcpkg with Visual Studio:

```
.\vcpkg\vcpkg integrate install
```

Then build solution:

```
mkdir build
cd build
cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=C:/git/vcpkg/scripts/buildsystems/vcpkg.cmake ..
```


On Linux you can use CMake instead of Automake:

```
cd logger-passport
mkdir build
cd build
cmake ..
make
```

If you prefer use clang instead of gcc:
```
cd logger-passport
mkdir build
cd build
export CC=/usr/bin/clang;export CXX=/usr/bin/clang++;cmake ..
make
```

### References

- [lorawan-network-server](https://github.com/commandus/lorawan-network-server)
- [lorawan-ws](https://github.com/commandus/lorawan-ws)
- [logger-huffman](https://github.com/commandus/logger-huffman)
