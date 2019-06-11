# Multi-Threading With Geant4 10.4

This reposity contains three examples how to interface Geant4 from your application and use its multi-threading capabilities:

* `g4-test-nomt`: No multi-threading, using standard `G4RunManager` for sequential execution of events
* `g4-test-g4mt`: Using the `G4MTRunManager` for Geant4-internal parallelization, seeting the number of threads from the command-line paramater
* `g4-test-ownmt`: Implements threading independent from `G4MTRunManager` which gives the user more control over how and when events are executed.

## Compile

```bash
mkdir build && cd build/
cmake ..
make
```

## Structure

### Geant4 Manager with no multithreading

`main_nomt.cpp` shows an example of using `G4RunManager` class. This class manages an event loop to execute a specified number of events. Each time the method `BeamOn` is called, the manager will initialize, run and terminate the event loop which run sequentially.

### Geant4 MultiThreaded Manager

`main_g4mt.cpp` shows an example of using `G4MTRunManager` class. This class inherits `G4RunManager` but it acts as a thread pool and is often identified as *master* run manager. The actual work is offloaded to instances of `G4WorkerRUnManager` class which are called *workers*.
The master will create the specified number of threads and for each thread a worker instance. A call to `BeamOn` will result in the creation of such threads and workers, initialization of each worker to share the same geometry and physics setup, the execution of the event loop distributed on the specified number of workers, and finally the termination and destructions of the threads and workers.
Each worker will perform the same work as what a normal `G4RunManager` is expected to do. The only difference is that it shares the geometry construction with the master manager and it will only execute part of the event loop. The master manager is he who determines the workload each worker will do. It also makes sure workers random number generators are seeded in a way so that ensures results can be reproduced.

### Custom Geant4 RunManager

`main_ownmt.cpp` shows an example of defining a custom run manager that doesn't control or spawn threads. On the otherhand, this manager works in accordance of a higher level framework that manages the event loop instead.
`SimpleMasterRunManager.hpp` defines an equivalent to `G4MTRunManager` except that it doesn't spawn its own threads. It only maintains a set of `SimpleWokerRunManager.hpp` which are the equivalent of `G4WorkerRunManager` for each thread.
The master manager expects to be used by a framework that defines its own event loop and as such its own threads. To handle this case, the master manager manipulates the `G4MTRunManager` API behaviour in a way that associates a worker manager for each calling thread. The event loop of the run manager is initialized early on, before calling `BeamOn` and a new method `Run` is defined which in turn call the `BeamOn` method on each worker.


