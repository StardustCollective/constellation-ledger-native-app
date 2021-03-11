

/** array of capital letter hex values */
static const char HEX_CAP[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };

/** converts a byte array in src to a hex array in dest, using only dest_len bytes of dest before stopping. */
void to_hex(char * dest, const unsigned char * src, const unsigned int dest_len) {
	for (unsigned int src_ix = 0, dest_ix = 0; dest_ix < dest_len; src_ix++, dest_ix += 2) {
		unsigned char src_c = *(src + src_ix);
		unsigned char nibble0 = (src_c >> 4) & 0xF;
		unsigned char nibble1 = src_c & 0xF;

		*(dest + dest_ix + 0) = HEX_CAP[nibble0];
		*(dest + dest_ix + 1) = HEX_CAP[nibble1];
	}
}
