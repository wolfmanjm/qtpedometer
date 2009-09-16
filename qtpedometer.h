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

	public slots:
 		void init();
	
	private slots:
		void updated(const QWhereaboutsUpdate &update);
		void stateChanged(QWhereabouts::State state);

	protected:
		void paintEvent(QPaintEvent *event);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void closeEvent (QCloseEvent *) ;

	private:
		Ui::MainWindow ui;
		QMutex mutex;
		bool hidden;
		QWhereaboutsUpdate lastUpdate;
};

#endif
