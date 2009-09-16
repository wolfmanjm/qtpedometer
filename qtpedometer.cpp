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

#define METERS_TO_FEET 3.2808399       /* Meters to U.S./British feet */
#define MPS_TO_MPH 2.2369363       /* Meters/second to miles per hour */

QtPedometer::QtPedometer(QWidget *parent, Qt::WFlags f) :  QWidget(parent, f)
{
	qDebug("In QtPedometer()");
	
#ifdef Q_WS_QWS
	setObjectName("Pedometer");
	QtopiaApplication::setInputMethodHint(this, QtopiaApplication::AlwaysOff);
	setWindowTitle(tr("Pedometer", "application header"));
#endif
	ui.setupUi(this);
	hidden= true;

	QTimer::singleShot(100, this, SLOT(init()));
}

QtPedometer::~QtPedometer()
{
	qDebug("In ~QtPedometer()");
}

void QtPedometer::init()
{
	qDebug("In QtPedometer:init()");
	// setup gps plugin, can be "gpsd" or use the default
	QString plugin;
	if(QApplication::arguments().size() > 1)
		plugin= QApplication::arguments().at(1);
	else
		plugin= "";

	qDebug("Using plugin %s\n", (const char *)plugin.toAscii());
	QWhereabouts *whereabouts;
	if(plugin.size() > 0){
		QString host= "";
		if(QApplication::arguments().size() > 2)
			host= QApplication::arguments().at(2);
		whereabouts= QWhereaboutsFactory::create(plugin, host);
	} else
		whereabouts= QWhereaboutsFactory::create();

	if (!whereabouts) {
		QMessageBox::warning(this, tr("Error"), tr("Cannot find a location data source."));
        return;
    }

	connect(whereabouts, SIGNAL(updated(QWhereaboutsUpdate)),
			SLOT(updated(QWhereaboutsUpdate)));
	connect(whereabouts, SIGNAL(stateChanged(QWhereabouts::State)),
            SLOT(stateChanged(QWhereabouts::State)));
 	
	whereabouts->startUpdates();

}

void QtPedometer::stateChanged(QWhereabouts::State state)
{
    switch (state) {
        case QWhereabouts::NotAvailable:
			ui.status->setText("GPS not available");
            break;
        case QWhereabouts::Initializing:
			ui.status->setText("Initializing");
            break;
        case QWhereabouts::Available:
			ui.status->setText("Waiting for fix");
			break;
        case QWhereabouts::PositionFixAcquired:
			ui.status->setText("Fix Acquired");
            break;
        default:
			ui.status->setText("Unknown Status");
    }
}

void QtPedometer::updated(const QWhereaboutsUpdate &update)
{
	QString pos= update.coordinate().toString(QWhereaboutsCoordinate::DegreesMinutesSecondsWithHemisphere);
	QStringList list= pos.split(",");

	ui.latitude->setText(list.at(0));
	ui.longitude->setText(list.at(1));

	ui.altitude->setText(QString::number(update.coordinate().altitude() * METERS_TO_FEET, 'f', 3)); // convert to feet

	if (update.dataValidityFlags() & QWhereaboutsUpdate::Course &&
		update.dataValidityFlags() & QWhereaboutsUpdate::GroundSpeed) {
		ui.track->setText(QString::number(update.course(), 'f', 2) + QChar(0x00B0));   // degrees symbol
		ui.speed->setText(QString::number(update.groundSpeed() * MPS_TO_MPH, 'f', 3)); // convert to miles per hour
		ui.climb->setText(QString::number(update.verticalSpeed() * MPS_TO_MPH, 'f', 3)); // convert to miles per hour
	}

	ui.time->setText(update.updateDateTime().toLocalTime().time().toString());

	lastUpdate= update;
}



void QtPedometer::paintEvent(QPaintEvent *)
{
	qDebug("In paintEvent");
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
