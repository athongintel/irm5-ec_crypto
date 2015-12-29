#include "AES.h"

AES::AES(const unsigned char* key)
{
	AES_set_encrypt_key(key, 256, &this->aes_enc_key);
	AES_set_decrypt_key(key, 256, &this->aes_dec_key);
}


bool AES::cbc_encrypt(char* message, char* encrypted){
	char iv[64];
	char ivcopy[64];
	char* base64encrypted;
	unsigned char* encryptedMessage = new unsigned char[1024];
	
	utils::getRandomString(iv, 32);
	//save a copy of iv, which will be modified after the encryption
	strcpy(ivcopy, iv);	
	
	//pre-compute the encryption length
	int encryptionLength;
	encryptionLength = ceil(strlen(message)/16.0)*16;
	//printf("\nen-length %d", encryptionLength);	
	AES_cbc_encrypt((unsigned char*) message, encryptedMessage, encryptionLength, &this->aes_enc_key, (unsigned char*) iv, AES_ENCRYPT);
	
	//encrypted;iv
	utils::Base64Encode(encryptedMessage, encryptionLength, &base64encrypted);
	//printf("\nEncrypted message: %s", base64encrypted);
	char finalMessage[1024];
	strcpy(finalMessage, base64encrypted);
	// convert concat iv
	utils::Base64Encode((unsigned char*)ivcopy, strlen(ivcopy), &base64encrypted);		
	strcat(finalMessage, ";");
	strcat(finalMessage, base64encrypted);
	//convert last time
	utils::Base64Encode((unsigned char*) finalMessage, strlen((char*) finalMessage), &base64encrypted);
	strcpy(encrypted, base64encrypted);	
	
	free(encryptedMessage);	
	return true;
}

bool AES::cbc_decrypt(char* message, char* decrypted){	
	char iv[64];
	unsigned char* decryptedMessage = new unsigned char[1024];
	unsigned char* base64decoded;
	unsigned char* base64decoded2;
	char* split[2];
	
	//just do inverse
	size_t size;
	if (!utils::Base64Decode(message, &size, &base64decoded)){
		printf("\ndecode message fail");		
		return false;
	}	
	utils::split((char*) base64decoded, ';', split);
	if (!utils::Base64Decode(split[0], &size, &base64decoded)){
		printf("\ndecode split[0] fail");
		return false;
	}
	if (!utils::Base64Decode(split[1], &size, &base64decoded2)){
		printf("\ndecode split[1] fail");
		return false;
	}
	
	bzero(decryptedMessage, 1024);
	AES_cbc_encrypt((unsigned char*) base64decoded, decryptedMessage, 1024, &this->aes_dec_key, base64decoded2, AES_DECRYPT);		
	strcpy(decrypted, (char*) decryptedMessage);	
	
	free(decryptedMessage);
	free(split[0]);
	free(split[1]);
	return true;
}

AES::~AES()
{
}

