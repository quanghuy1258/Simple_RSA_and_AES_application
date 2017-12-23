#pragma once
#ifndef RSA_H
#define RSA_H

#include "lib.h"
#include "bigint.h"
#include "err.h"

class RSA {
private:
	BigInt<64> _n, _e, _d;
public:
	BigInt<64> GetN() const;
	BigInt<64> GetE() const;
	BigInt<64> GetD() const;
	RSA& SetN(const BigInt<64>& n);
	RSA& SetE(const BigInt<64>& e);
	RSA& SetD(const BigInt<64>& d);
	RSA& Generate();
	ERR Encrypt(vector<unsigned char>& ciphertext, unsigned char& plaintext, unsigned char& type, int& id) const;
	ERR Decrypt(unsigned char& plaintext, vector<unsigned char>& ciphertext, unsigned char& type, int& id) const;
};

#endif 