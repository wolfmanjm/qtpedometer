#include <QtGui>

#include "compass.h"

Compass::Compass(QWidget *parent) : QWidget(parent)
{
	bearing= -45.0;
	azimuth= 90.0;
	show_azimuth= true;
}

void Compass::setBearing(qreal pt)
{
	bearing= -pt;
	update();
}

void Compass::setAzimuth(qreal pt)
{
	azimuth= pt;
	update();
}

void Compass::showAzimuth(bool flg)
{
	show_azimuth= flg;
	update();
}

void Compass::paintEvent(QPaintEvent *)
{
	//qDebug("In compass paint");

	static const QPoint northPointer[3] = {
		QPoint(8, 0),
		QPoint(-8, 0),
		QPoint(0, -70)
	};
	static const QPoint southPointer[3] = {
		QPoint(8, 0),
		QPoint(-8, 0),
		QPoint(0, -70)
	};
	static const QPoint azimuthPointer[3] = {
		QPoint(7, 0),
		QPoint(-7, 0),
		QPoint(0, -50)
	};

	QColor northColor(127, 0, 0);
	QColor southColor(0, 127, 0);
	QColor azimuthColor(0, 0, 127, 191);

	int side = qMin(width(), height());

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.translate(width() / 2, height() / 2);
	painter.scale(side / 200.0, side / 200.0);

	// Draw north & south pointer
	painter.setPen(Qt::NoPen);

	painter.save();

	painter.rotate(bearing);
	painter.setBrush(northColor);
	painter.drawConvexPolygon(northPointer, 3);
	painter.save();
	painter.rotate(180.0);
	painter.setBrush(southColor);
	painter.drawConvexPolygon(southPointer, 3);
	painter.restore();

	// Draw azimuth pointer
	if(show_azimuth){
		painter.setPen(Qt::NoPen);
		painter.setBrush(azimuthColor);
		painter.save();
		painter.rotate(azimuth);
		painter.drawConvexPolygon(azimuthPointer, 3);
		painter.restore();
	}
	painter.restore();

	// Draw ticks
	painter.setPen(northColor);

	for (int i = 0; i < 4; ++i) {
		painter.drawLine(88, 0, 96, 0);
		painter.rotate(90.0);
	}

	painter.setPen(southColor);

	painter.rotate(45.0);
	for (int j = 0; j < 40; ++j) {
		painter.drawLine(92, 0, 96, 0);
		painter.rotate(90.0);
	}
	
	// draw bezel
	painter.setPen(Qt::black);
	QRect center(-99, -99, 199, 199);
	painter.drawEllipse(center);

}
