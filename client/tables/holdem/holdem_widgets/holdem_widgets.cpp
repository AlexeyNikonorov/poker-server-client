#include "holdem_widgets.h"
#include <sstream>

#ifndef SCALE
#define SCALE
static float kx = 1.2, ky = 1.2;
#endif

using namespace HoldemWidgets;

SetStack::SetStack(QSvgWidget *parent) : QSvgWidget(parent),
                                         stack_l(new QLineEdit(this)) {
    load(QString("img/set_stack.svg"));
    setGeometry(kx*200, ky*200, kx*206, ky*80);
    
    stack_l->setFrame(false);
    stack_l->setGeometry(kx*20, ky*35, kx*130, ky*25);
    stack_l->setStyleSheet(QString("background:rgba(0,0,0,0);color:rgb(255,255,255);"));
}

QString SetStack::get() {
    return stack_l->text();
}

Player::Player(QSvgWidget *parent) : QSvgWidget(parent),
                                     state(OPEN),
                                     pl(new QSvgWidget(this)),
                                     name_l(new QLabel(pl)),
                                     stack_l(new QLabel(pl)),
                                     c1(new Card(this)),
                                     c2(new Card(this)) {
    resize(kx*145, ky*66);
    setObjectName(QString("empty"));
    
    pl->setGeometry(kx*53, ky*14, kx*77, ky*40);
    pl->load(QString("img/player_w_open.svg"));
    
    name_l->setGeometry(kx*22, ky*5, kx*48, ky*12);
    stack_l->setGeometry(kx*22, ky*23, kx*48, ky*12);
    
    c1->setGeometry(kx*5, ky*6, kx*38, ky*52);
    c2->setGeometry(kx*34, ky*6, kx*38, ky*52);
}

void Player::set_name(const char *name) {
    change_state(COMMON);
    name_l->setText(QString(name));
    setObjectName(QString(name));
}

void Player::set_stack(float bet, float stack) {
    std::ostringstream oss; oss.precision(2);
    oss << std::fixed << bet << "/" << stack;
    stack_l->setText(QString(oss.str().c_str()));
}

void Player::set_cards(const char *card1, const char *card2) {
    if (card1 != NULL) {
        c1->set(card1);
        c2->set(card2);
        c1->show();
        c2->show();
    } else {
        c1->close();
        c2->close();
    }
}

void Player::set_gain(float gain) {
    QString text = stack_l->text();
    std::ostringstream oss; oss.precision(2);
    oss << std::fixed << gain;
    QString gain_s = oss.str().c_str();
    stack_l->setText(text + "+" + gain_s);
}

void Player::change_state(int new_state) {
    if (state == new_state)
        return;
    switch (state = new_state) {
        case OPEN:
            pl->load(QString("img/player_w_open.svg"));
            return;
        case COMMON:
            pl->load(QString("img/player_w.svg"));
            return;
        case ACTIVE:
            pl->load(QString("img/player_b.svg"));
            return;
    }
}

CommCards::CommCards(QSvgWidget *parent) : QSvgWidget(parent),
                                           cards(new Card[NCARDS]),
                                           cards_count(0) {
    const int card_w = 37;
    const int card_h = 51;
    setGeometry(kx*190, ky*200, kx*(10 + NCARDS*card_w), ky*(10 + card_h));
    for (int i = 0; i < NCARDS; i++) {
        cards[i].setParent(this);
        cards[i].setGeometry(kx*i*card_w, ky*5, kx*card_w, ky*card_h);
    }
}

void CommCards::append(const char *card) {
    cards[cards_count].set(card);
    cards[cards_count].show();
    cards_count++;
}

void CommCards::clear() {
    cards_count = 0;
    for (int i = 0; i < NCARDS; i++) {
        cards[i].close();
    }
}

ActionBar::ActionBar(QSvgWidget *parent) : QSvgWidget(parent),
                                           fold_button(new SvgButton(this)),
                                           call_button(new SvgButton(this)),
                                           raise_button(new SvgButton(this)) {
    setGeometry(kx*376, ky*300, kx*210, ky*134);
    load(QString("img/action_bar.svg"));
    
    fold_button->set_img("img/fold_b.svg", "img/fold_h.svg");
    call_button->set_img("img/call_b.svg", "img/call_h.svg");
    raise_button->set_img("img/raise_b.svg", "img/raise_h.svg");
    
    fold_button->setGeometry(kx*20, ky*75, kx*56, ky*38);
    call_button->setGeometry(kx*76, ky*75, kx*56, ky*38);
    raise_button->setGeometry(kx*132, ky*75, kx*56, ky*38);
}
