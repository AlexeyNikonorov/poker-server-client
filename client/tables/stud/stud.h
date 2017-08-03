#ifndef STUD_H
#define STUD_H

#include "../../rapidjson/document.h"
#include "../../rapidjson/writer.h"
#include "../../rapidjson/stringbuffer.h"

#include "../../kernel/kernel.h"
#include "../../util_widgets/svg_button.h"
#include "stud_widgets.h"

#include <QObject>
#include <QSvgWidget>
#include <QString>

namespace Table {

class Stud : public Frame {
	Q_OBJECT
private:
	kernel *ker;
    StudWidgets::Player *players;
    StudWidgets::ActionBar *action_bar;
    StudWidgets::SetStack *set_stack;
    SvgButton *leave;
    int max_players;
public:
	Stud(kernel *_ker, int _max_players = 4, QSvgWidget *parent = 0);
	~Stud() {delete[] players; delete action_bar; delete set_stack; delete leave;}
	virtual void handler(const char *response);
private:
	void new_player(rapidjson::Value& content);
	void betting(rapidjson::Value& content);
	void end(rapidjson::Value& content);
	void showdown(rapidjson::Value& content);
private slots:
	void stack_request();
	void bring_in_request();
	void sb_request();
	void bb_request();
	void fold_request();
	void call_request();
	void raise_request();
	void leave_table();
};

};

#endif