PROGS := ctest cpptest crash mousetest msgtest shmtest vgatest fputest elltest nctest sdltest pthreadtest

all: dummy.file
	for dir in $(PROGS); do \
		make -C $$dir; \
	done

clean: dummy.file
	for dir in $(PROGS); do \
		make -C $$dir clean; \
	done

dummy.file:
