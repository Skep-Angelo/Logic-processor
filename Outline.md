# Project Overview: 4-Bit Logic Processor Simulation

This project simulates a 4-bit computer using LTSpice. The workflow involves assembling input code, running the simulation, and interpreting the output.

## Key Components

- **4bit processor.asc**: LTSpice schematic file containing the circuit design for the 4-bit processor simulation.
- **rausAssembler**: Tool that converts `input.raus` (assembly-like code) into binary data, stored in the `dataLtspice` folder as input voltages for the simulation.
- **LTSpice Simulation**: Reads input voltages from `dataLtspice`, simulates the circuit, and outputs results to `4bit processor.txt`.
- **interpretOutput.py**: Python script that reads `4bit processor.txt`, filters and processes the data, and writes the final output to `Output.txt`.
- **Supporting Files**: Additional files that assist in the assembly, simulation, and interpretation processes.

## Workflow

1. Assemble input code (`input.raus`) into binary using rausAssembler.
2. Load binary data as voltages into LTSpice via `dataLtspice` folder.
3. Run LTSpice simulation on `4bit processor.asc`.
4. Export simulation results to `4bit processor.txt`.
5. Process and filter results with `interpretOutput.py` to generate `Output.txt`.