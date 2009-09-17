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
#include <QNmeaWhereabouts>

#include "qtpedometer.h"
#include <math.h>

#define METERS_TO_FEET 3.2808399       /* Meters to U.S./British feet */
#define MPS_TO_MPH 2.2369363           /* Meters/second to miles per hour */
#define FEET_TO_MILES 0.000189393939

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
	update_count= 0;
	whereabouts= NULL;
	valid_update= false;
	running= false;
	init();
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
	if(QApplication::arguments().size() > 1){
		plugin= QApplication::arguments().at(1);
		qDebug("Using plugin %s\n", (const char *)plugin.toAscii());
	}else
		plugin= "";

	
	if(plugin.size() > 0){
		if(plugin == "sim"){
			QString fn= "/root/nmea_sample.txt";
			if(QApplication::arguments().size() > 2){
				fn= QApplication::arguments().at(2);
			}
			qDebug("using file: %s\n", (const char *)fn.toAscii());
			
			QFile *sampleFile= new QFile(fn, this);
			sampleFile->open(QIODevice::ReadOnly);
			QNmeaWhereabouts *wa = new QNmeaWhereabouts(this);
			wa->setUpdateMode(QNmeaWhereabouts::SimulationMode);
			wa->setSourceDevice(sampleFile);
			whereabouts= wa;
		}else{
			QString host= "";
			if(QApplication::arguments().size() > 2){
				host= QApplication::arguments().at(2);
				qDebug("to host: %s\n", (const char *)host.toAscii());
			}
			whereabouts= QWhereaboutsFactory::create(plugin, host);
		}
	} else
		whereabouts= QWhereaboutsFactory::create();

	if (whereabouts == NULL) {
		QMessageBox::warning(this, tr("Error"), tr("Cannot find a location data source."));
        return;
    }

	connect(whereabouts, SIGNAL(updated(QWhereaboutsUpdate)),
			SLOT(updated(QWhereaboutsUpdate)));
	connect(whereabouts, SIGNAL(stateChanged(QWhereabouts::State)),
            SLOT(stateChanged(QWhereabouts::State)));

	connect(ui.resetButton, SIGNAL(clicked()), this, SLOT(resetData()));
	connect(ui.pauseButton, SIGNAL(clicked()), this, SLOT(pauseData()));
	connect(ui.startButton, SIGNAL(clicked()), this, SLOT(startData()));
 	
	whereabouts->setUpdateInterval(1000); // update every second
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
	if (update.coordinate().type() == QWhereaboutsCoordinate::InvalidCoordinate){
		qDebug("Invalid coordinate");
		return;
	}

	QString pos= update.coordinate().toString(QWhereaboutsCoordinate::DegreesMinutesSecondsWithHemisphere);
	QStringList list= pos.split(",");

	ui.latitude->setText(list.at(0));
	ui.longitude->setText(list.at(1));

	if(update.coordinate().type() == QWhereaboutsCoordinate::Coordinate3D)
		ui.altitude->setText(QString::number(update.coordinate().altitude() * METERS_TO_FEET, 'f', 3)); // convert to feet

	if(update.dataValidityFlags() & QWhereaboutsUpdate::Course)
		ui.track->setText(QString::number(update.course(), 'f', 2) + QChar(0x00B0));   // degrees symbol
	if(update.dataValidityFlags() & QWhereaboutsUpdate::GroundSpeed)
		ui.speed->setText(QString::number(update.groundSpeed() * MPS_TO_MPH, 'f', 3)); // convert to miles per hour
	if(update.dataValidityFlags() & QWhereaboutsUpdate::VerticalSpeed)
		ui.climb->setText(QString::number(update.verticalSpeed() * MPS_TO_MPH, 'f', 3)); // convert to miles per hour

	ui.time->setText(update.updateDateTime().toLocalTime().time().toString() + " " + update.updateDateTime().date().toString(Qt::ISODate));

	// calculate average speed, and distance travelled for every 10'th update
	if(running){
		char str[16];
		int ms= running_time.elapsed();
		int hrs= ((ms/1000)/60)/60;
		int mins= ((ms/1000)/60) % 60;
		int secs= (ms/1000) % 60;
		snprintf(str, sizeof(str), "%02d:%02d:%02d", hrs, mins, secs);
		ui.runningTime->setText(str);

		if(update_count >= 10){
			calculateAverages(update.coordinate());
			update_count= 0;
			valid_update= true;
			last_update= update;
		}else
			update_count++;
	}

	if(update.dataValidityFlags() & QWhereaboutsUpdate::HorizontalAccuracy)
		qDebug("Horizontal Accuracy: %10.6f\n", update.horizontalAccuracy() * METERS_TO_FEET);

	if(update.dataValidityFlags() & QWhereaboutsUpdate::VerticalAccuracy)
		qDebug("Vertical Accuracy:  %10.6f\n", update.verticalAccuracy() * METERS_TO_FEET);

	if(update.dataValidityFlags() & QWhereaboutsUpdate::GroundSpeedAccuracy)
		qDebug("Ground Speed Accuracy:  %10.6f\n", update.groundSpeedAccuracy() * MPS_TO_MPH);

	if(update.dataValidityFlags() & QWhereaboutsUpdate::VerticalSpeedAccuracy)
		qDebug("Vertical Speed Accuracy:  %10.6f\n", update.verticalSpeedAccuracy() * MPS_TO_MPH);

	if(update.dataValidityFlags() & QWhereaboutsUpdate::CourseAccuracy)
		qDebug("Course Accuracy:  %10.6f\n", update.courseAccuracy());

	if(update.dataValidityFlags() & QWhereaboutsUpdate::UpdateTimeAccuracy)
		qDebug("Time Accuracy:  %10.6f\n", update.updateTimeAccuracy());

}

void QtPedometer::calculateAverages(const QWhereaboutsCoordinate &current_position)
{
	int ms= running_time.elapsed();

	if(!valid_update)
		return;

	// use simple minded approach for now, take the length of each 10 second leg and accumulate it
	// TODO will need to smooth out data and ignore ones that are out of whack
	
	qreal d= last_update.coordinate().distanceTo(current_position);
	distance += d; // in meters
	qDebug("distance= %10.6f, acc= %10.6f", d, distance);

	// display in miles, feet
	double feet= distance * METERS_TO_FEET;
	double miles= feet * FEET_TO_MILES;
	if(miles < 1.0){
		ui.distance->setText(QString::number(feet, 'f', 1) + " ft"); // + ", " + QString::number(miles, 'f', 2) + " ml");
	}else{
		double imiles;
		double ffeet= modf(miles, &imiles) / FEET_TO_MILES;
		ui.distance->setText(QString::number(imiles, 'f', 0) + "m " + QString::number(ffeet, 'f', 1));
	}
	
	// calculate average speed which is total distance covered divided by running_time
	qreal speed= distance / (ms/1000.0); // gets meters per sec
	ui.aveSpeed->setText(QString::number(speed * MPS_TO_MPH, 'f', 3) + " mph");
}

void QtPedometer::startData()
{
	int ret= QMessageBox::question(this, tr("Tracking"),
								   tr("Are you sure you want to start the data?"),
								   QMessageBox::Yes | QMessageBox::No);
	if(ret == QMessageBox::Yes){
		valid_update= false;
		distance= 0.0;
		running_time.start();
		running= true;
	}
}

void QtPedometer::pauseData()
{
}

void QtPedometer::resetData()
{
	int ret= QMessageBox::question(this, tr("Tracking"),
								   tr("Are you sure you want to reset the data?"),
								   QMessageBox::Yes | QMessageBox::No);
	if(ret == QMessageBox::Yes){
		// reset the data
		ui.aveSpeed->clear();
		ui.distance->clear();
		ui.runningTime->clear();
		valid_update= false;
		running= false;
	}

}


void QtPedometer::paintEvent(QPaintEvent *)
{
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
	if(whereabouts == NULL){
		whereabouts->stopUpdates();
	}

}
