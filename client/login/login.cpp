#include "login.h"
#include "../account/account.h"
#include <iostream>

Login::Login(kernel *_ker, QSvgWidget *parent) : Frame(parent),
                                                 ker(_ker),
                                                 email(new QLineEdit(this)),
                                                 pwd(new QLineEdit(this)),
                                                 submit(new SvgButton(this)) {
    resize(230, 270);
    load(QString("img/login.svg"));
    
    QString style_sheet = "color:rgb(255,255,255);background-color:rgba(0,0,0,0);";
    
    email->setFrame(false);
    email->setGeometry(25, 99, 180, 25);
    email->setStyleSheet(style_sheet);
    
    pwd->setFrame(false);
    pwd->setGeometry(25, 160, 180, 25);
    pwd->setStyleSheet(style_sheet);
    
    submit->setGeometry(90, 215, 50, 25);
    submit->set_img("img/submit_b.svg", NULL);

    connect(submit, SIGNAL(clicked()), this, SLOT(submit_request()));
    connect(email, SIGNAL(returnPressed()), this, SLOT(submit_request()));
    connect(pwd, SIGNAL(returnPressed()), this, SLOT(submit_request()));
}

void Login::submit_request() {
    QString email_s = email->text();
    QString pwd_s = pwd->text();

    if (email_s.isEmpty())
        return;

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);
    w.StartObject();
    w.Key("auth");
    w.StartObject();
    w.Key("email");
    w.String(email_s.toUtf8());
    w.Key("pwd");
    w.String(pwd_s.toUtf8());
    w.EndObject();
    w.EndObject();
    ker->send(sb.GetString());
}

void Login::handler(const char *response) {
    rapidjson::Document rsp;
    rsp.Parse(response);

    if (!rsp.IsObject()) {
        std::cout << "parser error" << std::endl;
        return;
    }
    
    if (rsp.HasMember("auth")) {
        auth(rsp["auth"]);
    }
}

void Login::auth(rapidjson::Value& content) {
    QString status = (QString) content["status"].GetString();
    if (status != "confirmed")
        return;

    ker->id = (int) content["id"].GetInt();
    ker->name = (char*) content["name"].GetString();
    ker->bankroll = (int) content["bankroll"].GetInt();
    ker->change_frame<Account>();
}