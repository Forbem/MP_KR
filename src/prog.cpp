#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

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

		ofstream encryptedFile("output/encrypted.txt");
		if(!encryptedFile.is_open()){
			cerr << "Не удалось открыть файл encrypted.txt " << endl;
		      	return 1;
		}
		
		for(uint64_t block : ciphertextBlocks){
			encryptedFile << uint64ToHex(block);
		}

		encryptedFile.close();

		ofstream keyFile("output/key_iv.txt");
		if(!keyFile.is_open()){
			cerr << "Невозможно открыть файл key_iv.txt " << endl;
		}
		
		keyFile << "Key: " << keyToHex(mainKey) << endl;
		keyFile << "IV: " << uint64ToHex(iv) << endl;
		keyFile.close();

		cout << "Текст зашифрован в файл encrypted.txt" << endl << "Ключи записаны в файл key_iv.txt" << endl;


	}else if(mode == 2){
		cout << "Чтение ключа " << endl;
		ifstream keyFile("output/key_iv.txt");

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

		ifstream encryptedFile("output/encrypted.txt");
		if (!encryptedFile.is_open()){
			cerr << "Ошибка: не удалось открыть файл encrypted.txt." << endl;
			return 1;
		}

		vector<uint64_t> ciphertextBlocks;
		string encryptedData;
		encryptedFile >> encryptedData;
	      	encryptedFile.close();

		for(size_t i = 0; i < encryptedData.length(); i += 16){
			string block = encryptedData.substr(i, 16);
			ciphertextBlocks.push_back(hexToUint64(block));
		}

		vector<uint64_t> decryptedBlocks = decryptCFB(mainKey, ciphertextBlocks, iv);
		string decryptedText = blocksToString(decryptedBlocks);
		
		ofstream decryptedFile("output/decrypted.txt");
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
