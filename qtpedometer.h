#ifndef QTPEDOMETER_H
#define QTPEDOMETER_H

#include <QMutex>
#include <QTimer>
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

	protected:
		void paintEvent(QPaintEvent *event);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void closeEvent (QCloseEvent *) ;

	private:
 		void init();
		void calculateAverages(const QWhereaboutsCoordinate &);

		Ui::MainWindow ui;
		QMutex mutex;
		bool hidden;
		QWhereaboutsUpdate last_update;
		QWhereabouts *whereabouts;
		int update_count;
		bool valid_update;
		QTime running_time;
		qreal distance;
		bool running;
};

#endif
