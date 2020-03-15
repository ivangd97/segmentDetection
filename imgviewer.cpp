/*
 *    Copyright (C) 2020 by RoboLab - University of Extremadura
 *
 */
#include "imgviewer.h"

ImgViewer::ImgViewer( int _width, int _height, uchar *img, QWidget *parent) : QGLWidget(parent), width(_width), height(_height)
{
    ocvimg = new Mat();
    resize ( width, height );
    win.setRect ( 0, 0, width, height );

    if ( img != NULL )
        qimg = new QImage ( img, width, height, QImage::Format_Indexed8 );
    else
        qimg = NULL;

    init();
}

ImgViewer::ImgViewer( int _width, int _height, uchar *img, QImage::Format format , QWidget *parent) : QGLWidget(parent), width(_width), height(_height)
{
    ocvimg = new Mat();
    resize ( width, height );
    win.setRect ( 0, 0, width, height );

    if ( img != NULL )
        qimg = new QImage ( img, width, height, format );
    else
        qimg = NULL;
    init();
}

ImgViewer::ImgViewer(int _width, int _height, QImage *img, QWidget *parent) : QGLWidget(parent), width (_width), height (_height)
{
    ocvimg = new Mat();
    resize (width,height );
    win.setRect ( 0, 0, width, height );

    if (img)
        qimg = img;
    else
        qimg = NULL;

    init();
}

ImgViewer::ImgViewer(QImage * img, QWidget *parent) : QGLWidget ( parent )
{
    ocvimg = new Mat();
    if (parent)
    {
        width = parent->width();
        height = parent->height();
    }
    win.setRect ( 0, 0, width, height );

    if (img)
        qimg = img;
    else
        qimg = NULL;

    init();
}

ImgViewer::ImgViewer(Mat * img, QWidget *parent) : QGLWidget ( parent )
{
    if (!img->empty())
    {
        ocvimg = img;
        width = ocvimg->cols;
        height = ocvimg->rows;
        qimg = new QImage ( width, height, QImage::Format_RGB888 );
    }
    else
    {
        if (parent)
        {
            width = parent->width();
            height = parent->height();
        }
        qimg = NULL;
    }

    resize (width,height );
    win.setRect ( 0, 0, width, height );

    init();
}


ImgViewer::ImgViewer(int _width, int _height, QWidget * parent): QGLWidget (parent), width (_width), height (_height)
{
	resize ( _width, _height );
	win.setRect ( 0,0,width,height );

    ocvimg = new Mat();
	qimg = NULL;
	init();
}

ImgViewer::ImgViewer(QWidget * parent): QGLWidget (parent)
{
	if (parent)
	{
		setParent ( parent );
		resize ( parent->width(), parent->height() );
		win.setRect ( 0,0,parent->width(),parent->height() );
	}
    ocvimg = new Mat();
	qimg=NULL;
	init();
}


ImgViewer::ImgViewer(const QRect &win_, QWidget *parent) : QGLWidget(parent)
{
	if (parent)
	{
		setParent ( parent );
		width = parent->width();
		height = parent->height();
		resize ( parent->width(), parent->height() );
	}
	setWindow ( win_ );
    ocvimg = new Mat();
	qimg=NULL;
	init();
}

ImgViewer::~ImgViewer()
{
}

void ImgViewer::autoResize()
{ 
	if (parent())
	{
		setFixedSize(parentWidget()->width(), parentWidget()->height()); 
		width = parentWidget()->width(); 
		height = parentWidget()->height();
		qimg->scaled(width,height);
	}
}

void ImgViewer::init( )
{
	imageScale = 1.;
	
	if (qimg!=NULL)
	{
		imageScale = width/qimg->width();
	}
	else
	{
		qimg = new QImage ( width, height, QImage::Format_Indexed8 );
		qimg->fill ( 240 );
	}

	//Gray color table
	ctable.resize ( 256 );
	for ( int i=0; i < 256; i++ )
		ctable[i] = qRgb ( i,i,i );
	qimg->setColorTable ( ctable );
	translating = false;
	effWin = win;
	QGLFormat f = format();
	if (f.sampleBuffers())
	{
		f.setSampleBuffers( true );
		setFormat( f );
	}

    onSelection = false;
	show();


}


void ImgViewer::setImage(QImage* img)
{
	if (qimg != NULL) 
		delete qimg;
	
	qimg = img;
}

void ImgViewer::setImage(Mat *img)
{

    if (!img->empty())
    {
        if (qimg != NULL)
            delete qimg;

        ocvimg = img;
        width = ocvimg->cols;
        height = ocvimg->rows;
        qimg = new QImage ( width, height, QImage::Format_RGB888 );

        resize (width,height );
        win.setRect ( 0, 0, width, height );

    }

}


void ImgViewer::paintEvent ( QPaintEvent * )
{
	QString s;
	QPainter painter ( this );
	painter.setRenderHint(QPainter::HighQualityAntialiasing);

    if(!ocvimg->empty())
    {
        Mat auxImage;
        switch(ocvimg->type())
        {
            case CV_8UC1:
                cvtColor(*ocvimg, auxImage, COLOR_GRAY2RGB);
                memcpy(qimg->bits(), auxImage.data , width*height*3*sizeof(uchar));
                break;
            case CV_8UC3:
                memcpy(qimg->bits(), ocvimg->data , width*height*3*sizeof(uchar));
                break;

        }

    }

    if ( qimg != NULL )
    {
            painter.drawImage ( QRectF(0., 0., imageScale*width, imageScale*height), *qimg, QRectF(0, 0, width, height) );
    }

    if(onSelection)
        drawSquare((iniCoorSelected+endCoorSelected)/2, abs(endCoorSelected.x()-iniCoorSelected.x()),abs(endCoorSelected.y()-iniCoorSelected.y()), Qt::green );


    painter.setWindow (effWin.toRect() );

	//Draw lines
	while ( !lineQueue.isEmpty() )
	{
		TLine l = lineQueue.dequeue();
		painter.setPen ( QPen ( QBrush ( l.color ),l.width ) );
		painter.drawLine ( l.line );
	}

	//Draw ellipses
	while ( !ellipseQueue.isEmpty() )
	{
		TEllipse e = ellipseQueue.dequeue();
		if ( e.fill == true )
			painter.setBrush ( e.color );
		else
			painter.setBrush ( Qt::transparent );
		painter.setPen ( e.color );
		if (fabs(e.ang) > 0.1)
		{
			painter.setPen ( e.color );
			painter.translate( e.center );
			painter.rotate( e.ang );
			painter.drawEllipse ( QPointF(0,0), e.rx, e.ry );
			painter.rotate( -e.ang );
			painter.translate( -e.center );
		}
		else
			painter.drawEllipse( e.center, e.rx, e.ry);
		if ( e.id>=0 )
		{
			painter.drawText ( e.rect, Qt::AlignCenter, s.setNum ( e.id ) );
		}
	}

	//Draw squares

    QPen pen = painter.pen();
    int penwidth = pen.width();
    while ( !squareQueue.isEmpty() )
    {
        TRect r = squareQueue.dequeue();
        if ( r.fill == true )
            painter.setBrush ( r.color );
        else
            painter.setBrush ( Qt::transparent );
        pen.setColor(r.color);
        pen.setWidth(r.width);
        painter.setPen(pen);
        if (fabs(r.ang) > 0.01 )
        {
            QPoint center = r.rect.center();
            painter.translate( center );
            painter.rotate( r.ang );
            painter.drawRoundedRect ( QRect( r.rect.topLeft() - center, r.rect.size() ) , 40 , 40 );
            painter.rotate( -r.ang );
            painter.translate( -center );
        }
        else
        {
            painter.drawRect( r.rect );
        }
        if ( r.id>=0 )
            painter.drawText ( QPointF ( r.rect.x(), r.rect.y() ),  s.setNum ( r.id ) );
    }
    pen.setWidth(penwidth);
    painter.setPen(pen);


	//Draw text
	while ( !textQueue.isEmpty() )
	{
		TText t = textQueue.dequeue();
		painter.setBrush ( Qt::transparent );
		painter.setPen ( t.color );
		painter.setWindow ( effWin.normalized().toRect() );
		QFont ant = painter.font();
		QFont temp ( "Helvetica", t.size );
		painter.setFont ( temp );
		painter.drawText( QRect(t.pos.x(), t.pos.y(), 0.82*t.text.size()*t.size, 1.2*t.size), Qt::AlignCenter, t.text);

		painter.setFont ( ant );
		painter.setWindow ( effWin.toRect() );
	}


}


void ImgViewer::drawSquare ( const QRect &rect, const QColor & col, bool fill, int id, float rot, float width)
{
	TRect r;
	r.rect = rect;
	r.color= col;
	r.id = id;
	r.ang = rot;
	r.fill = fill;
	r.width = width;
	squareQueue.enqueue ( r );
}


void ImgViewer::drawSquare ( const QPoint & center, int sideX, int sideY, const QColor & col, bool fill, int id, float rads, float width)
{
	TRect r;
	r.rect = QRect ( center.x()-sideX/2, (center.y()-sideY/2), sideX, sideY );
	r.rect.moveCenter(center);
	r.color= col;
	r.id = id;
	r.fill = fill;
	r.ang = rads*180./M_PI;
	r.width = width;
	squareQueue.enqueue ( r );
}

void ImgViewer::drawSquare ( const QPointF & center, int sideX, int sideY, const QColor & col, bool fill, int id, float rads, float width)
{
	TRect r;
	r.rect = QRect ( center.x()-sideX/2, center.y()-sideY/2, sideX, sideY );
	r.color= col;
	r.id = id;
	r.fill = fill;
	r.ang = rads*180./M_PI;
	r.width = width;
	squareQueue.enqueue ( r );
}


void ImgViewer::drawLine ( const QLine & line, const QColor & c, float width )
{
	TLine l;
	l.line=line;
	l.color=c;
	l.width = width;
	lineQueue.enqueue ( l );
}

void ImgViewer::drawLine ( const QLineF & line, const QColor & c, float width )
{
	TLine l;
	l.line=line;
	l.color=c;
	l.width = width;
	lineQueue.enqueue ( l );
}


void ImgViewer::drawPolyLine ( const QVector< QPoint > & pline, const QColor & c, int width )
{
	TLine l;
	if ( pline.size() > 1 )
	{
		for ( int i=1; i< pline.size(); i++ )
		{
			l.line.setPoints ( pline[i-1],pline[i] );
			l.color=c;
			l.width = width;
			lineQueue.enqueue ( l );
		}
	}
}

void ImgViewer::drawPolyLine ( const QVector< int > & xs, const QVector< int > & ys, const QColor & c, int width )
{
	TLine l;
	QPoint pant;
	if ( xs.size() > 1 )
	{
		pant.setX ( xs[0] );
		pant.setY ( ys[0] );
		for ( int i=1; i< xs.size(); i++ )
		{
			l.line.setPoints ( pant,QPoint ( xs[i],ys[i] ) );
			l.color=c;
			l.width = width;
			lineQueue.enqueue ( l );
			pant.setX ( xs[i] );
			pant.setY ( ys[i] );
		}
	}
}


void ImgViewer::drawEllipse ( const QRect & rect, const QColor &col, bool fill, int id , float rads)
{
	TEllipse e;
    e.center = QPoint(rect.x()+rect.width()/2, rect.y()+rect.height()/2);
    e.rx = rect.width();
    e.ry = rect.height();
	e.rect = rect;
	e.color= col;
	e.id = id;
	e.fill = fill;
	e.ang = rads;
	ellipseQueue.enqueue ( e );
}

void ImgViewer::drawEllipse ( const QPoint & centro, int radiusX,  int radiusY, const QColor & col, bool fill, int id ,  float rads)
{
	TEllipse e;
	e.center = centro;
	e.rx = radiusX;
	e.ry = radiusY;
	e.rect = QRect(centro.x()-radiusX, centro.y()-radiusY, radiusX*2, radiusY*2);
	e.color= col;
	e.id = id;
	e.fill = fill;
	e.ang = rads*180./M_PI;
	ellipseQueue.enqueue ( e );

}

void ImgViewer::drawEllipse ( const QPointF & centro, int radiusX,  int radiusY, const QColor & col, bool fill, int id ,  float rads)
{
	TEllipse e;
	e.center = centro;
	e.rx = radiusX;
	e.ry = radiusY;
	e.color= col;
	e.id = id;
	e.fill = fill;
	e.ang = rads*180./M_PI;
	ellipseQueue.enqueue ( e );
}


void ImgViewer::drawText ( const QPoint & pos, const QString & text, int size, const QColor & color )
{
	TText t;
	t.pos = pos;
	t.text = text;
	t.size = size;
	t.color = color;
	textQueue.enqueue ( t );
}



///Mouse events

void ImgViewer::mousePressEvent ( QMouseEvent *e )
{
    if ( e->button() == Qt::LeftButton )
	{
            iniCoorSelected.setX(e->x());
            iniCoorSelected.setY(e->y());
            endCoorSelected.setX(e->x());
            endCoorSelected.setY(e->y());

            onSelection = true;
            emit pressEvent();
	}
}

void ImgViewer::mouseMoveEvent(QMouseEvent *e)
{
       endCoorSelected.setX(e->x());
       endCoorSelected.setY(e->y());
}

void ImgViewer::mouseReleaseEvent ( QMouseEvent *e )
{
        if (e->button() == Qt::LeftButton)
                emit windowSelected((iniCoorSelected+endCoorSelected)/2, abs(endCoorSelected.x()-iniCoorSelected.x()),abs(endCoorSelected.y()-iniCoorSelected.y()));
        onSelection = false;
}

