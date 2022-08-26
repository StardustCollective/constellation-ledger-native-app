/*
 * MIT License, see root folder for full license.
 */

#ifndef SHARED_H
#define SHARED_H

static const char TXT_BLANK[] = "\0";

/** max width of a single line of text. */
#define MAX_TX_TEXT_WIDTH 18

/** max number of screens to display. */
#define MAX_TX_TEXT_SCREENS 9

/** max lines of text to display. */
#define MAX_TX_TEXT_LINES 3

/**
 * Nano S has 320 KB flash, 10 KB RAM, uses a ST31H320 chip.
 * This effectively limits the max size
 * So we can only display 9 screens of data, and can only sign transactions up to 1kb in size.
 * max size of a transaction, binary will not compile if we try to allow transactions over 1kb.
 */
#define MAX_TX_RAW_LENGTH 256

/** raw transaction data. */
extern unsigned char raw_tx[MAX_TX_RAW_LENGTH];

/** current index into raw transaction. */
extern unsigned int raw_tx_ix;

/** current length of raw transaction. */
extern unsigned int raw_tx_len;

/** max number of bytes for one line of text. */
#define CURR_TX_DESC_LEN (MAX_TX_TEXT_LINES * MAX_TX_TEXT_WIDTH)

/** max number of bytes for all text screens. */
#define MAX_TX_DESC_LEN (MAX_TX_TEXT_SCREENS * CURR_TX_DESC_LEN)

extern unsigned int max_scr_ix;

/** all text descriptions. */
extern char tx_desc[MAX_TX_TEXT_SCREENS][MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];


#endif