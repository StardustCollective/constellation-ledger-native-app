/*******************************************************************************
 *   Ledger Blue
 *   (c) 2016 Ledger
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include "os_io_seproxyhal.h"
#include "ui.h"
#include "constellation.h"
#include "bagl.h"
#include "base-encoding.h"
#include "shared.h"
#include "selector.h"
#include "format.h"

/** message security prefix length */
#define MESSAGE_PREFIX_LENGTH 31

/** message security prefix */
static const uint8_t message_prefix[MESSAGE_PREFIX_LENGTH] = { 0x19, 0x43, 0x6f, 0x6e, 0x73, 0x74, 0x65, 0x6c, 0x6c, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x53, 0x69, 0x67, 0x6e, 0x65, 0x64, 0x20, 0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x3a, 0xa };

/** message prefix delimeter length */
#define MESSAGE_PREFIX_DELIMETER_LENGTH 1 

/** message prefix delimeter */
static const uint8_t message_prefix_delimeter[MESSAGE_PREFIX_DELIMETER_LENGTH] = { 0xa };

/** Byte length of message size param */
#define MESSAGE_SIZE_LEN 4

#define DEBUG_OUT_ENABLED false

#define MAX_EXIT_TIMER 4098

#define EXIT_TIMER_REFRESH_INTERVAL 512

static void Timer_UpdateDescription() {
	snprintf(timer_desc, MAX_TIMER_TEXT_WIDTH, "%d", exit_timer / EXIT_TIMER_REFRESH_INTERVAL);
}

static void Timer_UpdateDisplay() {
	if ((exit_timer % EXIT_TIMER_REFRESH_INTERVAL) == (EXIT_TIMER_REFRESH_INTERVAL / 2)) {
		UX_REDISPLAY();
	}
}

static void Timer_Tick() {
	if (exit_timer > 0) {
		exit_timer--;
		Timer_UpdateDescription();
	}
}

static void Timer_Set() {
	exit_timer = MAX_EXIT_TIMER;
	Timer_UpdateDescription();
}

static void Timer_Restart() {
	if (exit_timer != MAX_EXIT_TIMER) {
		Timer_Set();
	}
}

static bool Timer_Expired() {
	return exit_timer <= 0;
}

/** IO buffer to communicate with the outside world. */
unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

/** start of the buffer, reject any transmission that doesn't start with this, as it's invalid. */
#define CLA 0x80

/** #### instructions start #### **/
/** instruction to sign transaction and send back the signature. */
#define INS_SIGN 0x02

/** instruction to send back the public key. */
#define INS_GET_PUBLIC_KEY 0x04

/** instruction to blind sign a message and send back the signature. */
#define INS_BLIND_SIGN 0x06

// TODO replace with better b64 implementation
static const char basis_64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int Base64encode_len(int len)
{
    return ((len + 2) / 3 * 4) + 1;
}

int Base64encode(char *encoded, const char *string, int len)
{
    int i;
    char *p;
	PRINTF("len: %d\n",len);
    p = encoded;
    for (i = 0; i < len - 2; i += 3) {
    *p++ = basis_64[(string[i] >> 2) & 0x3F];
    *p++ = basis_64[((string[i] & 0x3) << 4) |
                    ((int) (string[i + 1] & 0xF0) >> 4)];
    *p++ = basis_64[((string[i + 1] & 0xF) << 2) |
                    ((int) (string[i + 2] & 0xC0) >> 6)];
    *p++ = basis_64[string[i + 2] & 0x3F];
    }
    if (i < len) {
    *p++ = basis_64[(string[i] >> 2) & 0x3F];
    if (i == (len - 1)) {
        *p++ = basis_64[((string[i] & 0x3) << 4)];
        *p++ = '=';
    }
    else {
        *p++ = basis_64[((string[i] & 0x3) << 4) |
                        ((int) (string[i + 1] & 0xF0) >> 4)];
        *p++ = basis_64[((string[i + 1] & 0xF) << 2)];
    }
    *p++ = '=';
    }

    *p++ = '\0';
    return p - encoded;
}

/** #### instructions end #### */

/** some kind of event loop */
unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
	switch (channel & ~(IO_FLAGS)) {
	case CHANNEL_KEYBOARD:
		break;

	// multiplexed io exchange over a SPI channel and TLV encapsulated protocol
	case CHANNEL_SPI:
		if (tx_len) {
			io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

			if (channel & IO_RESET_AFTER_REPLIED) {
				reset();
			}
			// nothing received from the master so far
			//(it's a tx transaction)
			return 0;
		} else {
			return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
		}

	default:
		hashTainted = 1;
		THROW(INVALID_PARAMETER);
	}
	return 0;
}

/** refreshes the display if the public key was changed ans we are on the page displaying the public key */
static void refresh_public_key_display(void) {
	if ((uiState == UI_PUBLIC_KEY_1)|| (uiState == UI_PUBLIC_KEY_2)) {
		publicKeyNeedsRefresh = 1;
	}
}

static int init_msg_sign_buf(void) {
	raw_tx_ix = 0;
	// raw_tx_len = 0;
	raw_tx_len = MESSAGE_PREFIX_LENGTH;

	unsigned char * out = raw_tx + raw_tx_ix;
	memcpy(out, message_prefix, MESSAGE_PREFIX_LENGTH);
	raw_tx_ix += raw_tx_len;

	unsigned char message_length_bytes[MESSAGE_SIZE_LEN];
	unsigned char * message_without_apdu = G_io_apdu_buffer + APDU_HEADER_LENGTH;					
	// Get the message length.
	memmove(message_length_bytes, message_without_apdu, MESSAGE_SIZE_LEN);
	int message_length = (message_length_bytes[0] << 24) + (message_length_bytes[1] << 16) + (message_length_bytes[2] << 8) + (message_length_bytes[3]);
	int message_digits_length = getIntLength(message_length);
	// Convert message length int to ascii values
	char message_length_ascii_bytes[message_digits_length];
	intToBytes(message_length_ascii_bytes, message_length);

	out = raw_tx + raw_tx_ix;
	memcpy(out, message_length_ascii_bytes, message_digits_length);
	raw_tx_ix += message_digits_length; 

	out = raw_tx + raw_tx_ix;
	memcpy(out, message_prefix_delimeter, MESSAGE_PREFIX_DELIMETER_LENGTH);
	raw_tx_ix += MESSAGE_PREFIX_DELIMETER_LENGTH;

	return message_length;
}

/** main loop. */
static void constellation_main(void) {
	volatile unsigned int rx = 0;
	volatile unsigned int tx = 0;
	volatile unsigned int flags = 0;
	int msg_len = 0;

	// DESIGN NOTE: the bootloader ignores the way APDU are fetched. The only
	// goal is to retrieve APDU.
	// When APDU are to be fetched from multiple IOs, like NFC+USB+BLE, make
	// sure the io_event is called with a
	// switch event, before the apdu is replied to the bootloader. This avoid
	// APDU injection faults.
	for (;; ) {
		volatile unsigned short sw = 0;

		BEGIN_TRY
		{
			TRY
			{
				rx = tx;
				// ensure no race in catch_other if io_exchange throws an error
				tx = 0;
				rx = io_exchange(CHANNEL_APDU | flags, rx);
				flags = 0;

				// no apdu received, well, reset the session, and reset the
				// bootloader configuration
				if (rx == 0) {
					hashTainted = 1;
					THROW(0x6982);
				}

				// if the buffer doesn't start with the magic byte, return an error.
				if (G_io_apdu_buffer[0] != CLA) {
					hashTainted = 1;
					THROW(0x6E00);
				}

				// check the second byte (0x01) for the instruction.
				switch (G_io_apdu_buffer[1]) {

				// we're getting a transaction to sign, in parts.
				case INS_SIGN: {
					Timer_Restart();
					// check the third byte (0x02) for the instruction subtype.
					if ((G_io_apdu_buffer[2] != P1_MORE) && (G_io_apdu_buffer[2] != P1_LAST)) {
						hashTainted = 1;
						THROW(0x6A86);
					}

					// if this is the first transaction part, reset the hash and all the other temporary variables.
					if (hashTainted) {
						hashTainted = 0;
						raw_tx_ix = 0;
						raw_tx_len = 0;
					}

					// move the contents of the buffer into raw_tx, and update raw_tx_ix to the end of the buffer, to be ready for the next part of the tx.
					unsigned int len = get_apdu_buffer_length();
					unsigned char * in = G_io_apdu_buffer + APDU_HEADER_LENGTH;
					unsigned char * out = raw_tx + raw_tx_ix;
					if (raw_tx_ix + len > MAX_TX_RAW_LENGTH) {
						hashTainted = 1;
						THROW(0x6D08);
					}
					memmove(out, in, len);
					raw_tx_ix += len;

					// if this is the last part of the transaction, parse the transaction into human readable text, and display it.
					if (G_io_apdu_buffer[2] == P1_LAST) {
						raw_tx_len = raw_tx_ix;
						raw_tx_ix = 0;
						hash_data_ix = 0;
						curr_scr_ix = 0;
						memset(tx_desc, 0x00, sizeof(tx_desc));

						// select the transaction fields.
						select_display_fields();

						// Format the selected fields.
						format_display_values();
						
						// parse the transaction into machine readable hash.
						calc_hash();

						// display the UI, starting at the top screen which is "Sign Tx Now".
						ui_top_sign();
					}

					flags |= IO_ASYNCH_REPLY;

					// if this is not the last part of the transaction, do not display the UI, and approve the partial transaction.
					// this adds the TX to the hash.
					if (G_io_apdu_buffer[2] == P1_MORE) {
						io_seproxyhal_touch_approve(NULL);
					}
				}
				break;

				// we're asked for the public key.
				case INS_GET_PUBLIC_KEY: {
					Timer_Restart();

					cx_ecfp_public_key_t publicKey;
					cx_ecfp_private_key_t privateKey;

					if (rx < APDU_HEADER_LENGTH + BIP44_BYTE_LENGTH) {
						hashTainted = 1;
						THROW(0x6D09);
					}

					/** BIP44 path, used to derive the private key from the mnemonic by calling os_perso_derive_node_bip32. */
					unsigned char * bip44_in = G_io_apdu_buffer + APDU_HEADER_LENGTH;
					unsigned int bip44_path[BIP44_PATH_LEN];
					
					uint32_t i;
					for (i = 0; i < BIP44_PATH_LEN; i++) {
						bip44_path[i] = (bip44_in[0] << 24) | (bip44_in[1] << 16) | (bip44_in[2] << 8) | (bip44_in[3]);
						bip44_in += 4;
					}
					unsigned char privateKeyData[32];

					os_perso_derive_node_bip32(CX_CURVE_256K1, bip44_path, BIP44_PATH_LEN, privateKeyData, NULL);
					cx_ecdsa_init_private_key(CX_CURVE_256K1, privateKeyData, 32, &privateKey);

					// generate the public key.
					cx_ecdsa_init_public_key(CX_CURVE_256K1, NULL, 0, &publicKey);
					cx_ecfp_generate_pair(CX_CURVE_256K1, &publicKey, &privateKey, 1);


					// clear private key data
					cx_ecdsa_init_private_key(CX_CURVE_256K1, NULL, 0, &privateKey);
					// memset(&privateKey, 0x00, sizeof(privateKey));
					memset(privateKeyData, 0x00, sizeof(privateKeyData));

					display_public_key(publicKey.W);
					refresh_public_key_display();

					// push the public key onto the response buffer.
					memmove(G_io_apdu_buffer, publicKey.W, PUBLIC_KEY_LEN);
					tx = PUBLIC_KEY_LEN;

					// return 0x9000 OK.
					THROW(0x9000);
				}
				break;

				case INS_BLIND_SIGN: { // RFC 6979
					Timer_Restart();
					
					if(!blind_signing_enabled_bool){
						ui_blind_singing_must_enable_message();
						break;
					}

					// check the third byte (0x02) for the instruction subtype.
					if ((G_io_apdu_buffer[2] != P1_MORE) && (G_io_apdu_buffer[2] != P1_LAST)) {
						PRINTF("hash tainted 0x6a86\n");
						hashTainted = 1;
						THROW(0x6A86);
					}

					// if this is the first transaction part, 
					// reset the hash and all the other temporary variables.
					// append message prefix to fresh buffer, along with message lengh and delimeters
					if (hashTainted) {
						PRINTF("hash tainted - first txn\n");
						hashTainted = 0;
						msg_len = init_msg_sign_buf();
						PRINTF("msg_len: %d\n", msg_len);
						PRINTF("raw_tx_ix: %d\n", raw_tx_ix);
					}

					// move the contents of the buffer into raw_tx, 
					// and update raw_tx_ix to the end of the buffer, to be ready for the next part of the tx.
					unsigned int len = get_apdu_buffer_length() - 4; 				// skip message len
					unsigned char * in = G_io_apdu_buffer + APDU_HEADER_LENGTH + 4; // skip message len
					unsigned char * out = raw_tx + raw_tx_ix;
					if (raw_tx_ix + len > MAX_TX_RAW_LENGTH) {
						PRINTF("hash tainted 0x6d08\n");
						hashTainted = 1;
						THROW(0x6D08);
					}

					for(int i = 0; i < MAX_TX_RAW_LENGTH; i += 8) {
						PRINTF("%02x %02x %02x %02x %02x %02x %02x %02x \n", 
							raw_tx[i], raw_tx[i+1], raw_tx[i+2], raw_tx[i+3],
							raw_tx[i+4], raw_tx[i+5], raw_tx[i+6], raw_tx[i+7]); 
					}

					// convert message to b64 (dag4.keyStore does this)
					size_t outputsize = Base64encode(out, in, msg_len);
					PRINTF("output size %d\n", outputsize);
					for(int i = 0; i < MAX_TX_RAW_LENGTH; i += 8) {
						PRINTF("%02x %02x %02x %02x %02x %02x %02x %02x \n", 
							raw_tx[i], raw_tx[i+1], raw_tx[i+2], raw_tx[i+3],
							raw_tx[i+4], raw_tx[i+5], raw_tx[i+6], raw_tx[i+7]); 
					}
					raw_tx_ix += msg_len;

					PRINTF("pkt len: %d\n", len);
					PRINTF("raw_tx_ix: %d\n", raw_tx_ix);
					for(int i = 0; i < MAX_TX_RAW_LENGTH; i += 8) {
						// PRINTF("%02x %02x %02x %02x %02x %02x %02x %02x \n", 
						PRINTF("%02x%02x%02x%02x%02x%02x%02x%02x", 
							raw_tx[i], raw_tx[i+1], raw_tx[i+2], raw_tx[i+3],
							raw_tx[i+4], raw_tx[i+5], raw_tx[i+6], raw_tx[i+7]);
					}
					PRINTF("\n");

					// if this is the last part of the transaction, parse the transaction into human readable text, and display it.
					if (G_io_apdu_buffer[2] == P1_LAST) {
						// raw_tx_len = raw_tx_ix;
						// raw_tx_ix = 0;
						// hash_data_ix = 0;
						// curr_scr_ix = 0;
						// memset(tx_desc, 0x00, sizeof(tx_desc));

						// select the transaction fields.
						// select_display_fields();

						// Format the selected fields.
						// format_display_values();
						
						// parse the transaction into machine readable hash.
						// calc_hash();

						// display the UI, starting at the top screen which is "Sign Tx Now".
						// ui_top_sign();
						ui_top_blind_signing();
					}

					flags |= IO_ASYNCH_REPLY;

					// if this is not the last part of the transaction, 
					// send 0x9000
					if (G_io_apdu_buffer[2] == P1_MORE) {
						io_seproxyhal_touch_approve2(NULL);
					}
						
				}
				break;
				case 0xFF:                                                                                                                                 // return to dashboard
					goto return_to_dashboard;

				// we're asked to do an unknown command
				default:
					// return an error.
					hashTainted = 1;
					THROW(0x6D00);
					break;
				}
			}
			CATCH_OTHER(e)
			{
				switch (e & 0xF000) {
				case 0x6000:
				case 0x9000:
					sw = e;
					break;
				default:
					sw = 0x6800 | (e & 0x7FF);
					break;
				}
				// Unexpected exception => report
				G_io_apdu_buffer[tx] = sw >> 8;
				G_io_apdu_buffer[tx + 1] = sw;
				tx += 2;
			}
			FINALLY
			{
			}
		}
		END_TRY;
	}

return_to_dashboard: return;
}

/** display function */
void io_seproxyhal_display(const bagl_element_t *element) {
	io_seproxyhal_display_default((bagl_element_t *) element);
}

/* io event loop */
unsigned char io_event(unsigned char channel) {
	// nothing done with the event, throw an error on the transport layer if
	// needed
	UNUSED(channel);

	// can't have more than one tag in the reply, not supported yet.
	switch (G_io_seproxyhal_spi_buffer[0]) {
	case SEPROXYHAL_TAG_FINGER_EVENT:
		Timer_Restart();
		UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
		break;

	case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:                             // for Nano S
		Timer_Restart();
		UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
		break;

	case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
		//Timer_Restart();
		if (UX_DISPLAYED()) {
			// perform actions after all screen elements have been displayed
		} else {
			UX_DISPLAYED_EVENT();
		}
		break;

	case SEPROXYHAL_TAG_TICKER_EVENT:

#if defined(TARGET_NANOX) || defined(TARGET_NANOS2)
		UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {
		                    // don't redisplay if UX not allowed (pin locked in the common bolos
		                    // ux ?)
		                    if (UX_ALLOWED) {
		                        // redisplay screen
		                        UX_REDISPLAY();
							}
						});
#endif

		Timer_Tick();
		if (publicKeyNeedsRefresh == 1) {
			UX_REDISPLAY();
			publicKeyNeedsRefresh = 0;
		} else {
			if (Timer_Expired()) {
				os_sched_exit(0);
			} else {
				Timer_UpdateDisplay();
			}
		}
		break;

	// unknown events are acknowledged
	default:
		UX_DEFAULT_EVENT();
		break;
	}

	// close the event if not done previously (by a display or whatever)
	if (!io_seproxyhal_spi_is_status_sent()) {
		io_seproxyhal_general_status();
	}

	// command has been processed, DO NOT reset the current APDU transport
	return 1;
}

static void app_exit(void)
{
	BEGIN_TRY_L(exit) {
		TRY_L(exit) {
			os_sched_exit(-1);
		}
		FINALLY_L(exit) {
		}
	}
	END_TRY_L(exit);
}

/** boot up the app and intialize it */
__attribute__((section(".boot"))) int main(void) {
	// exit critical section
	__asm volatile ("cpsie i");

	curr_scr_ix = 0;
	max_scr_ix = 0;
	raw_tx_ix = 0;
	hashTainted = 1;
	uiState = UI_IDLE;

	// First things first, we need to start the timer.
	// If for some reason the 'io_event' callback is called with a ticker event,
	// then the ram space will not be initialized.
	Timer_Set();

	for (;;) {
		UX_INIT();
		os_boot();

		BEGIN_TRY
		{
			TRY
			{
				io_seproxyhal_init();

				USB_power(0);
				USB_power(1);

				// init the public key display to "no public key".
				display_no_public_key();

				// show idle screen.
				ui_idle();

#if defined(LISTEN_BLE) && defined(TARGET_NANOX)
				if (os_seph_features() &
						SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_BLE) {
					BLE_power(0, NULL);
					// restart IOs
					BLE_power(1, NULL);
				}
#endif // defined(LISTEN_BLE) && defined(TARGET_NANOX)

				// set timer
				Timer_Set();

				// run main event loop.
				constellation_main();
			}
			CATCH_OTHER(e)
			{
			}
			FINALLY
			{
			}
		}
		END_TRY;
	}
	app_exit();
	return 0;
}
