# HDLIT

HDLIT is a lightweight C++ command-line utility that scans a Verilog or SystemVerilog source file and generates a basic module instantiation template to standard output. The goal is to make it easy to point the tool at a `.v` or `.sv` file and immediately get a clean DUT instantiation block that can be pasted into a testbench or redirected into a file.

### Installation
```bash
git clone git@github.com:ryancramuh/hdlit.git
or
git clone https://github.com/ryancramuh/hdlit.git

# and then run
cd hdlit && make
# then run
sudo make install
```

The __hdlit__ executable parent folder is hdlit/bin/

### Example use:

```bash
hdlit VER.sv # prints instantation template in terminal
hdlit VER.sv >> module_that_uses_VER.sv # >> concatenates
``` 

### Tutorial: 

Start with by defining a Verilog or SystemVerilog HDL source:

```verilog 
    `timescale 1ns/1ps
    module VER
    #(
        parameter x = 32;
    )(
        input clk,
        input rst,
        input [x-1:0] data_i,
        output logic [x-1:0] data_o
    );

    /*
        user-defined implementation
    */

    endmodule
```

Then run __hdlit__ 
``` bash
hdlit VER.sv # prints instantation template in terminal
```

```bash
VER #(
    .x(x)
) u_VER (
    .clk(clk),
    .rst(rst),
    .data_i(data_i),
    .data_o(data_o)
);
```
