# RISC-V Disassembler

## Student Information
- **Name**: Benjamin Horowitz
- **ID**: 645002468
- **E-mail**: bhorowitz1@tulane.edu

## Notes
- This version does not account for whitespace within instructins (it skips over whitespace not in between instructions just fine).
- This version also does not account for any other characters. It assumes valid instructions.
- The following commands can be used to compile/run the project
- You must have CMake to compile this file with CMakeLists, also of course gcc but that's trivial
```bash
mkdir build
cd build
make
./riscv-sim ../test/test_full.txt  ../test/output.txt dis
```
