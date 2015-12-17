MAP?=thread
OBJ=reduce_linear.o bio.o map_$(MAP).o crt1.o __start_main.o
CFLAGS=-Wall -std=gnu99 -fPIC

all: libu.a # libu.so

libu.a: $(OBJ)
	$(RM) $@
	ar cr $@ $^

libu.so: $(OBJ)
	$(CC) -o $@ -fPIC -shared -pthread $^

clean:
	$(RM) *.o

fullclean:
	$(RM) libu.a libu.so
