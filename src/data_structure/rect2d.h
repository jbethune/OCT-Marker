#ifndef RECT2D_H
#define RECT2D_H

#include<data_structure/point2d.h>

#include<QRect>

template<typename T>
class Rect2DBase
{
	T xMin;
	T xMax;
	T yMin;
	T yMax;
public:
	explicit Rect2DBase(const Point2DBase<T>& p)
	: xMin(p.getX())
	, xMax(p.getX())
	, yMin(p.getY())
	, yMax(p.getY())
	{}

	explicit Rect2DBase(const QRect& r)
	: xMin(r.x())
	, xMax(r.x() + r.width())
	, yMin(r.y())
	, yMax(r.y() + r.height())
	{}

	Rect2DBase& operator+=(const Point2DBase<T>& p)
	{
		const T x = p.getX();
		const T y = p.getY();
		if(x < xMin)
			xMin = x;
		else if(x > xMax)
			xMax = x;
		if(y < yMin)
			yMin = y;
		else if(y > yMax)
			yMax = y;
		return *this;
	}

	void addBroder(T d)
	{
		xMin -= d;
		yMin -= d;
		xMax += d;
		yMax += d;
	}

	void scaleXY(T factorX, T factorY)
	{
		xMin /= factorX;
		xMax /= factorX;
		yMin /= factorY;
		yMax /= factorY;
	}

	QRect toQRect() const { return QRect(xMin, yMin, xMax-xMin, yMax-yMin); }

	T getXMin()                                               const { return xMin; }
	T getXMax()                                               const { return xMax; }
	T getYMin()                                               const { return yMin; }
	T getYMax()                                               const { return yMax; }
};

typedef Rect2DBase<int> Rect2DInt;
typedef Rect2DBase<double> Rect2D;

#endif // RECT2D_H
