# CMD_RM CMD_MKDIR CMD_CP LIBCACHEDIR CONFIG FLAGS
AUTOFROTZ_MAJ=1
AUTOFROTZ_MIN=0
REQUIRED_LIBS=core-1.0 bitset-1.0

AVAILABLE_LIBS::=$(shell parselibs availablelibs $(REQUIRED_LIBS))
LIBFLAGS::=$(foreach o,$(AVAILABLE_LIBS),-I$(LIBCACHEDIR)/$(o)/include) $(foreach o,$(AVAILABLE_LIBS),-L$(LIBCACHEDIR)/$(o)/lib-$(CONFIG)) $(foreach o,$(shell parselibs libnames $(AVAILABLE_LIBS)),-l$(o))

autofrotz: autofrotz.exe


o:
	$(CMD_MKDIR) $@


AUTOFROTZ_VMLINK_HDRS=libraries/autofrotz_vmlink.hpp

o/autofrotz_vmlink.o: libraries/autofrotz_vmlink.cpp $(AUTOFROTZ_VMLINK_HDRS) | o
	gcc $(FLAGS) $(LIBFLAGS) -x c++ -c $< -o $@

AUTOFROTZ_VM_HDRS=libraries/autofrotz_vm/dumb/dumb_frotz.h libraries/autofrotz_vm/common/frotz.h libraries/autofrotz_vm/common/setup.h $(AUTOFROTZ_VMLINK_HDRS)

o/autofrotz_vm_%.o: libraries/autofrotz_vm/common/%.cpp $(AUTOFROTZ_VM_HDRS) | o
	gcc $(FLAGS) $(LIBFLAGS) -w -DAUTOFROTZ -x c++ -c $< -o $@

o/autofrotz_vm_%.o: libraries/autofrotz_vm/auto/%.cpp $(wildcard libraries/autofrotz_vm/dumb/*.cpp) $(AUTOFROTZ_VM_HDRS) | o
	gcc $(FLAGS) $(LIBFLAGS) -w -DAUTOFROTZ -x c++ -c $< -o $@

AUTOFROTZ_HDRS=libraries/autofrotz.hpp $(AUTOFROTZ_VMLINK_HDRS) $(AUTOFROTZ_VM_HDRS)

o/autofrotz.o: libraries/autofrotz.cpp $(AUTOFROTZ_HDRS) | o
	gcc $(FLAGS) $(LIBFLAGS) -DLIB_MAJ=$(AUTOFROTZ_MAJ) -DLIB_MIN=$(AUTOFROTZ_MIN) -DDEPENDENCIES="$(shell parselibs dependenciesdefn $(AVAILABLE_LIBS))" -x c++ -c $< -o $@

AUTOFROTZ_OBJS=o/autofrotz.o $(patsubst libraries/autofrotz_%.cpp,o/autofrotz_%.o,$(wildcard libraries/autofrotz_*.cpp)) $(patsubst libraries/autofrotz_vm/common/%.cpp,o/autofrotz_vm_%.o,$(wildcard libraries/autofrotz_vm/common/*.cpp)) $(patsubst libraries/autofrotz_vm/auto/%.cpp,o/autofrotz_vm_%.o,$(wildcard libraries/autofrotz_vm/auto/*.cpp))


MAIN_HDRS=header.hpp $(AUTOFROTZ_HDRS)

o/%.o: %.cpp $(MAIN_HDRS) | o
	gcc $(FLAGS) $(LIBFLAGS) -x c++ -c $< -o $@

MAIN_OBJS=$(patsubst %.cpp,o/%.o,$(wildcard *.cpp))


LIBCACHEOUTDIR=$(LIBCACHEDIR)/autofrotz-$(AUTOFROTZ_MAJ).$(AUTOFROTZ_MIN)
autofrotz.exe: $(MAIN_OBJS) $(AUTOFROTZ_OBJS)
	gcc $(FLAGS) $^ -o $@ $(LIBFLAGS) -lstdc++
	$(CMD_RM) $(LIBCACHEOUTDIR)
	$(CMD_MKDIR) $(LIBCACHEOUTDIR)/lib-$(CONFIG)
	ar -rcsv $(LIBCACHEOUTDIR)/lib-$(CONFIG)/libautofrotz.a $(AUTOFROTZ_OBJS)
	$(CMD_MKDIR) $(LIBCACHEOUTDIR)/include
	$(CMD_CP) --target $(LIBCACHEOUTDIR)/include $(AUTOFROTZ_HDRS)
