#pragma once

#include <vector>
#include <ArduinoJson.h>
#include "cards/Card.h"

class CardRegistry {
public:
    static CardRegistry& getInstance();

    // Register a new modular card
    void registerCard(Card* card);

    // Lookup
    Card* getCard(const char* id);
    const std::vector<Card*>& getCards() const { return _cards; }

    // Reorder & sort according to getOrder()
    void sortCards();

    // Serialize all registered cards for Web API / LittleFS
    void serializeAll(JsonArray& array) const;

    // Apply incoming config JSON to registered cards
    void deserializeAll(const JsonArrayConst& array);

    // Batch operations
    void applyThemeToAll(const ThemeColors& theme);
    void updateVisibilityAll();

private:
    CardRegistry() = default;
    ~CardRegistry() = default;
    CardRegistry(const CardRegistry&) = delete;
    CardRegistry& operator=(const CardRegistry&) = delete;

    std::vector<Card*> _cards;
};
