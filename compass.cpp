#include <QtGui>

#include "compass.h"

Compass::Compass(QWidget *parent) : QWidget(parent)
{
	bearing= 0.0;
	azimuth= 0.0;
	show_azimuth= false;
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
		QPoint(0, -70)
	};

	// A nice vector arrow found online
	static const QPoint directionPointer[12] = {
		QPoint(-14, 30),
		QPoint(-5, 38),
		QPoint(0, 46),
		QPoint(5, 38),
		QPoint(14, 30),
		QPoint(4, 34),
		QPoint(4, -36),
		QPoint(14,-46),
		QPoint(0, -40),
		QPoint(-14, -46),
		QPoint(-4,-36),
		QPoint(-4, 34)
	};

	QColor northColor(Qt::white);
	QColor southColor(127, 0, 0);
	QColor azimuthColor(Qt::green);

	int side = qMin(width(), height());

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.translate(width() / 2, height() / 2);
	painter.scale(side / 200.0, side / 200.0);

	// make the background black
	QRect r1(-100, -100, 200, 200);
	QBrush br1(Qt::black);
	painter.fillRect(r1, br1);

	// Draw direction of motion pointer
	painter.save();
	painter.scale(2.0, 2.0);
	painter.rotate(180.0);
	painter.setPen(Qt::white);
	painter.drawConvexPolygon(directionPointer, 12);
	painter.restore();

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

	// Draw Labels N, E, S, W
	static const QString labels[4]={
		QString("N"),
		QString("E"),
		QString("S"),
		QString("W")
	};

	painter.save();
	QFont serifFont("Times", 6);
	painter.setFont(serifFont);
	painter.setPen(Qt::white);
	QRect labelRect(-16, -100, 32, 32);
	for(int i=0;i<4;i++){
		painter.drawText(labelRect, Qt::AlignHCenter, labels[i]);
		painter.rotate(90.0);
	}

	painter.restore();
	painter.restore();

	
	// draw bezel
	painter.setPen(Qt::white);
	QRect center(-99, -99, 199, 199);
	painter.drawEllipse(center);

}
