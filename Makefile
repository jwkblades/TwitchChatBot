COMPONENTS:=Core
DEPDIR := build
INCDIR := i
LIBS := -pthread
FLAGS = -Wall -pedantic -std=c++11 ${LIBS} -I3rdParty ${INC}
CC := g++
EXE = $(shell basename ${CURDIR})

INC := $(foreach directory, $(shell find ${COMPONENTS} -name "${INCDIR}" -a -type d), -I${directory})
SRCS := $(shell ag -g '\.cpp' --ignore-dir json/ --nocolor)
OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.ipp=.o)
OBJS := $(patsubst ./%, %, ${OBJS})
OBJS := $(patsubst %, ${DEPDIR}/%, ${OBJS})
$(shell mkdir -p ${DEPDIR} >/dev/null)
DEPFLAGS = -MT $@ -MMD -MF ${DEPDIR}/$*.d
COMPILE.cc = ${CC} ${DEPFLAGS} ${FLAGS} -c

${DEPDIR}/%.o: %.cpp
${DEPDIR}/%.o: %.cpp ${DEPDIR}/%.d
	@mkdir -p $(shell dirname $@)
	${COMPILE.cc} ${OUTPUT_OPTION} $<

${DEPDIR}/%.o: %.ipp
${DEPDIR}/%.o: %.ipp ${DEPDIR}/%.d
	@mkdir -p $(shell dirname $@)
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
