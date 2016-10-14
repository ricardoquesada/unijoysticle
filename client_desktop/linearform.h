#ifndef LINEARFORM_H
#define LINEARFORM_H

#include <QWidget>

namespace Ui {
class LinearForm;
}

class LinearForm : public QWidget
{
    Q_OBJECT

public:
    explicit LinearForm(QWidget *parent = 0);
    ~LinearForm();

private:
    Ui::LinearForm *ui;
};

#endif // LINEARFORM_H
