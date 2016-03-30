OPTIMIZATION_FLAGS ?= -O0 -g -ggdb

DEPFLAGS = -MT $@ -MMD -MP -MF objects/$*.Td

SRCS = BlockCacheEntry.cpp main.cpp BPlusTree.cpp FileSystem.cpp Transaction.cpp

all : main

objects/%.o : src/%.cpp objects/%.d | objects
	g++ -c -I include -std=gnu++11 $(DEPFLAGS) $(OPTIMIZATION_FLAGS) -o $@ $<
	mv -f objects/$*.Td objects/$*.d

objects/%.d: ;

.PRECIOUS: objects/%.d

-include $(patsubst %,objects/%.d,$(basename $(SRCS)))

main : $(patsubst %,objects/%.o,$(basename $(SRCS))) | devices
	g++ $(OPTIMIZATION_FLAGS) -o $@ $?

objects : 
	mkdir -p objects

devices : 
	mkdir -p devices

run : main
	./main

clean :
	-rm -rf objects
	-rm -f main

