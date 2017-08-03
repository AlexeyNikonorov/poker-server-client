#ifndef LOGIN_H
#define LOGIN_H

#include "../rapidjson/document.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/stringbuffer.h"
#include "../kernel/kernel.h"
#include "../util_widgets/svg_button.h"
#include <QObject>
#include <QSvgWidget>
#include <QString>
#include <QLineEdit>

class Login : public Frame {
    Q_OBJECT
private:
    kernel *ker;
    QLineEdit *email, *pwd;
    SvgButton *submit;
public:
    Login(kernel *_ker, QSvgWidget *parent = 0);
    ~Login() {delete email; delete pwd; delete submit;}
    virtual void handler(const char *response);
private:
    void auth(rapidjson::Value& content);
private slots:
    void submit_request();
};

#endif
