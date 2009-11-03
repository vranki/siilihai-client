#ifndef REPORTPARSER_H
#define REPORTPARSER_H

#include <QtGui/QDialog>
#include <parserreport.h>
#include "ui_reportparser.h"


class ReportParser : public QDialog
{
    Q_OBJECT

public:
    ReportParser(QWidget *parent, int id, QString name);
    ~ReportParser();
private slots:
    void apply();
signals:
	void parserReport(ParserReport pr);

private:
	int parserid;
    Ui::ReportParserClass ui;
};

#endif // REPORTPARSER_H
