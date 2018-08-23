# Multi-Threading With Geant4 10.4

This reposity contains three examples how to interface Geant4 from your application and use its multi-threading capabilities:

* `g4-test-nomt`: No multi-threading, using standard `G4RunManager` for sequential execution of events
* `g4-test-g4mt`: Using the `G4MTRunManager` for Geant4-internal parallelization, seeting the number of threads from the command-line paramater
* `g4-test-ownmt`: Attempt to implement a threading independent from `G4MTRunManager` which gives the user more control over how and when events are executed.

## Compile

```bash
mkdir build && cd build/
cmake ..
make
```
