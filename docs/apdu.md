# Constellation application: Technical Specifications

This page details the protocol implemented since version x.x (?) of the app.

## Framework

### APDUs

The messaging format of the app is compatible with the [APDU protocol](https://developers.ledger.com/docs/nano-app/application-structure/#apdu-interpretation-loop). 

The `P1` field is used to indicate whether there are more packets in the transaction pending. 
| P1 Value | P1 Name | DESCRIPTION | 
|----------|---------|-------------|
|   0x00   | `P1_MORE` | more packets on the way | 
|   0x80   | `P1_LAST` | final packets 	            |  

The main commands use `CLA = 0x80`. 
Any transmissions will be rejected that do not begin with this 

| CLA | INS | COMMAND NAME        | DESCRIPTION |
|-----|-----|---------------------|-------------|
| 0x80|  02 | `INS_SIGN` 	      | Sign a txn (more info?) |
| 0x80|  04 | `INS_GET_PUBLIC_KEY` | Return extended pubkey from a BIP44 path |
| 0x80|  06 | `INS_BLIND_SIGN`    | Sign a message with a key (from a BIP32 path?) |

## Status Words

| SW     | SW name                      | Description |
|--------|------------------------------|-------------|
| 0x6982 | `SW_NO_APDU_RECEIVED`		| No APDU received |
| 0x6985 | `SW_DENY`                    | Rejected by user |
| 0x6A86 | `SW_WRONG_P1`                | `P1` is incorrect |
| 0x6D00 | `SW_INS_NOT_SUPPORTED`       | No command exists with `INS` |
| 0x6D09 | `SW_INSUFFICIENT_DATA`       | Not enough data to process request |
| 0x6E08 | `SW_MAX_PKT_EXCEEDED`        | Max packet size has been exceeded |
| 0x6E00 | `SW_CLA_NOT_SUPPORTED`       | Bad `CLA` used for this application |
| 0x9000 | `SW_OK`                      | Success |

## Commands

### INS_SIGN (? TODO)

Returns a signature of the hashed txn input (appended to a prefix?).

#### Encoding

| *CLA* | *INS* |
|-------|-------|
| 0x80  | 0x02  |

**Input data**  

| Length | Name              | Description |
|--------|-------------------|-------------|
(?)

**Output data**

| Length | Description |
|--------|-------------|
| `<variable>` | The full hex encoded binary signature |

#### Description

(?) 

### INS_GET_PUBLIC_KEY

Returns an extended public key at the given derivation path, serialized as per BIP-32.

#### Encoding

**Command**

| *CLA* | *INS* |
|-------|-------|
| 0x80  | 0x04  |

**Input data**  

| Length | Name              | Description |
|--------|-------------------|-------------|
| `4`    | `bip44_path[0]`   | `purpose` |
| `4`    | `bip44_path[1]`   | `coin type` |
| `4`    | `bip44_path[2]`   | `account` |
| `4`    | `bip44_path[3]`   | `change` |
| `4`    | `bip44_path[4]`   | `address_index` |

**Output data**

| Length | Description |
|--------|-------------|
| `<variable>` | The full serialized extended public key as per BIP-32 |

#### Description

This command returns the extended public key for the given BIP 32 path.(?)
The paths defined in [BIP-44](https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki), [BIP-48](https://github.com/bitcoin/bips/blob/master/bip-0048.mediawiki), [BIP-49](https://github.com/bitcoin/bips/blob/master/bip-0049.mediawiki), [BIP-84](https://github.com/bitcoin/bips/blob/master/bip-0084.mediawiki) and [BIP-86](https://github.com/bitcoin/bips/blob/master/bip-0086.mediawiki), either in full or are at the deepest hardened level (excluding `change` and `address_index`), are considered standard.(?)

### INS_BLIND_SIGN

Returns a blind signature of a message appended to a prefix, then hashed.

#### Encoding

| *CLA* | *INS* |
|-------|-------|
| 0x80  | 0x06  |

**Input data**  

All packets in single or multiple packet scenerio begin with `P1` and `body_length`. 
Max `body_length` is 255 bytes.

| Length | Name              | Description |
|--------|-------------------|-------------|
| `1`    | `P1`				 | `P1_MORE` or `P1_LAST` |
| `1`	 | `body_length`	 | length of current packet body |

The first packet will always contain the `length` and begin the `payload` section. 

| Length | Name              | Description |
|--------|-------------------|-------------|
| `4`    | `length`   		 | total length of payload to be signed |
| `<variable>` | `payload`   | message to be signed, can span multiple packets | 

Max `length` is 768 bytes minus 20 bytes of bip44 path, minus 32 bytes of message prefix and is currently 706 bytes for the actual message to be signed

BIP44 path is the last data transmitted in either a single or multiple packet scenerio,
appended directly to `payload`.
| Length | Name              | Description |
|--------|-------------------|-------------|
| `4`    | `bip44_path[0]`   | purpose |
| `4`    | `bip44_path[1]`   | coin type |
| `4`    | `bip44_path[2]`   | account |
| `4`    | `bip44_path[3]`   | change |
| `4`    | `bip44_path[4]`   | address index |

**Output data**

On user approval of the signature request, a signature is returned. 

| Length | Description |
|--------|-------------|
| `<variable>` | The full hex encoded binary signature |

On user Deny, the Status Word `Deny` is returned. 

#### Description

This command will allow the ledger to buffer the message payload data before appending to a message prefix, hashing it and then blind sign it.
