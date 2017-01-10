FLAGS:=-Wall -pedantic -std=c++11 -pthread
CC:=g++
EXE=$(shell basename ${CURDIR})

DEPDIR := deps
SRCS := $(shell ag -g '\.[ci]pp' --ignore-dir json/ --nocolor)
OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.ipp=.o)
OBJS := $(patsubst ./%,%,${OBJS})
OBJS := $(patsubst %,${DEPDIR}/%,${OBJS})
$(shell mkdir -p ${DEPDIR} >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF ${DEPDIR}/$*.d
COMPILE.cc = ${CC} ${DEPFLAGS} ${FLAGS} -c

${DEPDIR}/%.o : %.cpp
${DEPDIR}/%.o : %.cpp ${DEPDIR}/%.d
	${COMPILE.cc} ${OUTPUT_OPTION} $<

${DEPDIR}/%.o : %.ipp
${DEPDIR}/%.o : %.ipp ${DEPDIR}/%.d
	${COMPILE.cc} ${OUTPUT_OPTION} $<

${EXE}: ${OBJS}
	${CC} ${FLAGS} -o $@ $^

.Phony: clean
clean:
	@rm -f  ${EXE}
	@rm -rf ${DEPDIR}

${DEPDIR}/%.d: ;
.PRECIOUS: ${DEPDIR}/%.d

-include $(patsubst %,${DEPDIR}/%.d,$(basename ${SRCS}))
