#ifndef HOLDEM_WIDGETS_H
#define HOLDEM_WIDGETS_H

#include "../../../util_widgets/svg_button.h"
#include <QSvgWidget>
#include <QString>
#include <QLabel>
#include <QLineEdit>

namespace HoldemWidgets {

class Card : public QSvgWidget {
public:
    Card(QSvgWidget *parent = 0) : QSvgWidget(parent) {}
    ~Card() {}
    void set(const char *card) {load(QString("img/cards/")+QString(card)+QString(".svg"));}
};

class Player : public QSvgWidget {
public:
    enum {
        HOLDEM_PLAYER_STATUS_FOLD = 1,
        HOLDEM_PLAYER_STATUS_COMMON = 2,
        HOLDEM_PLAYER_STATUS_ACTIVE = 4,
    };
    enum {OPEN = 0, COMMON = 1, ACTIVE = 2};
    int state;
private:
    QSvgWidget *pl;
    QLabel *name_l, *stack_l;
    Card *c1, *c2;
public:
    Player(QSvgWidget *parent = 0);
    ~Player() {
        delete name_l;
        delete stack_l;
        delete pl;
        delete c1;
        delete c2;
    }
    QString get_name() const {return name_l->text();}
    int get_state() const {return state;}
    void set_name(const char *name);
    void set_stack(float bet, float stack);
    void set_cards(const char *card1, const char *card2);
    void set_gain(float gain);
    void change_state(int new_state);
};

class CommCards : public QSvgWidget {
private:
    enum {NCARDS = 5};
    Card *cards;
    int cards_count;
public:
    CommCards(QSvgWidget *parent = 0);
    ~CommCards() {delete[] cards;}
    void append(const char *card);
    void clear();
};

class ActionBar : public QSvgWidget {
    Q_OBJECT 
public:
    SvgButton *fold_button, *call_button, *raise_button;
    ActionBar(QSvgWidget *parent = 0);
    ~ActionBar() {delete fold_button; delete call_button; delete raise_button;}
};

class SetStack : public QSvgWidget {
    Q_OBJECT
public:
    QLineEdit *stack_l;
public:
    SetStack(QSvgWidget *parent = 0);
    ~SetStack() {delete stack_l;}
    QString get();
};

};

#endif
