#include <QtGui>
#include "fhelp.h"


FHelp::FHelp()
	    : QMainWindow()
{
	    ui.setupUi(this);
}

void FHelp::on_BHome_clicked()
{
  ui.TBHelp->home();
}


void FHelp::on_BBack_clicked()
{
  ui.TBHelp->backward();
}


void FHelp::on_BForward_clicked()
{
  ui.TBHelp->forward();
}
