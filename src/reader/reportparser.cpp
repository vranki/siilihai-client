#include "reportparser.h"

ReportParser::ReportParser(QWidget *parent, int id, QString name) : QDialog(parent)
{
    ui.setupUi(this);
    parserid = id;
    ui.parserNameLabel->setText(name);
    connect(ui.sendButton, SIGNAL(clicked()), this, SLOT(apply()));
    connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(close()));
}

ReportParser::~ReportParser() {}

void ReportParser::apply() {
    int ci = ui.typeComboBox->currentIndex();
    ParserReport::ParserReportType type = ParserReport::PRTNoType;
    switch(ci) {
    case 1:
        type = ParserReport::PRTWorking;
        break;
    case 2:
        type = ParserReport::PRTNotWorking;
        break;
    case 3:
        type = ParserReport::PRTComment;
        break;
    }

    ParserReport r;
    r.type = type;
    r.comment = ui.commentsTextEdit->toPlainText();
    r.parserid = parserid;
    emit parserReport(&r);
    close();
}
