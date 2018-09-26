/********************************************************************************
** Form generated from reading UI file 'CloudRoutesDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLOUDROUTESDIALOG_H
#define UI_CLOUDROUTESDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_CloudRoutesDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *labelNoRoute;
    QListView *listView;
    QHBoxLayout *horizontalLayout;
    QProgressBar *progressBar;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *CloudRoutesDialog)
    {
        if (CloudRoutesDialog->objectName().isEmpty())
            CloudRoutesDialog->setObjectName(QStringLiteral("CloudRoutesDialog"));
        CloudRoutesDialog->resize(512, 384);
        verticalLayout = new QVBoxLayout(CloudRoutesDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        labelNoRoute = new QLabel(CloudRoutesDialog);
        labelNoRoute->setObjectName(QStringLiteral("labelNoRoute"));
        labelNoRoute->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(labelNoRoute);

        listView = new QListView(CloudRoutesDialog);
        listView->setObjectName(QStringLiteral("listView"));

        verticalLayout->addWidget(listView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        progressBar = new QProgressBar(CloudRoutesDialog);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        progressBar->setValue(0);
        progressBar->setTextVisible(true);
        progressBar->setInvertedAppearance(false);

        horizontalLayout->addWidget(progressBar);

        buttonBox = new QDialogButtonBox(CloudRoutesDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close);

        horizontalLayout->addWidget(buttonBox);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(CloudRoutesDialog);
        QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), CloudRoutesDialog, SLOT(close()));

        QMetaObject::connectSlotsByName(CloudRoutesDialog);
    } // setupUi

    void retranslateUi(QDialog *CloudRoutesDialog)
    {
        CloudRoutesDialog->setWindowTitle(QApplication::translate("CloudRoutesDialog", "Cloud Routes", 0));
        labelNoRoute->setText(QApplication::translate("CloudRoutesDialog", "No route available.", 0));
    } // retranslateUi

};

namespace Ui {
    class CloudRoutesDialog: public Ui_CloudRoutesDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLOUDROUTESDIALOG_H
