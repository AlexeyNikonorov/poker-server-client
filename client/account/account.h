#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "../kernel/kernel.h"
#include "account_widgets/account_widgets.h"
#include "../util_widgets/svg_button.h"

#include <QObject>
#include <QSvgWidget>
#include <QString>
#include <QTimer>

class Account : public Frame {
    Q_OBJECT
    friend TableList;
private:
    kernel *ker;
    QTimer *timer;
    TableList *table_list;
    AccountData *account_data;
public:
    Account(kernel *_ker, QSvgWidget *parent = 0);
    ~Account();
    void init_request();
    virtual void handler(const char *response);
private slots:
	void updateRequest();
};

#endif
