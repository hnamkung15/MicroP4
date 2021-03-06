# μP4 

## Overview
μP4 enables programming data plane in a portable, modular and composable manner. 
It comprises 
1. μP4 Architecture (μPA) - a logical Architecture for data plane
2. μP4 Compiler (μP4C) - to compile data plane programs written against μPA 
3. μP4 Language - A subtle variant of [P4-16](https://p4.org/p4-spec/docs/P4-16-v1.2.1.html)

μP4 allows to build libraries of packet-processing functions in data plane, reuse 
them to develop new programs and compile the programs to architectures of real 
target devices. μP4 maps its logical architecture to real targets like v1model and Tofino.

Using μP4C, programmers can 
1. Compile code to libraries (in the form of `.json` files)
2. Generate P4-16 source (`.p4`) specific to a given target architecture (v1model or TNA)

μP4C-generated P4-16 programs should be used with target-specific P4-16 compiler backends to generate executables.


## Getting started
μP4C is developed by extending a [fork](https://github.com/hksoni/p4c) of the reference
[P4-16 compiler](https://github.com/p4lang/p4c). The forked repository is
added as a submodule inside [extensions/csa/msa-examples](https://github.com/cornell-netlab/MicroP4/tree/master/extensions/csa/msa-examples).
This repository also contains a version of BMv2 compatible with the forked p4c, and is available as a submodule at [extensions/csa/msa-examples](https://github.com/hksoni/behavioral-model/tree/ed0174d54fc12f28b3b7371a7613d6303143daea).

### 1. Install dependencies and download μP4
#### 1.1 Dependencies
The dependencies for μP4 are the same as those required for the reference P4 compiler. We list the steps here for Ubuntu 18.04:
```bash
sudo apt-get install cmake g++ git automake libtool libgc-dev bison flex libfl-dev libgmp-dev \
   libboost-dev libboost-iostreams-dev libboost-graph-dev llvm pkg-config python python-scapy \
   python-ipaddr python-ply tcpdump
```

Install `protobuf` version 3.2.0 as follows:
```bash
sudo apt-get install autoconf automake libtool curl make g++ unzip
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.2.0/protobuf-cpp-3.2.0.zip
unzip protobuf-cpp-3.2.0.zip
cd protobuf-3.2.0
./configure
make
make check
sudo make install
sudo ldconfig
```
If you encounter any error, see detailed instructions [here](https://github.com/hksoni/p4c#dependencies).



#### 1.2 Download
Clone this repository: 
```
git clone --recursive https://github.com/cornell-netlab/MicroP4.git microp4 
```

If you forgot `--recursive` or `--recurse-submodules`
```
cd microp4
git submodule update --init --recursive
cd ..
```

#### Setup Environment
Set up the environment variable for μP4 root directory:
```bash
export UP4ROOT=$PWD/microp4
```


### 2. Install 
The previous commands download the source code of μP4 along with `p4c` and `BMv2` as submodules.
To generate v1model-specific P4 source for μP4 programs, installing only μP4C is enough.
However, to compile and test the generated programs with mininet, we also need to BMv2 and `p4c`.

#### 2.1 Build and install p4c and BMv2
To run executables generated for BMv2, install BMv2 as follows:
```bash
cd ${UP4ROOT}/extensions/csa/msa-examples/bmv2
bash ./install_deps.sh
./autogen.sh
./configure 'CXXFLAGS=-O0 -g' --enable-debugger    # Mandatory for μP4, because I will need logs in error scenarios. :)
make
sudo make install  # if you need to install bmv2
sudo ldconfig # for linux
```

To compile generated P4-16 programs, install `p4c` as follows.
```bash
cd ${UP4ROOT}/extensions/csa/msa-examples/p4c
mkdir build
cd build
cmake ..   # (optional add the debug flag) -DCMAKE_BUILD_TYPE=DEBUG  
make -j2
```

#### 2.2 Build and install μP4C
```bash
cd ${UP4ROOT}
mkdir build
cd build
cmake ..   # (optional add the debug flag) -DCMAKE_BUILD_TYPE=DEBUG  
make -j2   # This should create p4c-msa executable in the build directory 
cd ..
```

#### (Optional) Install Barefoot's SDE for Tofino if you want to test with Tofino
μP4C can generate P4 source specific Barefoot's Tofino architecture(TNA).
We recommend installing Barefoot SDE 9.0.0 inside [extensions/csa/msa-examples](https://github.com/cornell-netlab/MicroP4/tree/master/extensions/csa/msa-examples) directory.




### 3. Running examples
Before we dive into the details of how to write new μP4 Programs, we provide instructions to test example μP4 programs.
We have provided a set of example programs at [./extensions/csa/msa-examples](https://github.com/cornell-netlab/MicroP4/tree/master/extensions/csa/msa-examples) corresponding to those mentioned in our SIGCOMM paper.
Follow the [README](https://github.com/cornell-netlab/MicroP4/tree/master/extensions/csa/msa-examples/README.md) which outlines instructions to compile the composed programs and test it with BMv2 (or with Tofino). The directory `./extensions/csa/msa-examples` also contains PTF tests for Tofino.


### 4. How to write μP4 programs
Every μP4 Program must implement at least one of the interfaces defined as a part of 
μPA in [extensions/csa/p4include/msa.up4](https://github.com/cornell-netlab/MicroP4/blob/master/extensions/csa/p4include/msa.up4). 
μPA provides 3 interfaces, Unicast, Multicast and Orchestration. By implementing a μPA interface, a user-defined package type can be created. 

#### A quick introduction to μP4 program structure
```p4
// In the following example, MyProg is a user-defined package type.
// cpackage is a keyword to indicate MyProg is a composable package 
// with in-built `apply` method.
// h_t, M_t, i_t, o_t, io_t are user-defined concrete types supplied to 
// specialized H, M, I, O, and IO generic types in Unicast interface

cpackage MyProg : implements Unicast<h_t, m_t, i_t, o_t, io_t> {

  // extractor, emitter, pkt, im_t are declared in μPA (msa.p4)
  
  parser micro_parser(extractor ex, pkt p, im_t im, out h_t hdr,          
                      inout m_t meta, in i_t ia, inout io_t ioa) {
    // usual P4-16 parser block code goes here
  }
  
  control micro_control(pkt p, im_t im, inout h_t hdr, inout m_t m,   
                        in i_t ia, out o_t oa, inout io_t ioa) {
    // usual P4-16 control block code goes here
  }
  
  control micro_deparser(emitter em, pkt p, in H h) {
    // Deparser code
    // a sequence of em.emit(...) calls
  }
  
  // in-built apply BlockStatement.
  apply {
    micro_parser.apply(...);
    micro_control.apply(...);
    micro_deparser.apply(...);
  }
}
```

How to instantiate cpackage types:
   1. Instantiating `MyProg` in `micro_control` block
      ```
      MyProg() inst_my_prog;   // () is constructor parameter for future designs.
      ```
   2. Instantiating as `main` at file scope.
      ```
      MyProg() main; 
      ```

How to invoke an instance of `cpackage` type:

   1. Invoking `MyProg` using 5 runtime parameters. 
      First two (`p` and `im`) are instances of concrete types declared in μPA.
      The last three (`i`, `o`, and `io`) are instances of user-defined types used 
      to specialize generic types I, O and IO. 
      ```
      inst_my_prog.apply(p, im, i, o, io); 
      ```

   2. `main` instances can not be invoked explicitly.

#### An example
There are example programs at [extensions/csa/msa-examples] path.
For more details, have a look at
   1. [lib-src/ipv4.up4](extensions/csa/msa-examples/lib-src/ipv4.up4): a very simple IPv4 cpackage, and
   2. [main-programs/routerv4_main.up4](extensions/csa/msa-examples/main-programs/routerv4_main.up4) the `main` cpackage that uses IPv4 cpackage.


### 5. How to Use μP4C
   1. Creating libraries
      ```
      ./build/p4c-msa -o <<lib-name.json>> <<μp4 source file>>
      ```
      ##### Example
      `ipv4.p4` implements a μp4 program for IPv4 processing
      ```
      ./build/p4c-msa -o ipv4.json ./extensions/csa/msa-examples/lib-src/ipv4.up4
      ```

   2. Generating Target Source
      ```
      ./build/p4c-msa --target-arch  <<target>> -I <<path to target's .p4>>  \
                      -l <<lib-name.json>> <<main μp4 source file>>
      ```
      ##### Example
      This will generate `routerv4_main_v1model.p4`
      ```
      ./build/p4c-msa --target-arch  v1model -I ./build/p4include/ -l ipv4.json \
                      ./extensions/csa/msa-examples/main-programs/routerv4_main.up4
      ```

### 6. μP4 Tutorial
We have created a tutorial with three exercises to help you write μP4 programs. Please visit [μP4-Tutorial](https://github.com/cornell-netlab/MicroP4/tree/master/extensions/csa/tutorials) for more information.

### 7. Limitations
μP4 is our ongoing work and has certain limitations which we plan to address asap.
1. Checksum Computation: For current prototype, you should disable checksum compute at hosts side.
2. Packet Replication: Current μP4 Architecture provides required constructs like `Multicast` and `Orchestration` interfaces, `copy_from` methods of logical externs, buffers etc that allow programmers to express packet replications. We have algorithms and mechanisms outlined but current version of μP4 Compiler does not implement them. We hope to release the required implementation soon.
3. Stateful Packet Processing: Current μP4 Architecture design does not provide any particular constructs for it. However, we have plan in place to extend μP4 Architecture and Compiler to support it.
4. Control Plane APIs: μP4 Compiler inserts `@name` annotations for tables, but we have not tested it. For now, the best and easy way is to put constant entries in the tables of your programs. 
4. Variable Length Headers: μP4 Architecture supports it but μP4 compiler does not have proper implementation for it. 
5. Recursion: Not supported for now. You can try at your own risk.
Please refer the paper ["Composing Dataplane Programs with µP4"](https://hksoni.github.io/microp4.pdf) for more detailed discussion on limitations and our future plan.
