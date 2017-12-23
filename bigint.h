#pragma once
#ifndef BIGINT_H
#define BIGINT_H

#include "lib.h"
#include "primes.h"
#include "err.h"

const int BLOCKBYTE = (sizeof(unsigned long long) >> 1);
const int BLOCKBIT = (sizeof(unsigned long long) << 2);
const unsigned long long BLOCKMASK = ~((~(unsigned long long(0))) << BLOCKBIT);

template<int numblocks>
class BigInt {
private:
	unsigned long long _data[numblocks];
	friend class BigInt< (numblocks >> 1) >;
public:
	BigInt() {
		memset(_data, 0, sizeof(unsigned long long)*numblocks);
	}
	BigInt(const BigInt<numblocks>& bi) {
		memcpy(_data, bi._data, sizeof(unsigned long long)*numblocks);
	}
	BigInt(int data) {
		this->SetData(data);
	}
	BigInt(const vector<char>& data) {
		this->SetData(data);
	}

	~BigInt() {}

	BigInt<numblocks>& SetData(int data) {
		int i = 0;
		while (data != 0) {
			unsigned long long ulltmp = (unsigned int)(data);
			ulltmp &= BLOCKMASK;
			if (i < numblocks) _data[i++] = ulltmp;
			data = (BLOCKBIT >= sizeof(data)) ? 0 : (data >> BLOCKBIT);
		}
		while (i < numblocks) _data[i++] = 0;
		return (*this);
	}
	BigInt<numblocks>& SetData(const vector<char>& data) {
		memset(_data, 0, sizeof(unsigned long long)*numblocks);
		int numbytes = data.size();
		while (numbytes--) {
			if (numbytes / BLOCKBYTE >= numblocks) continue;
			_data[numbytes / BLOCKBYTE] = (((_data[numbytes / BLOCKBYTE]) << 8) | (unsigned char(data[numbytes])));
		}
		return (*this);
	}
	void GetData(vector<char>& data) const {
		int numbytes = numblocks * BLOCKBYTE;
		data.resize(numbytes);
		while (numbytes--) {
			data[numbytes] = 0;
			if (numbytes / BLOCKBYTE >= numblocks) continue;
			data[numbytes] |= (long long((_data[numbytes / BLOCKBYTE]) >> ((numbytes % BLOCKBYTE) << 3)));
		}
	}

	BigInt<numblocks>& operator = (const BigInt<numblocks>& bi) {
		memcpy(_data, bi._data, sizeof(unsigned long long)*numblocks);
		return (*this);
	}
	BigInt<numblocks> operator + (const BigInt<numblocks>& bi) const {
		BigInt<numblocks> tmp = (*this);
		tmp += bi;
		return tmp;
	}
	BigInt<numblocks> operator + (unsigned long long ull) const {
		BigInt<numblocks> tmp = (*this);
		tmp += ull;
		return tmp;
	}
	BigInt<numblocks>& operator += (const BigInt<numblocks>& bi) {
		unsigned long long c = 0;
		for (int i = 0; i < numblocks; ++i) {
			_data[i] += bi._data[i] + c;
			c = _data[i] >> BLOCKBIT;
			_data[i] &= BLOCKMASK;
		}
		return (*this);
	}
	BigInt<numblocks>& operator += (unsigned long long ull) {
		ull &= BLOCKMASK;
		_data[0] += ull;
		unsigned long long c = 0;
		for (int i = 0; i < numblocks; ++i) {
			_data[i] += c;
			c = _data[i] >> BLOCKBIT;
			_data[i] &= BLOCKMASK;
		}
		return (*this);
	}
	BigInt<numblocks> operator - (const BigInt<numblocks>& bi) const {
		BigInt<numblocks> tmp = (*this);
		tmp -= bi;
		return tmp;
	}
	BigInt<numblocks> operator - (unsigned long long ull) const {
		BigInt<numblocks> tmp = (*this);
		tmp -= ull;
		return tmp;
	}
	BigInt<numblocks>& operator -= (const BigInt<numblocks>& bi) {
		++_data[0];
		unsigned long long c = 0;
		for (int i = 0; i < numblocks; ++i) {
			_data[i] += ((bi._data[i]) ^ BLOCKMASK) + c;
			c = _data[i] >> BLOCKBIT;
			_data[i] &= BLOCKMASK;
		}
		return (*this);
	}
	BigInt<numblocks>& operator -= (unsigned long long ull) {
		ull &= BLOCKMASK;
		_data[0] += 1 + (ull ^ BLOCKMASK);
		unsigned long long c = _data[0] >> BLOCKBIT;
		_data[0] &= BLOCKMASK;
		for (int i = 1; i < numblocks; ++i) {
			_data[i] += BLOCKMASK + c;
			c = _data[i] >> BLOCKBIT;
			_data[i] &= BLOCKMASK;
		}
		return (*this);
	}
	BigInt<numblocks> operator * (const BigInt<numblocks>& bi) const {
		BigInt<numblocks> tmp;
		const BigInt<numblocks> *a, *b;
		int mostblock, bimostblock, cntblock, bicntblock; mostblock = bimostblock = cntblock = bicntblock = 0;
		for (int i = 0; i < numblocks; ++i) {
			if (_data[i] != 0) {
				mostblock = i + 1;
				++cntblock;
			}
			if (bi._data[i] != 0) {
				bimostblock = i + 1;
				++bicntblock;
			}
		}
		if (cntblock > bicntblock) {
			a = &bi;
			b = this;
			mostblock = bimostblock;
		}
		else {
			a = this;
			b = &bi;
		}
		unsigned long long c;
		for (int i = 0; i < mostblock; ++i) {
			if ((a->_data[i]) == 0) continue;
			c = 0;
			for (int j = 0; j < numblocks - i; ++j) {
				tmp._data[i + j] += ((a->_data[i]) * (b->_data[j])) + c;
				c = tmp._data[i + j] >> BLOCKBIT;
				tmp._data[i + j] &= BLOCKMASK;
			}
		}
		return tmp;
	}
	BigInt<numblocks> operator * (unsigned long long ull) const {
		ull &= BLOCKMASK;
		BigInt<numblocks> tmp;
		unsigned long long c = 0;
		for (int i = 0; i < numblocks; ++i) {
			tmp._data[i] += (_data[i] * ull) + c;
			c = tmp._data[i] >> BLOCKBIT;
			tmp._data[i] &= BLOCKMASK;
		}
		return tmp;
	}
	BigInt<numblocks>& operator *= (const BigInt<numblocks>& bi) {
		(*this) = (*this) * bi;
		return (*this);
	}
	BigInt<numblocks>& operator *= (unsigned long long ull) {
		(*this) = (*this) * ull;
		return (*this);
	}
	BigInt<numblocks> operator % (const BigInt<numblocks>& bi) const {
		BigInt<numblocks> tmp;
		int mostbit, bimostbit;
		for (mostbit = numblocks * BLOCKBIT - 1; mostbit >= 0; --mostbit) 
			if (this->TestBit(mostbit)) break;
		for (bimostbit = numblocks * BLOCKBIT - 1; bimostbit >= 0; --bimostbit) 
			if (bi.TestBit(bimostbit)) break;
		if ((mostbit == -1) || (bimostbit < 1)) return tmp;
		if (mostbit < bimostbit) return (*this);
		tmp = (*this) >> (mostbit - bimostbit + 1);
		for (int i = mostbit - bimostbit; i >= 0; --i) {
			tmp <<= 1;
			if (this->TestBit(i)) tmp.SetBit(0);
			if (tmp >= bi) tmp -= bi;
		}
		return tmp;
	}
	unsigned long long operator % (unsigned long long ull) {
		ull &= BLOCKMASK;
		unsigned long long ret = 0;
		if (ull == 0) return ret;
		for (int i = numblocks - 1; i >= 0; --i) {
			ret <<= BLOCKBIT;
			ret |= _data[i];
			ret %= ull;
		}
		return ret;
	}
	BigInt<numblocks>& operator %= (const BigInt<numblocks>& bi) {
		(*this) = (*this) % bi;
		return (*this);
	}
	BigInt<numblocks> operator & (const BigInt<numblocks>& bi) const {
		BigInt<numblocks> tmp = (*this);
		tmp &= bi;
		return tmp;
	}
	BigInt<numblocks>& operator &= (const BigInt<numblocks>& bi) {
		for (int i = 0; i < numblocks; ++i)
			_data[i] &= bi._data[i];
		return (*this);
	}
	BigInt<numblocks> operator | (const BigInt<numblocks>& bi) const {
		BigInt<numblocks> tmp = (*this);
		tmp |= bi;
		return tmp;
	}
	BigInt<numblocks>& operator |= (const BigInt<numblocks>& bi) {
		for (int i = 0; i < numblocks; ++i)
			_data[i] |= bi._data[i];
		return (*this);
	}
	BigInt<numblocks> operator ^ (const BigInt<numblocks>& bi) const {
		BigInt<numblocks> tmp = (*this);
		tmp ^= bi;
		return tmp;
	}
	BigInt<numblocks>& operator ^= (const BigInt<numblocks>& bi) {
		for (int i = 0; i < numblocks; ++i)
			_data[i] ^= bi._data[i];
		return (*this);
	}
	BigInt<numblocks> operator >> (unsigned int numBits) const {
		BigInt<numblocks> tmp;
		for (int i = 0; i < numblocks; ++i) {
			unsigned long long ulltmp1 = ((_data[i]) >> (numBits % BLOCKBIT));
			unsigned long long ulltmp2 = ((_data[i]) << (BLOCKBIT - numBits % BLOCKBIT));
			ulltmp2 &= BLOCKMASK;
			int nBlock1 = i - numBits / BLOCKBIT;
			int nBlock2 = i - numBits / BLOCKBIT - 1;
			if ((nBlock1 >= 0) && (nBlock1 < numblocks)) tmp._data[nBlock1] |= ulltmp1;
			if ((nBlock2 >= 0) && (nBlock2 < numblocks)) tmp._data[nBlock2] |= ulltmp2;
		}
		return tmp;
	}
	BigInt<numblocks>& operator >>= (unsigned int numBits) {
		(*this) = (*this) >> numBits;
		return (*this);
	}
	BigInt<numblocks> operator << (unsigned int numBits) const {
		BigInt<numblocks> tmp;
		for (int i = 0; i < numblocks; ++i) {
			unsigned long long ulltmp1 = ((_data[i]) << (numBits % BLOCKBIT));
			unsigned long long ulltmp2 = ((_data[i]) >> (BLOCKBIT - numBits % BLOCKBIT));
			ulltmp1 &= BLOCKMASK;
			int nBlock1 = i + numBits / BLOCKBIT;
			int nBlock2 = i + numBits / BLOCKBIT + 1;
			if ((nBlock1 >= 0) && (nBlock1 < numblocks)) tmp._data[nBlock1] |= ulltmp1;
			if ((nBlock2 >= 0) && (nBlock2 < numblocks)) tmp._data[nBlock2] |= ulltmp2;
		}
		return tmp;
	}
	BigInt<numblocks>& operator <<= (unsigned int numBits) {
		(*this) = ((*this) << numBits);
		return (*this);
	}
	bool operator > (const BigInt<numblocks>& bi) const {
		for (int i = numblocks - 1; i >= 0; --i) {
			if (_data[i] > bi._data[i]) return true;
			else if (_data[i] < bi._data[i]) return false;
		}
		return false;
	}
	bool operator >= (const BigInt<numblocks>& bi) const {
		for (int i = numblocks - 1; i >= 0; --i) {
			if (_data[i] > bi._data[i]) return true;
			else if (_data[i] < bi._data[i]) return false;
		}
		return true;
	}
	bool operator < (const BigInt<numblocks>& bi) const {
		for (int i = numblocks - 1; i >= 0; --i) {
			if (_data[i] < bi._data[i]) return true;
			else if (_data[i] > bi._data[i]) return false;
		}
		return false;
	}
	bool operator <= (const BigInt<numblocks>& bi) const {
		for (int i = numblocks - 1; i >= 0; --i) {
			if (_data[i] < bi._data[i]) return true;
			else if (_data[i] > bi._data[i]) return false;
		}
		return true;
	}
	bool operator == (const BigInt<numblocks>& bi) const {
		for (int i = numblocks - 1; i >= 0; --i) {
			if (_data[i] != bi._data[i]) return false;
		}
		return true;
	}
	bool operator != (const BigInt<numblocks>& bi) const {
		for (int i = numblocks - 1; i >= 0; --i) {
			if (_data[i] != bi._data[i]) return true;
		}
		return false;
	}

	BigInt<numblocks> Negate() const {
		BigInt<numblocks> tmp;
		for (int i = 0; i < numblocks; ++i) 
			tmp._data[i] = (_data[i]) ^ BLOCKMASK;
		tmp._data[0] += 1;
		unsigned long long c = 0;
		for (int i = 0; i < numblocks; ++i)	{
			tmp._data[i] += c;
			c = ((tmp._data[i]) >> BLOCKBIT);
			tmp._data[i] &= BLOCKMASK;
		}
		return tmp;
	}
	BigInt<numblocks>& ClearBit(int nBit) {
		unsigned long long ulltmp = 1; ulltmp <<= (nBit % BLOCKBIT); ulltmp = ~ulltmp;
		int nBlock = nBit / BLOCKBIT;
		if ((nBlock >= 0) && (nBlock < numblocks)) _data[nBlock] &= ulltmp;
		return (*this);
	}
	BigInt<numblocks>& SetBit(int nBit) {
		unsigned long long ulltmp = 1; ulltmp <<= (nBit % BLOCKBIT); 
		int nBlock = nBit / BLOCKBIT;
		if ((nBlock >= 0) && (nBlock < numblocks)) _data[nBlock] |= ulltmp;
		return (*this);
	}
	BigInt<numblocks>& FlipBit(int nBit) {
		unsigned long long ulltmp = 1; ulltmp <<= (nBit % BLOCKBIT); 
		int nBlock = nBit / BLOCKBIT;
		if ((nBlock >= 0) && (nBlock < numblocks)) _data[nBlock] ^= ulltmp;
		return (*this);
	}
	bool TestBit(int nBit) const {
		int nBlock = nBit / BLOCKBIT;
		unsigned long long ulltmp = 1;
		if ((nBlock >= 0) && (nBlock < numblocks)) {
			ulltmp = ((_data[nBlock]) >> (nBit % BLOCKBIT)) & ulltmp;
			if (ulltmp == 0) return false;
			else return true;
		}
		else return false;
	}
	unsigned long long GetNN() const {
		if (_data[0] == 0) return 0;
		unsigned long long t = 0, newt = 1, r = BLOCKMASK + 1, newr = _data[0], quotient, tmp;
		while (newr != 0) {
			quotient = r / newr;
			tmp = r % newr; r = newr; newr = tmp;
			tmp = quotient * newt; tmp = t + 1 + (~tmp); t = newt; newt = tmp;
		}
		if (r > 1) return 0;
		if ((t >> (sizeof(t) - 1)) == 1) t = t + _data[0];
		t = ((~t) + 1) & BLOCKMASK;
		return t;
	}
	BigInt<numblocks>& MontgomeryTransform(const BigInt<numblocks>& N, int p) {
		(*this) <<= ((numblocks - p) * BLOCKBIT);
		(*this) = (*this) % N;
		return (*this);
	}
	BigInt<numblocks>& MultiPrecisionREDC(const BigInt<numblocks>& N, int p, unsigned long long NN) {
		unsigned long long c, m, x, cc;
		if (((N._data[0] * NN) & BLOCKMASK) != BLOCKMASK) return (*this);
		cc = 0;
		for (int i = 0; i < numblocks - p; ++i) {
			if (_data[i] == 0) continue;
			c = 0;
			m = ((_data[i] * NN) & BLOCKMASK);
			for (int j = 0; j < p; ++j) {
				x = _data[i + j] + m*N._data[j] + c;
				_data[i + j] = (x & BLOCKMASK);
				c = (x >> BLOCKBIT);
			}
			for (int j = p; j < numblocks - i; j++) {
				x = _data[i + j] + c;
				_data[i + j] = (x & BLOCKMASK);
				c = (x >> BLOCKBIT);
			}
			x = cc + c;
			cc = (x & BLOCKMASK);
		}
		for (int i = 0; i < p; ++i) {
			_data[i] = _data[i + numblocks - p];
			_data[i + numblocks - p] = 0;;
		}
		_data[p] = cc;
		if ((*this) >= N) (*this) -= N;
		return (*this);
	}
	void DivMod(const BigInt<numblocks>& bi, BigInt<numblocks>& q, BigInt<numblocks>& r) const {
		memset(q._data, 0, sizeof(unsigned long long)*numblocks);
		memset(r._data, 0, sizeof(unsigned long long)*numblocks);
		int mostbit, bimostbit;
		for (mostbit = numblocks * BLOCKBIT - 1; mostbit >= 0; --mostbit)
			if (this->TestBit(mostbit)) break;
		for (bimostbit = numblocks * BLOCKBIT - 1; bimostbit >= 0; --bimostbit)
			if (bi.TestBit(bimostbit)) break;
		if ((mostbit == -1) || (bimostbit == -1)) return;
		if (bimostbit == 0) {
			q = (*this);
			return;
		}
		if (mostbit < bimostbit) {
			r = (*this);
			return;
		}
		r = ((*this) >> (mostbit - bimostbit + 1));
		for (int i = mostbit - bimostbit; i >= 0; --i) {
			r <<= 1;
			if (this->TestBit(i)) r.SetBit(0);
			if (r >= bi) {
				r -= bi;
				q.SetBit(i);
			}
		}
	}
	unsigned long long DivMod(unsigned long long ull, BigInt<numblocks>& q) const {
		ull &= BLOCKMASK;
		unsigned long long ret = 0;
		if (ull == 0) return ret;
		for (int i = numblocks - 1; i >= 0; --i) {
			ret <<= BLOCKBIT;
			ret |= _data[i];
			q._data[i] = ret / ull;
			ret %= ull;
		}
		return ret;
	}
	BigInt<numblocks> Gcd(const BigInt<numblocks>& num) const {
		BigInt<numblocks> quotient, tmp, n0(0), n1(1);
		BigInt<numblocks> r(num), newr(*this);
		if ((r == n0) || (newr == n0)) return n0;
		while (newr != n0) {
			r.DivMod(newr, quotient, tmp);
			r = newr; newr = tmp;
		}
		return r;
	}
	BigInt<numblocks> InverseMod(const BigInt<numblocks>& mod) const {
		BigInt<numblocks> quotient, tmp, n0(0), n1(1);
		BigInt<numblocks> t(n0), newt(n1);
		BigInt<numblocks> r(mod), newr(*this);		
		while (newr != n0) {
			r.DivMod(newr, quotient, tmp);
			r = newr; newr = tmp;
			tmp = t; t = newt; newt = tmp - quotient * newt;
		}
		if (r > n1) return n0;
		if (t.TestBit(numblocks*BLOCKBIT - 1)) t += mod;
		return t;
	}
	BigInt<numblocks> PowMod(const BigInt<numblocks>& num, const BigInt<numblocks>& mod) const {
		BigInt<numblocks> tmp(1);
		int i = numblocks * BLOCKBIT - 1;
		while (i >= 0) {
			if (num.TestBit(i)) break;
			--i;
		}
		while (i >= 0) {
			tmp = tmp * tmp;
			if (tmp >= mod) tmp = tmp % mod;
			if (num.TestBit(i)) {
				tmp = tmp * (*this); 
				if (tmp >= mod) tmp = tmp % mod;
			}
			--i;
		}
		return tmp;
	}
	BigInt<numblocks> PowMod(const BigInt<numblocks>& num, const BigInt<numblocks>& mod, int p, unsigned long long NN, const BigInt<numblocks>& n1, int numbit) const {
		BigInt<numblocks> tmp(n1);
		for (int i = numbit - 1; i >= 0; --i) {
			tmp = tmp * tmp;
			tmp.MultiPrecisionREDC(mod, p, NN);
			if (num.TestBit(i)) {
				tmp = tmp * (*this);
				tmp.MultiPrecisionREDC(mod, p, NN);
			}
		}
		return tmp;
	}
	bool isProbablePrime() const {
		BigInt<numblocks> n0(0), n1(1), n2(2);
		BigInt<numblocks> X((*this) - n1), d(X), tmp;
		if ((*this) == n2) return true;
		if ((*this) < n2) return false;
		int numbit, s, i, nblock, times;
		for (numbit = numblocks*BLOCKBIT; numbit > 0; --numbit) {
			if (d.TestBit(numbit - 1)) break;
		}
		for (s = 0; s < numblocks*BLOCKBIT; ++s) {
			if (d.TestBit(s)) break;
		}
		for (nblock = numblocks; nblock > 0; --nblock) {
			if (_data[nblock - 1] != 0) break;
		}
		if (s == 0) return false;
		d >>= s;
		unsigned long long NN = this->GetNN();
		n1.MontgomeryTransform(*this, nblock);
		X.MontgomeryTransform(*this, nblock);
		if (nblock >= 400) times = 10;
		else if (nblock >= 200) times = 20;
		else if (nblock >= 100) times = 30;
		else times = 40;
		while (times--) {
			tmp.SetData(PRIMES[times]).MontgomeryTransform(*this, nblock);
			tmp = tmp.PowMod(d, *this, nblock, NN, n1, numbit - s);
			if ((tmp == n1) || (tmp == X)) continue;
			for (i = 0; i < s; ++i) {
				tmp = tmp * tmp;
				tmp.MultiPrecisionREDC(*this, nblock, NN);
				if (tmp == n1) return false;
				if (tmp == X) break;
			}
			if (i == s) return false;
		}
		return true;
	}
	BigInt<numblocks>& GeneratePrime(int beginRangeBit, int endRangeBit) {
		beginRangeBit = abs(beginRangeBit);
		if ((beginRangeBit > ((numblocks*BLOCKBIT) >> 1)) || (beginRangeBit < 2)) return (*this);
		endRangeBit = abs(endRangeBit);
		if ((endRangeBit > ((numblocks*BLOCKBIT) >> 1)) || (beginRangeBit > endRangeBit)) return (*this);
		srand(time(NULL));
		int randRangeBit = beginRangeBit + abs(rand()) % (endRangeBit - beginRangeBit + 1);
		memset(_data, 0, sizeof(unsigned long long)*numblocks);
		while (1) {
			for (int i = 0; i <= randRangeBit / BLOCKBIT; ++i) _data[i] = abs(rand()); 
			_data[randRangeBit / BLOCKBIT] <<= (BLOCKBIT - (randRangeBit % BLOCKBIT));
			_data[randRangeBit / BLOCKBIT] &= BLOCKMASK;
			_data[randRangeBit / BLOCKBIT] >>= (BLOCKBIT - (randRangeBit % BLOCKBIT));
			this->SetBit(randRangeBit - 1);
			this->SetBit(0);
			if ((this->isProbablePrime()) == true) return (*this);
		}
		return (*this);
	}

	static ERR Encrypt(vector<unsigned char>& ciphertext, const vector<unsigned char>& plaintext, const BigInt<numblocks>& e, const BigInt<numblocks>& n) {
		int nblock, numbit, textblock;
		for (nblock = numblocks; nblock > 0; --nblock) {
			if (n._data[nblock - 1] != 0) break;
		}
		for (numbit = numblocks*BLOCKBIT; numbit > 0; --numbit) {
			if (e.TestBit(numbit - 1)) break;
		}
		if ((plaintext.size() % (BLOCKBYTE * (numblocks >> 1))) != 0) return ERR_PLAINTEXT_TO_BIGNUM;
		if (nblock <= (numblocks >> 1)) return ERR_NOT_ENOUGH_SIZE_N;
		BigInt< (numblocks << 1) > newe, newn, n1(1);
		memcpy(newe._data, e._data, sizeof(unsigned long long)*numblocks);
		memcpy(newn._data, n._data, sizeof(unsigned long long)*numblocks);
		n1.MontgomeryTransform(newn, numblocks);
		unsigned long long NN = newn.GetNN();
		textblock = plaintext.size() / (BLOCKBYTE * (numblocks >> 1));
		ciphertext.resize(textblock * nblock * BLOCKBYTE);
		for (int i = 0; i < textblock; ++i) {
			BigInt< (numblocks << 1) > tmp;
			for (int x = 0; x < (numblocks >> 1); ++x) {
				for (int y = BLOCKBYTE - 1; y >= 0; --y) {
					tmp._data[x] <<= 8;
					tmp._data[x] |= plaintext[i * BLOCKBYTE * (numblocks >> 1) + x * BLOCKBYTE + y];
				}
			}
			tmp.MontgomeryTransform(newn, numblocks);
			tmp = tmp.PowMod(newe, newn, numblocks, NN, n1, numbit);
			tmp.MultiPrecisionREDC(newn, numblocks, NN);
			for (int x = 0; x < nblock; ++x) {
				for (int y = 0; y < BLOCKBYTE; ++y) {
					ciphertext[i * BLOCKBYTE * nblock + x * BLOCKBYTE + y] = (unsigned char)tmp._data[x];
					tmp._data[x] >>= 8;
				}
			}
		}
		return NO_ERR;
	}
	static ERR Decrypt(vector<unsigned char>& plaintext, const vector<unsigned char>& ciphertext, const BigInt<numblocks>& d, const BigInt<numblocks>& n) {
		int nblock, numbit, textblock;
		for (nblock = numblocks; nblock > 0; --nblock) {
			if (n._data[nblock - 1] != 0) break;
		}
		for (numbit = numblocks*BLOCKBIT; numbit > 0; --numbit) {
			if (d.TestBit(numbit - 1)) break;
		}
		if (ciphertext.size() % (BLOCKBYTE * nblock) != 0) return ERR_CIPHERTEXT_TO_BIGNUM;
		if (nblock <= (numblocks >> 1)) return ERR_NOT_ENOUGH_SIZE_N;
		BigInt< (numblocks << 1) > newd, newn, n1(1);
		memcpy(newd._data, d._data, sizeof(unsigned long long)*numblocks);
		memcpy(newn._data, n._data, sizeof(unsigned long long)*numblocks);
		n1.MontgomeryTransform(newn, numblocks);
		unsigned long long NN = newn.GetNN();
		textblock = ciphertext.size() / (BLOCKBYTE * nblock);
		plaintext.resize(textblock * (numblocks >> 1) * BLOCKBYTE);
		for (int i = 0; i < textblock; ++i) {
			BigInt< (numblocks << 1) > tmp;
			for (int x = 0; x < nblock; ++x) {
				for (int y = BLOCKBYTE - 1; y >= 0; --y) {
					tmp._data[x] <<= 8;
					tmp._data[x] |= ciphertext[i * BLOCKBYTE * nblock + x * BLOCKBYTE + y];
				}
			}
			tmp.MontgomeryTransform(newn, numblocks);
			tmp = tmp.PowMod(newd, newn, numblocks, NN, n1, numbit);
			tmp.MultiPrecisionREDC(newn, numblocks, NN);
			for (int x = 0; x < (numblocks >> 1); ++x) {
				for (int y = 0; y < BLOCKBYTE; ++y) {
					plaintext[i * BLOCKBYTE * (numblocks >> 1) + x * BLOCKBYTE + y] = (unsigned char)tmp._data[x];
					tmp._data[x] >>= 8;
				}
			}
		}
		return NO_ERR;
	}
};
#endif