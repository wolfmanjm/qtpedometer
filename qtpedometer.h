#ifndef QTPEDOMETER_H
#define QTPEDOMETER_H

#include <QWhereabouts>
#include <QWhereaboutsFactory>

#include "ui_qtpedometer.h"
#include "compass.h"

class QtPedometer : public QWidget
{
	Q_OBJECT

	public:
		QtPedometer(QWidget *parent = 0, Qt::WFlags f = 0);
		virtual ~QtPedometer();

	private slots:
		void updated(const QWhereaboutsUpdate &update);
		void stateChanged(QWhereabouts::State state);
		bool resetData();
		void startData();
		void pauseData();
		void saveTrip();
		void setWayPoint();
		void clearWayPoint();
		void settings();

	protected:
		void paintEvent(QPaintEvent *event);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void closeEvent(QCloseEvent *) ;

	private:
 		void init();
		void calculateTrip(const QWhereaboutsUpdate &);
		void calculateWayPoint(const QWhereaboutsUpdate &);
		void createMenus();
		qreal distance3d(const QWhereaboutsCoordinate& from, const QWhereaboutsCoordinate& to);
		void setMetric(bool);

		Ui::MainWindow ui;
		Compass *compass;

		bool hidden;
		QWhereaboutsUpdate last_update;
		QWhereaboutsUpdate saved_update;
		QWhereaboutsUpdate current_update;
		QWhereaboutsUpdate way_point;
		QWhereabouts *whereabouts;
		QTime running_time;
		qreal distance;
		bool running;
		bool use_metric;
		double speed_threshold;
		int distance_sensitivity;
};

#endif
