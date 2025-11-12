SRC = src/*.c 
END = wall.bin 
END_D = wall_debug.bin
END_O = wall_optimal.bin

COMP = gcc
FLAGS = -I include

all: $(END)

$(END): $(SRC)
	$(COMP) $(SRC) -o $(END) $(FLAGS)

debug: $(END_D)

$(END_D): $(SRC)
	$(COMP) $(SRC) -o $(END_D) $(FLAGS) -g

optimal: $(END_O)

$(END_O): $(SRC)
	$(COMP) $(SRC) -o $(END_O) $(FLAGS) -g -O3

clean:
	rm -rf $(END) $(END_D) $(END_O)

.PHONY: all clean
