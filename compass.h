#ifndef COMPASS_H
#define COMPASS_H

#include <QWidget>

class Compass : public QWidget
{
	Q_OBJECT
		
 public:
	Compass(QWidget *parent = 0);
	void setBearing(qreal);
    void setAzimuth(qreal);
    void showAzimuth(bool);

 protected:
	void paintEvent(QPaintEvent *event);

 private:
	qreal bearing;
	qreal azimuth;
	bool show_azimuth;
};

#endif

