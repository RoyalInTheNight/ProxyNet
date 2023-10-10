#include "../../include/crypto/sha256.h"

int sha256::ROTR(const int n, const int x) { 
    return (x >> n) | (x << (32 - n));
}