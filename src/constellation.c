/*
 * MIT License, see root folder for full license.
 */
#include "constellation.h"
#include "base-encoding.h"

/** the position of the decimal point, 8 characters in from the right side */
#define DECIMAL_PLACE_OFFSET 8

/** length of the checksum used to convert a tx.output.script_hash into an Address. */
#define SCRIPT_HASH_CHECKSUM_LEN 4

#define MAX_BUFFER_LENGTH 128

unsigned char buffer[MAX_BUFFER_LENGTH];

/** MAX_TX_TEXT_WIDTH in blanks, used for clearing a line of text */
static const char TXT_BLANK[] = "\0";

static const char TXT_NUM_PARENTS[] = "NUM PARENTS\0";

// static const char TXT_PARENT[] = "PARENT\0";

static const char FROM_ADDRESS[] = "From Address\0";

static const char TO_ADDRESS[] = "To Address\0";

static const char ELLIPSES[] = "...\0";

static const char TXT_LAST_TX_REF_1[] = "LAST TX REF (1/2)\0";

static const char TXT_LAST_TX_REF_2[] = "LAST TX REF (2/2)\0";

static const char TXT_LAST_TX_ORDINAL[] = "LAST TX ORDINAL\0";

static const char TXT_FEE[] = "FEE\0";

static const char TXT_SALT[] = "SALT\0";

static const char TXT_ASSET_DAG[] = "$DAG\0";

/** text to display if an asset's base-10 encoded value is too low to display */
static const char TXT_LOW_VALUE[] = "Low Value\0";

/** a period, for displaying the decimal point. */
static const char TXT_PERIOD[] = ".";

/** Label when a public key has not been set yet */
static const char NO_PUBLIC_KEY_0[] = "No Public Key\0";
static const char NO_PUBLIC_KEY_1[] = "Requested Yet\0";

static const char NO_TX_REF[] = "(No Tx Ref)\0";


static const char ADDRESS_PREFIX[] = "DAG\0";

static const unsigned char PUBLIC_KEY_PREFIX[] = {
	0x30,0x56,0x30,0x10,0x06,0x07,0x2a,0x86,0x48,0xce,0x3d,0x02,0x01,0x06,0x05,0x2b,0x81,0x04,0x00,0x0a,0x03,0x42,0x00
};


/** converts a value to base10 with a decimal point at DECIMAL_PLACE_OFFSET, which should be 100,000,000 or 100 million, thus the suffix 100m */
static void to_base10_100m(const unsigned char * value, const unsigned int value_len, char * dest, __attribute__((unused)) const unsigned int dest_len) {

	// encode in base10
	char base10_buffer[MAX_TX_TEXT_WIDTH];
	unsigned int buffer_len = encode_base_10(value, value_len, base10_buffer, MAX_TX_TEXT_WIDTH-1, false);

	// place the decimal place.
	unsigned int dec_place_ix = buffer_len - DECIMAL_PLACE_OFFSET;
	if (buffer_len < DECIMAL_PLACE_OFFSET) {
		memmove(dest, TXT_LOW_VALUE, sizeof(TXT_LOW_VALUE));
	} else {
		memcpy(dest + dec_place_ix, TXT_PERIOD, sizeof(TXT_PERIOD));
		memcpy(dest, base10_buffer, dec_place_ix);
		memmove(dest + dec_place_ix + 1, base10_buffer + dec_place_ix, buffer_len - dec_place_ix);
	}
}

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

/** returns the next byte in raw_tx and increments raw_tx_ix. If this would increment raw_tx_ix over the end of the buffer, throw an error. */
static unsigned char next_raw_tx() {
	if (raw_tx_ix < raw_tx_len) {
		unsigned char retval = raw_tx[raw_tx_ix];
		raw_tx_ix += 1;
		return retval;
	} else {
		hashTainted = 1;
		THROW(0x6D05);
		// return 0;
	}
}

/** fills the array in arr with the given number of bytes from raw_tx. */
static void next_raw_tx_arr(unsigned char * arr, unsigned int length) {
	for (unsigned int ix = 0; ix < length; ix++) {
		*(arr + ix) = next_raw_tx();
	}
}

/** returns the minimum of two ints. */
static unsigned int min(unsigned int i0, unsigned int i1) {
	if (i0 < i1) {
		return i0;
	} else {
		return i1;
	}
}

/** returns the maximum of two ints. */
static unsigned int max(unsigned int i0, unsigned int i1) {
	if (i0 < i1) {
		return i1;
	} else {
		return i0;
	}
}

static void remove_leading_zeros(unsigned int scr_ix, unsigned int line_ix) {
	unsigned char found_nonzero = 0;
	unsigned int nonzero_ix = 0;
	for(unsigned int zero_ix = 0; (zero_ix < MAX_TX_TEXT_WIDTH-1) && (found_nonzero == 0); zero_ix++) {
		nonzero_ix = zero_ix;
		if(tx_desc[scr_ix][line_ix][zero_ix] != '0') {
			found_nonzero = 1;
		}
	}
	if(nonzero_ix > 0) {
		for(unsigned int ix = 0; ix < MAX_TX_TEXT_WIDTH; ix++) {
			if(ix <=  MAX_TX_TEXT_WIDTH - nonzero_ix) {
				tx_desc[scr_ix][line_ix][ix] = tx_desc[scr_ix][line_ix][ix + nonzero_ix];
			} else {
				tx_desc[scr_ix][line_ix][ix] = '\0';
			}
		}
	}
}

/** parse the raw transaction in raw_tx and fill up the screens in tx_desc. */
/** only parse out the send-to address and amount from the txos, skip the rest.  */
void display_tx_desc() {
	unsigned int scr_ix = 0;

	// Read the To and From address
	unsigned char num_parents = next_raw_tx();

	for(int parent_ix = 0; parent_ix < num_parents; parent_ix++ ) {
		memset(buffer, 0x00, sizeof(buffer));
		int buffer_len = next_raw_tx();
		if(buffer_len >= MAX_BUFFER_LENGTH) {
			THROW(0x6D04);
		}

		memset(buffer, 0x00, sizeof(buffer));
		next_raw_tx_arr(buffer,buffer_len);

		unsigned char * buffer_0 = buffer;
		unsigned char * buffer_1 = buffer_0 + MAX_TX_TEXT_WIDTH;
		unsigned char * buffer_2 = buffer_1 + MAX_TX_TEXT_WIDTH;

		if (scr_ix < MAX_TX_TEXT_SCREENS) {
			memset(tx_desc[scr_ix], '\0', CURR_TX_DESC_LEN);

			auto const char * header = (parent_ix == 0) ? FROM_ADDRESS : TO_ADDRESS;
			char shortAddress[20] = "";
            char subBuffStart[5];
            char subBuffEnd[5];

            memcpy(subBuffStart, &buffer_0[0], sizeof(subBuffStart));
            memcpy(subBuffEnd, &buffer_2[strlen(buffer_2) - 5], sizeof(subBuffEnd));

            strncat(shortAddress, &subBuffStart[0], sizeof(subBuffStart));
            strncat(shortAddress, &ELLIPSES[0], sizeof(ELLIPSES));
            strncat(shortAddress, &subBuffEnd[0], sizeof(subBuffEnd));
            
            memmove(tx_desc[scr_ix][0], header, MAX_TX_TEXT_WIDTH-1);
            memmove(tx_desc[scr_ix][1], shortAddress, MAX_TX_TEXT_WIDTH-1);
            memmove(tx_desc[scr_ix][2], TXT_BLANK, sizeof(TXT_BLANK));

			scr_ix++;
		}
	}

	// Read the Amount
	int buffer_len = next_raw_tx();
	if(buffer_len >= MAX_BUFFER_LENGTH) {
		THROW(0x6D06);
	}
	memset(buffer, 0x00, sizeof(buffer));
	next_raw_tx_arr(buffer,buffer_len);

	if (scr_ix < MAX_TX_TEXT_SCREENS) {
		memset(tx_desc[scr_ix], '\0', CURR_TX_DESC_LEN);
		memmove(tx_desc[scr_ix][0], TXT_ASSET_DAG, sizeof(TXT_ASSET_DAG));

		to_base10_100m(buffer, buffer_len, tx_desc[scr_ix][1], MAX_TX_TEXT_WIDTH-1);
		remove_leading_zeros(scr_ix,1);

		memmove(tx_desc[scr_ix][2], TXT_BLANK, sizeof(TXT_BLANK));
		scr_ix++;
	}

	// Skip last the tx ref
	buffer_len = next_raw_tx();
	if(buffer_len >= MAX_BUFFER_LENGTH) {
		THROW(0x6D06);
	}
	memset(buffer, 0x00, sizeof(buffer));
	next_raw_tx_arr(buffer,buffer_len);

	// Skip the last tx ordinal
	buffer_len = next_raw_tx();
	if(buffer_len >= MAX_BUFFER_LENGTH) {
		THROW(0x6D06);
	}
	memset(buffer, 0x00, sizeof(buffer));
	next_raw_tx_arr(buffer,buffer_len);

	// Read the fee
	buffer_len = next_raw_tx();
	if(buffer_len >= MAX_BUFFER_LENGTH) {
		THROW(0x6D06);
	}
	memset(buffer, 0x00, sizeof(buffer));
	next_raw_tx_arr(buffer,buffer_len);
	if (scr_ix < MAX_TX_TEXT_SCREENS) {
		memset(tx_desc[scr_ix], '\0', CURR_TX_DESC_LEN);
		memmove(tx_desc[scr_ix][0], TXT_FEE, sizeof(TXT_FEE));
		encode_base_10(buffer, buffer_len, tx_desc[scr_ix][1], MAX_TX_TEXT_WIDTH-1, false);
		remove_leading_zeros(scr_ix,1);
		memmove(tx_desc[scr_ix][2], TXT_BLANK, sizeof(TXT_BLANK));
		scr_ix++;
	}

	// Skip the Salt
	buffer_len = next_raw_tx();
	if(buffer_len >= MAX_BUFFER_LENGTH) {
		THROW(0x6D06);
	}
	memset(buffer, 0x00, sizeof(buffer));
	next_raw_tx_arr(buffer,buffer_len);

	max_scr_ix = scr_ix;

	while(scr_ix < MAX_TX_TEXT_SCREENS) {
		memmove(tx_desc[scr_ix][0], TXT_BLANK, sizeof(TXT_BLANK));
		memmove(tx_desc[scr_ix][1], TXT_BLANK, sizeof(TXT_BLANK));
		memmove(tx_desc[scr_ix][2], TXT_BLANK, sizeof(TXT_BLANK));

		scr_ix++;
	}
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
	ix += saltLen;

}
