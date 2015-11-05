Everstore is an high performance append-only event log database with ACID properties

## Installation

### Docker

#### Prerequisites

* docker
* curl

#### Build

Run the command below to build a docker image called "everstore". 

```
bash <(curl -s https://raw.githubusercontent.com/perandersson/everstore/master/install.sh)
```

#### Run

After the image has finished building then you can start it as a daemon process by using the command:

```
docker run -d -t --name=everstore -p 6929:6929 everstore:latest
```

The default port is 6929 and the default journal directory is located in /everstore/journals directory. 
You can expose this directory from the docker container by adding a -v flag (assuming that /everstore/journals is the journals directory):

```
docker run -d -t --name=everstore -p 6929:6929 -v /path/on/host:/everstore/journals everstore:latest
```

### CMake

### Prerequisites

* CMake 3.2 (or newer)
* On Linux: G++ 4.8 (or newer)
* On Mac: clang
* On PC: Visual Studio 2013 (or newer)

### Build

Download the source code from this GitHub repository and run the command:

```
cd /path/to/source/root
cmake .
make
```

### Run

The build files can be found in the bin directory. It's recommended to run the test-suite before actually using the server.

```
cd bin
./everstore-shared-test
```

You can then use it by running the command:

```
cd bin
./everstore-server
```

## Project requirements

* ACID
* Read-friendly event logs
* Fast
* Asynchronous
* Small memory footprint (max 2 MB per worker)
* Adapters for popular languages
* Conflict resolution

### The database must have ACID properties.

To be able to be competetive, the database must have ACID properties.

### Read-friendly event logs

Journals should be readable for non-technical people. It should also be possible to read the journals without any external programs.

### Fast

It should be low-latency and it should also be able to handle a lot of events simulatinously.

### Asynchronous

All new web-frameworks today support asynchronous request handling. Let's follow this trend.

### Small memory footprint

Why require massive amount of memory when it's not needed. 

### Adapters for popular languages

* Scala
* Java 8
* .NET

### Conflict resolution

It should be possible to resolve any conflicts occuring when committing a transaction. 

## Target platform

* Windows
* Linux (Shared posix mutex does not work correctly. Race condition occurs on very large load atm.)
* Docker

## ACID

### Atomicity

Everstore implements transactional behaviour for reading and writing to an event journal. By making use of the actual event types, we can ensure that types of the same type results in a conflict if they are saved at the same time. This also ensures that event types of different types (where the order between the events does not matter) do not result in a conflict.

### Consistency

The events are saved in a way that the server can repair itself from if a crash occurs or if the power is lost during a transactional write. The database automatically restores itself to the latest consistent state.

### Isolation

By making use of transactions and dedicated child-processes being responsible for different journals, we can ensure that issolation is achived between different journals or transactions.

### Durability

The server is split into two parts: The Host and the workers. The host is responsible for managing the workers - which is managed as child-processes for the host program. Due to the host being very small and performing almost no logic we lessen the risk of it crashing in runtime. 

Any database related logic is performed by the worker child-processes. When the host notices that one or more workers has crashed then it will begin the "restart" procedure:
* Kill the previous process and cleanup after it (if it's still running)
* Start the process once again
* Register any existing connections
* Repair any journals it's affacted during it's crash
* Start send traffic to it once again.

## What does a event log look like?

The actual content of the log depends on the adapter. The only requirement the server has on the event log is:
* No new-line characters
* No NULL characters (used as a delimiter between events)

An event-log row looks like this if we use the Scala adapter:

```
2015-08-02T16:23:02.580 examples.UserCreated {"username":"pa@speedledger.se"}
```

The first part is the timestamp when the "transaction" was committed. This one is always in UTC and is managed by the server. The second part is the event name. The Scala adapter uses the full-name of the event case class (as seen above). The last part if the data associated with the event. 

The server does not require the event row data to be JSON, it's up to the serialization mechanism in the adapter. The server saves whatever it receives from the adapter.

## Benchmarks

* CPU: Intel Core i7-4770 @ 3.4 GHz
* RAM: 16 GB 1600 Mhz
* HDD: Corsair SSD Force Series GS 240GB 2.5" SATA 6 Gb/s (SATA3.0), 555/525MB/s read/write, fast Toggle NAND

### 6000 journals (120 events / journal, 5 workers)

Peak Memory footprint:
```
everstore-worker.exe 1116 K
everstore-worker.exe 1116 K
everstore-worker.exe 1116 K
everstore-worker.exe 1116 K
everstore-worker.exe 1112 K
everstore-server.exe 824 K
```

VM-options: -Xmx1024M

```
EvtCount -> TimeInMS

720000 --> 1566
720000 --> 1387
720000 --> 1004
720000 --> 1015
720000 --> 1053
720000 --> 1168
720000 --> 1000
720000 --> 1083
720000 --> 998
720000 --> 1053
720000 --> 1014
720000 --> 1039
720000 --> 1010
720000 --> 1091
720000 --> 984
```

### 100 journals (12000 events / journal, 5 workers)

Peak Memory Footprint:
```
everstore-worker.exe 984 K
everstore-worker.exe 980 K
everstore-worker.exe 976 K
everstore-worker.exe 976 K
everstore-worker.exe 912 K
everstore-server.exe 823 K
```

VM-options: -Xmx1024M

```
EvtCount -> TimeInMS

1200000 --> 2147
1200000 --> 1380
1200000 --> 1376
1200000 --> 1336
1200000 --> 1570
1200000 --> 1218
1200000 --> 1338
1200000 --> 1411
1200000 --> 1199
1200000 --> 1297
1200000 --> 1281
1200000 --> 1249
1200000 --> 1455
1200000 --> 1252
```

