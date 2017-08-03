#include "stud.h"
#include "../../account/account.h"
#include <cmath>
#include <iostream>
#include <sstream>

#ifndef SCALE
#define SCALE
static float kx = 1.2, ky = 1.2;
#endif

using namespace StudWidgets;

Table::Stud::Stud(kernel *_ker, int _max_players, QSvgWidget *parent) : Frame(parent),
                                                                        ker(_ker),
                                                                        players(new Player[_max_players]),
                                                                        action_bar(new ActionBar(this)),
                                                                        set_stack(new SetStack(this)),
                                                                        leave(new SvgButton(this)),
                                                                        max_players(_max_players) {
    const int a = kx*(362 / 2);
    const int b = ky*(205 / 2);
    const int x0 = kx*116 + a;
    const int y0 = ky*158 + b - ky*30;
    const float two_pi = 6.28;
    float phi = 0.1;
    
    resize(kx*600, ky*450);
    load(QString("img/background.svg"));
    leave->setGeometry(kx*30, ky*80, kx*110, ky*30);
    leave->set_img("img/leave_b.svg", "img/leave_h.svg");
    action_bar->hide();

    for (int i = 0; i < max_players; i++) {
        players[i].setParent(this);
        phi += two_pi / max_players;
        int x = a*cos(phi) + x0 - players[i].width()/2;
        int y = b*sin(phi) + y0 - players[i].height()/2;
        players[i].move(x, y);
    }

    connect(set_stack->stack_l, SIGNAL(returnPressed()), this, SLOT(stack_request()));
    connect(action_bar->bring_in_button, SIGNAL(clicked()), this, SLOT(bring_in_request()));
    connect(action_bar->fold_button, SIGNAL(clicked()), this, SLOT(fold_request()));
    connect(action_bar->check_button, SIGNAL(clicked()), this, SLOT(call_request()));
    connect(action_bar->call_button, SIGNAL(clicked()), this, SLOT(call_request()));
    connect(action_bar->sb_button, SIGNAL(clicked()), this, SLOT(sb_request()));
    connect(action_bar->bb_button, SIGNAL(clicked()), this, SLOT(bb_request()));
    connect(action_bar->raise_button, SIGNAL(clicked()), this, SLOT(raise_request()));
    connect(leave, SIGNAL(clicked()), this, SLOT(leave_table()));
/*
    set_stack->close();
    for (int i = 0; i < max_players; ++i) {
        players[i].set_name("Player");
        players[i].set_stack(0.0, 100.0);
        players[i].set_card("facedown", 0);
        players[i].set_card("facedown", 1);
        players[i].set_card("c2", 2);

        new Shadow(players[i].label);
    }*/
}

void Table::Stud::stack_request() {
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

void Table::Stud::bring_in_request() {
    static const QString request = "{\"action\":\"bring_in\"}";
    ker->send(request);
    action_bar->hide();
}

void Table::Stud::fold_request() {
    static const QString request = "{\"action\":\"fold\"}";
    ker->send(request);
    action_bar->hide();
}

void Table::Stud::call_request() {
    static const QString request = "{\"action\":\"call\"}";
    ker->send(request);
    action_bar->hide();
}

void Table::Stud::sb_request() {
    static const QString request = "{\"action\":\"sb\"}";
    ker->send(request);
    action_bar->hide();
}

void Table::Stud::bb_request() {
    static const QString request = "{\"action\":\"bb\"}";
    ker->send(request);
    action_bar->hide();
}

void Table::Stud::raise_request() {
    float bet = action_bar->slider->value();
    bet /= 100;
    std::ostringstream oss; oss.precision(2);
    oss << std::fixed << "{\"action\":" << bet << "}";
    ker->send(QString(oss.str().c_str()));
    action_bar->hide();
}

void Table::Stud::leave_table() {
    ker->change_frame<Account>();
}

void Table::Stud::handler(const char *response) {
    rapidjson::Document rsp;
    rsp.Parse(response);

    if (!rsp.IsObject()) {
        std::cout << "parser error" << std::endl;
        return;
    }

    if (rsp.HasMember("new_player")) {
        new_player(rsp["new_player"]);
    }
    else if (rsp.HasMember("betting")) {
        betting(rsp["betting"]);
    }
    else if (rsp.HasMember("end")) {
        end(rsp["end"]);
    }
    else if (rsp.HasMember("showdown")) {
        showdown(rsp["showdown"]);
    }
}

void Table::Stud::new_player(rapidjson::Value& content) {
    for (int i = 0; i < max_players; ++i) {
        if (players[i].get_state() == Player::OPEN) {
            const char *name = content["name"].GetString();
            float bet = content.HasMember("bet") ? content["bet"].GetDouble() : 0.0;
            float stack = content["stack"].GetDouble();
            players[i].set_name(name);
            players[i].set_stack(bet, stack);
            return;
        }
    }
}

void Table::Stud::betting(rapidjson::Value& content) {
    rapidjson::Value& players_r = content["players"];

    for (int i = 0; i < max_players; ++i) {
        players[i].clear();
    }

    for (rapidjson::SizeType i = 0; i < players_r.Size(); ++i) {
        rapidjson::Value& player_data = players_r[i];
        const char *name = player_data["name"].GetString();
        Player *player = findChild<Player*>(QString(name));
        
        if (player == NULL) {
            new_player(player_data);
            player = findChild<Player*>(QString(name));
        } else {
            float bet = player_data["bet"].GetDouble();
            float stack = player_data["stack"].GetDouble();
            player->set_stack(bet, stack);
        }

        if (player_data["state"].GetInt() == Player::PLAYER_STATUS_ACTIVE) {
            player->change_state(Player::ACTIVE);
            if (player->get_name() == ker->name) {
                if (player_data.HasMember("betting_options")) {
                    action_bar->show2(player_data["betting_options"].GetInt());
                } else {
                    action_bar->show();
                }
            }
        } else {
            player->change_state(Player::COMMON);
        }

        if (!player_data.HasMember("cards")) {
            continue;
        }
        rapidjson::Value& cards = player_data["cards"];
        for (rapidjson::SizeType i = 0; i < cards.Size(); ++i) {
            player->set_card(cards[i].GetString(), i);
        }
    }

    if (!content.HasMember("private_cards")) {
        return;
    }
    Player *self = findChild<Player*>(QString(ker->name));
    rapidjson::Value& cards = content["private_cards"];
    for (rapidjson::SizeType i = 0; i < cards.Size(); ++i) {
        self->set_downcard(cards[i].GetString(), i);
    }
}

void Table::Stud::end(rapidjson::Value& content) {
    for (rapidjson::SizeType i = 0; i < content.Size(); ++i) {
        rapidjson::Value& item = content[i];
        const char *name = item["name"].GetString();
        float gain = item["gain"].GetDouble();
        Player *player = findChild<Player*>(QString(name));
        if (player == NULL)
            continue;
        player->set_gain(gain);
    }
}

void Table::Stud::showdown(rapidjson::Value& content) {
    for (rapidjson::SizeType i = 0; i < content.Size(); ++i) {
        rapidjson::Value& item = content[i];
        const char *name = item["name"].GetString();
        rapidjson::Value& downcards = item["downcards"];
        Player *player = findChild<Player*>(QString(name));
        if (player == NULL) {
            continue;
        }
        if (player->get_name() == ker->name) {
            continue;
        }
        for (rapidjson::SizeType j = 0; j < downcards.Size(); ++j) {
            player->set_downcard(downcards[j].GetString(), (int) j);
        }
    }
}