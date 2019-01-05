LDLIBS=-lgc

set-based-solver: set-based-solver.o gc-list.o gc-bits.o gc-set.o gc-pair.o gc-tagged.o
gc-template: gc-template.o gc-list.o
gc-list-test: gc-list-test.o gc-list.o
gc-bits-test: gc-bits-test.o gc-bits.o

gc-template.o: list.h gc-template.c
set-based-solver.o: list.h set.h set-based-solver.c

gc-list-test.o: list.h gc-list-test.c
gc-list.o: list.h gc-list.c

gc-bits-test.o: bits.h gc-bits-test.c
gc-bits.o: bits.h gc-bits.c

gc-set.o: set.h bits.h gc-set.c

gc-pair.o: pair.h gc-pair.c

gc-tagged.o: tagged.h pair.h gc-tagged.c

.PHONY: clean
clean:
	$(RM) set-based-solver gc-template gc-list-test gc-bits-test *.o