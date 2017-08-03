#ifndef HOLDEM_H
#define HOLDEM_H

#include "holdem_widgets/holdem_widgets.h"
#include "../kernel/kernel.h"
#include "../util_widgets/svg_button.h"
#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include <QObject>
#include <QSvgWidget>
#include <QString>

namespace Table {

class Holdem : public Frame {
    Q_OBJECT
private:
    kernel *ker;
    HoldemWidgets::Player *players;
    HoldemWidgets::ActionBar *action_bar;
    HoldemWidgets::CommCards *comm_cards;
    HoldemWidgets::SetStack *set_stack;
    SvgButton *leave;
    int max_players;
public:
    Holdem(kernel *_ker, int _max_players = 4, QSvgWidget *parent = 0);
    ~Holdem() {
        delete[] players;
        delete action_bar;
        delete comm_cards;
        delete set_stack;
        delete leave;
    }
    virtual void handler(const char *response);
    virtual void init_request();
private:
    void set_players(rapidjson::Value& content);
    void set_active(rapidjson::Value& content);
    void set_priv_cards(rapidjson::Value& content);
    void set_action(rapidjson::Value& content);
    void set_comm_cards(rapidjson::Value& content);
    void new_player(const char *name, int bet, int stack);
private:
    void new_player(rapidjson::Value& content);
    void updates(rapidjson::Value& content);
    void init(rapidjson::Value& content);
    void betting(rapidjson::Value& content);
    void reveal(rapidjson::Value& content);
    void winners(rapidjson::Value& content);
    void reset(rapidjson::Value& content);
    void notify(rapidjson::Value& content);
    void reconnect(rapidjson::Value& content);

    void _new_player(rapidjson::Value& content);
    void _players(rapidjson::Value& content);
    void _priv_cards(rapidjson::Value& content);
    void _comm_cards(rapidjson::Value& content);
    void _showdown(rapidjson::Value& content);
    void _winners(rapidjson::Value& content);
private slots:
    void stack_request();
    void fold_request();
    void call_request();
    void raise_request();
    void leave_table();
};

};

#endif
