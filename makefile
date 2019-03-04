CC = g++
CFLAGS = -O1
SRC = main.cpp file_list.cpp md5.cpp
OBJ = ${SRC:.cpp = .o}

loser : ${OBJ}
	${CC} ${CFLAGS} -o loser ${OBJ}

file_list.o : file_list.cpp
	${CC} -o file_list.cpp

md5.0 : md5.h
	${CC} -o mmd5.cpp	

.PHONY : clean
clean : 
	rm -f core *.o
