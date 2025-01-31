CC = gcc
EXEC = player

space = $(subst ,, )
RPATHS = $(subst -L,,$(EXTRA_LIB))
RPATHS := $(subst $(space),:,$(RPATHS))

CFLAGS = -Iinclude $(EXTRA_INC)
LDFLAGS = -Llib $(EXTRA_LIB) -lraylib -lavformat -lavcodec -lswscale -lavutil -lm -Wl,-rpath="lib:$(RPATHS)" 

build:
	$(CC) -o $(EXEC) src/main.c src/input_media.c $(CFLAGS) $(LDFLAGS)

clean:
	rm $(EXEC)
