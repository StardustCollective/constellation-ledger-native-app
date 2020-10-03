/*
 * MIT License, see root folder for full license.
 */

#ifndef CONSTELLATION_H
#define CONSTELLATION_H

#include "os.h"
#include "hex.h"
#include "cx.h"
#include <stdbool.h>
#include "os_io_seproxyhal.h"
#include "ui.h"

/** length of the public key */
#define PUBLIC_KEY_LEN 65

/** length of the public key prefix */
#define PUBLIC_KEY_PREFIX_LEN 23

/** length of the encoded public key */
#define PUBLIC_KEY_ENCODED_LEN PUBLIC_KEY_PREFIX_LEN + PUBLIC_KEY_LEN

/** length of the base58 encoding of the public key */
#define BASE58_ENCODED_ADDRESS_LEN 120

/** length of the suffix of the base58 key used for the address */
#define BASE58_ENCODED_ADDRESS_SUFFIX_LEN 36

/** length of a tx.output Address before encoding, which is the length of <address_version>+<script_hash>+<checksum> */
#define ADDRESS_LEN BASE58_ENCODED_ADDRESS_SUFFIX_LEN + 4

extern unsigned char public_key_encoded[33];

extern unsigned char address[ADDRESS_LEN];

/** parse the raw transaction in raw_tx and fill up the screens in tx_desc. */
void display_tx_desc(void);

/** calculates the hash based on the tx */
void calc_hash(void);

/** displays the "no public key" message, prior to a public key being requested. */
void display_no_public_key(void);

/** displays the public key, assumes length is 65. */
void display_public_key(const unsigned char * public_key);

#endif // CONSTELLATION_H
