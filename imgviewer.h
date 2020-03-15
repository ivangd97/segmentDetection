/*
 *    Copyright (C) 2020 by RoboLab - University of Extremadura
 *
 */

#ifndef ImgViewer_H
#define ImgViewer_H

#include <QtGui>
#include <stdint.h>
#include <iostream>

#include <QGLWidget>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

class ImgViewer : public QGLWidget
{
Q_OBJECT
public:
    ImgViewer(int _width, int _height, uchar *img=NULL, QWidget *parent = 0);
    ImgViewer(int _width, int _height, uchar *img=NULL, QImage::Format format=QImage::Format_Indexed8, QWidget *parent = 0);
    ImgViewer(int _width, int _height, QImage *img=NULL, QWidget *parent = 0);
    ImgViewer(int _width, int _height, QWidget *parent = 0);
    ImgViewer(QImage *img, QWidget *parent = 0);
    ImgViewer(Mat *img, QWidget *parent = 0);
    ImgViewer(QWidget *parent = 0);
    ImgViewer(const QRect & win_, QWidget *parent = 0);
    ~ImgViewer();
	void init();
	void setImage(QImage *img);
    void setImage(Mat *img);
	void paintEvent(QPaintEvent *);
	void setWindow(const QRect & win_) { effWin = win = win_; }
	void drawSquare(const QRect &, const QColor &,  bool fill=false, int id= -1, float rads=0, float width=0);
	void drawSquare(const QPoint &, int sideX, int sideY, const QColor &,  bool fill=false , int id= -1, float rads=0, float width=0);
	void drawSquare(const QPointF &, int sideX, int sideY, const QColor &,  bool fill=false , int id= -1, float rads=0, float width=0);
	void drawLine(const QLine &line, const QColor & c, float width=0);
	void drawLine(const QLineF &line, const QColor & c, float width=0);
	void drawPolyLine(const QVector<QPoint> & pline, const QColor & c, int width=1);
	void drawPolyLine(const QVector<int> & xs, const QVector<int> & ys, const QColor & c, int width=1);
	void drawEllipse(const QRect &, const QColor &, bool fill= false, int id =-1, float rads=0);
	void drawEllipse(const QPointF &, int radiusX, int radiusY, const QColor &, bool fill=false, int id =-1, float rads=0);
	void drawEllipse(const QPoint &, int radiusX, int radiusY, const QColor &, bool fill=false, int id =-1, float rads=0);
	void drawText(const QPoint & pos, const QString & text, int size, const QColor & color);
	void scaleImage(float sscale) { imageScale = sscale; setFixedSize(sscale*width, sscale*height); }

	QRectF getWindow() { return win;}

	uint32_t getWidth() { return width; }
	uint32_t getHeight() { return height; }
	void autoResize();
	uchar *imageBuffer() { if (qimg != NULL) return qimg->bits(); return NULL; }
	

protected:
	float imageScale;

	struct TRect
	{
		QRect rect;
		QColor color;
		int id;
		float ang;
		bool fill;
		float width;
	};

	struct TEllipse
	{
		QRect rect;
		QPointF center;
		float rx, ry;
		QColor color;
		int id;
		bool fill;
		float ang;
	};

	struct TLine
	{
		QLineF line;
		QColor color;
		float width;
	};

	struct TText
	{
		QPoint pos;
		int size;
		QColor color;
		QString text;
		float width;
	};

	int width, height;
	QRectF win;
	QRectF effWin;
	QQueue<TRect> squareQueue;
	QQueue<TLine> lineQueue;
	QQueue<TEllipse> ellipseQueue;
	QQueue<TText> textQueue;

	QImage *qimg;
    Mat *ocvimg;
	QVector<QRgb> ctable; //For gray conversion
	QPoint inicio, actual;
	bool translating;
	QPointF backPos;

    QPointF iniCoorSelected, endCoorSelected;
    bool onSelection;

signals:
    void windowSelected(QPointF center, int sizeX, int sizeY);
    void pressEvent();
protected:
	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
};

#endif
