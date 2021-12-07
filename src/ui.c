/*
 * MIT License, see root folder for full license.
 */

#include "ui.h"
#include "glyphs.h"

/** default font */
#define DEFAULT_FONT BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER

#define DEFAULT_FONT_BLUE BAGL_FONT_OPEN_SANS_LIGHT_14px | BAGL_FONT_ALIGNMENT_CENTER | BAGL_FONT_ALIGNMENT_MIDDLE

/** text description font. */
#define TX_DESC_FONT BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER

/** the timer */
int exit_timer;

/** display for the timer */
char timer_desc[MAX_TIMER_TEXT_WIDTH];

/** UI state enum */
enum UI_STATE uiState;

/** notification to restart the hash */
unsigned char hashTainted;

/** notification to refresh the view, if we are displaying the public key */
unsigned char publicKeyNeedsRefresh;

static const char BASE_16_ALPHABET[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

/** index of the current screen. */
unsigned int curr_scr_ix;

/** max index for all screens. */
unsigned int max_scr_ix;

/** raw transaction data. */
unsigned char raw_tx[MAX_TX_RAW_LENGTH];

/** current index into raw transaction. */
unsigned int raw_tx_ix;

/** current length of raw transaction. */
unsigned int raw_tx_len;

/** all text descriptions. */
char tx_desc[MAX_TX_TEXT_SCREENS][MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

/** currently displayed text description. */
char curr_tx_desc[MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

/** currently displayed public key */
char current_public_key[MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

/** hash to go into kryto serialize */
unsigned char hash_data[HASH_DATA_SIZE];

/** hash ix to go into kryto serialize */
unsigned int hash_data_ix;

static const unsigned char KRYO_PREFIX[] = {0x03,0x01};

/** UI was touched indicating the user wants to deny te signature request */
static const bagl_element_t * io_seproxyhal_touch_deny(const bagl_element_t *e);

#ifndef TARGET_NANOX
/** UI was touched indicating the user wants to exit the app */
static const bagl_element_t * io_seproxyhal_touch_exit(const bagl_element_t *e);

/** display part of the transaction description */
static void ui_display_tx_desc_1(void);
static void ui_display_tx_desc_2(void);

/** display the UI for signing a transaction */
static void ui_sign(void);

/** display the UI for denying a transaction */
static void ui_deny(void);

/** show the public key screen */
static void ui_public_key_1(void);
static void ui_public_key_2(void);

/** move up in the transaction description list */
static const bagl_element_t * tx_desc_up(const bagl_element_t *e);

/** move down in the transaction description list */
static const bagl_element_t * tx_desc_dn(const bagl_element_t *e);
#endif

/** sets the tx_desc variables to no information */
static void clear_tx_desc(void);

////////////////////////////////////  NANO X //////////////////////////////////////////////////
#ifdef TARGET_NANOX

UX_STEP_NOCB(
    ux_confirm_single_flow_1_step,
    nn,
    {
        // &C_icon_eye,
        "Review",
        "Transaction"
	});
UX_STEP_NOCB(
    ux_confirm_single_flow_2_step,
    bnn,
    {
        // number of parents.
        tx_desc[0][0],
        tx_desc[0][1],
        tx_desc[0][2]
	});
UX_STEP_NOCB(
    ux_confirm_single_flow_3_step,
    bnnn,
    {
        "From Address",
        tx_desc[1][0],
        tx_desc[1][1],
        tx_desc[1][2]
	});
UX_STEP_NOCB(
    ux_confirm_single_flow_4_step,
    bnnn,
    {
        "To Address",
        tx_desc[2][0],
        tx_desc[2][1],
        tx_desc[2][2]
	});
UX_STEP_NOCB(
    ux_confirm_single_flow_5_step,
    bnn,
    {
        // "Amount",
        tx_desc[3][0],
        tx_desc[3][1],
        tx_desc[3][2]
	});
UX_STEP_NOCB(
    ux_confirm_single_flow_6_step,
    bnn,
    {
        // "Last Ref 01",
        tx_desc[4][0],
        tx_desc[4][1],
        tx_desc[4][2]
	});
UX_STEP_NOCB(
    ux_confirm_single_flow_7_step,
    bnn,
    {
        // "Last Ref 02",
        tx_desc[5][0],
        tx_desc[5][1],
        tx_desc[5][2]
	});
UX_STEP_NOCB(
    ux_confirm_single_flow_8_step,
    bnn,
    {
        // "Ordinal",
        tx_desc[6][0],
        tx_desc[6][1],
        tx_desc[6][2]
	});
UX_STEP_NOCB(
    ux_confirm_single_flow_9_step,
    bnn,
    {
        // "Fee",
        tx_desc[7][0],
        tx_desc[7][1],
        tx_desc[7][2]
	});
UX_STEP_NOCB(
    ux_confirm_single_flow_10_step,
    bnn,
    {
        // "Salt",
        tx_desc[8][0],
        tx_desc[8][1],
        tx_desc[8][2]
	});
UX_STEP_VALID(
    ux_confirm_single_flow_11_step,
    nn,
    io_seproxyhal_touch_approve(NULL),
    {
        // &C_icon_validate_14,
        "Accept",
        "Transaction"
	});
UX_STEP_VALID(
    ux_confirm_single_flow_12_step,
    nn,
    io_seproxyhal_touch_deny(NULL),
    {
        // &C_icon_crossmark,
        "Reject",
        "Transaction"
	});
UX_FLOW(ux_confirm_single_flow,
        &ux_confirm_single_flow_1_step,
        &ux_confirm_single_flow_2_step,
        &ux_confirm_single_flow_3_step,
        &ux_confirm_single_flow_4_step,
        &ux_confirm_single_flow_5_step,
        &ux_confirm_single_flow_6_step,
        &ux_confirm_single_flow_7_step,
        &ux_confirm_single_flow_8_step,
        &ux_confirm_single_flow_9_step,
        &ux_confirm_single_flow_10_step,
        &ux_confirm_single_flow_11_step,
        &ux_confirm_single_flow_12_step
        );

UX_STEP_NOCB(
    ux_display_public_flow_step,
    bnnn,
    {
        "Address",
        current_public_key[0],
        current_public_key[1],
        current_public_key[2]
	});
UX_STEP_VALID(
    ux_display_public_go_back_step,
    nn,
    ui_idle(),
    {
        // &C_icon_back,
        "Go",
        "Back"
	});

UX_FLOW(ux_display_public_flow,
        &ux_display_public_flow_step,
        &ux_display_public_go_back_step
        );

void display_account_address(){
  uiState = UI_PUBLIC_KEY_1;
	if(G_ux.stack_count == 0) {
		ux_stack_push();
	}
	ux_flow_init(0, ux_display_public_flow, NULL);
}

UX_STEP_NOCB(
    ux_idle_flow_1_step,
    nn,
    {
        "Application",
        "is ready",
	});
UX_STEP_VALID(
    ux_idle_flow_2_step,
    nn,
    display_account_address(),
    {
        // &C_icon_eye,
        "Display",
        "Account"
	});
UX_STEP_NOCB(
    ux_idle_flow_3_step,
    bn,
    {
        "Version",
        APPVERSION,
	});
UX_STEP_VALID(
    ux_idle_flow_4_step,
    bn,
    os_sched_exit(-1),
    {
        // &C_icon_dashboard,
        "Quit",
        ""
	});

UX_FLOW(ux_idle_flow,
        &ux_idle_flow_1_step,
        &ux_idle_flow_2_step,
        &ux_idle_flow_3_step,
        &ux_idle_flow_4_step
        );


#endif
////////////////////////////////////////////////////////////////////////////////////////////////



#ifndef TARGET_NANOX

/** UI struct for the idle screen */
static const bagl_element_t bagl_ui_idle_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{       {       BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL},
	/* center text */
	{       {       BAGL_LABELINE, 0x02, 0, 12, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DEFAULT_FONT, 0 }, "Constellation"},
	/* left icon is a X */
	{       {       BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_CROSS }, NULL},
	/* right icon is an eye. */
	{       {       BAGL_ICON, 0x00, 117, 11, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_EYE_BADGE }, NULL},

/* */
};

/**
 * buttons for the idle screen
 *
 * exit on Left button, or on Both buttons. Do nothing on Right button only.
 */
static unsigned int bagl_ui_idle_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	UNUSED(button_mask_counter);

	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		ui_public_key_1();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		io_seproxyhal_touch_exit(NULL);
		break;
	}

	return 0;
}



/** UI struct for the idle screen */
static const bagl_element_t bagl_ui_public_key_nanos_1[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{       {       BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL},
	/* first line of description of current public key */
	{       {       BAGL_LABELINE, 0x02, 10, 10, 108, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, TX_DESC_FONT, 0 }, current_public_key[0]},
	/* second line of description of current public key */
	{       {       BAGL_LABELINE, 0x02, 10, 21, 108, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, TX_DESC_FONT, 0 }, current_public_key[1]},
	/* right icon is a X */
	{       {       BAGL_ICON, 0x00, 113, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_CROSS }, NULL},
	/* left icon is down arrow  */
	{       {       BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL},
/* */
};


/** UI struct for the idle screen */
static const bagl_element_t bagl_ui_public_key_nanos_2[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{       {       BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL},
	/* second line of description of current public key */
	{       {       BAGL_LABELINE, 0x02, 10, 10, 108, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, TX_DESC_FONT, 0 }, current_public_key[1]},
	/* third line of description of current public key  */
	{       {       BAGL_LABELINE, 0x02, 10, 21, 108, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, TX_DESC_FONT, 0 }, current_public_key[2]},
	/* right icon is a X */
	{       {       BAGL_ICON, 0x00, 113, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_CROSS }, NULL},
	/* left icon is up arrow  */
	{       {       BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL},

/* */
};

/**
 * buttons for the idle screen
 *
 * exit on Left button, or on Both buttons. Do nothing on Right button only.
 */
static unsigned int bagl_ui_public_key_nanos_1_button(unsigned int button_mask, unsigned int button_mask_counter) {
	UNUSED(button_mask_counter);

	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		ui_idle();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		ui_public_key_2();
		break;
	}


	return 0;
}


/**
 * buttons for the idle screen
 *
 * exit on Left button, or on Both buttons. Do nothing on Right button only.
 */
static unsigned int bagl_ui_public_key_nanos_2_button(unsigned int button_mask, unsigned int button_mask_counter) {
	UNUSED(button_mask_counter);

	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		ui_idle();
		break;
	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		ui_public_key_1();
		break;
	}
	return 0;
}


/** UI struct for the top "Sign Transaction" screen, Nano S. */
static const bagl_element_t bagl_ui_top_sign_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{       {       BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL},
	/* top left bar */
//	{       {       BAGL_RECTANGLE, 0x00, 3, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL},
	/* top right bar */
//	{       {       BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL},
	/* center text */
	{       {       BAGL_LABELINE, 0x02, 0, 20, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DEFAULT_FONT, 0 }, "Review Tx"},
	/* left icon is up arrow  */
	{       {       BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL},
	/* right icon is down arrow */
	{       {       BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL},
/* */
};

/**
 * buttons for the top "Sign Transaction" screen
 *
 * up on Left button, down on right button, sign on both buttons.
 */
static unsigned int bagl_ui_top_sign_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	UNUSED(button_mask_counter);

	switch (button_mask) {
	/*
	   case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
	   io_seproxyhal_touch_approve(NULL);
	   break;
	 */

	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		tx_desc_dn(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		tx_desc_up(NULL);
		break;
	}
	return 0;
}

/** UI struct for the bottom "Sign Transaction" screen, Nano S. */
static const bagl_element_t bagl_ui_sign_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{       {       BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL},
	/* top left bar */
	{       {       BAGL_RECTANGLE, 0x00, 3, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL},
	/* top right bar */
	{       {       BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL},
	/* center text */
	{       {       BAGL_LABELINE, 0x02, 0, 20, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DEFAULT_FONT, 0 }, "Sign Tx"},
	/* left icon is up arrow  */
	{       {       BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL},
	/* right icon is down arrow */
	{       {       BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL},
/* */
};

/**
 * buttons for the bottom "Sign Transaction" screen
 *
 * up on Left button, down on right button, sign on both buttons.
 */
static unsigned int bagl_ui_sign_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	UNUSED(button_mask_counter);

	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		io_seproxyhal_touch_approve(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		tx_desc_dn(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		tx_desc_up(NULL);
		break;
	}
	return 0;
}

/** UI struct for the bottom "Deny Transaction" screen, Nano S. */
static const bagl_element_t bagl_ui_deny_nanos[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{       {       BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL},
	/* top left bar */
	{       {       BAGL_RECTANGLE, 0x00, 3, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL},
	/* top right bar */
	{       {       BAGL_RECTANGLE, 0x00, 113, 1, 12, 2, 0, 0, BAGL_FILL, 0xFFFFFF, 0x000000, 0, 0 }, NULL},
	/* center text */
	{       {       BAGL_LABELINE, 0x02, 0, 20, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, DEFAULT_FONT, 0 }, "Deny Tx"},
	/* left icon is up arrow  */
	{       {       BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL},
	{       {       BAGL_ICON, 0x00, 117, 13, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL},
/* */
};

/**
 * buttons for the bottom "Deny Transaction" screen
 *
 * up on Left button, down on right button, deny on both buttons.
 */
static unsigned int bagl_ui_deny_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
	UNUSED(button_mask_counter);

	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
		io_seproxyhal_touch_deny(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		tx_desc_dn(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		tx_desc_up(NULL);
		break;
	}
	return 0;
}

/** UI struct for the transaction description screen, Nano S. */
static const bagl_element_t bagl_ui_tx_desc_nanos_1[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{       {       BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL},
	/* screen 1 number */
	/* first line of description of current screen */
	{       {       BAGL_LABELINE, 0x02, 10, 15, 108, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, TX_DESC_FONT, 0 }, curr_tx_desc[0]},
	/* second line of description of current screen */
	{       {       BAGL_LABELINE, 0x02, 10, 26, 108, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, TX_DESC_FONT, 0 }, curr_tx_desc[1]},
	/* left icon is up arrow  */
	{       {       BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL},
	/* right icon is down arrow */
	{       {       BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL},
/* */
};

/** UI struct for the transaction description screen, Nano S. */
static const bagl_element_t bagl_ui_tx_desc_nanos_2[] = {
// { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id},
// text, touch_area_brim, overfgcolor, overbgcolor, tap, out, over,
// },
	{       {       BAGL_RECTANGLE, 0x00, 0, 0, 128, 32, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL},
	/* screen 2 number */
	{       {       BAGL_LABELINE, 0x02, 0, 10, 20, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, TX_DESC_FONT, 0 }, "2/2"},
	/* second line of description of current screen */
	{       {       BAGL_LABELINE, 0x02, 10, 15, 108, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, TX_DESC_FONT, 0 }, curr_tx_desc[1]},
	/* third line of description of current screen  */
	{       {       BAGL_LABELINE, 0x02, 10, 26, 108, 11, 0x80 | 10, 0, 0, 0xFFFFFF, 0x000000, TX_DESC_FONT, 0 }, curr_tx_desc[2]},
	/* left icon is up arrow  */
	{       {       BAGL_ICON, 0x00, 3, 12, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_UP }, NULL},
	/* right icon is down arrow */
	{       {       BAGL_ICON, 0x00, 117, 13, 8, 6, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_ICON_DOWN }, NULL},
/* */
};

/**
 * buttons for the transaction description screen
 *
 * up on Left button, down on right button.
 */
static unsigned int bagl_ui_tx_desc_nanos_1_button(unsigned int button_mask, unsigned int button_mask_counter) {
	UNUSED(button_mask_counter);

	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		tx_desc_dn(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		tx_desc_up(NULL);
		break;
	}
	return 0;
}

/**
 * buttons for the transaction description screen
 *
 * up on Left button, down on right button.
 */
static unsigned int bagl_ui_tx_desc_nanos_2_button(unsigned int button_mask, unsigned int button_mask_counter) {
	UNUSED(button_mask_counter);

	switch (button_mask) {
	case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
		tx_desc_dn(NULL);
		break;

	case BUTTON_EVT_RELEASED | BUTTON_LEFT:
		tx_desc_up(NULL);
		break;
	}
	return 0;
}
#endif

#ifndef TARGET_NANOX
/** if the user wants to exit go back to the app dashboard. */
static const bagl_element_t *io_seproxyhal_touch_exit(const bagl_element_t *e) {
	UNUSED(e);
	// Go back to the dashboard
	os_sched_exit(0);
	return NULL;                     // do not redraw the widget
}
#endif

#ifndef TARGET_NANOX
/** copy the current row of the tx_desc buffer into curr_tx_desc to display on the screen */
static void copy_tx_desc(void) {
	memmove(curr_tx_desc, tx_desc[curr_scr_ix], CURR_TX_DESC_LEN);
	curr_tx_desc[0][MAX_TX_TEXT_WIDTH - 1] = '\0';
	curr_tx_desc[1][MAX_TX_TEXT_WIDTH - 1] = '\0';
	curr_tx_desc[2][MAX_TX_TEXT_WIDTH - 1] = '\0';
}
#endif

#ifndef TARGET_NANOX
/** processes the Up button */
static const bagl_element_t * tx_desc_up(const bagl_element_t *e) {
	UNUSED(e);

	switch (uiState) {
	case UI_TOP_SIGN:
		ui_deny();
		break;

	case UI_TX_DESC_1:
		if (curr_scr_ix == 0) {
			ui_top_sign();
		} else {
			curr_scr_ix--;
			copy_tx_desc();
			ui_display_tx_desc_2();
		}
		break;

	case UI_TX_DESC_2:
		ui_display_tx_desc_1();
		break;

	case UI_SIGN:
		curr_scr_ix = max_scr_ix - 1;
		copy_tx_desc();
		ui_display_tx_desc_1();
		break;

	case UI_DENY:
		ui_sign();
		break;

	default:
		hashTainted = 1;
		THROW(0x6D02);
		break;
	}
	return NULL;
}

/** processes the Down button */
static const bagl_element_t * tx_desc_dn(const bagl_element_t *e) {
	UNUSED(e);

	switch (uiState) {
	case UI_TOP_SIGN:
		curr_scr_ix = 0;
		copy_tx_desc();
		ui_display_tx_desc_1();
		break;

	case UI_TX_DESC_1:
		ui_display_tx_desc_2();
		break;

	case UI_TX_DESC_2:
		if (curr_scr_ix == max_scr_ix - 1) {
			ui_sign();
		} else {
			curr_scr_ix++;
			copy_tx_desc();
			ui_display_tx_desc_1();
		}
		break;

	case UI_SIGN:
		ui_deny();
		break;

	case UI_DENY:
		ui_top_sign();
		break;

	default:
		hashTainted = 1;
		THROW(0x6D01);
		break;
	}
	return NULL;
}
#endif

unsigned int utf8Length(unsigned char * buffer, unsigned int value) {
	unsigned int utfLengthAsHex = 0;
	if (value >> 6 == 0) {
		utfLengthAsHex = 1;
		buffer[0] = (value | 0x80);         // Set bit 8.
	} else if (value >> 13 == 0) {
		utfLengthAsHex = 2;
		buffer[0] = (value | 0x40 | 0x80);         // Set bit 7 and 8.
		buffer[1] = (value >> 6);
	} else if (value >> 20 == 0) {
		utfLengthAsHex = 3;
		buffer[0] = (value | 0x40 | 0x80);         // Set bit 7 and 8.
		buffer[1] = ((value >> 6) | 0x80);         // Set bit 8.
		buffer[2] = (value >> 13);
	} else if (value >> 27 == 0) {
		utfLengthAsHex = 4;
		buffer[0] = (value | 0x40 | 0x80);         // Set bit 7 and 8.
		buffer[1] = ((value >> 6) | 0x80);         // Set bit 8.
		buffer[2] = ((value >> 13) | 0x80);         // Set bit 8.
		buffer[3] = (value >> 20);
	} else {
		utfLengthAsHex = 5;
		buffer[0] = (value | 0x40 | 0x80);         // Set bit 7 and 8.
		buffer[1] = ((value >> 6) | 0x80);         // Set bit 8.
		buffer[2] = ((value >> 13) | 0x80);         // Set bit 8.
		buffer[3] = ((value >> 20) | 0x80);         // Set bit 8.
		buffer[4] = (value >> 27);
	}
	return utfLengthAsHex;
}


/** processes the transaction approval. the UI is only displayed when all of the TX has been sent over for signing. */
const bagl_element_t*io_seproxyhal_touch_approve(const bagl_element_t *e) {
	UNUSED(e);

  unsigned int tx = 0;
	if (G_io_apdu_buffer[2] == P1_LAST) {
		unsigned int raw_tx_len_except_bip44 = raw_tx_len - BIP44_BYTE_LENGTH;
		unsigned char * bip44_in = raw_tx + raw_tx_len_except_bip44;

		/** BIP44 path, used to derive the private key from the mnemonic by calling os_perso_derive_node_bip32. */
		unsigned int bip44_path[BIP44_PATH_LEN];
		uint32_t i;
		for (i = 0; i < BIP44_PATH_LEN; i++) {
			bip44_path[i] = (bip44_in[0] << 24) | (bip44_in[1] << 16) | (bip44_in[2] << 8) | (bip44_in[3]);
			bip44_in += 4;
		}

		// Hash is finalized, send back the signature
		unsigned char result256[CX_SHA256_SIZE];
		memset(result256, 0x00, sizeof(result256));

		cx_sha256_t hash_context_256;
		cx_hash_t * hash_ptr_256 = (cx_hash_t *)&hash_context_256;
		cx_sha256_init(&hash_context_256);


		unsigned char utfLengthAsHex[6];
		unsigned int utfLengthAsHexLen = utf8Length(utfLengthAsHex,hash_data_ix+1);
		cx_hash(hash_ptr_256, 0, KRYO_PREFIX, sizeof(KRYO_PREFIX), result256, sizeof(result256));
		cx_hash(hash_ptr_256, 0, utfLengthAsHex, utfLengthAsHexLen, result256, sizeof(result256));
		cx_hash(hash_ptr_256, CX_LAST, hash_data, hash_data_ix, result256, sizeof(result256));

		//encode the result and hash again.
		unsigned char result256hex[CX_SHA256_SIZE * 2];
		memset(result256hex, 0x00, sizeof(result256hex));
		unsigned int result256hex_ix = 0;
		for(unsigned int ix = 0; ix < CX_SHA256_SIZE * 2; ix++) {
			unsigned char c = result256[ix];
			unsigned int i0 = (c>>4)&0xF;
			unsigned int i1 = c&0xF;
			result256hex[result256hex_ix++] = BASE_16_ALPHABET[i0];
			result256hex[result256hex_ix++] = BASE_16_ALPHABET[i1];
		}

		unsigned char result512[CX_SHA512_SIZE];
		memset(result512, 0x00, sizeof(result512));
		cx_hash_sha512(result256hex, sizeof(result256hex), result512, sizeof(result512));

    unsigned char result[32];
    memset(result, 0x00, sizeof(result));
    memmove(result, result512, sizeof(result));
		// #define CX_LAST (1 << 0)
		// #define CX_RND_PRNG (1 << 9)
		// #define CX_RND_TRNG (2 << 9)
		// #define CX_RND_RFC6979 (3 << 9)
		// #define CX_RND_PROVIDED (4 << 9)

    cx_ecfp_private_key_t privateKey;
    unsigned char privateKeyData[32];
    os_perso_derive_node_bip32(CX_CURVE_256K1, bip44_path, BIP44_PATH_LEN, privateKeyData, NULL);
    cx_ecdsa_init_private_key(CX_CURVE_256K1, privateKeyData, 32, &privateKey);

    tx = cx_ecdsa_sign((void*) &privateKey, CX_RND_RFC6979, CX_SHA256, result, sizeof(result), G_io_apdu_buffer, sizeof(G_io_apdu_buffer), NULL);

		cx_ecdsa_init_private_key(CX_CURVE_256K1, NULL, 0, &privateKey);
		memset(privateKeyData, 0x00, sizeof(privateKeyData));

		// G_io_apdu_buffer[0] &= 0xF0; // discard the parity information
		hashTainted = 1;
		clear_tx_desc();
		raw_tx_ix = 0;
		raw_tx_len = 0;

		// add hash to the response, so we can see where the bug is.
		G_io_apdu_buffer[tx++] = 0xFF;
		G_io_apdu_buffer[tx++] = 0xFF;
		for (int ix = 0; ix < 32; ix++) {
			G_io_apdu_buffer[tx++] = result256[ix];
		}
		G_io_apdu_buffer[tx++] = 0xFF;
		G_io_apdu_buffer[tx++] = 0xFF;

		for (unsigned int ix = 0; ix < sizeof(KRYO_PREFIX); ix++) {
			G_io_apdu_buffer[tx++] = KRYO_PREFIX[ix];
		}
		for (unsigned int ix = 0; ix < utfLengthAsHexLen; ix++) {
			G_io_apdu_buffer[tx++] = utfLengthAsHex[ix];
		}
		for (unsigned int ix = 0; ix < hash_data_ix; ix++) {
			G_io_apdu_buffer[tx++] = hash_data[ix];
		}
	}
	G_io_apdu_buffer[tx++] = 0x90;
	G_io_apdu_buffer[tx++] = 0x00;
	// Send back the response, do not restart the event loop
	io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
	// Display back the original UX
	ui_idle();
	return 0;                     // do not redraw the widget
}

/** deny signing. */
static const bagl_element_t *io_seproxyhal_touch_deny(const bagl_element_t *e) {
	UNUSED(e);

	hashTainted = 1;
	clear_tx_desc();
	raw_tx_ix = 0;
	raw_tx_len = 0;
	G_io_apdu_buffer[0] = 0x69;
	G_io_apdu_buffer[1] = 0x85;
	// Send back the response, do not restart the event loop
	io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
	// Display back the original UX
	ui_idle();
	return 0;                     // do not redraw the widget
}

#ifndef TARGET_NANOX
/** show the public key screen */
void ui_public_key_1(void) {
	uiState = UI_PUBLIC_KEY_1;
	if (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG) {
	} else {
		UX_DISPLAY(bagl_ui_public_key_nanos_1, NULL);
	}
}

/** show the public key screen */
void ui_public_key_2(void) {
	uiState = UI_PUBLIC_KEY_2;
	if (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG) {
		UX_DISPLAY(bagl_ui_public_key_nanos_1, NULL);
	} else {
		UX_DISPLAY(bagl_ui_public_key_nanos_2, NULL);
	}
}
#endif

/** show the idle screen. */
void ui_idle(void) {
	uiState = UI_IDLE;

#if defined(TARGET_NANOS)
	UX_DISPLAY(bagl_ui_idle_nanos, NULL);
#elif defined(TARGET_NANOX)
	// reserve a display stack slot if none yet
	if(G_ux.stack_count == 0) {
		ux_stack_push();
	}
	ux_flow_init(0, ux_idle_flow, NULL);
#endif // #if TARGET_ID
}

#ifndef TARGET_NANOX
/** show the transaction description screen. */
static void ui_display_tx_desc_1(void) {
	uiState = UI_TX_DESC_1;
#if defined(TARGET_NANOS)
	UX_DISPLAY(bagl_ui_tx_desc_nanos_1, NULL);
#endif // #if TARGET_ID
}


/** show the transaction description screen. */
static void ui_display_tx_desc_2(void) {
	uiState = UI_TX_DESC_2;
// #if defined(TARGET_NANOS)
// 	UX_DISPLAY(bagl_ui_tx_desc_nanos_2, NULL);
// #endif // #if TARGET_ID
}

/** show the bottom "Sign Transaction" screen. */
static void ui_sign(void) {
	uiState = UI_SIGN;
#if defined(TARGET_NANOS)
	UX_DISPLAY(bagl_ui_sign_nanos, NULL);
#endif // #if TARGET_ID
}
#endif

/** show the top "Sign Transaction" screen. */
void ui_top_sign(void) {
	uiState = UI_TOP_SIGN;

#if defined(TARGET_NANOS)
	UX_DISPLAY(bagl_ui_top_sign_nanos, NULL);
#elif defined(TARGET_NANOX)
	// reserve a display stack slot if none yet
	if(G_ux.stack_count == 0) {
		ux_stack_push();
	}
	ux_flow_init(0, ux_confirm_single_flow, NULL);
#endif // #if TARGET_ID
}

#ifndef TARGET_NANOX
/** show the "deny" screen */
static void ui_deny(void) {
	uiState = UI_DENY;
#if defined(TARGET_NANOS)
	UX_DISPLAY(bagl_ui_deny_nanos, NULL);
#endif // #if TARGET_ID
}
#endif

/** returns the length of the transaction in the buffer. */
unsigned int get_apdu_buffer_length() {
	unsigned int len0 = G_io_apdu_buffer[APDU_BODY_LENGTH_OFFSET];
	return len0;
}

/** sets the tx_desc variables to no information */
static void clear_tx_desc(void) {
	for(uint8_t i=0; i<MAX_TX_TEXT_SCREENS; i++) {
		for(uint8_t j=0; j<MAX_TX_TEXT_LINES; j++) {
			tx_desc[i][j][0] = '\0';
			tx_desc[i][j][MAX_TX_TEXT_WIDTH - 1] = '\0';
		}
	}
}
