#pragma once

#include <Arduino.h>

// ==========================================
// Display Themes & Mode Definitions
// ==========================================

enum class DisplayMode {
    DAY,
    NIGHT
};

struct ThemeColors {
    uint16_t bg;             // Screen background
    uint16_t headerBg;       // Top banner background
    uint16_t cardBg;         // Content cards background
    uint16_t cardBorder;     // Card outline and separators
    uint16_t accentColor;    // Card titles and highlights
    uint16_t timeColor;      // Main clock digits
    uint16_t dateColor;      // Date string text
    uint16_t mutedText;      // Secondary informational labels
    uint16_t modeBadgeBg;    // Header mode badge background
    uint16_t modeBadgeText;  // Header mode badge text
    const char* modeLabel;   // "DAY" or "NIGHT"
};

// Helper: Convert "#RRGGBB" or "RRGGBB" string to 16-bit RGB565
inline uint16_t hexToRGB565(const char* hex) {
    if (!hex) return 0xFFFF;
    if (hex[0] == '#') hex++;
    if (strlen(hex) < 6) return 0xFFFF;

    long rgb = strtol(hex, nullptr, 16);
    uint8_t r = (rgb >> 16) & 0xFF;
    uint8_t g = (rgb >> 8) & 0xFF;
    uint8_t b = rgb & 0xFF;

    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Helper: Convert 16-bit RGB565 to "#RRGGBB" string
inline String rgb565ToHex(uint16_t color) {
    uint8_t r = (color >> 11) & 0x1F;
    uint8_t g = (color >> 5) & 0x3F;
    uint8_t b = color & 0x1F;

    uint8_t r8 = (r * 527 + 23) >> 6;
    uint8_t g8 = (g * 259 + 33) >> 6;
    uint8_t b8 = (b * 527 + 23) >> 6;

    char buf[8];
    snprintf(buf, sizeof(buf), "#%02X%02X%02X", r8, g8, b8);
    return String(buf);
}

// Default Day Theme: Crisp cyan, clean white, slate navy cards
static const ThemeColors THEME_DAY = {
    .bg            = 0x0842, // Deep slate navy
    .headerBg      = 0x0926, // Deep dark cyan-navy
    .cardBg        = 0x10A4, // Dark slate card
    .cardBorder    = 0x2969, // Subtle cyan-slate border
    .accentColor   = 0x07FD, // Crisp bright cyan (#00FFFF)
    .timeColor     = 0x07FD, // Crisp bright cyan
    .dateColor     = 0xFFFF, // Pure white
    .mutedText     = 0x9CD3, // Muted light slate gray
    .modeBadgeBg   = 0x01E8, // Deep teal background
    .modeBadgeText = 0x07FD, // Cyan text
    .modeLabel     = "DAY"
};

// Default Night Theme: Pure black background, warm low-emission amber digits
static const ThemeColors THEME_NIGHT = {
    .bg            = 0x0000, // True pitch black (minimizes IPS light bleed)
    .headerBg      = 0x0000, // True pitch black
    .cardBg        = 0x0800, // Deep warm dark maroon
    .cardBorder    = 0x2900, // Subtle dim warm border
    .accentColor   = 0xD340, // Warm amber
    .timeColor     = 0xFA00, // Rich warm amber digits
    .dateColor     = 0x8BC8, // Muted warm dusk text
    .mutedText     = 0x5A84, // Very dim secondary text
    .modeBadgeBg   = 0x2880, // Dark amber badge background
    .modeBadgeText = 0xFA00, // Amber badge text
    .modeLabel     = "NIGHT"
};
