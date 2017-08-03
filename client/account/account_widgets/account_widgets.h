#ifndef ACCOUNT_WIDGETS_H
#define ACCOUNT_WIDGETS_H

#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include "../../util_widgets/svg_button.h"
#include <QObject>
#include <QSvgWidget>
#include <QString>
#include <QLabel>
#include <QFont>
#include <QFile>
#include <QTextStream>
#include <sstream>

#ifndef SCALE
#define SCALE
static float kx = 1.2, ky = 1.2;
#endif

class Row;
class TableList;

class Row : public QSvgWidget {
    Q_OBJECT
    friend TableList;
private:
    enum {GAME_TYPE_HOLDEM = 1, GAME_TYPE_STUD = 2};
    int game_type;
    int table_id;
    int curr_players;
    int max_players;
    SvgButton *join;
public:
    Row(QSvgWidget *parent = 0) : QSvgWidget(parent),
                                  game_type(0),
                                  table_id(0),
                                  curr_players(0),
                                  max_players(0),
                                  join(new SvgButton(this)) {
        resize(kx*508, ky*18);
        join->setGeometry(kx*274, ky*1, kx*31, ky*16);
        join->load(QString("img/join2_b.svg"));
        QObject::connect(join, SIGNAL(clicked()), this, SLOT(emit_join_request()));
    }
    ~Row() {delete join;}
    void unpack(rapidjson::Value& data) {        
        game_type = data["game"].GetInt();
        table_id = data["id"].GetInt();
        curr_players = data["curr_players"].GetInt();
        max_players = data["max_players"].GetInt();

        QFile file("img/row2.svg");
        file.open(QIODevice::ReadOnly);
        QTextStream ts(&file);
        QString svg = ts.readAll();
        file.close();
        
        QString field;
        switch (game_type) {
            case GAME_TYPE_HOLDEM:
                field = "Hold'Em";
                break;
            case GAME_TYPE_STUD:
                field = "Stud";
                break;
            default:
                field = "Error";
        }
        
        svg.replace(QString("$GAME"), field);        
        field = QString::number(data["curr_players"].GetInt())
              + QString(" / ")
              + QString::number(data["max_players"].GetInt());
        svg.replace(QString("$PLAYERS"), field);
        field = QString("$") + QString::number(data["sblind"].GetInt())
              + QString(" / ")
              + QString("$") + QString::number(data["bblind"].GetInt());
        svg.replace(QString("$BLINDS"), field);
        load(svg.toUtf8());
    }
private slots:
    void emit_join_request() {emit join_request(game_type, max_players, table_id);}
signals:
    void join_request(int game_type, int max_players, int table_id);
};

class TableList : public QSvgWidget {
    Q_OBJECT
private:
    enum {LEN = 10};
    Row *_rows;
    int _unpacked;
public:
    TableList(QSvgWidget *parent = 0) : QSvgWidget(parent),
                                        _rows(new Row[LEN]),
                                        _unpacked(0) {
        setGeometry(kx*45, ky*170, kx*510, ky*260);
        load(QString("img/table_list3.svg"));
        for (int i = 0; i < LEN; ++i) {
            _rows[i].setParent(this);
            _rows[i].move(kx*1, ky*(22 + 18*i));
            _rows[i].hide();
        }
    }
    ~TableList() {delete[] _rows;}
    void fill(rapidjson::Value& content) {
        const int contentSize = content.Size();
        if (_unpacked == 0) {
            for (int i = 0; i < contentSize; ++i) {
                _rows[i].unpack(content[i]);
                _rows[i].show();
            }
            _unpacked = contentSize;
        } else if (_unpacked == contentSize) {
            for (int i = 0; i < contentSize; ++i) {
                _rows[i].unpack(content[i]);
            }
        } else if (_unpacked < contentSize) {
            int i;
            for (i = 0; i < _unpacked; ++i)
                _rows[i].unpack(content[i]);
            for (; i < contentSize; ++i) {
                _rows[i].unpack(content[i]);
                _rows[i].show();
            }
            _unpacked = contentSize;
        } else if (_unpacked > contentSize) {
            int i;
            for (i = 0; i < contentSize; ++i) {
                _rows[i].unpack(content[i]);
            }
            for (; i < _unpacked; ++i) {
                _rows[i].hide();
            }
            _unpacked = contentSize;
        }
    }
    template<class T> void bind(T *parent) {
        for (int i = 0; i < _unpacked; ++i) {
            disconnect(_rows+i, 0, 0, 0);
            connect(_rows+i, SIGNAL(join_request(int, int, int)), parent->ker, SLOT(join_game(int, int, int)));
        }
    }
};

class AccountData : public QSvgWidget {
private:
	QLabel *_name, *_bankroll;
public:
	AccountData(QSvgWidget *parent = 0) : QSvgWidget(parent),
										  _name(new QLabel(this)),
										  _bankroll(new QLabel(this)) {
	  	setGeometry(kx*455, ky*40, kx*120, ky*100);
	  	load(QString("img/account_data.svg"));

        _name->setAlignment(Qt::AlignCenter);
        _bankroll->setAlignment(Qt::AlignCenter);
	  	_name->setGeometry(kx*0, ky*20, kx*120, ky*25);
	  	_bankroll->setGeometry(kx*0, ky*75, kx*120, ky*25);

	  	QFont font("Sans", 10);
	  	_name->setFont(font);
	  	_bankroll->setFont(font);

	  	QString styleSheet("color:rgb(40,40,40);");
	  	_name->setStyleSheet(styleSheet);
	  	_name->setStyleSheet(styleSheet);
	}
	~AccountData() {delete _name; delete _bankroll;}
	void setData(const char *name, float bankroll) {
        std::ostringstream oss; oss.precision(2);
        oss << std::fixed << bankroll;
        _name->setText(QString(name));
        _bankroll->setText(QString(oss.str().c_str()));
	}
};

#endif