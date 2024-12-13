# Компилятор
CXX = g++

# Флаги компиляции
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Исходные файлы
SRC = src/prog.cpp src/utils.cpp src/transformation_table.cpp

# Путь для исполняемого файла
OUT = bin/run.exe

# Папка для вывода
OUTPUT_DIR = output

# Определение ОС
ifeq ($(OS),Windows_NT)
    MKDIR_BIN = if not exist bin mkdir bin
    MKDIR_OUTPUT = if not exist $(OUTPUT_DIR) mkdir $(OUTPUT_DIR)
    RMDIR = rmdir /s /q
    RUN = .\\$(OUT)
else
    MKDIR_BIN = mkdir -p bin
    MKDIR_OUTPUT = mkdir -p $(OUTPUT_DIR)
    RMDIR = rm -rf
    RUN = ./$(OUT)
endif

# Правило сборки по умолчанию
all: $(OUT)
	$(MKDIR_OUTPUT)
	$(RUN)

# Правило для сборки исполняемого файла
$(OUT): $(SRC)
	$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

# Правило для запуска программы
run: $(OUT)
	$(RUN)

# Правило для очистки
clean:
	$(RMDIR) bin
	$(RMDIR) $(OUTPUT_DIR)
