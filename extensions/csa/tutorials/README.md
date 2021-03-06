# μP4 Tutorials

Welcome to μP4 tutorials! This is a step by step tutorial that teach you the 
art of composing network dataplane programs. Our goal is to make you [Chopin](https://en.wikipedia.org/wiki/Fr%C3%A9d%C3%A9ric_Chopin) 
of Dataplane Programming.

### Outline
There are three exercises in this tutorial. The objective is to build a router
using libraries(μP4 Modules) of routing protocols. 
1. Exercise 1: to learn composiing μP4 Modules. You are provided with 
   necessary μP4 Modules. The focus is on introducing language constrcuts 
   required to compose these modules.
2. Exercise 2: to write μP4 Modules. You are provided with basic code structure
   for μP4 Modules and you are required to add implementation. The focus is on
   familiarizing with some constructs of μP4 language and architecture.
3. Exercise 3: to get familiar with end-to-end development dataplane program
   libraries and applications. You are required to integrate μP4 Modules
   developed by and provided to you.

### Prerequisites: 
You MUST have completed [step 1](https://github.com/cornell-netlab/MicroP4#1-install-dependencies-and-download-%CE%BCp4) 
and [2](https://github.com/cornell-netlab/MicroP4#2-install) to download and install μP4.
Reminder, make sure `UP4ROOT` is set with appropriate path.

### Exercise 1: Composing μP4 Module
In this exercise, you will learn how to compose μP4 Modules. You are given all 
the required code for two μP4 Modules. However, they are not composed together. 
Concretely, μP4 module
1. Router - is going to be considered as the main module
2. ipv4 - is an independent one that processes IPv4 headers. 
As a part of this exercise, you need to complete the implementaion of [./exercise-1/Router.up4](https://github.com/cornell-netlab/MicroP4/blob/master/extensions/csa/tutorials/exercise-1/Router.up4).

[./exercise-1/Router.up4](https://github.com/cornell-netlab/MicroP4/blob/master/extensions/csa/tutorials/exercise-1/Router.up4) does not route packets.
#### Step 1: Compile & Run
##### Compile with μP4
```bash
cd ${UP4ROOT}/extensions/csa/tutorials/exercise-1
${UP4ROOT}/build/p4c-msa  --target-arch v1model -I ${UP4ROOT}/p4include Router.up4
```
This will generate `Router_v1model.p4` that is required to compile with p4c.
##### Compile with P4
```bash
cd ${UP4ROOT}/extensions/csa/tutorials/exercise-1
../p4c-compile.sh  Router_v1model.p4
```
Next, you should verify that all packets are dropped using BMv2
##### Run
```bash
cd ${UP4ROOT}/extensions/csa/tutorials/exercise-1
../run-tutorial.sh
```
Enter `h1 ping h2` on mininet command prompt to verify all the packets are dropped.
The python script executed by the above command is `Router_v1model_sw.py` located 
[here](https://github.com/hksoni/behavioral-model/blob/b1647b16dee94db24dcef33cc31da844499be782/mininet/tutorials/Router_v1model_sw.py)

Now, you need to compose [./exercise-1/Router.up4](https://github.com/cornell-netlab/MicroP4/blob/master/extensions/csa/tutorials/exercise-1/Router.up4) with a
μP4 module implementing a routing protocol. We have provided implementation of IPv4
in [./exercise-1/ipv4.up4](https://github.com/cornell-netlab/MicroP4/blob/master/extensions/csa/tutorials/exercise-1/ipv4.up4). 

#### Step 2: Compose
To compose μP4 modules, their interfaces should be used. Interfaces provide 
information of μP4 modules' runtime behaviour without exposing their 
implementation detail. Every μP4 module can either take 
1. one packet as input and retuen one packet - Unicast
2. one packet as input and return multiple packets - Multicast
3. multiple packets as inout and return multiple packets - Orchestration
In this example, it is required to identify interface implemented by the 
IPv4 module in [ipv4.up4](https://github.com/cornell-netlab/MicroP4/blob/master/extensions/csa/tutorials/exercise-1/ipv4.up4).
```
cpackage IPv4 : implements Unicast<ipv4_hdr_t, empty_t, empty_t, bit<16>, empty_t>
```
specifies that the module implements `Unicast` interface by specializing its 
generic runtime parameters `H, M, I, O, IO`.
```
cpackage Unicast<H, M, I, O, IO> /* runtime parametes */
    (pkt p, im_t im, in I in_param, out O out_param, inout IO inout_param) {
...
}
H = ipv4_hdr_t, M, I, IO = empty_t, O = bit<16>
```
The types of runtime parameters of Ipv4 module are 
`(pkt, im_t, empty_t, bit<16>, empty_t)`.

##### Declare
Every μP4 module needs to be declared within the same scope it is used. To use 
`IPv4` without its definition, it can be declared as a user-defined type using 
`cpackage` keyword and its runtime parameters, as shown below.
```
cpackage IPv4(pkt p, im_t im, in empty_t ia, out bit<16> nh, inout empty_t ioa);
```
##### Instantiate
μP4 modules declaration provide sufficient information to use them or 
instantiate them using.
```P4
IPv4() ipv4_i; // () is constructor params, must be empty for now.
```
##### Invoke
Every μP4 module instance can be invoked by using its built-in method `apply`.
The `apply` method takes arguments for runtime parameters of the module.
```P4
ipv4_i.apply(...); // args for pkt p, im_t im, 
                   // in empty_t ia, out bit<16> nh, inout empty_t ioa
```

##### Table Entries
You should observe the table entries in Router.up4 and ipv4.up4.

#### Step 3: Re-compile & Run
This time compilation is little different than the Step 1.
Note `-l ipv4.json` flag in the last command below. This links library 
μP4 modules.
```bash
cd ${UP4ROOT}/extensions/csa/tutorials/exercise-1
// compiles module as a library
${UP4ROOT}/build/p4c-msa -I ${UP4ROOT}/build/p4include -o ipv4.json ipv4.up4 
${UP4ROOT}/build/p4c-msa --target-arch v1model -I ${UP4ROOT}/build/p4include \
                          -l ipv4.json Router.up4
```
##### Run
You can again run with the same commands as Step 1.
If you have implemented correctly ICMP packets generated by ping should pass.
We have set required IP and MAC addresses in concert with the mininet script.

### Exercise 2: Writing new μP4 Modules
In this exercise, you will learn to write your own μP4 Module. You need to 
implement a routing protocol like IPv4. The module should have `bit<16> nh` as 
a part of its `out` arguments. 
#### Step 1: Write a μP4 Module to process IPv6 header
1. The boilerplate code to develop the module is given at [./exercise-2/ipv6.up4](https://github.com/cornell-netlab/MicroP4/tree/master/extensions/csa/tutorials/exercise-2).
2. IPv6 should implement `Unicast` interface by providing implementation for 
  * `micro_parser`
  * `micro_control`
  * `micro_deparser`
3. sub-languages of P4: To implement the above constructs, you can use 
sub-lanuages of [P4](https://p4.org/p4-spec/docs/P4-16-v1.2.1.html) described in 
[12](https://p4.org/p4-spec/docs/P4-16-v1.2.1.html#sec-packet-parsing) & [13](https://p4.org/p4-spec/docs/P4-16-v1.2.1.html#sec-control).
4. μP4 Constructs:  Compared to P4, μP4 provides `extractor` and `emitter` for
packet parsing and reassembly. You can look at [./exercise-1/ipv4.up4](https://github.com/cornell-netlab/MicroP4/blob/master/extensions/csa/tutorials/exercise-1/ipv4.up4) 
for their example usage. To know more about `extractor` and `emitter` of 
μP4 Architecture, you can look at [msa.up4](https://github.com/cornell-netlab/MicroP4/blob/master/extensions/csa/p4include/msa.up4)

#### Step 2: Compile IPv6 μP4 Module
```bash
cd ${UP4ROOT}/extensions/csa/tutorials/exercise-2
// compiles IPv6 module as a library
${UP4ROOT}/build/p4c-msa -I ${UP4ROOT}/build/p4include -o ipv6.json ipv6.up4 
```

### Exercise 3: Composing Dataplane program using your μP4 Modules
In this exercise, you need to integrate IPv6 Module with `Router`from Exercise 1.
#### Step 1: Modify `Router.up4`
1. You can use [./exercise-3/Router.up4](https://github.com/cornell-netlab/MicroP4/tree/master/extensions/csa/tutorials/exercise-3/Router.up4) having 
required boilerplate code.
2. Add your code at `exercise-3` bookmarks in the file. 
3. You might need to add const entries in your IPv6 table. Our mininet script
   configures network according to the following values.
```bash
ipv6 dest                             nexthop
0x20010000000000000000000000000001 -> 1
0x20010000000000000000000000000002 -> 2
```

#### Step 2: Compile `Router.up4`
You need to compile the router with IPv4 and IPv6 modules.
```bash
cd ${UP4ROOT}/extensions/csa/tutorials/exercise-3
${UP4ROOT}/build/p4c-msa -I ${UP4ROOT}/build/p4include -o ipv4.json ipv4.up4 
${UP4ROOT}/build/p4c-msa -I ${UP4ROOT}/build/p4include -o ipv6.json ipv6.up4 
${UP4ROOT}/build/p4c-msa  --target-arch v1model -I ${UP4ROOT}/build/p4include \
                          -l ipv4.json,ipv6.json  Router.up4
```
##### Compile with P4
```bash
cd ${UP4ROOT}/extensions/csa/tutorials/exercise-3
../p4c-compile.sh  Router_v1model.p4
```


#### Step 3: Pass IPv4 and IPv6 pings
Next, you should verify that IPv4 and IPv6 packets are forwarded.
##### Run
```bash
cd ${UP4ROOT}/extensions/csa/tutorials/exercise-3
../run-tutorial.sh

// On  mininet term, for IPv4 ping
mininet> h1 ping h2
// for IPv6 ping
mininet> h1 ping -6 2001::2

ctrl + d
```
We have provided a solution for every exercise.

bravo! You have developed a modular router and completed Chopin 1O1 tutorial.
