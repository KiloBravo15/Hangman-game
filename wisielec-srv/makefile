CPP_SOURCES = $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)
HEADERS = $(wildcard src/*.hpp) $(wildcard src/**/*.hpp)
OBJ = ${patsubst src/%.cpp, obj/%.o, $(CPP_SOURCES)}
OUTPUT = bin/wisielec-srv
FLAGS = -Wall -Wextra -std=c++2a -O0

all: folders ${OUTPUT}
run: all
	./${OUTPUT} 8080

${OUTPUT}: ${OBJ}
	g++ -o $@ $^ $(FLAGS)

obj/%.o: src/%.cpp ${HEADERS}
	g++ -c $< -o $@ $(FLAGS)

clean:
	rm -rf obj bin

# Tworzy foldery dla plików obiektowych i wykonywalnych
# Kopiuje strukturę katalogów z src, aby kompilator nie narzekał
folders:
	mkdir obj 2> /dev/null || (exit 0)
	mkdir bin 2> /dev/null || (exit 0)
	(cd src && find -type d | xargs -I{} mkdir -p "../obj/{}")
