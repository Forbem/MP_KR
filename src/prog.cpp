#include <iostream>
#include <random>
#include <vector>
#include <iomanip>
using namespace std;

int transformation_table[8][16] = { 
	{12, 4, 6, 2, 10, 5, 11, 9, 14, 8, 13, 7, 0, 3, 15, 1},
	{6, 8, 2, 3, 9, 10, 5, 12, 1, 14, 4, 7, 11, 13, 0, 15},
	{11, 3, 5, 8, 2, 15, 10, 13, 14, 1, 7, 4, 12, 9, 6, 0},
	{12, 8, 2, 1, 13, 4, 15, 6, 7, 0, 10, 5, 3, 14, 9, 11},
	{7, 15, 5, 10, 8, 1, 6, 13, 0, 9, 3, 14, 11, 4, 2, 12},
	{5, 13, 15, 6, 9, 2, 12, 10, 11, 7, 8, 1, 4, 3, 14, 0},
	{8, 14, 2, 5, 6, 9, 1, 12, 15, 4, 11, 0, 13, 10, 3, 7},
	{1, 7, 14, 13, 0, 5, 8, 3, 4, 15, 10, 6, 9, 12, 11, 2} 
};     //0  1   2   3  4  5  6  7  8  9  10  11  12  13 14 15

struct Result{
	uint32_t a_1;
	uint32_t a_0;
};

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

void printKey(vector<uint8_t>& key){
	for(const auto& byte : key){
		cout << hex << setw(2) << setfill('0') << (int)byte;
	}
	cout << dec << endl;
}

// алгоритм развертывания ключа
vector<uint32_t> expandKey(vector<uint8_t>& mainKey){
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
	//	cout << i << " " << hex <<static_cast<int>(parts[i]); 
		uint8_t transformed_nibble = transformation_table[i][parts[i]];
	//	cout << "t(a_" << 7 - i << ") = 0x" << hex << static_cast<int>(transformed_nibble) << endl;
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

uint64_t GStar(uint32_t k, uint32_t a_1, uint32_t a_0){
	uint32_t g_k_a0 = g(k, a_0);
	uint32_t part1 = g_k_a0 ^ a_1;
	uint32_t part2 = a_0;
	return (static_cast<uint64_t>(part1) << 32) | part2;
}

uint64_t Encrypt(vector <uint8_t>& mainKey, uint64_t A){
	vector<uint32_t> keys = expandKey(mainKey);
	uint32_t a1 = static_cast<uint32_t>(A >> 32);
	uint32_t a0 = static_cast<uint32_t>(A & 0xFFFFFFFF);
	cout << hex << a1 << " " << a0 << endl;
	Result res = {a1, a0};
	cout << hex << a1 << " " << a0 << endl;
	for(int i = 0; i < 31; i++){
		res = G(keys[i], res.a_1, res.a_0);
	}

	cout << hex << res.a_1 << " " << res.a_0 << endl;
	uint64_t GStarRes = GStar(keys[31], res.a_1, res.a_0);
	return GStarRes;
}

uint64_t Decrypt(vector <uint8_t>& mainKey, uint64_t A){
	vector<uint32_t> keys = expandKey(mainKey);
	uint32_t a1 = static_cast<uint32_t>(A >> 32);
	uint32_t a0 = static_cast<uint32_t>(A & 0xFFFFFFFF);
//	cout << hex << a1 << " " << a0 << endl;
	Result res = {a1, a0};
//	cout << hex << a1 << " " << a0 << endl;
	for(int i = 31; i > 0; i--){
		res = G(keys[i], res.a_1, res.a_0);
	}

//	cout << hex << res.a_1 << " " << res.a_0 << endl;
	uint64_t GStarRes = GStar(keys[0], res.a_1, res.a_0);
	return GStarRes;
}

int main(){
	vector<uint8_t> mainKey = {
		0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
		0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
		0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
		0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
	}; //generateKey();
	cout << "Generated key: ";
	printKey(mainKey);

	vector<uint32_t> keys = expandKey(mainKey);
	for(int i = 0; i < 32; i++){
		cout << "K_" << dec << i + 1  << " = " << hex << keys[i] << endl;
	}

	uint32_t test_a = 0xfdb97531;
	int i = 0;
	while(i < 4){
		cout << "t(" << hex << test_a << ") = ";
		test_a = transform(test_a);
		cout << hex << test_a << endl;
		i++;
	}
	i = 0;
	cout << endl;

	uint32_t test_k = 0x87654321;
	test_a = 0xfedcba98;
	uint32_t temp;
	while(i < 4){
		temp = test_k;
		cout << "g[" << hex << test_k << "](" << test_a << ") = ";
		test_k = g(test_k, test_a);
		cout << hex << test_k << endl;
		test_a = temp;
		i++;
			
	}
/*
	uint32_t test_a1 = 0x80251e99;
	uint32_t test_a0 = 0x2b96eca6;

	Result result = G(keys[0], test_a1, test_a0);

	cout << hex << GStar(keys[28], test_a1, test_a0) << endl;
	cout << hex << result.a_1 << " " << result.a_0 << endl;
*/
	uint64_t A = 0xfedcba9876543210;
	uint64_t cypher = Encrypt(mainKey, A);
	cout << "Encrypted: " << hex << cypher << endl;
	uint64_t original = Decrypt(mainKey, cypher);
	cout << "Decrypted: " << hex << original << endl;
	return 0;
}
