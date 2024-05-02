#include "game.h"


#define DIGIT_TO_WCHAR(d) (0xFF10|(d))


// Pass the precise timer value to dGameDisplay_c::setTime(), instead
// of just the number of seconds rounded up
kmWrite32(0x800e3b28, 0x60000000);  // nop
kmWrite32(0x800e3b2c, 0x60000000);  // nop
kmWrite32(0x800e3b30, 0x60000000);  // nop


// Replacement for dGameDisplay_c::setTime()
kmBranchDefCpp(0x80159c00, NULL, void, dGameDisplay_c *this_, int value) {
    if (this_->timer == value) {
        return;
    }
    this_->timer = value;

    // Integer part is calculated identically to how the game normally does it
    int integerPart = (value + 0xfff) >> 12;

    // Fractional part is clamped to 00-99
    int fractionPart = (((value + 0xfff) * 100) >> 12) - (integerPart * 100);
    if (fractionPart < 0) fractionPart = 0;
    if (fractionPart > 99) fractionPart = 99;

    wchar_t buf[7] = {
        DIGIT_TO_WCHAR(integerPart / 100),
        DIGIT_TO_WCHAR((integerPart / 10) % 10),
        DIGIT_TO_WCHAR(integerPart % 10),
        L'.',
        DIGIT_TO_WCHAR(fractionPart / 10),
        DIGIT_TO_WCHAR(fractionPart % 10),
        L'\0'
    };

    this_->timerBox->setText(buf, 0);
}
