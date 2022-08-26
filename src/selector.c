

#include "shared.h"
#include "string.h"
#include <stdio.h>

/** the position of the decimal point, 8 characters in from the right side */
#define DECIMAL_PLACE_OFFSET 8

#define MAX_BUFFER_LENGTH 128

unsigned char buffer[MAX_BUFFER_LENGTH];

static const char FROM_ADDRESS[] = "From Address\0";

static const char TO_ADDRESS[] = "To Address\0";

static const char TXT_FEE[] = "FEE\0";

static const char ELLIPSES[] = "...\0";

static const char TXT_ASSET_DAG[] = "$DAG\0";

/** returns the next byte in raw_tx and increments raw_tx_ix. If this would increment raw_tx_ix over the end of the buffer, throw an error. */
static unsigned char next_raw_tx() {
	if (raw_tx_ix < raw_tx_len) {
		unsigned char retval = raw_tx[raw_tx_ix];
		raw_tx_ix += 1;
		return retval;
	}

	return 0;
}

/** fills the array in arr with the given number of bytes from raw_tx. */
static void next_raw_tx_arr(unsigned char * arr, unsigned int length) {
	for (unsigned int ix = 0; ix < length; ix++) {
		*(arr + ix) = next_raw_tx();
	}
}

/** parse the raw transaction in raw_tx and fill up the screens in tx_desc. */
/** only parse out the send-to address and amount from the txos, skip the rest.  */
void select_display_fields()
{
	unsigned int scr_ix = 0;

	// Read the To and From address
	unsigned char num_parents = next_raw_tx();

	for (int parent_ix = 0; parent_ix < num_parents; parent_ix++)
	{
		memset(buffer, 0x00, sizeof(buffer));
		int buffer_len = next_raw_tx();

		memset(buffer, 0x00, sizeof(buffer));
		next_raw_tx_arr(buffer, buffer_len);

		unsigned char *buffer_0 = buffer;
		unsigned char *buffer_1 = buffer_0 + MAX_TX_TEXT_WIDTH;
		unsigned char *buffer_2 = buffer_1 + MAX_TX_TEXT_WIDTH;

		memset(tx_desc[scr_ix], '\0', CURR_TX_DESC_LEN);

		auto const char *header = (parent_ix == 0) ? FROM_ADDRESS : TO_ADDRESS;
		char shortAddress[20] = "";

		char subBuffStart[5];
		char subBuffEnd[5];

		memcpy(subBuffStart, &buffer_0[0], sizeof(subBuffStart));
		memcpy(subBuffEnd, &buffer_2[strlen((const char *)buffer_2) - 5], sizeof(subBuffEnd));

		strncat(shortAddress, &subBuffStart[0], sizeof(subBuffStart));
		strncat(shortAddress, &ELLIPSES[0], sizeof(ELLIPSES));
		strncat(shortAddress, &subBuffEnd[0], sizeof(subBuffEnd));

		memmove(tx_desc[scr_ix][0], header, MAX_TX_TEXT_WIDTH - 1);
		memmove(tx_desc[scr_ix][1], shortAddress, MAX_TX_TEXT_WIDTH - 1);
		memmove(tx_desc[scr_ix][2], TXT_BLANK, sizeof(TXT_BLANK));

		scr_ix++;
	}

	// Read the Amount
	int buffer_len = next_raw_tx();
	char amount_length_char = buffer_len + '0';
	memset(buffer, 0x00, sizeof(buffer));
	next_raw_tx_arr(buffer, buffer_len);

	memset(tx_desc[scr_ix], '\0', CURR_TX_DESC_LEN);

	memmove(tx_desc[scr_ix][0], TXT_ASSET_DAG, sizeof(TXT_ASSET_DAG));
	memmove(tx_desc[scr_ix][1], buffer, buffer_len);
	memset(tx_desc[scr_ix][2], amount_length_char, sizeof(amount_length_char));

	scr_ix++;

	// Skip last the tx ref
	buffer_len = next_raw_tx();
	memset(buffer, 0x00, sizeof(buffer));
	next_raw_tx_arr(buffer, buffer_len);

	// Skip the last tx ordinal
	buffer_len = next_raw_tx();
	memset(buffer, 0x00, sizeof(buffer));
	next_raw_tx_arr(buffer, buffer_len);

	// Read the fee
	buffer_len = next_raw_tx();
	char fee_length_char = buffer_len + '0';
	memset(buffer, 0x00, sizeof(buffer));
	next_raw_tx_arr(buffer, buffer_len);

	memset(tx_desc[scr_ix], '\0', CURR_TX_DESC_LEN);
	memmove(tx_desc[scr_ix][0], TXT_FEE, sizeof(TXT_FEE));
	memmove(tx_desc[scr_ix][1], buffer, buffer_len);
	memset(tx_desc[scr_ix][2], fee_length_char, sizeof(fee_length_char));
	scr_ix++;

	// Skip the Salt
	buffer_len = next_raw_tx();
	memset(buffer, 0x00, sizeof(buffer));
	next_raw_tx_arr(buffer, buffer_len);

	max_scr_ix = scr_ix;

	while (scr_ix < MAX_TX_TEXT_SCREENS)
	{
		memmove(tx_desc[scr_ix][0], TXT_BLANK, sizeof(TXT_BLANK));
		memmove(tx_desc[scr_ix][1], TXT_BLANK, sizeof(TXT_BLANK));
		memmove(tx_desc[scr_ix][2], TXT_BLANK, sizeof(TXT_BLANK));

		scr_ix++;
	}
}
