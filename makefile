IDIR =fyp2016
CC=g++
CFLAGS=-I$(IDIR)

ODIR=obj
CDIR=fyp2016
LDIR =fyp2016

LIBS=-lSDL -lSDLnet

_DEPS = $(CDIR)/*.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = $(ODIR)/*.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

make: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 