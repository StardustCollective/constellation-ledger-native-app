/*
 * MIT License, see root folder for full license.
 */
#include "constellation.h"
#include "base-encoding.h"
#include "shared.h"

/** length of the checksum used to convert a tx.output.script_hash into an Address. */
#define SCRIPT_HASH_CHECKSUM_LEN 4

/** Label when a public key has not been set yet */
static const char NO_PUBLIC_KEY_0[] = "No Public Key\0";
static const char NO_PUBLIC_KEY_1[] = "Requested Yet\0";

static const char ADDRESS_PREFIX[] = "DAG\0";

static const unsigned char PUBLIC_KEY_PREFIX[] = {
	0x30,0x56,0x30,0x10,0x06,0x07,0x2a,0x86,0x48,0xce,0x3d,0x02,0x01,0x06,0x05,0x2b,0x81,0x04,0x00,0x0a,0x03,0x42,0x00
};

void display_no_public_key() {
	memmove(current_public_key[0], TXT_BLANK, sizeof(TXT_BLANK));
	memmove(current_public_key[1], TXT_BLANK, sizeof(TXT_BLANK));
	memmove(current_public_key[2], TXT_BLANK, sizeof(TXT_BLANK));
	memmove(current_public_key[0], NO_PUBLIC_KEY_0, sizeof(NO_PUBLIC_KEY_0));
	memmove(current_public_key[1], NO_PUBLIC_KEY_1, sizeof(NO_PUBLIC_KEY_1));
	publicKeyNeedsRefresh = 0;
}

void display_public_key(const unsigned char * public_key) {
	memmove(current_public_key[0], TXT_BLANK, sizeof(TXT_BLANK));
	memmove(current_public_key[1], TXT_BLANK, sizeof(TXT_BLANK));
	memmove(current_public_key[2], TXT_BLANK, sizeof(TXT_BLANK));

	unsigned char public_key_encoded[PUBLIC_KEY_ENCODED_LEN];
	memmove(public_key_encoded, PUBLIC_KEY_PREFIX, PUBLIC_KEY_PREFIX_LEN);
	memmove(public_key_encoded + PUBLIC_KEY_PREFIX_LEN, public_key, PUBLIC_KEY_LEN);

	unsigned char address_hash_result[CX_SHA256_SIZE];
	cx_hash_sha256(public_key_encoded, PUBLIC_KEY_ENCODED_LEN, address_hash_result, CX_SHA256_SIZE);

	char base58_encoded[BASE58_ENCODED_ADDRESS_LEN];
	encode_base_58(address_hash_result, CX_SHA256_SIZE, base58_encoded, BASE58_ENCODED_ADDRESS_LEN, false);

	char end[BASE58_ENCODED_ADDRESS_SUFFIX_LEN];
	memmove(end, base58_encoded + BASE58_ENCODED_ADDRESS_LEN - BASE58_ENCODED_ADDRESS_SUFFIX_LEN, BASE58_ENCODED_ADDRESS_SUFFIX_LEN);

	int sum = 0;
	for(int i = 0; i < BASE58_ENCODED_ADDRESS_SUFFIX_LEN; i++) {
		char letter = end[i];
		if(letter >= '0') {
			if(letter <= '9') {
				sum += (letter - '0');
			}
		}
	}
	char par[1];
	par[0] = '0' + (sum % 9);

	char address[ADDRESS_LEN];
	memmove(address, ADDRESS_PREFIX, 3);
	memmove(address + 3, par, 1);
	memmove(address + 4, end, BASE58_ENCODED_ADDRESS_SUFFIX_LEN);
	unsigned int address_len_0 = 13;
	unsigned int address_len_1 = 13;
	unsigned int address_len_2 = 14;
	char * address_0 = address;
	char * address_1 = address + address_len_0;
	char * address_2 = address + address_len_0 + address_len_1;

	memmove(current_public_key[0], address_0, address_len_0);
	memmove(current_public_key[1], address_1, address_len_1);
	memmove(current_public_key[2], address_2, address_len_2);
	publicKeyNeedsRefresh = 0;
}

void add_hex_data_to_hash(unsigned char * in, const unsigned int len) {
	char dest[2];
	for(unsigned int copy_ix = 0; copy_ix < len; copy_ix++) {
		memset(dest, 0x00, sizeof(dest));
		to_hex(dest, in + copy_ix, sizeof(dest));
		if(hash_data_ix + 2 > HASH_DATA_SIZE) {
			THROW(0x6D33);
		}
		hash_data[hash_data_ix + 0] = dest[0];
		hash_data[hash_data_ix + 1] = dest[1];
		hash_data_ix += 2;
	}
}

void add_data_to_hash(unsigned char * in, const unsigned int len) {
	for(unsigned int copy_ix = 0; copy_ix < len; copy_ix++) {
		if(hash_data_ix + copy_ix > HASH_DATA_SIZE) {
			THROW(0x6D34);
		}
		hash_data[hash_data_ix + copy_ix] = *(in + copy_ix);
	}
	hash_data_ix += len;
}

void add_number_to_hash(const unsigned char number) {
	if(number > 99) {
		THROW(0x6D32);
	}
	if(number < 10) {
		if(hash_data_ix + 1 > HASH_DATA_SIZE) {
			THROW(0x6D35);
		}
		hash_data[hash_data_ix++] = '0' + (number % 10);
	} else {
		if(hash_data_ix + 2 > HASH_DATA_SIZE) {
			THROW(0x6D36);
		}
		hash_data[hash_data_ix++] = '0' + (number/10);
		hash_data[hash_data_ix++] = '0' + (number % 10);
	}
}

void add_base10_and_len_to_hash(unsigned char * in, const unsigned int len) {
	char base10[MAX_TX_TEXT_WIDTH];
	unsigned int base10_len = encode_base_10(in, len, base10, MAX_TX_TEXT_WIDTH-1, false);
	unsigned int base10_start = 0;
	while((base10[base10_start] == '0') && (base10_start < base10_len-1)) {
		base10_start++;
	}
	unsigned int base10_true_len = base10_len-base10_start;
	add_number_to_hash(base10_true_len);
	add_data_to_hash((unsigned char *)base10 + base10_start, base10_true_len);
}

void add_base16_and_len_to_hash(unsigned char * in, const unsigned int len) {
	char base16[MAX_TX_TEXT_WIDTH*2];
	unsigned int base16_len = encode_base_16(in, len, base16, sizeof(base16)-1, false);
	unsigned int base16_start = 0;
	while((base16[base16_start] == '0') && (base16_start < base16_len-1)) {
		base16_start++;
	}
	unsigned int base16_true_len = base16_len-base16_start;
	add_number_to_hash(base16_true_len);
	add_data_to_hash((unsigned char *)base16 + base16_start, base16_true_len);
}

void calc_hash(void) {

	// decoding hash
	memset(hash_data, 0x00, sizeof(hash_data));
	hash_data_ix = 0;

	unsigned int ix = 0;

	// *** decoding parents ***
	unsigned int maxParents = raw_tx[ix++];
	// add num parents to hash
	add_number_to_hash(maxParents);

	for(unsigned int parentIx = 0; parentIx < maxParents; parentIx++) {
		unsigned int parentLen = raw_tx[ix++];
		// add parent length to hash
		add_number_to_hash(parentLen);
		// add parent to hash
		add_data_to_hash(raw_tx + ix, parentLen);
		ix += parentLen;
	}

	// // *** decoding amount ***
	unsigned int amountLen = raw_tx[ix++];
	add_base16_and_len_to_hash(raw_tx + ix, amountLen);
	ix += amountLen;

	// *** decoding lastTxRefHash
	unsigned int lastTxRefHashLen = raw_tx[ix++];
	add_number_to_hash(lastTxRefHashLen);
	add_data_to_hash(raw_tx + ix, lastTxRefHashLen);
	ix += lastTxRefHashLen;

	// *** decoding lastTxRefOrdinal
	unsigned int lastTxRefOrdinalLen = raw_tx[ix++];
	add_base10_and_len_to_hash(raw_tx + ix, lastTxRefOrdinalLen);
	ix += lastTxRefOrdinalLen;

	// *** decoding fee
	unsigned int feeLen = raw_tx[ix++];
	add_base10_and_len_to_hash(raw_tx + ix, feeLen);
	ix += feeLen;

	// *** decoding salt
	unsigned int saltLen = raw_tx[ix++];
	add_base16_and_len_to_hash(raw_tx + ix, saltLen);
}
