# makefile for Lua standard library

LUA= ../../../lua5.1

LTASKDEFS= -DLUATASK_API= -DLUATASK_PTHREAD_STACK_SIZE=2097152/16

OBJS= ltask.o queue.o syncos.o

T= $(LUA)/lib/task.so

all:	$T

$T:	$(OBJS)
	ld -o $(LUA)/lib/task.so -lpthread -shared $(OBJS)

clean:
	rm -f $(OBJS) $T

ltask.o:	../../../src/LuaTask/ltask.c
	cc -Wall -I $(LUA)/include -I../../../src $(LTASKDEFS) -o$*.o -c $?

queue.o:	../../../src/LuaTask/queue.c
	cc -Wall -I $(LUA)/include -I../../../src $(LTASKDEFS) -o$*.o -c $?

syncos.o:	../../../src/LuaTask/syncos.c
	cc -Wall -I $(LUA)/include -I../../../src $(LTASKDEFS) -o$*.o -c $?


