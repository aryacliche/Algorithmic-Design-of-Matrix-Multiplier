# build software version of testbench (to check the "desired behaviour")
AHIR_INCLUDE=$(AHIR_RELEASE)/include
AHIR_LIB=$(AHIR_RELEASE)/lib
VHDL_LIB=$(AHIR_RELEASE)/vhdl
VHDL_VHPI_LIB=$(AHIR_RELEASE)/CtestBench/vhdl
FUNCTIONLIB=$(AHIR_RELEASE)/functionLibrary/
# we only need to expose some of the modules to the testbench. Others need to be visible
TOPMODULES=-t storeA -t storeB  -t loadC  -t mmul -t latencyTest

# five steps from C to vhdl simulator.
HW: aalink aa2vc vc2vhdl vhdltb ghdlsim

# Aa to vC
aalink: 
	AaLinkExtMem $(SRC)/$(main_file_name) | vcFormat > prog.linked.aa
	AaOpt -B prog.linked.aa | vcFormat > prog.linked.opt.aa

aa2vc: prog.linked.opt.aa
	Aa2VC -O -C prog.linked.opt.aa | vcFormat > prog.vc

# vC to VHDL
vc2vhdl: prog.vc
	vc2vhdl  -U  -O -S 4 -I 2 -v -a -C -e ahir_system -w -s ghdl $(TOPMODULES) -f prog.vc
	vhdlFormat < ahir_system_global_package.unformatted_vhdl > ahir_system_global_package.vhdl
	vhdlFormat < ahir_system.unformatted_vhdl > ahir_system.vhdl
	vhdlFormat < ahir_system_test_bench.unformatted_vhdl > ahir_system_test_bench.vhdl

# build testbench and ghdl executable
# note the use of libVhpi in building the testbench.
vhdltb: ahir_system.vhdl ahir_system_test_bench.vhdl ./src/testbench/testbench.c vhdlCStubs.h vhdlCStubs.c
	gcc -c vhdlCStubs.c  -I$./src/ -I./ -I$(AHIR_INCLUDE)
	gcc -c ./src/testbench/testbench.c -I$(AHIR_INCLUDE) -I$./src -I./
	gcc -o testbench_hw testbench.o vhdlCStubs.o  -L$(AHIR_LIB) -lSocketLibPipeHandler -lpthread

ghdlsim: ahir_system.vhdl ahir_system_test_bench.vhdl ./src/testbench/testbench.c vhdlCStubs.h vhdlCStubs.c
	ghdl --clean
	ghdl --remove
	ghdl -i --work=GhdlLink  $(VHDL_LIB)/GhdlLink.vhdl
	ghdl -i --work=aHiR_ieee_proposed  $(VHDL_LIB)/aHiR_ieee_proposed.vhdl
	ghdl -i --work=ahir  $(VHDL_LIB)/ahir.vhdl
	ghdl -i --work=work ahir_system_global_package.vhdl
	ghdl -i --work=work ahir_system.vhdl
	ghdl -i --work=work ahir_system_test_bench.vhdl
	ghdl -m --work=work -Wl,-L$(AHIR_LIB) -Wl,-lVhpi ahir_system_test_bench 

clean:
	rm -rf *.o* *.cf *.*vhdl vhdlCStubs.* *.vcd in_data* out_data* testbench_sw testbench_hw ahir_system_test_bench vhpi.log *.aa *.vc *.lso xst *.ngc *_xmsgs *.xrpt pipeHandler.log *.srp *.ghw *.dot

PHONY: original clean

# Targets with different main_file_name values
original: 
	$(eval main_file_name=original.aa)
	$(eval SRC=./src/original)
	$(MAKE) HW

original_unrolled: 
	$(eval main_file_name=original_unrolled.aa)
	$(eval SRC=./src/original)
	$(MAKE) HW

parta: 
	$(eval main_file_name=parta.aa)
	$(eval SRC=./src/parta)
	$(MAKE) HW

partb: 
	$(eval main_file_name=master.incl.aa)
	$(eval SRC=./src/partb)
	AaPreprocess -I $(SRC)/ $(SRC)/master.aa -o $(SRC)/master.incl.aa
	$(MAKE) HW

partc: 
	$(eval main_file_name=partc.aa)
	$(eval SRC=./src/partc)
	$(MAKE) HW


# Make it stop!