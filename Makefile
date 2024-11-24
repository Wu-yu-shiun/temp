//hw2

CC= gcc
CFLAGS= -Wall -Wextra -Werror -O3 -Wpedantic
TARGET= sched_demo_313551122

all: $(TARGET)

$(TARGET): sched_demo_313551122.c
	$(CC) $(CFLAGS) -o $(TARGET) sched_demo_313551122.c -lpthread

clean:
	rm -f $(TARGET)

//hw3
obj-m := kfetch_mod_313551122.o
PWD := $(CURDIR)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
load:
	sudo insmod kfetch_mod_313551122.ko
unload:
	sudo rmmod kfetch_mod_313551122
