#include "lib.h"
#include "bigint.h"
#include "rsa.h"
#include "AES.h"

void CreateAndSaveRSAKey() {
	RSA rsa;
	cout << "RSA key generation: Start" << endl;
	rsa.Generate();
	cout << "RSA key generation: Finish" << endl;
	wstring nfilename; nfilename += PUBLICKEY_FOLDER; nfilename += L"\\"; nfilename += N_FILE;
	wstring efilename; efilename += PUBLICKEY_FOLDER; efilename += L"\\"; efilename += E_FILE;
	wstring dfilename; dfilename += PRIVATEKEY_FOLDER; dfilename += L"\\"; dfilename += D_FILE;
	ofstream nfile; nfile.open(nfilename, ios::binary);
	ofstream efile; efile.open(efilename, ios::binary);
	ofstream dfile; dfile.open(dfilename, ios::binary);
	if (nfile.is_open() == false || efile.is_open() == false || dfile.is_open() == false) {
		cout << "!!! Cannot open file" << endl;
		return;
	}
	vector<char> buffer;
	rsa.GetN().GetData(buffer);
	nfile.write(buffer.data(), buffer.size());
	rsa.GetE().GetData(buffer);
	efile.write(buffer.data(), buffer.size());
	rsa.GetD().GetData(buffer);
	dfile.write(buffer.data(), buffer.size());
	nfile.close(); efile.close(); dfile.close();
	cout << "Save RSA key: Success" << endl;
}

void RSAEncryption() {
	RSA rsa;
	BigInt<64> num;
	wstring nfilename; nfilename += PUBLICKEY_FOLDER; nfilename += L"\\"; nfilename += N_FILE;
	wstring efilename; efilename += PUBLICKEY_FOLDER; efilename += L"\\"; efilename += E_FILE;
	ifstream nfile; nfile.open(nfilename, ios::binary);
	ifstream efile; efile.open(efilename, ios::binary);
	if (nfile.is_open() == false) {
		wcout << L"!!! Cannot open file: " << nfilename << endl;
		wcout << L"!!! Please add file: " << nfilename << endl;
		return;
	}
	if (efile.is_open() == false) {
		wcout << L"!!! Cannot open file: " << efilename << endl;
		wcout << L"!!! Please add file: " << efilename << endl;
		return;
	}
	vector<char> buffer; buffer.resize(256);
	copy_n((istreambuf_iterator<char>(nfile)), 256, buffer.begin());
	num.SetData(buffer); rsa.SetN(num);
	fill(buffer.begin(), buffer.end(), 0);
	copy_n((istreambuf_iterator<char>(efile)), 256, buffer.begin());
	num.SetData(buffer); rsa.SetE(num);
	fill(buffer.begin(), buffer.end(), 0);
	nfile.close(); efile.close();
	cout << "Load RSA public key: Success" << endl;
	HANDLE hFind;
	wstring pattern(ASYM_PLAINTEXT_FOLDER); pattern.append(L"\\*");
	WIN32_FIND_DATA data;
	cout << "*************************************************" << endl;
	if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
		srand(time(NULL));
		wstring plaintextpath(ASYM_PLAINTEXT_FOLDER); plaintextpath.append(L"\\");
		wstring ciphertextpath(ASYM_CIPHERTEXT_FOLDER); ciphertextpath.append(L"\\");
		wstring filename;
		ifstream plaintextfile;
		ofstream ciphertextfile;
		vector<unsigned char> cipher;
		wstringstream wss;
		do {
			filename = data.cFileName;
			if (filename == L"." || filename == L"..") continue;
			wcout << L"Encrypting plaintext file: " << filename << endl;
			plaintextfile.open(plaintextpath + filename, ios::binary);
			unsigned char olducc, type = 1; int id = abs(rand());
			plaintextfile.read((char*)&olducc, 1);
			for (int i = 0; ; ++i) {
				unsigned char ucc;
				plaintextfile.read((char*)&ucc, 1);
				if (plaintextfile.eof()) type |= 2;
				ERR err = rsa.Encrypt(cipher, olducc, type, id);
				olducc = ucc;
				if (err == ERR_NOT_ENOUGH_SIZE_N) {
					cout << "!!! N too small" << endl;
					break;
				}
				else if (err == ERR_NO_PUBLICKEY) {
					cout << "!!! No publickey" << endl;
					break;
				}
				else if (err != NO_ERROR) {
					cout << "!!! ERROR" << endl;
					break;
				}
				wss << ciphertextpath << filename << i;
				ciphertextfile.open(wss.str(), ios::binary);
				wcout << L"Ciphertext file: " << wss.str() << endl;
				wss.str(L"");
				ciphertextfile.write((char*)cipher.data(), cipher.size());
				ciphertextfile.close();
				cipher.clear();
				if (plaintextfile.eof()) break;
				type = 0; ++id;
			}
			plaintextfile.close();
			wcout << L"Encrypted plaintext file: " << filename << endl;
			cout << "*************************************************" << endl;
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
	else cout << "!!! No plaintext file to encrypt" << endl;
}

void RSADecryption() {
	RSA rsa;
	BigInt<64> num;
	wstring nfilename; nfilename += PUBLICKEY_FOLDER; nfilename += L"\\"; nfilename += N_FILE;
	wstring dfilename; dfilename += PRIVATEKEY_FOLDER; dfilename += L"\\"; dfilename += D_FILE;
	ifstream nfile; nfile.open(nfilename, ios::binary);
	ifstream dfile; dfile.open(dfilename, ios::binary);
	if (nfile.is_open() == false) {
		wcout << L"!!! Cannot open file: " << nfilename << endl;
		wcout << L"!!! Please add file: " << nfilename << endl;
		return;
	}
	if (dfile.is_open() == false) {
		wcout << L"!!! Cannot open file: " << dfilename << endl;
		wcout << L"!!! Please add file: " << dfilename << endl;
		return;
	}
	vector<char> buffer; buffer.resize(256);
	copy_n((istreambuf_iterator<char>(nfile)), 256, buffer.begin());
	num.SetData(buffer); rsa.SetN(num);
	fill(buffer.begin(), buffer.end(), 0);
	copy_n((istreambuf_iterator<char>(dfile)), 256, buffer.begin());
	num.SetData(buffer); rsa.SetD(num);
	fill(buffer.begin(), buffer.end(), 0);
	nfile.close(); dfile.close();
	cout << "Load RSA private key: Success" << endl;
	HANDLE hFind;
	wstring pattern(ASYM_CIPHERTEXT_FOLDER); pattern.append(L"\\*");
	WIN32_FIND_DATA data;
	cout << "*************************************************" << endl;
	if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
		srand(time(NULL));
		wstring plaintextpath(ASYM_PLAINTEXT_FOLDER); plaintextpath.append(L"\\");
		wstring ciphertextpath(ASYM_CIPHERTEXT_FOLDER); ciphertextpath.append(L"\\");
		wstring filename;
		ofstream plaintextfile;
		ifstream ciphertextfile;
		vector<unsigned char> cipher;
		wstringstream wss;
		set<wstring> listfile;
		do {
			filename = data.cFileName;
			if (filename == L"." || filename == L"..") continue;
			for (int i = filename.size(); i > 0; --i) {
				if (filename[i - 1] < L'0' || filename[i - 1] > L'9') {
					listfile.insert(filename.substr(0, i));
					break;
				}
			}
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
		for (set<wstring>::iterator it = listfile.begin(); it != listfile.end(); ++it){
			filename = *it;
			wcout << L"Decrypting ciphertext file: " << filename << endl;
			plaintextfile.open(plaintextpath + filename, ios::binary);
			unsigned char ucc = 0, type = 0; int id = -2, oldid = -2;
			for (int i = 0; ; ++i) {
				wss << ciphertextpath << filename << i;
				ciphertextfile.open(wss.str(), ios::binary);
				if (ciphertextfile.is_open() == false) {
					cout << "!!! Ciphertext file: Missing file" << endl;
					break;
				}
				wcout << L"Ciphertext file: " << wss.str() << endl;
				wss.str(L"");
				ciphertextfile.seekg(0, ios::end);
				cipher.resize(ciphertextfile.tellg());
				ciphertextfile.seekg(0, ios::beg);
				copy((istreambuf_iterator<char>(ciphertextfile)), (istreambuf_iterator<char>()), cipher.begin());
				ciphertextfile.close();
				ERR err = rsa.Decrypt(ucc, cipher, type, id);
				if (err == ERR_CIPHERTEXT_TO_BIGNUM) {
					cout << "!!! Cannot divide ciphertext file into blocks" << endl;
					break;
				}
				else if (err == ERR_NOT_ENOUGH_SIZE_N) {
					cout << "!!! N too small" << endl;
					break;
				}
				else if(err == ERR_TEXT_STRUCTURE) {
					cout << "!!! Ciphertext file: Edited by someone" << endl;
					break;
				}
				else if (err == ERR_NO_PRIVATEKEY) {
					cout << "!!! No privatekey" << endl;
					break;
				}
				else if (err != NO_ERROR) {
					cout << "!!! ERROR" << endl;
					break;
				}
				plaintextfile.write((char*)&ucc, 1);
				bool check = true;
				if (check && ((type >> 2) != 0)) {
					cout << "!!! Ciphertext file: Unidentified type" << endl;
					check = false;
				}
				if (i == 0) {
					if (check && ((type & 1) == 0)) {
						cout << "!!! Ciphertext file: Missing head file" << endl;
						check = false;
					}
					else oldid = id;
				}
				else {
					if (check && ((type & 1) != 0)) {
						cout << "!!! Ciphertext file: Duplicate head file" << endl;
						check = false;
					}
					if (check && (id != oldid + 1)) {
						cout << "!!! Ciphertext file: Edited by someone or Missing file or Wrong order" << endl;
						check = false;
					}
					else oldid = id;
				}
				if ((check == false) || ((type & 2) != 0)) break;
			}
			plaintextfile.close();
			wcout << L"Decrypted ciphertext file: " << filename << endl;
			cout << "*************************************************" << endl;
		}
	}
	else cout << "!!! No ciphertext file to decrypt" << endl;
}

void AESEncryption() {
	wstring keyfilename; keyfilename += SECRETKEY_FOLDER; keyfilename += L"\\"; keyfilename += KEY_FILE;
	ifstream testfile; testfile.open(keyfilename);
	if (testfile.is_open() == false) {
		wcout << L"!!! Cannot open file: " << keyfilename << endl;
		wcout << L"!!! Please add file: " << keyfilename << endl;
		return;
	}
	testfile.close();
	cout << "AES key: Found" << endl;
	HANDLE hFind;
	wstring pattern(SYM_PLAINTEXT_FOLDER); pattern.append(L"\\*");
	WIN32_FIND_DATA data;
	cout << "*************************************************" << endl;
	if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
		wstring plaintextpath(SYM_PLAINTEXT_FOLDER); plaintextpath.append(L"\\");
		wstring ciphertextpath(SYM_CIPHERTEXT_FOLDER); ciphertextpath.append(L"\\");
		wstring filename;
		do {
			filename = data.cFileName;
			if (filename == L"." || filename == L"..") continue;
			wcout << L"Encrypting plaintext file: " << filename << endl;
			wchar_t *scr = wcsdup((plaintextpath + filename).c_str());
			wchar_t *keyfile = wcsdup(keyfilename.c_str());
			wchar_t *des = wcsdup((ciphertextpath + filename).c_str());
			DWORD ret = AES_Encrypt(scr, keyfile, des);
			free(scr); free(keyfile); free(des);
			if (ret == (unsigned long long(-1))) wcout << L"!!! Ciphertext file: Failed" << endl;
			else wcout << L"Ciphertext file: " << (ciphertextpath + filename).c_str() << endl;
			wcout << L"Encrypted plaintext file: " << filename << endl;
			cout << "*************************************************" << endl;
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
	else cout << "!!! No plaintext file to encrypt" << endl;
}

void AESDecryption() {
	wstring keyfilename; keyfilename += SECRETKEY_FOLDER; keyfilename += L"\\"; keyfilename += KEY_FILE;
	ifstream testfile; testfile.open(keyfilename);
	if (testfile.is_open() == false) {
		wcout << L"!!! Cannot open file: " << keyfilename << endl;
		wcout << L"!!! Please add file: " << keyfilename << endl;
		return;
	}
	testfile.close();
	cout << "AES key: Found" << endl;
	HANDLE hFind;
	wstring pattern(SYM_CIPHERTEXT_FOLDER); pattern.append(L"\\*");
	WIN32_FIND_DATA data;
	cout << "*************************************************" << endl;
	if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
		wstring plaintextpath(SYM_PLAINTEXT_FOLDER); plaintextpath.append(L"\\");
		wstring ciphertextpath(SYM_CIPHERTEXT_FOLDER); ciphertextpath.append(L"\\");
		wstring filename;
		do {
			filename = data.cFileName;
			if (filename == L"." || filename == L"..") continue;
			wcout << L"Decrypting ciphertext file: " << filename << endl;
			wchar_t *scr = wcsdup((ciphertextpath + filename).c_str());
			wchar_t *keyfile = wcsdup(keyfilename.c_str());
			wchar_t *des = wcsdup((plaintextpath + filename).c_str());
			DWORD ret = AES_Decrypt(scr, keyfile, des);
			free(scr); free(keyfile); free(des);
			if (ret == (unsigned long long(-1))) wcout << L"!!! Plaintext file: Failed" << endl;
			else wcout << L"Plaintext file: " << (plaintextpath + filename).c_str() << endl;
			wcout << L"Decrypted plaintext file: " << filename << endl;
			cout << "*************************************************" << endl;
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
	else cout << "!!! No plaintext file to encrypt" << endl;
}

int main() {
	CreateDirectory(PRIVATEKEY_FOLDER, NULL);
	CreateDirectory(PUBLICKEY_FOLDER, NULL);
	CreateDirectory(SECRETKEY_FOLDER, NULL);
	CreateDirectory(ASYM_CIPHERTEXT_FOLDER, NULL);
	CreateDirectory(ASYM_PLAINTEXT_FOLDER, NULL);
	CreateDirectory(SYM_CIPHERTEXT_FOLDER, NULL);
	CreateDirectory(SYM_PLAINTEXT_FOLDER, NULL);
	while (1) {
		system("cls");
		cout << "******************** GROUP Q ********************" << endl;
		cout << "* 0. Create and save RSA key                    *" << endl;
		cout << "* 1. RSA Encryption                             *" << endl;
		cout << "* 2. RSA Decryption                             *" << endl;
		cout << "* 3. AES Encryption                             *" << endl;
		cout << "* 4. AES Decryption                             *" << endl;
		cout << "* 5. Exit                                       *" << endl;
		cout << "*************************************************" << endl;
		cout << "Choose? ";
		int choose = 0;
		cin >> choose;
		system("cls");
		choose = abs(choose) % 6;
		switch (choose) {
		case 0:
			CreateAndSaveRSAKey();
			break;
		case 1:
			RSAEncryption();
			break;
		case 2:
			RSADecryption();
			break;
		case 3:
			AESEncryption();
			break;
		case 4:
			AESDecryption();
			break;
		case 5:
			return 0;
		}
		system("pause");
	}
	return 0;
}