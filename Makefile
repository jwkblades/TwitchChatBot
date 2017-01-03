FLAGS:=-Wall -pedantic -std=c++11
CC:=g++

EXE=TwitchChatBot

.cpp.o:
	${CC} ${FLAGS} -o $@ -c $^

${EXE}: main.o Socket.o RAIIMutex.o
	${CC} ${FLAGS} -o $@ $^

.Phony: clean

clean:
	@rm -f *.o ${EXE}
