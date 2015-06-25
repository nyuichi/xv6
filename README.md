# xv6 GAIA port
## xv6 port
This is a port of xv6 to GAIA, our own CPU architecture.  
We did this at the student experiment at The University of Tokyo.  
Rewriting xv6 is mainly done by @nullpo-head, @warelle and @m-hys.  
Many improvements are also done by other team members.  

To make xv6 work on GAIA, we did
* Rewrite all x86 dependent codes such as booting, interrupt, peripherals or task switch.  
    This was indeed heavy, but not as heavy as expected at first.
* Support page coloring.  
    To handle the cache coherency problem GAIA has at present.   

About 1000 lines modifications are needed for these tasks.  
For diff, see https://github.com/wasabiz/xv6/compare/6f8ad7a249fc9b47a449f0f35ec79aa0b01b0069...master

There is an experimental online demo of xv6-gaia by Javascript. [Try it!](https://nullpo-head.github.io/emcc-gaia-simu/xv6.html).

## Original xv6
The original xv6 is a simple Unix like OS for education by MIT, which is targeted at x86.  
For more detail, see http://pdos.csail.mit.edu/6.828/2012/v6.html

## What we did for porting
Porting needs much more than just rewriting xv6. Here is what we did for porting.  
### Create toolchain
* Create the almost C89 compatible compiler  
  Maybe this is the biggest task. We made an almost C89 compatible compiler, UCC.  
  This is done by @kw-udon, @b-inary and @wasabiz.  
  The repository is at https://github.com/kw-udon/ucc/.  
* Create the assembler  
  We made a simple but powerful assembler by python.
  This is done by @b-inary.  
  Repository: https://github.com/b-inary/gaia-software
* Create the linker  
  The linking is done by some ruby scripts and makefile.
  This is done by @nullpo-head  

### Create CPU simulator
  A software CPU simulator is necessary to develop CPU.  
  @b-inary mainly made this.  
  
### Create CPU
* Design CPU architecture  
  @b-inary and @wasabiz designed GAIA architecture.  
  @nullpo-head also joined for interruption and MMU.
* Create CPU by VHDL  
  GAIA is implemented on FPGA, Virtex-5.  
  @wasabiz wrote all VHDL source by hisself.  
  The repositroy is at https://github.com/wasabiz/gaia3

  


## Improvements

We also did some improvements.  
* Add user programs  
  pwd, tiny vi clone, sl, 2048 and many user programs are added.  
  Try sl at least once!!  
* Clean up the source codes  
  We separated kernel and user programs into different directories.   
  We also introduced a subset of standard libc. You can include '\<stdio.h\>', instead of '"user.h"'.
  
## Team members
Xv6 porting is done in 4 months by
* b-inary (UCC, simulator, assembler), https://github.com/b-inary
* censored (MMU), https://github.com/censored--
* kw_udon (UCC), https://github.com/kw-udon
* MasWag (FPU), https://github.com/maswag
* mh (xv6), https://github.com/m-hys
* nullpo_head (xv6, simulator), https://github.com/nullpo-head
* warelle (xv6, simulator), https://github.com/warelle
* wasabiz (UCC, CPU), https://github.com/wasabiz

