#include "kernel.h"
#include "../login/login.h"
#include "../account/account.h"
#include "../tables/holdem/holdem.h"
#include "../tables/stud/stud.h"

kernel::kernel(QObject *parent) : QObject(parent),
                                  socket(new QTcpSocket(this)),
                                  frame(new Login(this)) {}

void kernel::run() {
    socket->connectToHost(HOST, PORT);
    connect(socket, SIGNAL(readyRead()), this, SLOT(listener()));
    frame->show();
}

void kernel::listener() {
    char response[1024];
    socket->readLine(response, 1024);
    std::cout << response << std::endl;
    frame->handler(response);
}

void kernel::send(const QString& request) {
    socket->write(request.toUtf8());
}

void kernel::change_frame(Frame *new_frame) {
    socket->close();
    socket->connectToHost(HOST, PORT);
    frame->close();
    frame = new_frame;
    frame->init_request();
    frame->show();
}

void kernel::join_game(int game_type, int max_players, int table_id) {
    socket->close();
    socket->connectToHost(HOST, PORT);
    frame->close();
    delete frame;

    switch (game_type) {
        case 1:
        	frame = static_cast<Frame*>(new Table::Holdem(this, max_players));
            break;
        case 2:
        	frame = static_cast<Frame*>(new Table::Stud(this, max_players));
            break;
    }

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);
    w.StartObject();
    w.Key("join");
    w.StartObject();
    w.Key("game_type");
    w.Int(game_type);
    w.Key("table_id");
    w.Int(table_id);
    w.Key("id");
    w.Int(id);
    w.Key("name");
    w.String(name.toUtf8());
    w.EndObject();
    w.EndObject();
    send(sb.GetString());
    frame->show();
}