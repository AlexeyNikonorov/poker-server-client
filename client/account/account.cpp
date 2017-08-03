#include "account.h"
#include "../tables/holdem/holdem.h"

Account::Account(kernel *_ker, QSvgWidget *parent) : Frame(parent),
                                                     ker(_ker),
                                                     timer(new QTimer(this)),
                                                     table_list(new TableList(this)),
                                                     account_data(new AccountData(this)) {
    resize(kx*600, ky*450);
    load(QString("img/account_bg2.svg"));
    account_data->setData("player1", 5000);
    timer->start(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateRequest()));
}

Account::~Account() {
    timer->stop();
    delete timer;
    delete table_list;
}

void Account::init_request() {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);
    w.StartObject();
    w.Key("account");
    w.StartObject();
    w.Key("id");
    w.Int(ker->id);
    w.Key("name");
    w.String(ker->name.toUtf8());
    w.EndObject();
    w.EndObject();
    ker->send(sb.GetString());
}

void Account::updateRequest() {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);
    w.StartObject();
    w.Key("table_data");
    w.StartObject();
    w.Key("id");
    w.Int(ker->id);
    w.Key("name");
    w.String(ker->name.toUtf8());
    w.EndObject();
    w.EndObject();
    ker->send(sb.GetString());
    timer->start(10000);
}

void Account::handler(const char *response) {
    rapidjson::Document rsp;
    rsp.Parse(response);
    if (!rsp.IsObject()) {
        std::cout << "parser error" << std::endl;
        return;
    }
    
    if (rsp.HasMember("join")) {
        bool status = (bool) rsp["join"]["status"].GetBool();
        if (status) {
            timer->stop();
            ker->change_frame<Table::Holdem>();
        }
    } else if (rsp.HasMember("table_data")) {
        table_list->fill(rsp["table_data"]);
        table_list->bind<Account>(this);
    }
}
