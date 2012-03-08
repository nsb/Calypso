#include <QDialog>
#include "ui_fabout.h"

class FAbout : public QDialog,public Ui::FAbout
{
Q_OBJECT

public:
    FAbout();

protected:
    void closeEvent (QCloseEvent * event);

private slots:
    void reject();
    void changeEvent (QEvent* event);
};
