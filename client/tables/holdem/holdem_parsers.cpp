#include "holdem.h"

using namespace HoldemWidgets;

void Table::Holdem::new_player(const char *name, int bet, int stack) {
    for (int i = 0; i < max_players; i++) {
        if (players[i].state == Player::OPEN) {
            players[i].set_name(name);
            players[i].set_stack(bet, stack);
            break;
        }
    }
}

void Table::Holdem::new_player(rapidjson::Value& content) {
    for (int i = 0; i < max_players; i++) {
        if (players[i].state == Player::OPEN) {
            const char *name = (const char*) content["name"].GetString();
            int bet = (int) content["bet"].GetInt();
            int stack = (int) content["stack"].GetInt();
            players[i].set_name(name);
            players[i].set_stack(bet, stack);
            return;
        }
    }
}

void Table::Holdem::updates(rapidjson::Value& content) {
    set_players(content["players"]);
    if (content.HasMember("flop")) {
        set_comm_cards(content["flop"]);
    }
}

void Table::Holdem::reconnect(rapidjson::Value& content) {
    set_stack->close();
    set_players(content["players"]);
    set_priv_cards(content["cards"]);
    set_active(content["active"]);
    if (content.HasMember("flop")) {
        set_comm_cards(content["flop"]);
    }
}

void Table::Holdem::betting(rapidjson::Value& content) {
    set_action(content["action"]);
    set_active(content["active"]);
    if (content.HasMember("flop")) {
        set_comm_cards(content["flop"]);
    }
}

void Table::Holdem::init(rapidjson::Value& content) {
    set_players(content["players"]);
    set_priv_cards(content["cards"]);
    set_active(content["active"]);
}

void Table::Holdem::reveal(rapidjson::Value& content) {
    for (rapidjson::SizeType i = 0; i < content.Size(); i++) {
        rapidjson::Value& revealed = content[i];
        const char *name = (char*) revealed[0].GetString();
        rapidjson::Value& cards = revealed[1];
        const char *card1 = (char*) cards[0].GetString();
        const char *card2 = (char*) cards[1].GetString();
        Player *player_widget = findChild<Player*>(QString(name));
        player_widget->set_cards(card1, card2);
    }
}

void Table::Holdem::winners(rapidjson::Value& content) {
    for (rapidjson::SizeType i = 0; i < content.Size(); i++) {
        rapidjson::Value& winner = content[i];
        const char *name = (const char*) winner["name"].GetString();
        int gain = (int) winner["gain"].GetInt();
        Player* player_widget = findChild<Player*>(QString(name));
        player_widget->set_gain(gain);
    }
}

void Table::Holdem::reset(rapidjson::Value& content) {
    (void) content;
    comm_cards->clear();
    action_bar->hide();
    for (int i = 0; i < max_players; i++) {
        players[i].set_cards(NULL, NULL);
        if (players[i].state == Player::ACTIVE) {
            players[i].change_state(Player::COMMON);
        }
    }
}

void Table::Holdem::notify(rapidjson::Value& content) {
    QString s = (char*) content.GetString();
    if (s == "turn") {
        
    }
}

void Table::Holdem::set_players(rapidjson::Value& r_players) {
    for (rapidjson::SizeType i = 0; i < r_players.Size(); i++) {
        rapidjson::Value& r_player = r_players[i];
        const char *name = (const char*) r_player["name"].GetString();
        int bet = (int) r_player["bet"].GetInt();
        int stack = (int) r_player["stack"].GetInt();
        Player *player_widget = findChild<Player*>(QString(name));
        if (player_widget != NULL) {
            player_widget->set_stack(bet, stack);
        } else {
            new_player(name, bet, stack);
        }
    }
}

void Table::Holdem::set_priv_cards(rapidjson::Value& r_cards) {
    const char *card1 = (const char*) r_cards[0].GetString();
    const char *card2 = (const char*) r_cards[1].GetString();
    Player *self = findChild<Player*>(QString(ker->name));
    self->set_cards(card1, card2);
}

void Table::Holdem::set_active(rapidjson::Value& r_active) {
    const char *act_name = (const char*) r_active["name"].GetString();
    Player *act_player = findChild<Player*>(QString(act_name));
    if (act_player == NULL) {
        return;
    }
    act_player->change_state(Player::ACTIVE);
    if (QString(act_name) == QString(ker->name)) {
        action_bar->show();
    }
}

void Table::Holdem::set_action(rapidjson::Value& r_action) {
    const char *name = (const char*) r_action[1].GetString();
    Player *act_player = findChild<Player*>(QString(name));
    int bet = (int) r_action[3].GetInt();
    int stack = (int) r_action[4].GetInt();
    act_player->set_stack(bet, stack);
    act_player->change_state(Player::COMMON);
}

void Table::Holdem::set_comm_cards(rapidjson::Value& r_comm_cards) {
    for (rapidjson::SizeType i = 0; i < r_comm_cards.Size(); i++) {
        const char *card = (const char*) r_comm_cards[i].GetString();
        comm_cards->append(card);
    }
}
