/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"

#include <QMessageBox>
#include <QLabel>
#include <QtSerialPort/QSerialPort>
#include <QTime>

// MySQL
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QString>

#include <QtSql>
#include <QtWidgets>
#include "connection.h"

void initializeModel(QSqlTableModel *model)
{
    createConnection();
    //! [0]
        model->setTable("user");
    //! [0]

        //model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    //! [1]
      //  model->setRelation(2, QSqlRelation("city", "id", "name"));
    //! [1] //! [2]
      //  model->setRelation(3, QSqlRelation("country", "id", "name"));
    //! [2]

    //! [3]
        model->setHeaderData(0, Qt::Horizontal, QObject::tr("user_id"));
        model->setHeaderData(1, Qt::Horizontal, QObject::tr("name"));
        model->setHeaderData(2, Qt::Horizontal, QObject::tr("surname"));
       // model->setHeaderData(3, Qt::Horizontal, QObject::tr("Country"));
    //! [3]

        model->select();
}

QTableView *createView(const QString &title, QSqlTableModel *model)
{
//! [4]
    QTableView *view = new QTableView;
    view->setModel(model);
    view->setItemDelegate(new QSqlRelationalDelegate(view));
//! [4]
    view->setWindowTitle(title);
    return view;
}

int printSqlDatabase(void)
{
  // QWidget app();
printf("\n\ninside printSqlDatabase\n\n");

   // createRelationalTables();

    QSqlTableModel model;

    initializeModel(&model);

    QTableView *view = createView(QObject::tr("Relational Table Model"), &model);
    view->show();



    //return app.show();
    return 666;
}



//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
//! [0]
    ui->setupUi(this);
    console = new Console;
    console->setEnabled(false);
    setCentralWidget(console);
//! [1]
    serial = new QSerialPort(this);
//! [1]
    settings = new SettingsDialog;

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);

    status = new QLabel;
    ui->statusBar->addWidget(status);

    initActionsConnections();

    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleError);

//! [2]
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
//! [2]
    connect(console, &Console::getData, this, &MainWindow::writeData);


   // initMySQLConnection();
//! [3]
}
//! [3]

MainWindow::~MainWindow()
{
    delete settings;
    delete ui;
}

//! [4]
void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
   // serial->setReadBufferSize(20);
    if (serial->open(QIODevice::ReadWrite)) {
        console->setEnabled(true);
        console->setLocalEchoEnabled(p.localEchoEnabled);
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        showStatusMessage(tr("Open error"));
    }
}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{
    if (serial->isOpen())
        serial->close();
    console->setEnabled(false);
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
}
//! [5]

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Terminal"),
                       tr("The <b>Simple Terminal</b> example demonstrates how to "
                          "use the Qt Serial Port module in modern GUI applications "
                          "using Qt, with a menu bar, toolbars, and a status bar."));
}

void MainWindow::displayDatabase()
{
    printSqlDatabase();
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
    serial->write(data);
}
//! [6]

//! [7]
void MainWindow::readData()
{
    // Read data
        static QByteArray byteArray;
        byteArray += serial->readAll();


        //we want to read all message not only chunks
        if(!QString(byteArray).contains("\n")){
           return;
        }

        console->putData(byteArray);

        //sanitize data
        QString data = QString( byteArray ).remove("\r").remove("\n");
        byteArray.clear();


        printf("data: %s\n", data.toStdString().c_str());

        if (!rfidConnected && data == QString("LOG: RFID is ready")) {
            printf("Entering connected state...\n");
            serial->write(QByteArray("APP READY"));
            printf("\n");
            rfidConnected = true;
            return;
        }
        if (rfidConnected && !userValid){
            currentUser = data.toUInt();
            if(isUserInDatabase(currentUser)) {
                userValid = true;
                printf("User found in DB\n");
            } else {
                printf("No such user in DB. Add user?\n");
                // process user adding here
            }
            return;
        }
        if (userValid){
            currentDevice = data.toUInt();
            if (currentDevice == currentUser) {
                printf("Error, device id %d equals user id\n", currentDevice);
                userValid = false;
                return;
            }
            if(isDeviceAssignedToUser(currentUser, currentDevice)) {
                deassignDeviceFromUser(currentUser, currentDevice);
                printf("User %d returned device %d\n", currentUser, currentDevice);
            } else {
                assignDeviceToUser(currentUser, currentDevice);
                printf("User %d borrowed device %d\n", currentUser, currentDevice);
            }
            userValid = false;
            return;
        }



}
//! [7]

//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}
//! [8]

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionConfigure, &QAction::triggered, settings, &MainWindow::show);
    connect(ui->actionClear, &QAction::triggered, console, &Console::clear);
    connect(ui->actionSql, &QAction::triggered, this, &MainWindow::displayDatabase);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);

}

void MainWindow::initMySQLConnection(void)
{
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("rfid");
    db.setUserName("root");
    db.setPassword("");

}

bool MainWindow::queryMysqlDatabase(QString query){
    printf("Query: %s\n", query.toStdString().c_str());
    if(!db.open()){
        printf("Nieudane polaczenie z baza danych\n");
        return false;
        }
    else
    {
        printf("Udane polaczenie z baza danych\n");

        QSqlQuery qry;
        if(!qry.exec(query))
            printf("Brak rekordow w tabeli Stats");

        while(qry.next())
        {
            int id = qry.value("user_id").toInt();
            QString name = qry.value(1).toString();
            QString surname = qry.value(2).toString();

            printf("user: %d, name: %s, surname: %s\n", id, name.toStdString().c_str(), surname.toStdString().c_str());
        }

        db.close();
    }
    return true;
}

bool MainWindow::isUserInDatabase(unsigned uid){

    return queryMysqlDatabase("SELECT * FROM user WHERE user_id=" + QString::number(uid));

}

bool isDeviceAssignedToUser(unsigned uid, unsigned did) {
    return true;

}

bool assignDeviceToUser(unsigned uid, unsigned did){
    return false;
}

bool deassignDeviceFromUser(unsigned uid, unsigned did){
    return false;

}

