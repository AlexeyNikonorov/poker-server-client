#ifndef STUD_WIDGETS_H
#define STUD_WIDGETS_H

#include "../../util_widgets/svg_button.h"
#include <QSvgWidget>
#include <QLabel>
#include <QString>
#include <QFont>
#include <QLineEdit>
#include <QSlider>
#include <QGraphicsBlurEffect>
#include <QMoveEvent>
#include <iostream>
#include <sstream>

#ifndef SCALE
#define SCALE
static float kx = 1.2, ky = 1.2;
#endif

namespace StudWidgets {

class Shadow : public QSvgWidget {
private:
	QGraphicsBlurEffect *_blur;
	QSvgWidget *_shadow;
public:
	Shadow(QSvgWidget *target, QSvgWidget *parent = 0) : QSvgWidget(parent),
														 _blur(new QGraphicsBlurEffect()),
														 _shadow(new QSvgWidget(this)) {
		const int r = 2, d = 2*r;

		setParent(target->parentWidget());
		target->raise();

		resize(target->width() + kx*d, target->height() + ky*d);
		move(target->x() - kx*r, target->y() - ky*r);

		_blur->setBlurRadius(kx*r);
		_shadow->move(kx*r, ky*r);
		_shadow->resize(target->size());
		_shadow->load(QString("img/shadows/smooth_filled.svg"));
		_shadow->setGraphicsEffect(_blur);
	}
	~Shadow() {delete _blur; delete _shadow;}
};

class Card : public QSvgWidget {
private:
	QSvgWidget *_shadow;
	QSvgWidget *_card;
	QGraphicsBlurEffect *_blur;
public:
    Card(QSvgWidget *parent = 0) : QSvgWidget(parent),
    							   _shadow(new QSvgWidget(this)),
    							   _card(new QSvgWidget(this)),
    							   _blur(new QGraphicsBlurEffect()) {
    	resize(kx*40, ky*51);
    	_blur->setBlurRadius(kx*2);
    	_shadow->setGraphicsEffect(_blur);
    	_shadow->setGeometry(kx*3, ky*3, kx*34, ky*45);
    	_card->setGeometry(kx*3, ky*3, kx*34, ky*45);
    }
    ~Card() {delete _blur; delete _shadow; delete _card;}
    void set(const char *card) {
    	_shadow->load(QString("img/shadows/smooth_filled.svg"));
    	_card->load(QString("img/cards/")+QString(card)+QString(".svg"));
    }
};

class Player : public QSvgWidget {
public://private:
    enum {UPCARDS = 4, DOWNCARDS = 3, TOTALCARDS = 7};
	Card *cards;
	QSvgWidget *label;
    QLabel *name_l;
    QLabel *stack_l;
    int state;
    float bet;
    float stack;
public:
    enum {OPEN = 0, COMMON = 1, ACTIVE = 2};
    enum { PLAYER_STATUS_OFFLINE = 0,
           PLAYER_STATUS_NEW = 1,
           PLAYER_STATUS_FOLD = 2,
           PLAYER_STATUS_COMMON = 4,
           PLAYER_STATUS_ACTIVE = 8 };
	Player(QSvgWidget *parent = 0) : QSvgWidget(parent),
                                     cards(new Card[TOTALCARDS]),
								     label(new QSvgWidget(this)),
                                     name_l(new QLabel(label)),
                                     stack_l(new QLabel(label)),
                                     state(OPEN),
                                     bet(0),
                                     stack(0) {
        resize(kx*134, ky*77);
        for (int i = 0; i < TOTALCARDS; i++) {
            cards[i].setParent(this);
            cards[i].raise();
            if (i == 0 || i == 1 || i == 6) {
                cards[i].move(kx*(2+i*16), ky*4);
            } else {
                cards[i].move(kx*(2+i*16), ky*2);
            }
        }

        label->setGeometry(kx*31, ky*35, kx*72, ky*38);
        label->load(QString("img/player_w_open.svg"));
        label->raise();
        new Shadow(label);

        name_l->setAlignment(Qt::AlignCenter);
        stack_l->setAlignment(Qt::AlignCenter);
        name_l->setGeometry(kx*0, ky*5, kx*72, ky*12);
        stack_l->setGeometry(kx*0, ky*23, kx*72, ky*12);

        QFont font("Sans", 8);
        name_l->setFont(font);
        stack_l->setFont(font);
        
        QString styleSheet = "color:rgb(0,0,0);";
        name_l->setStyleSheet(styleSheet);
        stack_l->setStyleSheet(styleSheet);
    }
    ~Player() {
        delete stack_l;
        delete name_l;
        delete label;
        delete[] cards;
    }
    int get_state() const {return state;}
    QString get_name() const {return objectName();}
    float get_bet() const {return bet;}
    float get_stack() const {return stack;}
    void clear() {
        setObjectName(QString("open"));
        name_l->clear();
        stack_l->clear();
        bet = 0;
        stack = 0;
        for (int i = 0; i < TOTALCARDS; ++i)
            cards[i].hide();
        state = OPEN;
        label->load(QString("img/player_w_open.svg"));
    }
    void set_name(const char *name) {
        change_state(COMMON);
        setObjectName(QString(name));
        name_l->setText(QString(name));
    }
    void set_stack(float _bet, float _stack) {
        bet = _bet;
        stack = _stack;
        std::ostringstream oss; oss.precision(2);
        oss << std::fixed << _bet << "/" << _stack;
        stack_l->setText(QString(oss.str().c_str()));
    }
    void set_card(const char *card, int n) {
        if (n >= TOTALCARDS)
            return;
        cards[n].set(card);
        cards[n].show();
    }
    void set_downcard(const char *card, int n) {
        switch (n) {
            case 0: set_card(card, 0);
                    return;
            case 1: set_card(card, 1);
                    return;
            case 2: set_card(card, 6);
                    return;
        }
    }
    void set_gain(float gain) {
        QString text = stack_l->text();
        std::ostringstream oss; oss.precision(2);
        oss << std::fixed << gain;
        QString gain_s = oss.str().c_str();
        stack_l->setText(text + "+" + gain_s);
    }
    void change_state(int new_state) {
        if (state == new_state)
            return;
        switch (state = new_state) {
            case OPEN:
                label->load(QString("img/player_w_open.svg"));
                return;
            case COMMON:
                label->load(QString("img/player_w.svg"));
                return;
            case ACTIVE:
                label->load(QString("img/player_b.svg"));
                return;
        }
    }
};

class ActionBar : public QSvgWidget {
    Q_OBJECT
public:
    enum { BETTING_OPTION_BRING_IN = 1,
           BETTING_OPTION_FOLD = 2,
           BETTING_OPTION_CHECK = 64,
           BETTING_OPTION_CALL = 4,
           BETTING_OPTION_SB = 8,
           BETTING_OPTION_BB = 16,
           BETTING_OPTION_RAISE = 32, };
    SvgButton *fold_button, *check_button, *call_button, *raise_button,
              *bring_in_button, *sb_button, *bb_button;
    QLabel *bet_value;
    QSlider *slider;
    ActionBar(QSvgWidget *parent = 0) : QSvgWidget(parent),
                                        fold_button(new SvgButton(this)),
                                        check_button(new SvgButton(this)),
                                        call_button(new SvgButton(this)),
                                        raise_button(new SvgButton(this)),
                                        bring_in_button(new SvgButton(this)),
                                        sb_button(new SvgButton(this)),
                                        bb_button(new SvgButton(this)),
                                        bet_value(new QLabel(this)),
                                        slider(new QSlider(Qt::Horizontal, this)) {
        setGeometry(kx*376, ky*300, kx*210, ky*134);
        load(QString("img/action_bar.svg"));
        
        fold_button->set_img("img/fold_b.svg", "img/fold_h.svg");
        bring_in_button->set_img("img/bringin_b.svg", "img/bringin_h.svg");

        call_button->set_img("img/call_b.svg", "img/call_h.svg");
        check_button->set_img("img/check_b.svg", "img/check_h.svg");
        sb_button->set_img("img/sb_b.svg", "img/sb_h.svg");
        bb_button->set_img("img/bb_b.svg", "img/bb_h.svg");
        raise_button->set_img("img/raise_b.svg", "img/raise_h.svg");

        {
            const int x = kx*20,
                      y = ky*85,
                      w = kx*56,
                      h = ky*38;
            fold_button->setGeometry(x, y, w, h);
            call_button->setGeometry(x+w, y, w, h);
            check_button->setGeometry(x+w, y, w, h);
            raise_button->setGeometry(x+w*2, y, w, h);

            const int x2 = x,
                      y2 = y - ky*30,
                      w2 = w,
                      h2 = h - ky*13;
            bring_in_button->setGeometry(x2, y2, w2, h2);
            sb_button->setGeometry(x2+w2, y2, w2, h2);
            bb_button->setGeometry(x2+w2*2, y2, w2, h2);
        }

        QFont font;
        font.setPointSize(12);
        bet_value->setFont(font);
        bet_value->setStyleSheet(QString("color:rgb(255,255,255);"));
        bet_value->setGeometry(kx*30,ky*5,kx*150,ky*25);

        slider->setGeometry(kx*30,ky*20,kx*150,ky*30);
        slider->setMaximum(100);
        slider->setMinimum(0);

        connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(on_slider_moved(int)));
    }
    ~ActionBar() {
        delete fold_button;
        delete call_button;
        delete check_button;
        delete raise_button;
        delete bring_in_button;
        delete sb_button;
        delete bb_button;
        delete bet_value;
        delete slider;
    }
    virtual void hide() {
        bet_value->setText(QString());
        slider->setSliderPosition(slider->minimum());
        static_cast<QSvgWidget*>(this)->hide();
    }
    void show2(int options) {
        static_cast<QSvgWidget*>(this)->show();
        slider->hide();
        fold_button->hide();
        check_button->hide();
        bring_in_button->hide();
        call_button->hide();
        sb_button->hide();
        bb_button->hide();
        raise_button->hide();

        if (options & BETTING_OPTION_FOLD) {
            fold_button->show();
        }
        if (options & BETTING_OPTION_CHECK) {
            check_button->show();
        }
        else if (options & BETTING_OPTION_CALL) {
            call_button->show();
        }
        if (options & BETTING_OPTION_RAISE) {
            raise_button->show();
            slider->show();
        }
        if (options & BETTING_OPTION_BRING_IN) {
            bring_in_button->show();
        }
        if (options & BETTING_OPTION_SB) {
            sb_button->show();
        }
        if (options & BETTING_OPTION_BB) {
            bb_button->show();
        }
    }
private slots:
    void on_slider_moved(int value) {
        if (value == slider->minimum()) {
            bet_value->setText(QString());
            return;
        }

        QString str = QString("<center>Raise: ")+QString::number(value)+QString("</center>");
        bet_value->setText(str);
    }
};

class SetStack : public QSvgWidget {
    Q_OBJECT
public:
    QLineEdit *stack_l;
    SetStack(QSvgWidget *parent = 0) : QSvgWidget(parent),
                                       stack_l(new QLineEdit(this)) {
        load(QString("img/set_stack.svg"));
        setGeometry(kx*200, ky*200, kx*206, ky*80);
        
        stack_l->setFrame(false);
        stack_l->setGeometry(kx*20, ky*35, kx*130, ky*25);
        stack_l->setStyleSheet(QString("background:rgba(0,0,0,0);color:rgb(255,255,255);"));
    }
    ~SetStack() {delete stack_l;}
    QString get() const {return stack_l->text();}
};

};

#endif
