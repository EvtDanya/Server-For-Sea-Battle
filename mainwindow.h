#pragma once

#include <QCloseEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QScrollBar>
#include "server.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

     virtual void closeEvent(QCloseEvent *event) override;
public slots:
    void addText(QString);
    void setPort();
private slots:

    void on_questionBtn_clicked();

    void on_exitBtn_clicked();

    void on_startServerBtn_clicked();

private:
    Ui::MainWindow *ui;
    Server *server;
    Logger *logger;
};



