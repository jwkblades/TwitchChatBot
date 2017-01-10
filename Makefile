FLAGS:=-Wall -pedantic -std=c++11 -pthread
CC:=g++
EXE=$(shell basename ${CURDIR})

SRCS := $(shell find . -name '*.[ci]pp')
OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.ipp=.o)
DEPDIR := deps
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
COMPILE.cc = ${CC} ${DEPFLAGS} ${FLAGS} -c

%.o : %.cpp
%.o : %.cpp $(DEPDIR)/%.d
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

%.o : %.ipp
%.o : %.ipp $(DEPDIR)/%.d
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

${EXE}: ${OBJS}
	${CC} ${FLAGS} -o $@ $^

.Phony: clean
clean:
	@rm -f *.o ${EXE}
	@rm -rf ${DEPDIR}

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))
