/*
 * MIT License, see root folder for full license.
 */

#include "base-encoding.h"
#include "os.h"
#include "shared.h"

#define DECIMAL_PLACE_OFFSET 8

static const char TXT_LOW_VALUE[] = "Low Value\0";

static const char TXT_PERIOD[] = ".";

static void to_base10_100m(const unsigned char *value, const unsigned int value_len, char *dest)
{

	// encode in base10
	char base10_buffer[MAX_TX_TEXT_WIDTH];
	unsigned int buffer_len = encode_base_10(value, value_len, base10_buffer, MAX_TX_TEXT_WIDTH - 1, false);

	// place the decimal place.
	unsigned int dec_place_ix = buffer_len - DECIMAL_PLACE_OFFSET;
	if (buffer_len < DECIMAL_PLACE_OFFSET)
	{
		memmove(dest, TXT_LOW_VALUE, sizeof(TXT_LOW_VALUE));
	}
	else
	{
		memcpy(dest + dec_place_ix, TXT_PERIOD, sizeof(TXT_PERIOD));
		memcpy(dest, base10_buffer, dec_place_ix);
		memmove(dest + dec_place_ix + 1, base10_buffer + dec_place_ix, buffer_len - dec_place_ix);
	}
}

static void remove_leading_zeros(unsigned int scr_ix, unsigned int line_ix)
{
	unsigned char found_nonzero = 0;
	unsigned int nonzero_ix = 0;
	for (unsigned int zero_ix = 0; (zero_ix < MAX_TX_TEXT_WIDTH - 1) && (found_nonzero == 0); zero_ix++)
	{
		nonzero_ix = zero_ix;
		if (tx_desc[scr_ix][line_ix][zero_ix] != '0')
		{
			found_nonzero = 1;
		}
	}
	if (nonzero_ix > 0)
	{
		for (unsigned int ix = 0; ix < MAX_TX_TEXT_WIDTH; ix++)
		{
			if (ix <= MAX_TX_TEXT_WIDTH - nonzero_ix)
			{
				tx_desc[scr_ix][line_ix][ix] = tx_desc[scr_ix][line_ix][ix + nonzero_ix];
			}
			else
			{
				tx_desc[scr_ix][line_ix][ix] = '\0';
			}
		}
	}
}

void format_display_values(void)
{

	// Format the Amount
	unsigned char amountTemp[sizeof(tx_desc[2][1])];
	unsigned int amountLength = tx_desc[2][2][0] - '0';

	memmove(amountTemp, tx_desc[2][1], sizeof(tx_desc[2][1]));
	memset(tx_desc[2][1], 0x00, sizeof(tx_desc[2][1]));
	memset(tx_desc[2][2], 0x00, sizeof(tx_desc[2][2]));

	memmove(tx_desc[2][2], TXT_BLANK, sizeof(TXT_BLANK));

	to_base10_100m(amountTemp, amountLength, tx_desc[2][1]);
	remove_leading_zeros(2, 1);

	// Format the Fee
	unsigned char feeTemp[sizeof(tx_desc[3][1])];
	unsigned int feeLength = tx_desc[3][2][0] - '0';

	memmove(feeTemp, tx_desc[3][1], sizeof(tx_desc[3][1]));

	memset(tx_desc[3][1], 0x00, sizeof(tx_desc[3][1]));
	memset(tx_desc[3][2], 0x00, sizeof(tx_desc[3][2]));

	memmove(tx_desc[3][2], TXT_BLANK, sizeof(TXT_BLANK));

	encode_base_10(feeTemp, feeLength, tx_desc[3][1], MAX_TX_TEXT_WIDTH - 1, false);
	remove_leading_zeros(3, 1);
}