#include <QtGlobal>

#ifdef Q_WS_QWS
#include <QAction>
#include <QMenu>
#include <QtopiaApplication>
#else
#include <QApplication>
#endif

#include <QMessageBox>
#include <QtDebug>

#include "qtpedometer.h"
#include <math.h>

#include <iostream>
#include <string>
#include <unistd.h>


QtPedometer::QtPedometer(QWidget *parent, Qt::WFlags f) :  QWidget(parent, f)
{
	qDebug("In QtPedometer()");
	
#ifdef Q_WS_QWS
	setObjectName("Pedometer");
	QtopiaApplication::setInputMethodHint(this, QtopiaApplication::AlwaysOff);
	setWindowTitle(tr("Pedometer", "application header"));
#endif
	ui.setupUi(this);
	state= -1;
	timer= time(NULL);
	hidden= true;

	QTimer::singleShot(100, this, SLOT(init()));
}

QtPedometer::~QtPedometer()
{
	qDebug("In ~QtPedometer()");
}

void QtPedometer::init()
{
	// setup gps
	QString plugin;
	if(QApplication::arguments().size() > 1)
		plugin= QApplication::arguments().at(1);
	else
		plugin= "";

	qDebug("Using plugin %s\n", (const char *)plugin.toAscii());

}

void QtPedometer::paintEvent(QPaintEvent *)
{
	QMutexLocker locker(&mutex);

}

void QtPedometer::showEvent(QShowEvent *)
{
	qDebug("In show");
	hidden= false;
}

void QtPedometer::hideEvent(QHideEvent *)
{
	qDebug("In hide");
	hidden= true;
}

void QtPedometer::closeEvent (QCloseEvent *)
{
	qDebug("In close");

}
