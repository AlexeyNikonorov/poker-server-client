#include "holdem.h"
#include "../../account/account.h"
#include <iostream>
#include <sstream>
#include <cmath>

enum {
    BETTING_OPTION_FOLD = 1,
    BETTING_OPTION_CALL = 2,
    BETTING_OPTION_CHECK = 4,
    BETTING_OPTION_RAISE = 8,
    BETTING_OPTION_RERAISE = 16,
    BETTING_OPTION_ALLIN = 32,
};

#ifndef SCALE
#define SCALE
static float kx = 1.2, ky = 1.2;
#endif

using namespace HoldemWidgets;

Table::Holdem::Holdem(kernel *_ker, int _max_players, QSvgWidget *parent) : Frame(parent),
                                                                            ker(_ker),
                                                                            players(new Player[_max_players]),
                                                                            action_bar(new ActionBar(this)),
                                                                            comm_cards(new CommCards(this)),
                                                                            set_stack(new SetStack(this)),
                                                                            leave(new SvgButton(this)),
                                                                            max_players(_max_players) {
    const int a = kx*(362 / 2);
    const int b = ky*(205 / 2);
    const int x0 = kx*116 + a - kx*30; 
    const int y0 = ky*158 + b - ky*20;
    float phi = 0.1;
    
    resize(kx*600, ky*450);
    load(QString("img/background.svg"));
    leave->setGeometry(kx*30, ky*80, kx*110, ky*30);
    leave->set_img("img/leave_b.svg", "img/leave_h.svg");
    
    for (int i = 0; i < max_players; i++) {
        players[i].setParent(this);
        phi += 6.28 / max_players;
        int x = a*cos(phi) + x0 - players[i].width()/2;
        int y = b*sin(phi) + y0 - players[i].height()/2;
        players[i].move(x, y);
    }
    
    action_bar->raise();
    action_bar->hide();
    
    connect(set_stack->stack_l, SIGNAL(returnPressed()), this, SLOT(stack_request()));
    connect(action_bar->fold_button, SIGNAL(clicked()), this, SLOT(fold_request()));
    connect(action_bar->call_button, SIGNAL(clicked()), this, SLOT(call_request()));
    connect(action_bar->raise_button, SIGNAL(clicked()), this, SLOT(raise_request()));
    connect(leave, SIGNAL(clicked()), this, SLOT(leave_table()));
}

void Table::Holdem::init_request() {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);
    w.StartObject();
    w.Key("join");
    w.StartObject();
    w.Key("table_id");
    w.Int(0);
    w.Key("id");
    w.Int(ker->id);
    w.Key("name");
    w.String(ker->name.toUtf8());
    w.EndObject();
    w.EndObject();
    ker->send(sb.GetString());
}

void Table::Holdem::stack_request() {
    std::istringstream iss(set_stack->get().toStdString());
    float stack;
    if ((iss >> stack).fail())
        return;
    std::ostringstream oss; oss.precision(2);
    oss << std::fixed << "{\"new_player\":{\"id\":" << ker->id <<
                                ",\"name\":\"" << ker->name.toStdString() <<
                                "\",\"stack\":" << stack << "}}";
    ker->send(QString(oss.str().c_str()));
    set_stack->close();
}

void Table::Holdem::fold_request() {
    ker->send("{\"action\":{\"kind\":1,\"value\":0.0}}");
    action_bar->hide();
}

void Table::Holdem::call_request() {
    ker->send("{\"action\":{\"kind\":2,\"value\":0.0}}");
    action_bar->hide();
}

void Table::Holdem::raise_request() {
    ker->send("{\"action\":6}");
    action_bar->hide();
}

void Table::Holdem::leave_table() {
    ker->change_frame<Account>();
}
/*
void Table::Holdem::handler(const char *response) {
    rapidjson::Document rsp;
    rsp.Parse(response);
    if (!rsp.IsObject()) {
        std::cout << "parser error" << std::endl;
        return;
    }
    if (rsp.HasMember("betting")) {
        betting(rsp["betting"]);
    }
    else if (rsp.HasMember("init")) {
        init(rsp["init"]);
    }
    else if (rsp.HasMember("reveal")) {
        reveal(rsp["reveal"]);
    }
    else if (rsp.HasMember("winners")) {
        winners(rsp["winners"]);
    }
    else if (rsp.HasMember("reset")) {
        reset(rsp["reset"]);
    }
    else if (rsp.HasMember("notify")) {
        notify(rsp["notify"]);
    }
    else if (rsp.HasMember("new_player")) {
        new_player(rsp["new_player"]);
    }
    else if (rsp.HasMember("updates")) {
        updates(rsp["updates"]);
    }
    else if (rsp.HasMember("reconnect")) {
        reconnect(rsp["reconnect"]);
    }
}
*/

void Table::Holdem::handler(const char *response) {
    rapidjson::Document doc;
    doc.Parse(response);
    if (!doc.IsObject()) {
        std::cout << "parser error\n";
        return;
    }

    if (doc.HasMember("new_player")) {
        _new_player(doc["new_player"]);
    }
    if (doc.HasMember("players")) {
        _players(doc["players"]);
    }
    if (doc.HasMember("priv_cards")) {
        _priv_cards(doc["priv_cards"]);
    }
    if (doc.HasMember("comm_cards")) {
        _comm_cards(doc["comm_cards"]);
    }
    if (doc.HasMember("showdown")) {
        _showdown(doc["showdown"]);
    }
    if (doc.HasMember("winners")) {
        _winners(doc["winners"]);
    }
}

void Table::Holdem::_new_player(rapidjson::Value& content) {
    for (int i = 0; i < max_players; ++i) {
        if (players[i].get_state() != Player::OPEN) {
            continue;
        }
        const char *name = content["name"].GetString();
        float bet = content.HasMember("bet") ? content["bet"].GetDouble() : 0.0;
        float stack = content["stack"].GetDouble();
        players[i].set_name(name);
        players[i].set_stack(bet, stack);
        return;
    }
}

void Table::Holdem::_players(rapidjson::Value& content) {
    for (rapidjson::SizeType i = 0; i < content.Size(); ++i) {
        rapidjson::Value& player_data = content[i];
        const char *name = player_data["name"].GetString();
        float bet = player_data["bet"].GetDouble();
        float stack = player_data["stack"].GetDouble();
        int status = player_data["status"].GetInt();
        Player *player_widget = findChild<Player*>(QString(name));
        if (player_widget == NULL) {
            for (int j = 0; j < max_players; ++j) {
                if (players[j].get_state() != Player::OPEN)
                    continue;
                players[j].set_name(name);
                players[j].set_stack(bet, stack);
                break;
            }
            player_widget = findChild<Player*>(QString(name));
        } else {
            player_widget->set_stack(bet, stack);
        }
        if (status == Player::HOLDEM_PLAYER_STATUS_ACTIVE) {
            player_widget->change_state(Player::ACTIVE);
            if (player_widget->get_name() == ker->name) {
                action_bar->show();
            }
        } else {
            player_widget->change_state(Player::COMMON);
        }
    }
}

void Table::Holdem::_priv_cards(rapidjson::Value& content) {
    Player *self = findChild<Player*>(ker->name);
    if (self == NULL) {
        return;
    }
    const char *card1 = content[0].GetString();
    const char *card2 = content[1].GetString();
    self->set_cards(card1, card2);
}

void Table::Holdem::_comm_cards(rapidjson::Value& content) {
    comm_cards->clear();
    for (rapidjson::SizeType i = 0; i < content.Size(); ++i) {
        const char *card = content[i].GetString();
        comm_cards->append(card);
    }
}

void Table::Holdem::_showdown(rapidjson::Value& content) {
    for (rapidjson::SizeType i = 0; i < content.Size(); ++i) {
        rapidjson::Value& item = content[i];
        const char *name = item["name"].GetString();
        const char *card1 = item["priv_cards"][0].GetString();
        const char *card2 = item["priv_cards"][1].GetString();
        if (QString(name) == ker->name) {
            continue;
        }
        Player *player_widget = findChild<Player*>(QString(name));
        if (player_widget == NULL) {
            continue;
        }
        player_widget->set_cards(card1, card2);
    }
}

void Table::Holdem::_winners(rapidjson::Value& content) {
    for (rapidjson::SizeType i = 0; i < content.Size(); ++i) {
        rapidjson::Value& item = content[i];
        const char *name = item["name"].GetString();
        float gain = item["gain"].GetDouble();
        Player *player_widget = findChild<Player*>(QString(name));
        if (player_widget == NULL) {
            continue;
        }
        player_widget->set_gain(gain);
    }
}