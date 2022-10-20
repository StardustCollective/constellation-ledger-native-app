/*
 * MIT License, see root folder for full license.
 */
#ifndef UI_H
#define UI_H

#include "os.h"
#include "cx.h"
#include "ux.h"
#include "hex.h"
#include "base-encoding.h"
#include <stdbool.h>
#include <string.h>
#include "os_io_seproxyhal.h"
#include "bagl.h"     
#include "shared.h"  

/** the timer */
extern int exit_timer;

/** max with of timer display */
#define MAX_TIMER_TEXT_WIDTH 4

/** display for the timer */
extern char timer_desc[MAX_TIMER_TEXT_WIDTH];

/** length of the APDU (application protocol data unit) header. */
#define APDU_HEADER_LENGTH 5

/** offset in the APDU header which says the length of the body. */
#define APDU_BODY_LENGTH_OFFSET 4

/** String Length */
#define MESSAGE_SIZE_LEN_STRING (MESSAGE_SIZE_LEN*2)

/** for signing, indicates this is the last part of the transaction. */
#define P1_LAST 0x80

/** for signing, indicates this is not the last part of the transaction, there are more parts coming. */
#define P1_MORE 0x00

/** length of BIP44 path */
#define BIP44_PATH_LEN 5

/** length of BIP44 path, in bytes */
#define  BIP44_BYTE_LENGTH (BIP44_PATH_LEN * sizeof(unsigned int))

/** size of the hash data. */
#define HASH_DATA_SIZE 256

/** max number of hex bytes that can be displayed (2 hex characters for 1 byte of data) */
#define MAX_HEX_BUFFER_LEN (MAX_TX_TEXT_WIDTH / 2)

/** UI currently displayed */
enum UI_STATE {
	UI_INIT, 
	UI_IDLE, 
	UI_IDLE_SETTINGS,
	UI_TOP_SIGN, 
	UI_TX_DESC_1,
	UI_TX_DESC_2, 
	UI_SIGN, UI_DENY, 
	UI_PUBLIC_KEY_1, 
	UI_PUBLIC_KEY_2, 
	UI_TOP_BLIND_SIGNING, 
	UI_BLIND_SIGNING_WARNING, 
	UI_BLIND_SIGNING_REJECT, 
	UI_BLIND_SIGNING_ACCEPT,
	UI_BLIND_SIGNING_ENABLE_WARNING,
	UI_BLIND_SIGNING_SETTINGS,
	UI_BLIND_SIGNING_SETTING_GO_BACK
};

/** UI state enum */
extern enum UI_STATE uiState;

/** notification to restart the hash */
extern unsigned char hashTainted;

/** notification to refresh the view, if we are displaying the public key */
extern unsigned char publicKeyNeedsRefresh;

/** index of the current screen. */
extern unsigned int curr_scr_ix;

/** hash. */
extern unsigned char hash_data[HASH_DATA_SIZE];

/** current length of hash. */
extern unsigned int hash_data_ix;

/** Is blind signing enabled */
extern bool blind_signing_enabled_bool;

/** currently displayed text description. */
extern char curr_tx_desc[MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

/** currently displayed public key */
extern char current_public_key[MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

/** process a partial transaction */
const bagl_element_t * io_seproxyhal_touch_approve(const bagl_element_t *e);

/** sign a message */
const bagl_element_t * io_seproxyhal_touch_approve2(const bagl_element_t *e);

/** show the idle UI */
void ui_idle(void);

/** show the "Sign TX" ui, starting at the top of the Tx display */
void ui_top_sign(void);

/** show the "Blind Signing" ui, starting at the top of the blind signing display */
void ui_top_blind_signing(void);

/** show the "Blind signing must be enabled" flow */
void ui_blind_singing_must_enable_message(void);

/** return the length of the communication buffer */
unsigned int get_apdu_buffer_length();

int getIntLength (int n);

void intToBytes(char *buf, int n);

#endif // UI_H
