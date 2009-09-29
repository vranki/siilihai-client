#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

#include <parserdatabase.h>
#include <forumdatabase.h>
#include <forumsubscription.h>

#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ParserDatabase &pd, ForumDatabase &fd, QWidget *parent = 0 );
    ~MainWindow();
    void updateForumList();
signals:
	void subscribeForum();
public slots:
	void subscribeForumSlot();
private:
    Ui::MainWindowClass ui;
    ParserDatabase &pdb;
    ForumDatabase &fdb;
};

#endif // MAINWINDOW_H
