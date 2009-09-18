#include <QtGlobal>

#ifdef Q_WS_QWS
#include <QAction>
#include <QMenu>
#include <QtopiaApplication>
#include <QSoftMenuBar>
#else
#include <QApplication>
#endif


#include <QMessageBox>
#include <QtDebug>
#include <QNmeaWhereabouts>
#include <QCloseEvent>
#include <QTextStream>

#include "qtpedometer.h"
#include <math.h>

#define METERS_TO_FEET 3.2808399            /* Meters to U.S./British feet */
#define METERS_TO_MILES 0.000621371192      /* Meters to U.S./British feet */
#define MPS_TO_MPH 2.2369363                /* Meters/second to miles per hour */
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
	speed_threshold= 0.1788159993648455703; // MPS = 0.4 MPH
	createMenus();
	init();
}

QtPedometer::~QtPedometer()
{
	qDebug("In ~QtPedometer()");
}

void QtPedometer::createMenus()
{
    QMenu *contextMenu;
    contextMenu = QSoftMenuBar::menuFor(this);

    QAction *saveAct= new QAction(tr("Save Trip..."), this);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveTrip()));
	contextMenu->addAction(saveAct);
    //contextMenu->addSeparator();
}

void QtPedometer::init()
{
	qDebug("In QtPedometer:init()");

	ui.startButton->setDisabled(true);
	ui.pauseButton->setDisabled(true);

	// setup gps plugin, can be "gpsd" or use the default
	QString plugin;
	if(QApplication::arguments().size() > 1){
		plugin= QApplication::arguments().at(1);
		qDebug("Using plugin %s\n", (const char *)plugin.toAscii());
	}else
		plugin= "";

	
	if(plugin.size() > 0){
		// use a simulation for testing purposes, reads NMEA data from the given file
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
			// Use gpsd to the given host (gpsd must be started)
			QString host= "";
			if(QApplication::arguments().size() > 2){
				host= QApplication::arguments().at(2);
				qDebug("to host: %s\n", (const char *)host.toAscii());
			}
			whereabouts= QWhereaboutsFactory::create(plugin, host);
		}
	} else
		// use the default device, which is a custom plugin on FR qith Qtmoko
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
	connect(ui.setWaypoint, SIGNAL(clicked()), this, SLOT(setWayPoint()));
	connect(ui.clearWaypoint, SIGNAL(clicked()), this, SLOT(clearWayPoint()));
 	
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
			qDebug("got fix");
			ui.status->setText("Fix Acquired");
			ui.startButton->setDisabled(false);
			ui.pauseButton->setDisabled(false);
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

	current_update= update;

	QString pos= update.coordinate().toString(QWhereaboutsCoordinate::DegreesMinutesSecondsWithHemisphere);
	QStringList list= pos.split(",");

	ui.latitude->setText(list.at(0));
	ui.longitude->setText(list.at(1));

	if(update.coordinate().type() == QWhereaboutsCoordinate::Coordinate3D)
		ui.altitude->setText(QString::number(update.coordinate().altitude() * METERS_TO_FEET, 'f', 3) + " ft"); // convert to feet

	if(update.dataValidityFlags() & QWhereaboutsUpdate::Course)
		ui.track->setText(QString::number(update.course(), 'f', 2) + QChar(0x00B0));   // degrees symbol
	if(update.dataValidityFlags() & QWhereaboutsUpdate::GroundSpeed)
		ui.speed->setText(QString::number(update.groundSpeed() * MPS_TO_MPH, 'f', 3) + " mph"); // convert to miles per hour
	if(update.dataValidityFlags() & QWhereaboutsUpdate::VerticalSpeed)
		ui.climb->setText(QString::number(update.verticalSpeed() * MPS_TO_MPH, 'f', 3)); // convert to miles per hour

	ui.time->setText(update.updateDateTime().toLocalTime().time().toString() + " " + update.updateDateTime().date().toString(Qt::ISODate));

	// calculate average speed, and distance travelled
	if(running){
		calculateTrip(update);
	}

	// if the way point is set then calculate and display the current distance to it
	if(!way_point.isNull())
		calculateWayPoint(update);

	// mostly for debugging
	if(update.dataValidityFlags() & QWhereaboutsUpdate::HorizontalAccuracy){
		horiz_accuracy=  update.horizontalAccuracy();
		qDebug("Horizontal Accuracy: %10.6f\n", update.horizontalAccuracy() * METERS_TO_FEET);
	}

	if(update.dataValidityFlags() & QWhereaboutsUpdate::VerticalAccuracy)
		qDebug("Vertical Accuracy:  %10.6f\n", update.verticalAccuracy() * METERS_TO_FEET);

	if(update.dataValidityFlags() & QWhereaboutsUpdate::GroundSpeedAccuracy){
		speed_accuracy=  update.groundSpeedAccuracy();
		qDebug("Ground Speed Accuracy:  %10.6f\n", update.groundSpeedAccuracy() * MPS_TO_MPH);
	}

	if(update.dataValidityFlags() & QWhereaboutsUpdate::VerticalSpeedAccuracy)
		qDebug("Vertical Speed Accuracy:  %10.6f\n", update.verticalSpeedAccuracy() * MPS_TO_MPH);

	if(update.dataValidityFlags() & QWhereaboutsUpdate::CourseAccuracy)
		qDebug("Course Accuracy:  %10.6f\n", update.courseAccuracy());

	if(update.dataValidityFlags() & QWhereaboutsUpdate::UpdateTimeAccuracy)
		qDebug("Time Accuracy:  %10.6f\n", update.updateTimeAccuracy());

}

// works out the Trip values
void QtPedometer::calculateTrip(const QWhereaboutsUpdate &update)
{
	// display trip time
	char str[16];
	int ms= running_time.elapsed();
	int hrs= ((ms/1000)/60)/60;
	int mins= ((ms/1000)/60) % 60;
	int secs= (ms/1000) % 60;
	snprintf(str, sizeof(str), "%02d:%02d:%02d", hrs, mins, secs);
	ui.runningTime->setText(str);

	// calculate trip distance covered I read that this may be more
	// accurate, get the speed calculated from the GPS, and use it to
	// determine the distance covered since the last speed update
	if(update.dataValidityFlags() & QWhereaboutsUpdate::GroundSpeed){
		// get measured speed
		qreal speed= update.groundSpeed();
		qDebug("speed= %10.6f m/s", speed);

		// if we are going less than the threshold then presume we are not moving
		if(speed < speed_threshold){
			speed= 0.0;
		}

		// get elapsed time
		QTime t= update.updateTime();
		if(valid_update){
			// calculate distance travelled
			QTime last= last_update.updateTime();
			int delta= last.msecsTo(t);
			qDebug("delta= %d ms", delta);
			qreal d= speed * (delta/1000.0);
			distance += d;
			qDebug("distance: %10.6f, %10.6f", d, distance);
		}else{
			valid_update= true;
		}
		last_update= update;
	}

#if 0
	// Old way of doing it, too inaccurate
	const QWhereaboutsCoordinate &current_position= update.coordinate();
	// for averages we just use every tenth update
	if(update_count < 10){
		update_count++;
		return;
	}else{
		update_count= 0;
		valid_update= true;
		last_update= update;
	}

	// use simple minded approach for now, take the length of each 10 second leg and accumulate it
	// NOTE this is very inaccurate accumulkated error is enormous
	qreal d= last_update.coordinate().distanceTo(current_position);
	distance += d; // in meters
	qDebug("distance= %10.6f, acc= %10.6f", d, distance);
#endif

   
#if 0
	// display in miles and feet if less than a mile otherwise in feet
	double feet= distance * METERS_TO_FEET;
	double miles= feet * FEET_TO_MILES;
	if(miles < 1.0){
		ui.distance->setText(QString::number(feet, 'f', 1) + " ft"); // + ", " + QString::number(miles, 'f', 2) + " ml");
	}else{
		double imiles;
		double ffeet= modf(miles, &imiles) / FEET_TO_MILES;
		ui.distance->setText(QString::number(imiles, 'f', 0) + "mi " + QString::number(ffeet, 'f', 1) + "ft");
	}
#else
	if(ui.feetButton->isChecked()){
		// display decimal feet
		qreal feet= distance * METERS_TO_FEET;
		ui.distance->setText(QString::number(feet, 'f', 1) + " ft");		
	}else{
		// display decimal miles
		qreal miles= distance * METERS_TO_MILES;
		ui.distance->setText(QString::number(miles, 'f', 4) + " mi");
	}
#endif
	// calculate average speed which is total distance covered divided by running_time
	qreal speed= distance / (ms/1000.0); // gets meters per sec
	ui.aveSpeed->setText(QString::number(speed * MPS_TO_MPH, 'f', 3) + " mph");
}

void QtPedometer::startData()
{
	int ret= QMessageBox::question(this, tr("Trip"),
								   tr("Are you sure you want to start the trip?"),
								   QMessageBox::Yes | QMessageBox::No);
	if(ret == QMessageBox::Yes){
		valid_update= false;
		distance= 0.0;
		running_time.start();
		running= true;
		ui.pauseButton->setText("Pause");
	}
}

void QtPedometer::pauseData()
{
	running= !running;
	if(!running){
		valid_update= false;
	}
	ui.pauseButton->setText(running ? "Pause" : "Resume");
}

void QtPedometer::resetData()
{
	int ret= QMessageBox::question(this, tr("Trip"),
								   tr("Are you sure you want to reset the trip?"),
								   QMessageBox::Yes | QMessageBox::No);
	if(ret == QMessageBox::Yes){
		// reset the data
		ui.aveSpeed->clear();
		ui.distance->clear();
		ui.runningTime->clear();
		valid_update= false;
		running= false;
		ui.pauseButton->setText("Pause");
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

void QtPedometer::closeEvent(QCloseEvent *event)
{
	qDebug("In close");
	int ret= QMessageBox::question(this, tr("Pedometer"),
								   tr("Are you sure you want to exit?"),
								   QMessageBox::Yes | QMessageBox::No);
	if(ret == QMessageBox::Yes){
		if(whereabouts == NULL){
			whereabouts->stopUpdates();
		}
        event->accept();
    } else {
        event->ignore();
    }
}

void QtPedometer::saveTrip()
{
	if(ui.runningTime->text().isEmpty()){
		QMessageBox::warning(this, tr("Trip"), tr("Nothing to save."));
		return;
	}
	
	// TODO need to put this is a configurations screen
	QString fileName = "/media/card/trip.txt";
	QFile file(fileName);
	if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Append)) {
		QMessageBox::warning(this, tr("Pedometer"),
							 tr("Cannot write file %1:\n%2.")
							 .arg(fileName)
							 .arg(file.errorString()));
		return;
	}

	QDateTime now= QDateTime::currentDateTime();
	QTextStream out(&file);
	//QApplication::setOverrideCursor(Qt::WaitCursor);
	out << "Comment: " << ui.tripComment->text() << endl;
	out << "Date: " << now.toString(Qt::ISODate) << endl;
	out << "Elapsed time: " << ui.runningTime->text() << endl;
	out << "Distance: " << ui.distance->text() << endl;
	out << "Speed: " << ui.aveSpeed->text() << endl;
	out << "Speed Accuracy: " << speed_accuracy << endl;
	out << "Horiz Accuracy: " << horiz_accuracy << endl;
	out << "=====================" << endl;

	//QApplication::restoreOverrideCursor();

	return;
}

// set the waypoint
void QtPedometer::setWayPoint()
{
	QString pos= current_update.coordinate().toString(QWhereaboutsCoordinate::DegreesMinutesSecondsWithHemisphere);
	QStringList list= pos.split(",");

	ui.wayPtLatitude->setText(list.at(0));
	ui.wayPtLongitude->setText(list.at(1));
	way_point= current_update;
}
void QtPedometer::clearWayPoint()
{
	ui.wayPtLatitude->clear();
	ui.wayPtLongitude->clear();
	way_point.clear();
}

// This calcualtes and displays either the 2D distance or 3D distance
// between the current position and the way point
void QtPedometer::calculateWayPoint(const QWhereaboutsUpdate &update)
{
	qreal distance= 0.0;
	if(ui.twoDCheck->isChecked() 
	   || update.coordinate().type() != QWhereaboutsCoordinate::Coordinate3D
	   || way_point.coordinate().type() != QWhereaboutsCoordinate::Coordinate3D)
	{
		ui.twoDCheck->setChecked(true);
		// calculate 2D distance ignoring altitude
		distance= way_point.coordinate().distanceTo(update.coordinate());
	}else{
		// calculate 3D distance
		distance= distance3d(way_point.coordinate(), update.coordinate());
	}

	if(ui.wayMilesCheck->isChecked()){
		// display decimal miles
		qreal miles= distance * METERS_TO_MILES;
		ui.wayPointDistance->setText(QString::number(miles, 'f', 4) + " mi");
	}else{
		// display decimal feet
		qreal feet= distance * METERS_TO_FEET;
		ui.wayPointDistance->setText(QString::number(feet, 'f', 1) + " ft");		
	}
}

#define PI 3.14159265
#define DEG2RAD(deg) (deg * PI / 180.0)

qreal QtPedometer::distance3d(const QWhereaboutsCoordinate& from, const QWhereaboutsCoordinate& to)
{
	// take into account center of earth
	double alt1= from.altitude() * 6370000.0;
	double alt2= to.altitude() * 6370000.0;

	// convert degrees to radians
	double lat1= DEG2RAD(from.latitude());
	double long1= DEG2RAD(from.longitude());
	double lat2= DEG2RAD(to.latitude());
	double long2= DEG2RAD(to.longitude());

	// convert from lat, long, alt to cartesian coordinates
	double x0= alt1 * cos(lat1) * sin(long1);
	double y0= alt1 * sin(lat1);
	double z0= alt1 * cos(lat1) * cos(long1);

	double x1= alt2 * cos(lat2) * sin(long2);
	double y1= alt2 * sin(lat2);
	double z1= alt2 * cos(lat2) * cos(long2);


	// then calculate distance between the two points
	double dist= sqrt( pow((x1-x0), 2) + pow((y1-y0), 2) + pow((z1-z0), 2) );

	return (qreal)dist;
}
