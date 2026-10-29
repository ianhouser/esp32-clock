#include "cards/CardRegistry.h"
#include <algorithm>

CardRegistry& CardRegistry::getInstance() {
    static CardRegistry instance;
    return instance;
}

void CardRegistry::registerCard(Card* card) {
    if (!card) return;
    for (auto* existing : _cards) {
        if (strcmp(existing->getId(), card->getId()) == 0) {
            return; // Already registered
        }
    }
    _cards.push_back(card);
    sortCards();
}

Card* CardRegistry::getCard(const char* id) {
    if (!id) return nullptr;
    for (auto* card : _cards) {
        if (strcmp(card->getId(), id) == 0) {
            return card;
        }
    }
    return nullptr;
}

void CardRegistry::sortCards() {
    std::sort(_cards.begin(), _cards.end(), [](const Card* a, const Card* b) {
        return a->getOrder() < b->getOrder();
    });
}

void CardRegistry::serializeAll(JsonArray& array) const {
    for (const auto* card : _cards) {
        JsonObject obj = array.add<JsonObject>();
        obj["id"] = card->getId();
        obj["title"] = card->getTitle();
        obj["description"] = card->getDescription();
        obj["icon"] = card->getIcon();
        obj["enabled"] = card->isEnabled();
        obj["order"] = card->getOrder();

        JsonObject configObj = obj["config"].to<JsonObject>();
        card->serializeConfig(configObj);
    }
}

void CardRegistry::deserializeAll(const JsonArrayConst& array) {
    for (JsonObjectConst obj : array) {
        const char* id = obj["id"];
        if (!id) continue;

        Card* card = getCard(id);
        if (card) {
            if (obj["enabled"].is<bool>()) {
                card->setEnabled(obj["enabled"].as<bool>());
            }
            if (obj["order"].is<int>()) {
                card->setOrder(obj["order"].as<int>());
            }
            if (obj["config"].is<JsonObjectConst>()) {
                JsonObjectConst configObj = obj["config"].as<JsonObjectConst>();
                card->deserializeConfig(configObj);
            }
        }
    }
    sortCards();
    updateVisibilityAll();
}

void CardRegistry::applyThemeToAll(const ThemeColors& theme) {
    for (auto* card : _cards) {
        card->applyTheme(theme);
    }
}

void CardRegistry::updateVisibilityAll() {
    for (auto* card : _cards) {
        card->setVisible(card->isEnabled());
    }
}
