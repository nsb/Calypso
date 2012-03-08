#include <QDialog>
#include "ui_fhelp.h"

class FHelp : public QMainWindow
{
Q_OBJECT

public:
 FHelp();
 Ui::FHelp ui;

private slots:
void on_BHome_clicked();
void on_BBack_clicked();
void on_BForward_clicked();

private:

};
