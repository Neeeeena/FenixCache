OPTIMIZATION_FLAGS ?= -O0 -g -ggdb

DEPFLAGS = -MT $@ -MMD -MP -MF objects/$*.Td

SRCS = BlockCacheEntry.cpp globals.cpp BPlusTree.cpp FileSystem.cpp Transaction.cpp

all : main

objects/%.o : src/%.cpp objects/%.d | objects
	g++ -c -I include -std=gnu++11 $(DEPFLAGS) $(OPTIMIZATION_FLAGS) -o $@ $<
	mv -f objects/$*.Td objects/$*.d

objects/%.d: ;

.PRECIOUS: objects/%.d

-include $(patsubst %,objects/%.d,$(basename $(SRCS)))
-include objects/main.d
-include objects/TestEventListener.d
-include objects/InsertStressTestEventListener.d
-include objects/InsertReversedStressTestEventListener.d
-include objects/InsertZigZagStressTestEventListener.d
-include objects/InsertRemoveStressTestEventListener.d
-include objects/InsertRemoveReversedStressTestEventListener.d

main : $(patsubst %,objects/%.o,$(basename $(SRCS))) objects/main.o | devices
	g++ $(OPTIMIZATION_FLAGS) -o $@ $?

TestEventListener : $(patsubst %,objects/%.o,$(basename $(SRCS))) objects/TestEventListener.o | devices
	g++ $(OPTIMIZATION_FLAGS) -o $@ $?

InsertStressTestEventListener : $(patsubst %,objects/%.o,$(basename $(SRCS))) objects/InsertStressTestEventListener.o | devices
	g++ $(OPTIMIZATION_FLAGS) -o $@ $?

InsertReversedStressTestEventListener : $(patsubst %,objects/%.o,$(basename $(SRCS))) objects/InsertReversedStressTestEventListener.o | devices
	g++ $(OPTIMIZATION_FLAGS) -o $@ $?

InsertZigZagStressTestEventListener : $(patsubst %,objects/%.o,$(basename $(SRCS))) objects/InsertZigZagStressTestEventListener.o | devices
	g++ $(OPTIMIZATION_FLAGS) -o $@ $?

InsertRemoveStressTestEventListener : $(patsubst %,objects/%.o,$(basename $(SRCS))) objects/InsertRemoveStressTestEventListener.o | devices
	g++ $(OPTIMIZATION_FLAGS) -o $@ $?

InsertRemoveReversedStressTestEventListener : $(patsubst %,objects/%.o,$(basename $(SRCS))) objects/InsertRemoveReversedStressTestEventListener.o | devices
	g++ $(OPTIMIZATION_FLAGS) -o $@ $?

objects : 
	mkdir -p objects

devices : 
	mkdir -p devices

run : main
	./main

test : main TestEventListener InsertStressTestEventListener InsertReversedStressTestEventListener \
       InsertZigZagStressTestEventListener InsertRemoveStressTestEventListener \
       InsertRemoveReversedStressTestEventListener 
	./InsertRemoveReversedStressTestEventListener
	./InsertRemoveStressTestEventListener
	./InsertZigZagStressTestEventListener
	./InsertReversedStressTestEventListener
	./InsertStressTestEventListener
	./TestEventListener
	./main
	@echo  All tests ran correctly

clean :
	-rm -rf objects
	-rm -f main TestEventListener InsertStressTestEventListener \
               InsertReversedStressTestEventListener InsertZigZagStressTestEventListener \
               InsertRemoveStressTestEventListener InsertRemoveReversedStressTestEventListener

