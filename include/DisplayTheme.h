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

// Day Theme: Crisp cyan, clean white, slate navy cards
static const ThemeColors THEME_DAY = {
    .bg            = 0x0842, // Deep slate navy
    .headerBg      = 0x0926, // Deep dark cyan-navy
    .cardBg        = 0x10A4, // Dark slate card
    .cardBorder    = 0x2969, // Subtle cyan-slate border
    .accentColor   = 0x07FD, // Crisp bright cyan
    .timeColor     = 0x07FD, // Crisp bright cyan
    .dateColor     = 0xFFFF, // Pure white
    .mutedText     = 0x9CD3, // Muted light slate gray
    .modeBadgeBg   = 0x01E8, // Deep teal background
    .modeBadgeText = 0x07FD, // Cyan text
    .modeLabel     = "DAY"
};

// Night Theme: Pure black background, warm low-emission amber digits (preserves night-adapted vision)
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
