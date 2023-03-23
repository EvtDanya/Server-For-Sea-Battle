#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setFixedSize(this->size());

    this->ui->plainTextEdit->setReadOnly(true);

    this->server = new Server();
    this->logger = new Logger();
    connect(server, &Server::addText, this,&MainWindow::addText);
    connect(server, &Server::getPort, this,&MainWindow::setPort);
}

MainWindow::~MainWindow()
{
    delete logger;
    delete server;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    this->server->close();
    event->accept();
}

void MainWindow::addText(QString text)
{
    this->ui->plainTextEdit->appendPlainText(text);
    this->ui->plainTextEdit->verticalScrollBar()->setValue(this->ui->plainTextEdit->verticalScrollBar()->maximum());
    this->logger->add(text.toStdString());
}

void MainWindow::setPort()
{

}

void MainWindow::on_questionBtn_clicked()
{
    QMessageBox::information(this, "About","Enter port you want or use default port.\n"
                                           "Then click \"Start server\" and return to the connection window in game.\n"
                                           "Now you can connect to this server, using ip address and entered port and start game with ur friend :)");
}

void MainWindow::on_exitBtn_clicked()
{
    this->server->close();
    QApplication::quit();
}

void MainWindow::on_startServerBtn_clicked()
{
    QMessageBox::information(this, "Info","While playing locale game dont close this program!");
    this->server->setPort(this->ui->spinBox->value());
    this->server->startServer();
    this->ui->startServerBtn->setEnabled(false);
}
