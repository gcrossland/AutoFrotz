# CMD_RM CMD_MKDIR CMD_CP LIBCACHEDIR PLATFORM CONFIG FLAGS
AUTOFROTZ_MAJ=1
AUTOFROTZ_MIN=0
LIBS=core-1.0

PLATFORMFLAGS::=-D$(PLATFORM)
LIBFLAGS::=$(foreach o,$(LIBS),-I$(LIBCACHEDIR)/$(o)/include) $(foreach o,$(LIBS),-L$(LIBCACHEDIR)/$(o)/lib-$(CONFIG)) $(foreach o,$(shell parselibs libnames $(LIBS)),-l$(o))
DEPENDENCIESFLAGS::=-DDEPENDENCIES="$(shell parselibs dependenciesdefn $(LIBS))"

autofrotz: autofrotz.exe


o:
	$(CMD_MKDIR) $@


AUTOFROTZ_VMLINK_HDRS=libraries/autofrotz_vmlink.hpp

o/autofrotz_vmlink.o: libraries/autofrotz_vmlink.cpp $(AUTOFROTZ_VMLINK_HDRS) | o
	gcc $(PLATFORMFLAGS) $(LIBFLAGS) $(FLAGS) -x c++ -c $< -o $@

AUTOFROTZ_VM_HDRS=libraries/autofrotz_vm/dumb/dumb_frotz.h libraries/autofrotz_vm/common/frotz.h libraries/autofrotz_vm/common/setup.h $(AUTOFROTZ_VMLINK_HDRS)

o/autofrotz_vm_%.o: libraries/autofrotz_vm/common/%.cpp $(AUTOFROTZ_VM_HDRS) | o
	gcc $(PLATFORMFLAGS) $(LIBFLAGS) -std=gnu++11 -Wl,--demangle -O0 -gstabs3 -DAUTOFROTZ -x c++ -c $< -o $@

o/autofrotz_vm_%.o: libraries/autofrotz_vm/auto/%.cpp $(wildcard libraries/autofrotz_vm/dumb/*.cpp) $(AUTOFROTZ_VM_HDRS) | o
	gcc $(PLATFORMFLAGS) $(LIBFLAGS) -std=gnu++11 -Wl,--demangle -O0 -gstabs3 -DAUTOFROTZ -x c++ -c $< -o $@

AUTOFROTZ_HDRS=libraries/autofrotz.hpp $(AUTOFROTZ_VMLINK_HDRS) $(AUTOFROTZ_VM_HDRS)

o/autofrotz.o: libraries/autofrotz.cpp $(AUTOFROTZ_HDRS) | o
	gcc $(PLATFORMFLAGS) $(LIBFLAGS) $(DEPENDENCIESFLAGS) -DLIB_MAJ=$(AUTOFROTZ_MAJ) -DLIB_MIN=$(AUTOFROTZ_MIN) $(FLAGS) -x c++ -c $< -o $@

AUTOFROTZ_OBJS=o/autofrotz.o $(patsubst libraries/autofrotz_%.cpp,o/autofrotz_%.o,$(wildcard libraries/autofrotz_*.cpp)) $(patsubst libraries/autofrotz_vm/common/%.cpp,o/autofrotz_vm_%.o,$(wildcard libraries/autofrotz_vm/common/*.cpp)) $(patsubst libraries/autofrotz_vm/auto/%.cpp,o/autofrotz_vm_%.o,$(wildcard libraries/autofrotz_vm/auto/*.cpp))


MAIN_HDRS=header.hpp $(AUTOFROTZ_HDRS)

o/%.o: %.cpp $(MAIN_HDRS) | o
	gcc $(PLATFORMFLAGS) $(LIBFLAGS) $(FLAGS) -x c++ -c $< -o $@

MAIN_OBJS=$(patsubst %.cpp,o/%.o,$(wildcard *.cpp))


LIBCACHEOUTDIR=$(LIBCACHEDIR)/autofrotz-$(AUTOFROTZ_MAJ).$(AUTOFROTZ_MIN)
autofrotz.exe: $(MAIN_OBJS) $(AUTOFROTZ_OBJS)
	gcc $(PLATFORMFLAGS) $(FLAGS) $^ -o $@ $(LIBFLAGS) -lstdc++
	$(CMD_RM) $(LIBCACHEOUTDIR)
	$(CMD_MKDIR) $(LIBCACHEOUTDIR)/lib-$(CONFIG)
	ar -rcsv $(LIBCACHEOUTDIR)/lib-$(CONFIG)/libautofrotz.a $(AUTOFROTZ_OBJS)
	$(CMD_MKDIR) $(LIBCACHEOUTDIR)/include
	$(CMD_CP) --target $(LIBCACHEOUTDIR)/include $(AUTOFROTZ_HDRS)
