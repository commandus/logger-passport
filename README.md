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

## Files

Each file consists of one or more sensor plumes.

Each sensor has MAC address.

Usually sensor has 2 or 3 coefficients to spline temperature using polynomial: 

```
Tpoly = a * Traw + b + c
```
By default, coefficient C = 0.0 (when two coefficients are used).

Some sensors have 9 coefficients to spline temperature using polynomial in 3 temperature ranges:

- -273 .. -3 a, b, c
- [-3 .. 3] a1, b1, c1
- 3 .. 30] as is
- 30 .. max a2, b2, c2

```
Tpoly = a * Traw + b + c in range <-3C
Tpoly = a1 * Traw + b1 + c1 in range >=-3C and <= 3
Tpoly = Traw in range  >3 and <= 30
Tpoly = a2 * Traw + b2 + c2 in range >30
```

Each sensor has MAC address and 2, 3 or 9 coefficients to spline temperature using polynomial:

Passport file structure is:
```
<FILE> := {<PLUME>} NEND
<PLUME> := <PLUME_ID>{<SENSOR>} 
<PLUME_ID> := N<YEAR>-<N>
<SENSOR> := <MAC><A><B>[<C>] <LF> <A1><B1><C1> <LF> <A2><B2><C2>
<MAC> := hex
<A>, <B>... := double
<YEAR> := 00..99
<N> := 1..99
```


For instance passport file N10.txt
```
N10-1
# comment
289FBD9B0600008C	1,0117	-0,12379
NEND
```

## Library

Static library liblogger-passport.a (-l logger-passport).

There are two way to use a library:

- include headers with types (first way)
- include only one "void *" type header (second way)

### First way

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

### Second way

Header file: logger-parseText.h

Source code example:

```
#include "logger-parseText.h"
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

### Calls

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
- parsePacket() trying to parseText received packet. Packets stored in memory.  
- sqlInsertPackets() find completed packets
- flushLoggerParser()
- doneLoggerParser()

## Command line tools

- logger-passport-print get packets in hex and print out temperature values

### logger-passport-print

logger-passport-print utility 

- print packet data to stdout
- calculate corrected temperature for specified sensor(s)

For instance, calculate corrected temperature for sensor MAC address 2880E19B06000047
using password files in the tests/passport directory (with subdirectories) :
```
./logger-passport-print tests/passport -c 25.3750 -m 2880E19B06000047
25.3750
```

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
