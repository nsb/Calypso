#include <QtGui>
#include "fabout.h"
#include "kommuteversion.h"


FAbout::FAbout()
: QDialog()
{
    setupUi(this);
    QString text0=label_release->text();
    text0.replace("x.xx",kommuteVersion());
    label_release->setText(text0);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void FAbout::closeEvent (QCloseEvent * event)
{
    QWidget::closeEvent(event);
}

void FAbout::reject()
{
    close();
}

void FAbout::changeEvent (QEvent* event)
{
    if (event->type () == QEvent::LanguageChange)
    {
        // apply the new language
        // all designed forms have retranslateUi() method
        retranslateUi (this);
    }

    QWidget::changeEvent (event);
}

