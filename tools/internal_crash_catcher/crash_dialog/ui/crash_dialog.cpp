/*
 * Crash dialog
 * Copyright (C) 2016  Michał Walenciak <MichalWalenciak@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "crash_dialog.hpp"

#include <functional>

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>
#include <QPushButton>

#include "idebugger.hpp"
#include "ui_crash_dialog.h"


CrashDialog::CrashDialog(IDebugger* debugger): QDialog(), ui(nullptr), m_debugger(debugger)
{
    ui = new Ui::CrashDialog;
    ui->setupUi(this);

    const QFileInfo execInfo(debugger->exec());
    const QString fileName = execInfo.fileName();

    ui->crashLabel->setText( tr("Program %1 has crashed.\n"
                                "Below you can find more technical details.\n"
                                "It is also possible to send a bug report")
                                .arg(fileName)
    );

    QPushButton* reportButton = ui->buttonBox->addButton(tr("Report"), QDialogButtonBox::ActionRole);
    QPushButton* runButton = ui->buttonBox->addButton(tr("Run again"), QDialogButtonBox::ActionRole);
    QPushButton* closeButton = ui->buttonBox->addButton(tr("Close"), QDialogButtonBox::AcceptRole);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(reportButton, &QPushButton::clicked, this, &CrashDialog::report);
    connect(runButton, &QPushButton::clicked, this, &CrashDialog::run);

    using namespace std::placeholders;

    auto bt = std::bind(&CrashDialog::backtrace, this, _1);
    m_debugger->requestBacktrace(bt);
}


CrashDialog::~CrashDialog()
{
    delete ui;
}


void CrashDialog::backtrace(const std::vector<QString>& bt)
{
    for(const QString& line: bt)
        ui->plainTextEdit->appendPlainText(line);
}


void CrashDialog::report()
{
    const QString reportPath = QDir::tempPath() + "/photo_broom_crash_report.txt";

    QFile reportFile(reportPath);
    reportFile.open(QIODevice::WriteOnly);

    QTextStream reportStream(&reportFile);
    reportStream << ui->plainTextEdit->toPlainText();
    reportStream.flush();

    reportFile.close();

    const QString email = "Kicer86@gmail.com";
    const QString subject = "PhotoBroom crash report";
    const QString message = "Report attached";

    const QString url = QString("mailto:%1?subject=%2&body=%3&attachment=%4")
                        .arg(email)
                        .arg(subject)
                        .arg(message)
                        .arg(reportPath);

    QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode));
}


void CrashDialog::run()
{
    QProcess::startDetached(m_debugger->exec());
    accept();
}
