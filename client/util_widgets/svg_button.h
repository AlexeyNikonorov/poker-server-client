#ifndef SVG_BUTTON_H
#define SVG_BUTTON_H

#include <QObject>
#include <QSvgWidget>
#include <QString>
#include <QSvgRenderer>
#include <QPainter>

#if 0

class SvgButton : public QSvgWidget {
    Q_OBJECT
private:
    QSvgRenderer *_basic, *_hover;
public:
    SvgButton(QSvgWidget *parent = 0) : QSvgWidget(parent),
                                        _basic(NULL),
                                        _hover(NULL) {setCursor(Qt::PointingHandCursor);}
    void set_img(const char *basic, const char *hover) {
        if (basic != NULL) {
            _basic = new QSvgRenderer();
            _basic->load(QString(basic));
            QPainter p(this);
            _basic->render(&p);
        }
        if (hover != NULL) {
            _hover = new QSvgRenderer();
            _hover->load(QString(hover));
        }
    }
private:
    virtual void mousePressEvent(QMouseEvent *ev) {
        (void) ev;
        emit clicked();
    }
    virtual void enterEvent(QEvent *ev) {
        (void) ev;
        if (_hover != NULL) {
            QPainter p(this);
            _hover->render(&p);
            //render(_painter);
        }
    }
    virtual void leaveEvent(QEvent *ev) {
        (void) ev;
        if (_hover != NULL) {
            QPainter p(this);
            _basic->render(&p);
            //render(_painter);
        }
    }
signals: 
    void clicked();
};

#else

class SvgButton : public QSvgWidget {
    Q_OBJECT
private:
    QString img_basic;
    QString img_hover;
public:
    SvgButton(QSvgWidget *parent = 0) : QSvgWidget(parent) {setCursor(Qt::PointingHandCursor);}
    void set_img(const char *basic, const char *hover) {
        img_basic = QString(basic);
        load(img_basic);
        if (hover == NULL) {
            img_hover = QString();
        } else {
            img_hover = QString(hover);
        }
    }
private:
    void mousePressEvent(QMouseEvent *ev) {
        (void) ev;
        emit clicked();
    }
    void enterEvent(QEvent *ev) {
        (void) ev;
        if (!img_hover.isEmpty()) {
            load(img_hover);
        }
    }
    void leaveEvent(QEvent *ev) {
        (void) ev;
        if (!img_hover.isEmpty()) {
            load(img_basic);
        }
    }
signals: 
    void clicked();
};

#endif

#endif
