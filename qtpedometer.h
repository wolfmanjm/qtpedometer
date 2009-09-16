#ifndef QTPEDOMETER_H
#define QTPEDOMETER_H

#include <QMutex>
#include <QTimer>

#include "ui_qtpedometer.h"

class QtPedometer : public QWidget
{
	Q_OBJECT

	public:
		QtPedometer(QWidget *parent = 0, Qt::WFlags f = 0);
		virtual ~QtPedometer();

		void init();

	protected:
		void paintEvent(QPaintEvent *event);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void closeEvent (QCloseEvent *) ;

	private:
		Ui::MainWindow ui;
		QMutex mutex;

		QString sentence;
		time_t timer;	/* time of last state change */
		int state;	/* or MODE_NO_FIX=1, MODE_2D=2, MODE_3D=3 */
		bool hidden;
};

#endif
