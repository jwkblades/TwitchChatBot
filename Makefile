FLAGS:=-Wall -pedantic -std=c++11 -pthread
CC:=g++

EXE=TwitchChatBot

.cpp.o:
	${CC} ${FLAGS} -o $@ -c $^

${EXE}: main.o Socket.o RAIIMutex.o PostOffice.o Address.o Message.o Throttler.o ScopeExit.o utils.o
	${CC} ${FLAGS} -o $@ $^

.Phony: clean

clean:
	@rm -f *.o ${EXE}
