# Компилятор
CXX = g++

# Флаги компиляции
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Исходные файлы
SRC = src/prog.cpp src/utils.cpp src/transformation_table.cpp

# Путь для исполняемого файла
OUT = bin/run

# Папка для вывода
OUTPUT_DIR = output/

# Правило сборки по умолчанию
all: $(OUT)
	mkdir -p $(OUTPUT_DIR)
	./$(OUT)

# Правило для сборки исполняемого файла
$(OUT): $(SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

# Правило для запуска программы
run: $(OUT)
	./$(OUT)

# Правило для очистки
clean:
	rm -rf bin/* $(OUTPUT_DIR)*

