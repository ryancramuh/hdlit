# HDLIT

HDLIT is a lightweight C++ command-line utility that scans a Verilog or SystemVerilog source file and generates a basic module instantiation template to standard output. The goal is to make it easy to point the tool at a `.v` or `.sv` file and immediately get a clean DUT instantiation block that can be pasted into a testbench or redirected into a file.

Example use:

```bash
./hdlit uart_rx.sv
./hdlit uart_rx.sv >> module_that_uses_uart_rx.sv # >> concatenates