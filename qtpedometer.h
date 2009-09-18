#ifndef QTPEDOMETER_H
#define QTPEDOMETER_H

#include <QWhereabouts>
#include <QWhereaboutsFactory>

#include "ui_qtpedometer.h"

class QtPedometer : public QWidget
{
	Q_OBJECT

	public:
		QtPedometer(QWidget *parent = 0, Qt::WFlags f = 0);
		virtual ~QtPedometer();

	private slots:
		void updated(const QWhereaboutsUpdate &update);
		void stateChanged(QWhereabouts::State state);
		void resetData();
		void startData();
		void pauseData();
		void saveTrip();
		void setWayPoint();
		void clearWayPoint();

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

		Ui::MainWindow ui;
		bool hidden;
		QWhereaboutsUpdate last_update;
		QWhereaboutsUpdate current_update;
		QWhereaboutsUpdate way_point;
		QWhereabouts *whereabouts;
		int update_count;
		bool valid_update;
		QTime running_time;
		qreal distance;
		bool running;
		qreal horiz_accuracy, speed_accuracy;
		qreal speed_threshold;
};

#endif
