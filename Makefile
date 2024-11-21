# Компилятор
CXX = g++

# Флаги компиляции
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Исходные файлы
SRC = src/prog.cpp src/transformation_table.cpp

# Путь для исполняемого файла
OUT = bin/run

# Правило сборки по умолчанию
all: $(OUT)
	./$(OUT)

# Правило для сборки исполняемого файла
$(OUT): $(SRC)
	@mkdir -p $(dir $@)  # Создаем директорию bin, если она не существует
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

# Правило для запуска программы
run: $(OUT)
	./$(OUT)

# Правило для очистки
clean:
	rm -r bin/*

