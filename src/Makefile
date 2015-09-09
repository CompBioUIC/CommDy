COMPILE=gcc
CFLAGS=-std=c99 -O3 -ftree-vectorize

targets= bb5 color_ind2 interval_lowerbound

default: $(targets)

bb5: bb5.o ExhaustiveSearch.o gtmfile.o IntArrayList.o \
	PartialColoring.o PartialColoringHeap.o assert.h util.o
	$(COMPILE) $(CFLAGS) -o $@ $^

interval_lowerbound: interval_lowerbound.o ExhaustiveSearch.o \
	gtmfile.o IntArrayList.o PartialColoring.o PartialColoringHeap.o assert.h util.o
	$(COMPILE) $(CFLAGS) -o $@ $^

color_ind2: assert.h gtmfile.o IntArrayList.o color_ind2.o util.o
	$(COMPILE) $(CFLAGS) -o $@ $^

%.o: %.c
	$(COMPILE) $(CFLAGS) -c $<

clean:
	rm -f *.o $(targets)
