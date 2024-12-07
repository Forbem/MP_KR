#include <iostream>
#include <random>
#include <vector>
#include <iomanip>
#include <fstream>
#include "transformation_table.h"

using namespace std;

struct Result{
	uint32_t a_1;
	uint32_t a_0;
};


// @ функции для преобразований @
// Преобразование uint64 в строку в шестнадцатеричном формате
string uint64ToHex(uint64_t value){
	stringstream ss;
	ss << hex << setw(16) << setfill('0') << value;
	return ss.str();
}

// Преобразование строки с шестнадцатеричными числами в uint64_t
uint64_t hexToUint64(const string& hex) {
	return stoull(hex, nullptr, 16);
}

// Преобразование ключа в строку в шестнадцатеричном формате
string keyToHex(const vector<uint8_t>& key){
	stringstream ss;
	for(uint8_t byte : key){
		ss << hex << setw(2) << setfill('0') << (int)byte;
	}
	return ss.str();
}

// Преобразование строки в ключ
vector<uint8_t> hexToKey(const string& hex) {
	vector<uint8_t> key;
	for (size_t i = 0; i < hex.length(); i += 2) {
		string byteString = hex.substr(i, 2);
		uint8_t byte = static_cast<uint8_t>(stoi(byteString, nullptr, 16));
		key.push_back(byte);
	}
	return key;
}

// генерация iv 
uint64_t generateIV(){
	random_device rd;
	mt19937 generator(rd());
	uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
	return dist(generator);
}

// Строка в вектор uint64_t
vector<uint64_t> stringToBlocks(const string& input) {
    	vector<uint64_t> blocks;           // Вектор для хранения 64-битных блоков
	size_t length = input.size();      // Длина строки
	size_t fullBlocks = length / 8;    // Количество полных блоков (по 8 байтов)

	// Проходим по всем полным блокам 
	for (size_t i = 0; i < fullBlocks; ++i) {
      		uint64_t block = 0;        // создаем 64-битный блок
        	for (size_t j = 0; j < 8; ++j) {
			// Добавляем каждый символ строки в блок, сдвигая его в нужное положение
            		block |= static_cast<uint64_t>(static_cast<unsigned char>(input[i * 8 + j])) << (8 * (7 - j));
        	}
        	blocks.push_back(block); // Добавляем блок в вектор
    	}

    	// Обработка последнего неполного блока
    	uint64_t lastBlock = 0;
    	size_t remaining = length % 8;  // Количество оставшихся символов (меньше 8)
    	for (size_t i = 0; i < remaining; ++i) {
       	 	lastBlock |= static_cast<uint64_t>(static_cast<unsigned char>(input[fullBlocks * 8 + i])) << (8 * (7 - i));
    	}
	// Добавление padding (заполнение оставшихся байтов значением количества недостающих байтов)
    	uint8_t paddingValue = static_cast<uint8_t>(8 - remaining);
    	for (size_t i = remaining; i < 8; ++i) {
        	lastBlock |= static_cast<uint64_t>(paddingValue) << (8 * (7 - i));
    	}

    	blocks.push_back(lastBlock);	// Добавляем последний блок в вектор
    	return blocks;                  // Возвращаем вектор блоков
}

// вектор в строку
string blocksToString(const vector<uint64_t>& blocks) {
    	string output;        // Строка для результата
    	for (size_t i = 0; i < blocks.size(); ++i) {
        	for (int j = 7; j >= 0; --j) {
		// Извлекаем каждый байт из 64-битного блока и добавляем его в строку
            	char byte = static_cast<char>((blocks[i] >> (8 * j)) & 0xFF);
            	output += byte;
        	}
    	}

    	// Удаляем padding
    	uint8_t paddingValue = static_cast<uint8_t>(output.back()); // Последний символ содержит значение padding
    	if (paddingValue > 0 && paddingValue <= 8) {
        	output = output.substr(0, output.size() - paddingValue); // Удаляем последние paddingValue символов
    	}

    	return output;      // Возвращаем строку
}

// генерация ключа для магмы
vector<uint8_t> generateKey(){
	vector<uint8_t> key(32);
	random_device rd;
	mt19937 generator(rd());
	uniform_int_distribution<int> dist(0, 255);

	for(auto& byte : key){
		byte = static_cast<uint8_t>(dist(generator));
	}
	return key;
}

// печать ключа на экран
void printKey(vector<uint8_t>& key){
	for(const auto& byte : key){
		cout << hex << setw(2) << setfill('0') << (int)byte;
	}
	cout << dec << endl;
}

// Вывод блоков
void printBlocks(const vector<uint64_t>& blocks){
	for(size_t i = 0; i < blocks.size(); i++){
		cout << "Block " << i + 1 << ": 0x" << hex << setw(16) << setfill('0') << blocks[i] << endl;
	}
}

// алгоритм развертывания ключа
vector<uint32_t> expandKey(const vector<uint8_t>& mainKey){
	vector<uint32_t> keys(32);

	for(int i = 0; i < 8; i++){
		keys[i] = (mainKey[4 * i + 0] << 24) |
			(mainKey[4 * i + 1] << 16) |
			(mainKey[4 * i + 2] << 8) |
			(mainKey[4 * i + 3]);
	}
	for(int i = 8; i < 24; i++){
		keys[i] = keys[i % 8];
	}

	for(int i = 24; i < 32; i++){
		keys[i] = keys[7 - (i % 8)];
	}
	return keys;
}

// парсер строки в вектор
vector<uint8_t> parse(uint32_t a){
	vector<uint8_t> parts(8);
	for(int i = 0; i < 8; i++){
		parts[i] = (a >> (4*i)) & 0xF;
	}
	return parts;
}

// преобразование t(a)
uint32_t transform(uint32_t a){
	vector<uint8_t> parts = parse(a);
	uint32_t result = 0;
	for(int i = 7; i >= 0; i--){
		uint8_t transformed_nibble = transformation_table[i][parts[i]];
		result |= (transformed_nibble << (4 * i));
	}
	return result;
}

// преобразование g[k](a)
uint32_t g(uint32_t k, uint32_t a){
	uint32_t temp = transform((a + k) & 0xFFFFFFFF);	
	uint32_t shifted = (temp << 11) | (temp >> (32 - 11));
	return(shifted);
}

// преобразование G[k](a_1, a_0)
Result G(uint32_t k, uint32_t a_1, uint32_t a_0){
	uint32_t temp = a_0;
	a_0 = g(k, a_0) ^ a_1;
	a_1 = temp;
	return {a_1, a_0};
}

// преобразование G*[k](a_1, a_0)
uint64_t GStar(uint32_t k, uint32_t a_1, uint32_t a_0){
	uint32_t g_k_a0 = g(k, a_0);
	uint32_t part1 = g_k_a0 ^ a_1;
	uint32_t part2 = a_0;
	return (static_cast<uint64_t>(part1) << 32) | part2;
}

// Алгоритм зашифрования
uint64_t Encrypt(const vector <uint8_t>& mainKey, uint64_t A){
	vector<uint32_t> keys = expandKey(mainKey);
	uint32_t a1 = static_cast<uint32_t>(A >> 32);
	uint32_t a0 = static_cast<uint32_t>(A & 0xFFFFFFFF);
	Result res = {a1, a0};
	for(int i = 0; i < 31; i++){
		res = G(keys[i], res.a_1, res.a_0);
	}

	uint64_t GStarRes = GStar(keys[31], res.a_1, res.a_0);
	return GStarRes;
}

// Алгоритм расшифрования
uint64_t Decrypt(const vector <uint8_t>& mainKey, uint64_t A){
	vector<uint32_t> keys = expandKey(mainKey);
	uint32_t a1 = static_cast<uint32_t>(A >> 32);
	uint32_t a0 = static_cast<uint32_t>(A & 0xFFFFFFFF);
	Result res = {a1, a0};
	for(int i = 31; i > 0; i--){
		res = G(keys[i], res.a_1, res.a_0);
	}

	uint64_t GStarRes = GStar(keys[0], res.a_1, res.a_0);
	return GStarRes;
}

// Шифрование CFB
vector<uint64_t> encryptCFB(const vector<uint8_t>& mainKey, const vector<uint64_t>& plaintextBlocks, uint64_t iv){
	vector<uint64_t> ciphertextBlocks;
	uint64_t feedback = iv;

	for(uint64_t block : plaintextBlocks){
		uint64_t encryptedFeedback = Encrypt(mainKey, feedback);
		uint64_t cipherBlock = block ^ encryptedFeedback;
		ciphertextBlocks.push_back(cipherBlock);
		feedback = cipherBlock;
	}

	return ciphertextBlocks;
}

// Дешифрование CFB
vector<uint64_t> decryptCFB(const vector<uint8_t>& mainKey, const vector<uint64_t>& ciphertextBlocks, uint64_t iv){
	vector<uint64_t> plaintextBlocks;
	uint64_t feedback = iv;

	for(uint64_t cipherBlock : ciphertextBlocks){
		uint64_t encryptedFeedback = Encrypt(mainKey, feedback);
		uint64_t plainBlock = cipherBlock ^ encryptedFeedback;
		plaintextBlocks.push_back(plainBlock);
		feedback = cipherBlock;
	}

	return plaintextBlocks;
}

int main(){
	int mode;
	cout << "Выберите режим Шифрования - 1 или Дешифрования - 2 " << endl;	
	cin >> mode;

	if(mode == 1){
		string plaintextFile;
		cout << "Введите название файла: " << endl;
		cin.ignore();
		getline(cin, plaintextFile);

		ifstream inputFile(plaintextFile);
		if(!inputFile.is_open()){
			cerr << "Ошибка открытия файла " << plaintextFile << endl;
			return 1;	
		}
		stringstream buffer;
		buffer << inputFile.rdbuf();
		string plaintext = buffer.str();
		inputFile.close();

		uint64_t iv = generateIV();
		vector<uint8_t> mainKey = generateKey();

		vector<uint64_t> plaintextBlocks = stringToBlocks(plaintext);
		vector<uint64_t> ciphertextBlocks = encryptCFB(mainKey, plaintextBlocks, iv);

		printBlocks(plaintextBlocks);

		ofstream encryptedFile("encrypted.txt");
		if(!encryptedFile.is_open()){
			cerr << "Не удалось открыть файл encrypted.txt " << endl;
		      	return 1;
		}
		
		for(uint64_t block : ciphertextBlocks){
			encryptedFile << uint64ToHex(block) << " ";
		}

		encryptedFile.close();

		ofstream keyFile("key_iv.txt");
		if(!keyFile.is_open()){
			cerr << "Невозможно открыть файл key_iv.txt " << endl;
		}
		
		keyFile << "Key: " << keyToHex(mainKey) << endl;
		keyFile << "IV: " << uint64ToHex(iv) << endl;
		keyFile.close();

		cout << "Текст зашифрован в файл encrypted.txt" << endl << "Ключи записаны в файл key_iv.txt" << endl;


	}else if(mode == 2){
		cout << "Чтение ключа " << endl;
		ifstream keyFile("key_iv.txt");

		if(!keyFile.is_open()){
			cerr << "Невозможно открыть файл key_iv.txt";
			return 1;
		}	

		string keyHex, ivHex, line;
		getline(keyFile, line);
		keyHex = line.substr(line.find(":") + 2);
		getline(keyFile, line);
		ivHex = line.substr(line.find(":") + 2);
		keyFile.close();

		vector<uint8_t> mainKey = hexToKey(keyHex);
		uint64_t iv = hexToUint64(ivHex);

		ifstream encryptedFile("encrypted.txt");
		if (!encryptedFile.is_open()){
			cerr << "Ошибка: не удалось открыть файл encrypted.txt." << endl;
			return 1;
		}

		vector<uint64_t> ciphertextBlocks;
		string block;
		while (encryptedFile >> block){	
			ciphertextBlocks.push_back(hexToUint64(block));
    		}
	      	encryptedFile.close();

		vector<uint64_t> decryptedBlocks = decryptCFB(mainKey, ciphertextBlocks, iv);
		string decryptedText = blocksToString(decryptedBlocks);
		
		ofstream decryptedFile("decrypted.txt");
		if (!decryptedFile.is_open()) {
			cerr << "Ошибка: не удалось создать файл decrypted.txt." << endl;
		       	return 1;
		}

		decryptedFile << decryptedText;
		decryptedFile.close();

		cout << "Текст расшифрован и записан в decrypted.txt." << endl;
	}else{
		cout << "Ошибка: неизвестный режим." << endl;
	}
	

	return 0;
}
