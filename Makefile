CC= gcc
CFLAGS= -Wall -Wextra -Werror -O3 -Wpedantic
TARGET= sched_demo_313551122

all: $(TARGET)

$(TARGET): sched_demo_<student_id>.c
    $(CC) $(CFLAGS) -o $(TARGET) sched_demo_313551122.c -lpthread

clean:
    rm -f $(TARGET)
