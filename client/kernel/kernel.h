#ifndef KERNEL_H
#define KERNEL_H

#define HOST "127.0.0.1"
//#define HOST "37.204.115.238"
#define PORT 5000

#include <iostream>
#include <QObject>
#include <QSvgWidget>
#include <QTcpSocket>
#include <QString>

class Frame : public QSvgWidget {
public:
    Frame(QSvgWidget *parent = 0) : QSvgWidget(parent) {}
    virtual void handler(const char *response) {(void) response;}
    virtual void init_request() {}
};

class kernel : public QObject {
    Q_OBJECT
private:
    QTcpSocket *socket;
    Frame *frame;
public:
    int id;
    QString name;
    int bankroll;
public:
    kernel(QObject *parent = 0);
    void run();
    void send(const QString& request);
    void change_frame(Frame *new_frame);
    template<class T> void change_frame() {
        socket->close();
        socket->connectToHost(HOST, PORT);
        frame->close();
        delete frame;
        T *new_frame = new T(this);
        frame = static_cast<Frame*>(new_frame);
        frame->init_request();
        frame->show();
    }
private slots:
    void listener();
public slots:
    void join_game(int game_type, int max_players, int table_id);
};

#endif