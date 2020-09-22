# relay

Demo project for operating and monitoring electrical relays.
System/User controls relay by DO (Digital Output) line - "Control" and monitor its state by DI (Digital Input) line - "Feedback" (might be optional).
"ON"/"OFF" are DI/DO logical states which represent real voltage values on the lines and depend on relay and system configuration (for e.g. 12V (high) corresponds "ON" and  0V (GND, low) to
"OFF").

![](https://github.com/nkh-lab/relay/blob/master/doc/component_diagram.png)

## Relay state diagram
Describes relay states including error states as well: "ERROR_CONST_OPEN" - relay is constantly open and couldn't be closed, "ERROR_WELDED" - relay is welded and couldn't be open.

![](https://github.com/nkh-lab/relay/blob/master/doc/state_diagram.png)

## Relay sequence diagram
![](https://github.com/nkh-lab/relay/blob/master/doc/sequence_diagram.png)

## Simulation
Project provides simulation for correct and wrong modes to cover different test cases.
Log examples from simulation runs: ![logs](https://github.com/nkh-lab/relay/tree/master/logs)
