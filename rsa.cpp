#include "rsa.h"

BigInt<64> RSA::GetE() const {
	return _e;
}

BigInt<64> RSA::GetN() const {
	return _n;
}

BigInt<64> RSA::GetD() const {
	return _d;
}

RSA& RSA::SetN(const BigInt<64>& n) {
	_n = n;
	return (*this);
}

RSA& RSA::SetE(const BigInt<64>& e) {
	_e = e;
	return (*this);
}

RSA& RSA::SetD(const BigInt<64>& d) {
	_d = d;
	return (*this);
}

RSA& RSA::Generate() {
	BigInt<64> p; p.GeneratePrime(960, 1024);
	BigInt<64> q; q.GeneratePrime(768, 832);
	_n = p*q; p -= 1; q -= 1;
	BigInt<64> gcd = p.Gcd(q);
	BigInt<64> lambda;
	(p*q).DivMod(gcd, lambda, _d);
	while (1) {
		_e.GeneratePrime(16, 32);
		_d = _e.InverseMod(lambda);
		if (_d.TestBit(0)) break;
	}
	return (*this);
}

ERR RSA::Encrypt(vector<unsigned char>& ciphertext, unsigned char& plaintext, unsigned char& type, int& id) const {
	if (_n.TestBit(0) == false || _e.TestBit(0) == false) return ERR_NO_PUBLICKEY;
	srand(time(NULL));
	int numbytes = 32 * BLOCKBYTE;
	vector<unsigned char> text;
	text.resize(numbytes);
	int dataoffset = (numbytes >> 2) + abs(rand()) % (numbytes >> 2);
	int i = 0;
	text[i++] = 0;
	while (i < dataoffset) {
		text[i] = 0;
		text[i - 1] |= ((abs(rand()) | 1) & 15);
		text[i] |= (text[i - 1] << 4);
		++i;
	}
	text[i++] = plaintext;
	text[i++] = type;
	unsigned char* tmpid = (unsigned char*)&id;
	for (int j = 0; j < sizeof(id); ++j)
		text[i++] = tmpid[j];
	text[i++] = 0;
	while (i < numbytes) {
		text[i] = 0;
		text[i - 1] |= ((abs(rand()) | 1) & 15);
		text[i] |= (text[i - 1] << 4);
		++i;
	}
	return BigInt<64>::Encrypt(ciphertext, text, _e, _n);
}

ERR RSA::Decrypt(unsigned char& plaintext, vector<unsigned char>& ciphertext, unsigned char& type, int& id) const {
	if (_n.TestBit(0) == false || _d.TestBit(0) == false) return ERR_NO_PRIVATEKEY;
	vector<unsigned char> text;
	ERR err = BigInt<64>::Decrypt(text, ciphertext, _d, _n);
	if (err != NO_ERR) return err;
	int numbytes = text.size();
	int i = 0;
	if ((text[i++] >> 4) != 0) return ERR_TEXT_STRUCTURE;
	while (true) {
		if ((text[i - 1] & 15) != (text[i] >> 4)) 
			return ERR_TEXT_STRUCTURE;
		if ((text[i++] & 15) == 0) break;
		if (i == numbytes) 
			return ERR_TEXT_STRUCTURE;
	};
	plaintext = text[i++];
	type = text[i++];
	unsigned char* tmpid = (unsigned char*)&id;
	for (int j = 0; j < sizeof(id); ++j)
		tmpid[j] = text[i++];
	if ((text[i++] >> 4) != 0) return ERR_TEXT_STRUCTURE;
	while (true) {
		if ((text[i - 1] & 15) != (text[i] >> 4)) 
			return ERR_TEXT_STRUCTURE;
		if ((text[i++] & 15) == 0) 
			break;
		if (i == numbytes) 
			return ERR_TEXT_STRUCTURE;
	}
	return NO_ERR;
}