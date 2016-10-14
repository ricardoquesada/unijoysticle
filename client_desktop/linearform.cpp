#include "linearform.h"
#include "ui_linearform.h"

#include <QSlider>


LinearForm::LinearForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LinearForm)
{
    ui->setupUi(this);

    connect(ui->horizontalSlider, &QSlider::valueChanged, [&](int value){
        ui->label->setText(QString::number(value));
    });
}

LinearForm::~LinearForm()
{
    delete ui;
}
