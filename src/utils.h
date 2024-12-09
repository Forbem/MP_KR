#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <random>
#include <vector>
#include <iomanip>
#include <fstream>
#include <cstdint>
#include <sstream>

struct Result {
	uint32_t a_1;
	uint32_t a_0;
};

std::string uint64ToHex(uint64_t value);
uint64_t hexToUint64(const std::string& hex);
std::string keyToHex(const std::vector<uint8_t>& key);
std::vector<uint8_t> hexToKey(const std::string& hex);
uint64_t generateIV();
std::vector<uint64_t> stringToBlocks(const std::string& input);
std::string blocksToString(const std::vector<uint64_t>& blocks);
std::vector<uint8_t> generateKey();
void printKey(std::vector<uint8_t>& key);
void printBlocks(const std::vector<uint64_t>& blocks);
std::vector<uint32_t> expandKey(const std::vector<uint8_t>& mainKey);
uint32_t transform(uint32_t a);
uint32_t g(uint32_t k, uint32_t a);
Result G(uint32_t k, uint32_t a_1, uint32_t a_0);
uint64_t GStar(uint32_t k, uint32_t a_1, uint32_t a_0);
uint64_t Encrypt(const std::vector<uint8_t>& mainKey, uint64_t A);
uint64_t Decrypt(const std::vector<uint8_t>& mainKey, uint64_t A);
std::vector<uint64_t> encryptCFB(const std::vector<uint8_t>& mainKey, const std::vector<uint64_t>& plaintextBlocks, uint64_t iv);
std::vector<uint64_t> decryptCFB(const std::vector<uint8_t>& mainKey, const std::vector<uint64_t>& ciphertextBlocks, uint64_t iv);

#endif 
