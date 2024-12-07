#include <iostream>
#include <random>
#include <vector>
#include <iomanip>
#include "transformation_table.h"

using namespace std;

struct Result{
	uint32_t a_1;
	uint32_t a_0;
};

// генерация iv 
uint64_t generateIV(){
	random_device rd;
	mt19937 generator(rd());
	uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
	return dist(generator);
}

// Строка в вектор uint64_t
vector<uint64_t> stringToBlocks(const string& input) {
    	vector<uint64_t> blocks;
	size_t length = input.size();
	size_t fullBlocks = length / 8;

	// Process full blocks
	for (size_t i = 0; i < fullBlocks; ++i) {
      		uint64_t block = 0;
        	for (size_t j = 0; j < 8; ++j) {
            		block |= static_cast<uint64_t>(static_cast<unsigned char>(input[i * 8 + j])) << (8 * (7 - j));
        	}
        	blocks.push_back(block);
    	}

    	// Process the final block with padding
    	uint64_t lastBlock = 0;
    	size_t remaining = length % 8;
    	for (size_t i = 0; i < remaining; ++i) {
       	 	lastBlock |= static_cast<uint64_t>(static_cast<unsigned char>(input[fullBlocks * 8 + i])) << (8 * (7 - i));
    	}
    	uint8_t paddingValue = static_cast<uint8_t>(8 - remaining);
    	for (size_t i = remaining; i < 8; ++i) {
        	lastBlock |= static_cast<uint64_t>(paddingValue) << (8 * (7 - i));
    	}
    	blocks.push_back(lastBlock);

    	return blocks;
}

// вектор в строку
string blocksToString(const vector<uint64_t>& blocks) {
    	string output;
    	for (size_t i = 0; i < blocks.size(); ++i) {
        	for (int j = 7; j >= 0; --j) {
            	char byte = static_cast<char>((blocks[i] >> (8 * j)) & 0xFF);
            	output += byte;
        	}
    	}

    	// Remove padding
    	uint8_t paddingValue = static_cast<uint8_t>(output.back());
    	if (paddingValue > 0 && paddingValue <= 8) {
        	output = output.substr(0, output.size() - paddingValue);
    	}

    	return output;
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
	vector<uint8_t> mainKey = generateKey();
	cout << "Generated key: ";
	printKey(mainKey);
	uint64_t iv = generateIV();
	cout << hex << "IV: " << generateIV() << endl;
/*		
	vector<uint32_t> keys = expandKey(mainKey);
	uint64_t A = 0xfedcba9876543210;
	uint64_t cypher = Encrypt(mainKey, A);
	cout << "Encrypted: " << hex << cypher << endl;
	uint64_t original = Decrypt(mainKey, cypher);
	cout << "Decrypted: " << hex << original << endl;
*/
	string plaintext = "This is a secret message";
	cout << "Plaintext: " << plaintext << endl;

	vector<uint64_t> plaintextBlocks = stringToBlocks(plaintext);
	//cout << hex << plaintextBlocks << endl;
	
	vector<uint64_t> ciphertextBlocks = encryptCFB(mainKey, plaintextBlocks, iv);
	cout << "Ciphertext (hex blocks): ";
	for(uint64_t block : ciphertextBlocks){
		cout << hex << setw(16) << setfill('0') << block << " ";
	}
	cout << endl;

	vector<uint64_t> decryptedBlocks = decryptCFB(mainKey, ciphertextBlocks, iv);
	string decryptedText = blocksToString(decryptedBlocks);
	cout << "Decrypted Text: " << decryptedText << endl;

	return 0;
}
