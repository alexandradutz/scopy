/* -*- c++ -*- */
/*
 * Copyright 2012 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "DisplayPlot.h"
#include "osc_scale_engine.h"

#include <qwt_scale_engine.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_legend.h>
#include <qwt_plot_layout.h>

#include <QColor>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <QDebug>

using namespace adiscope;

/*
 * OscScaleDraw class implementation
 */

OscScaleDraw::OscScaleDraw():
	m_floatPrecision(3),
	m_unit(""),
	m_metricPrefix(""),
	m_magnitude(1.0),
	m_formatter(NULL)
{
}

OscScaleDraw::OscScaleDraw(PrefixFormatter *formatter, QString unitType):
	m_floatPrecision(3),
	m_unit(unitType),
	m_metricPrefix(""),
	m_magnitude(1.0),
	m_formatter(formatter)
{
	if (unitType.isEmpty())
		if (formatter) {
			double tmp;
			formatter->getFormatAttributes(1.0, m_metricPrefix, tmp);
		}
}

void OscScaleDraw::setFloatPrecision(unsigned int numDigits)
{
	m_floatPrecision = numDigits;
}

unsigned int OscScaleDraw::getFloatPrecison()
{
	return m_floatPrecision;
}

void OscScaleDraw::setUnitType(QString unit)
{
	if (m_unit != unit) {
		m_unit = unit;

		// Trigger a new redraw of scale labels since there's a new unit that needs to be redrawn
		invalidateCache();
	}
}

QString OscScaleDraw::getUnitType()
{
	return m_unit;
}

QwtText OscScaleDraw::label( double value ) const
{
	return QLocale().toString( value / m_magnitude , 'f', m_floatPrecision) + ' ' + m_metricPrefix + m_unit;
}

void OscScaleDraw::draw(QPainter *painter , const QPalette& palette) const
{
	QwtScaleDiv scDiv = scaleDiv();

	updateMetrics();

	painter->save();
	QPen pen = painter->pen();
	pen.setWidth( penWidth() );
	pen.setCosmetic( false );
	painter->setPen( pen );

	if ( hasComponent( QwtAbstractScaleDraw::Labels ) )
	{
	    painter->save();
	    painter->setPen( palette.color( QPalette::Text ) ); // ignore pen style

	    const QList<double> &majorTicks =
		scDiv.ticks( QwtScaleDiv::MajorTick );

	    for ( int i = 0; i < majorTicks.count(); i++ )
	    {
		const double v = majorTicks[i];
		if ( scDiv.contains( v ) )
		    drawLabel( painter, v );
	    }

	    painter->restore();
	}

	if ( hasComponent( QwtAbstractScaleDraw::Ticks ) )
	{
	    painter->save();

	    QPen pen = painter->pen();
	    pen.setColor( palette.color( QPalette::WindowText ) );
	    pen.setCapStyle( Qt::FlatCap );

	    painter->setPen( pen );

	    for ( int tickType = QwtScaleDiv::MinorTick;
		tickType < QwtScaleDiv::NTickTypes; tickType++ )
	    {
		const double tickLen = tickLength((QwtScaleDiv::TickType)tickType);
		if ( tickLen <= 0.0 )
		    continue;

		const QList<double> &ticks = scDiv.ticks( tickType );
		for ( int i = 0; i < ticks.count(); i++ )
		{
		    const double v = ticks[i];
		    if ( scDiv.contains( v ) )
			drawTick( painter, v, tickLen );
		}
	    }

	    painter->restore();
	}

	if ( hasComponent( QwtAbstractScaleDraw::Backbone ) )
	{
	    painter->save();

	    QPen pen = painter->pen();
	    pen.setColor( palette.color( QPalette::WindowText ) );
	    pen.setCapStyle( Qt::FlatCap );

	    painter->setPen( pen );

	    drawBackbone( painter );

	    painter->restore();
	}

	painter->restore();
}

void OscScaleDraw::drawBackbone(QPainter *) const
{
	// Skip drawing the baseline of the scale
}

void OscScaleDraw::drawTick(QPainter *, double, double) const
{
	// Skip drawing the tikcs
}

void OscScaleDraw::updateMetrics() const
{
	const QwtScaleDiv scDiv = scaleDiv();
	double lower = fabs(scDiv.lowerBound());
	double upper = fabs(scDiv.upperBound());
	double max = (upper > lower) ? upper : lower;

	if (m_formatter)
		m_formatter->getFormatAttributes(max, m_metricPrefix, m_magnitude);
}

void OscScaleDraw::invalidateTheCache()
{
	invalidateCache();
}


/*
 * PlotItemScaleDraw class implementation
 */


PlotItemScaleDraw::PlotItemScaleDraw(): QwtScaleDraw()
{
}

QwtText PlotItemScaleDraw::label(double) const
{
	// Skip drawing tick labels
	return QwtText("");
}

void PlotItemScaleDraw::drawBackbone( QPainter * ) const
{
	// Skip drawing the baseline of the scale
}

/*
 * EdgelessPlotScaleItem class implementation
 */
EdgelessPlotScaleItem::EdgelessPlotScaleItem(
	QwtScaleDraw::Alignment alignment, const double pos ):
    QwtPlotScaleItem(alignment, pos)
{
}

void EdgelessPlotScaleItem::updateScaleDiv( const QwtScaleDiv& xScaleDiv,
    const QwtScaleDiv& yScaleDiv )
{
	QwtPlotScaleItem::updateScaleDiv(getEdgelessScaleDiv(xScaleDiv),
					getEdgelessScaleDiv(yScaleDiv));
}

/*
 * EdgelessPlotGrid class implementation
 */
EdgelessPlotGrid::EdgelessPlotGrid():
    QwtPlotGrid()
{
}

void EdgelessPlotGrid::updateScaleDiv( const QwtScaleDiv& xScaleDiv,
    const QwtScaleDiv& yScaleDiv )
{
	QwtPlotGrid::updateScaleDiv(getEdgelessScaleDiv(xScaleDiv),
					getEdgelessScaleDiv(yScaleDiv));
}

OscPlotZoomer::OscPlotZoomer(QWidget *parent, bool doReplot) :
	QwtPlotZoomer(parent, doReplot)
{
}

void OscPlotZoomer::rescale()
{
	DisplayPlot *plt = static_cast<DisplayPlot *>(plot());

	if ( !plt )
	    return;

	const QStack<QRectF> &stack = zoomStack();
	uint index = zoomRectIndex();
	const QRectF &rect = stack[index];
	if ( rect != scaleRect() )
	{
	    const bool doReplot = plt->autoReplot();
	    plt->setAutoReplot( false );

	    double x1 = rect.left();
	    double x2 = rect.right();

	    if ( !plt->axisScaleDiv( xAxis() ).isIncreasing() )
		qSwap( x1, x2 );

	    double width = fabs(x1 - x2);
	    plt->setHorizUnitsPerDiv(width / plt->xAxisNumDiv());
	    plt->setHorizOffset(x1 + (width / 2));

	    double y1 = rect.top();
	    double y2 = rect.bottom();
	    if ( !plt->axisScaleDiv( yAxis() ).isIncreasing() )
		qSwap( y1, y2 );

	    double height = fabs(y1 - y2);
	    plt->setVertUnitsPerDiv(height / plt->yAxisNumDiv());
	    plt->setVertOffset(y1 + (height / 2));

	    plt->setAutoReplot( doReplot );

	    plt->replot();
	}
}

/*
 * PlotAxisConfiguration
 */
PlotAxisConfiguration::PlotAxisConfiguration(int axisPos, int axisIdx, DisplayPlot *plot):
	d_axis(QwtAxisId(axisPos, axisIdx)), d_plot(plot)
{
	Qt::CursorShape shape;

	switch (axisPos) {
		case QwtPlot::yLeft:
			shape = Qt::CursorShape::SizeVerCursor;
		break;

		case QwtPlot::xBottom:
			shape = Qt::CursorShape::SizeHorCursor;
		break;

		default:
			return;
	}

	// Set cursor shape when hovering over the axis
	setCursorShapeOnHover(shape);

	// Avoid jumping when labels with more/less digits
	// appear/disappear when scrolling vertically
	if (axisPos == QwtPlot::yLeft) {
		const QFontMetrics fm(d_plot->axisWidget(d_axis)->font());
		QwtScaleDraw *scaleDraw = d_plot->axisScaleDraw(d_axis);
		scaleDraw->setMinimumExtent( fm.width("100.00") );
	}

	// This helps creating a fixed 5 X 5 grid
	d_plot->setAxisScale(d_axis, -5.0, 5.0, 1);

	d_ptsPerDiv = 1.0;
	d_offset = 0.0;

	if (axisPos == QwtPlot::yLeft) {
		d_mouseGestures = new VertMouseGestures(d_plot->axisWidget(d_axis), d_axis);
		d_mouseGestures->setEnabled(false);

		QObject::connect(this->d_mouseGestures, SIGNAL(wheelDown(int)),
			d_plot, SLOT(vertAxisScaleIncrease()));
		QObject::connect(this->d_mouseGestures, SIGNAL(wheelUp(int)),
			d_plot, SLOT(vertAxisScaleDecrease()));

		QObject::connect(this->d_mouseGestures, SIGNAL(upMovement(double)),
			d_plot, SLOT(onVertAxisOffsetDecrease()));
		QObject::connect(this->d_mouseGestures, SIGNAL(downMovement(double)),
			d_plot, SLOT(onVertAxisOffsetIncrease()));
	} else if (axisPos == QwtPlot::xBottom) {
		d_mouseGestures = new HorizMouseGestures(d_plot->axisWidget(d_axis), d_axis);
		d_mouseGestures->setEnabled(false);

		QObject::connect(this->d_mouseGestures, SIGNAL(wheelDown(int)),
			d_plot, SLOT(horizAxisScaleIncrease()));
		QObject::connect(this->d_mouseGestures, SIGNAL(wheelUp(int)),
			d_plot, SLOT(horizAxisScaleDecrease()));

		QObject::connect(this->d_mouseGestures, SIGNAL(rightMovement(double)),
			d_plot, SLOT(onHorizAxisOffsetDecrease()));
		QObject::connect(this->d_mouseGestures, SIGNAL(leftMovement(double)),
			d_plot, SLOT(onHorizAxisOffsetIncrease()));
	}
}

PlotAxisConfiguration::~PlotAxisConfiguration()
{
}

QwtAxisId& PlotAxisConfiguration::axis()
{
	return d_axis;
}

void PlotAxisConfiguration::setPtsPerDiv(double value)
{
	d_ptsPerDiv = value;
}

double PlotAxisConfiguration::ptsPerDiv()
{
	return d_ptsPerDiv;
}

void PlotAxisConfiguration::setOffset(double value)
{
	d_offset = value;
}

double PlotAxisConfiguration::offset()
{
	return d_offset;
}

void PlotAxisConfiguration::setCursorShapeOnHover(Qt::CursorShape shape)
{
	d_hoverCursorShape = shape;
	QwtScaleWidget *scaleWidget = d_plot->axisWidget(d_axis);
	scaleWidget->setCursor(shape);
}

void PlotAxisConfiguration::setMouseGesturesEnabled(bool en)
{
	d_mouseGestures->setEnabled(en);
}

/*
 * DisplayPlot class
 */

DisplayPlot::DisplayPlot(int nplots, QWidget* parent,
			 unsigned int xNumDivs, unsigned int yNumDivs)
  : QwtPlot(parent), d_nplots(nplots), d_stop(false)
{
  d_CurveColors << QColor("#ff7200") << QColor("#9013fe") << QColor(Qt::green)
       << QColor(Qt::cyan) << QColor(Qt::magenta)
       << QColor(Qt::yellow) << QColor(Qt::gray) << QColor(Qt::darkRed)
       << QColor(Qt::darkGreen) << QColor(Qt::darkBlue) << QColor(Qt::darkGray)
       << QColor(Qt::black);

  qRegisterMetaType<QColorList>("QColorList");
  resize(parent->width(), parent->height());

  d_autoscale_state = false;

  d_yAxisUnit = "";
  d_xAxisUnit = "";

  setXaxisNumDiv(xNumDivs);
  setYaxisNumDiv(yNumDivs);

  d_usingLeftAxisScales = true;

  // Disable polygon clipping
#if QWT_VERSION < 0x060000
  QwtPainter::setDeviceClipping(false);
#else
  QwtPainter::setPolylineSplitting(false);
#endif

#if QWT_VERSION < 0x060000
  // We don't need the cache here
  canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
  canvas()->setPaintAttribute(QwtPlotCanvas::PaintPacked, false);
#endif

  QColor default_palette_color = QColor("white");
  setPaletteColor(default_palette_color);

  d_panner = new QwtPlotPanner(canvas());
  d_panner->setAxisEnabled(QwtPlot::yRight, false);
  d_panner->setMouseButton(Qt::MidButton, Qt::ControlModifier);

  // emit the position of clicks on widget
  d_picker = new QwtDblClickPlotPicker(canvas());

#if QWT_VERSION < 0x060000
  connect(d_picker, SIGNAL(selected(const QwtDoublePoint &)),
      this, SLOT(onPickerPointSelected(const QwtDoublePoint &)));
#else
  d_picker->setStateMachine(new QwtPickerDblClickPointMachine());
  connect(d_picker, SIGNAL(selected(const QPointF &)),
	  this, SLOT(onPickerPointSelected6(const QPointF &)));
#endif

  // Configure horizontal axis
  bottomHorizAxisInit();

  // One vertical axis by default
  setLeftVertAxesCount(1);

  setActiveVertAxis(0);

  plotLayout()->setAlignCanvasToScales(true);

  // Use PlotItems to construct the plot axis
  EdgelessPlotScaleItem *scaleItem_r = new EdgelessPlotScaleItem(QwtScaleDraw::LeftScale);
  PlotItemScaleDraw *yLscaleDraw = new PlotItemScaleDraw();
  yLscaleDraw->setAlignment(QwtScaleDraw::LeftScale);
  scaleItem_r->setScaleDraw(yLscaleDraw);
  scaleItem_r->setFont(this->axisWidget(QwtPlot::yLeft)->font());
  QPalette palette_r = scaleItem_r->palette();
  palette_r.setBrush(QPalette::Foreground, QColor("#6E6E6F"));
  palette_r.setBrush(QPalette::Text, QColor("#6E6E6F"));
  scaleItem_r->setPalette(palette_r);
  scaleItem_r->setBorderDistance(0);
  scaleItem_r->attach(this);

  EdgelessPlotScaleItem *scaleItem_l = new EdgelessPlotScaleItem(QwtScaleDraw::RightScale);
  PlotItemScaleDraw *yRscaleDraw = new PlotItemScaleDraw();
  yRscaleDraw->setAlignment(QwtScaleDraw::RightScale);
  scaleItem_l->setScaleDraw(yRscaleDraw);
  scaleItem_l->setFont(this->axisWidget(QwtPlot::yLeft)->font());
  QPalette palette_l = scaleItem_l->palette();
  palette_l.setBrush(QPalette::Foreground, QColor("#6E6E6F"));
  palette_l.setBrush(QPalette::Text, QColor("#6E6E6F"));
  scaleItem_l->setPalette(palette_l);
  scaleItem_l->setBorderDistance(0);
  scaleItem_l->attach(this);

  EdgelessPlotScaleItem *scaleItem_t = new EdgelessPlotScaleItem(QwtScaleDraw::BottomScale);
  PlotItemScaleDraw *xBscaleDraw = new PlotItemScaleDraw();
  xBscaleDraw->setAlignment(QwtScaleDraw::BottomScale);
  scaleItem_t->setScaleDraw(xBscaleDraw);
  scaleItem_t->setFont(this->axisWidget(QwtPlot::yLeft)->font());
  QPalette palette_t = scaleItem_t->palette();
  palette_t.setBrush(QPalette::Foreground, QColor("#6E6E6F"));
  palette_t.setBrush(QPalette::Text, QColor("#6E6E6F"));
  scaleItem_t->setPalette(palette_t);
  scaleItem_t->setBorderDistance(0);
  scaleItem_t->attach(this);

  EdgelessPlotScaleItem *scaleItem_b = new EdgelessPlotScaleItem(QwtScaleDraw::TopScale);
  PlotItemScaleDraw *xTscaleDraw = new PlotItemScaleDraw();
  xTscaleDraw->setAlignment(QwtScaleDraw::TopScale);
  scaleItem_b->setScaleDraw(xTscaleDraw);
  scaleItem_b->setFont(this->axisWidget(QwtPlot::yLeft)->font());
  QPalette palette_b = scaleItem_b->palette();
  palette_b.setBrush(QPalette::Foreground, QColor("#6E6E6F"));
  palette_b.setBrush(QPalette::Text, QColor("#6E6E6F"));
  scaleItem_b->setPalette(palette_b);
  scaleItem_b->setBorderDistance(0);
  scaleItem_b->attach(this);

  this->plotLayout()->setCanvasMargin(0, QwtPlot::yLeft);
  this->plotLayout()->setCanvasMargin(0, QwtPlot::yRight);
  this->plotLayout()->setCanvasMargin(0, QwtPlot::xTop);
  this->plotLayout()->setCanvasMargin(0, QwtPlot::xBottom);

  ((QFrame*) canvas())->setLineWidth(0);

  // Avoid jumping when labels with more/less digits
  // appear/disappear when scrolling vertically

  QwtLegend* legendDisplay = new QwtLegend(this);

#if QWT_VERSION < 0x060100
  legendDisplay->setItemMode(QwtLegend::CheckableItem);
  insertLegend(legendDisplay);
  connect(this, SIGNAL(legendChecked(QwtPlotItem *, bool)),
	  this, SLOT(legendEntryChecked(QwtPlotItem *, bool)));
#else /* QWT_VERSION < 0x060100 */
  legendDisplay->setDefaultItemMode(QwtLegendData::Checkable);
  insertLegend(legendDisplay);
  connect(legendDisplay, SIGNAL(checked(const QVariant&, bool, int)),
	  this, SLOT(legendEntryChecked(const QVariant&, bool, int)));
#endif /* QWT_VERSION < 0x060100 */

  // Plot needs a grid
  d_grid = new EdgelessPlotGrid();
  QColor majorPenColor("#353537");
  d_grid->setMajorPen(majorPenColor, 1.0, Qt::DashLine);
  d_grid->attach(this);
}

DisplayPlot::~DisplayPlot()
{
	// d_zoomer and d_panner deleted when parent deleted

	// Since some curves may not be attached to the plot they won't get deleted
	for (auto it = d_plot_curve.begin(); it != d_plot_curve.end() ; ++it) {
		QwtPlotCurve * qpc = (*it);
		qpc->detach();
		delete qpc;
	}
}

void
DisplayPlot::disableLegend()
{
  // Haven't found a good way to toggle it on/off
  insertLegend(NULL);
}

void
DisplayPlot::setYaxis(double min, double max)
{
  setAxisScale(QwtPlot::yLeft, min, max);
  if(!d_autoscale_state)
    d_zoomer->setZoomBase();
}

void
DisplayPlot::setXaxis(double min, double max)
{
  setAxisScale(QwtPlot::xBottom, min, max);
  d_zoomer->setZoomBase();
}

void
DisplayPlot::setLineLabel(int which, QString label)
{
  d_plot_curve[which]->setTitle(label);
}

QString
DisplayPlot::getLineLabel(int which)
{
  return d_plot_curve[which]->title().text();
}

void
DisplayPlot::setLineColor(int which, QColor color)
{
    if (which < d_plot_curve.size()) {
    // Set the color of the pen
    QPen pen(d_plot_curve[which]->pen());
    pen.setColor(color);
    d_plot_curve[which]->setPen(pen);
    // And set the color of the markers
#if QWT_VERSION < 0x060000
    //d_plot_curve[which]->setBrush(QBrush(QColor(color)));
    d_plot_curve[which]->setPen(pen);
    QwtSymbol sym = (QwtSymbol)d_plot_curve[which]->symbol();
    setLineMarker(which, sym.style());
#else
    QwtSymbol *sym = (QwtSymbol*)d_plot_curve[which]->symbol();
    if(sym) {
      sym->setColor(color);
      sym->setPen(pen);
      d_plot_curve[which]->setSymbol(sym);
    }
#endif
  }
}

QColor
DisplayPlot::getLineColor(int which) const
{
  // If that plot doesn't exist then return black.
  if (which < d_plot_curve.size())
    return d_plot_curve[which]->pen().color();
  return QColor("black");
}

// Use a preprocessor macro to create a bunch of hooks for Q_PROPERTY and hence the stylesheet.
#define SETUPLINE(i, im1) \
    void DisplayPlot::setLineColor ## i (QColor c) {setLineColor(im1, c);} \
    const QColor DisplayPlot::getLineColor ## i () const {return getLineColor(im1);} \
    void DisplayPlot::setLineWidth ## i (int width) {setLineWidth(im1, width);} \
    int DisplayPlot::getLineWidth ## i () const {return getLineWidth(im1);} \
    void DisplayPlot::setLineStyle ## i (Qt::PenStyle ps) {setLineStyle(im1, ps);} \
    const Qt::PenStyle DisplayPlot::getLineStyle ## i () const {return getLineStyle(im1);} \
    void DisplayPlot::setLineMarker ## i (QwtSymbol::Style ms) {setLineMarker(im1, ms);} \
    const QwtSymbol::Style DisplayPlot::getLineMarker ## i () const {return getLineMarker(im1);} \
    void DisplayPlot::setMarkerAlpha ## i (int alpha) {setMarkerAlpha(im1, alpha);} \
    int DisplayPlot::getMarkerAlpha ## i () const {return getMarkerAlpha(im1);}
SETUPLINE(1, 0)
SETUPLINE(2, 1)
SETUPLINE(3, 2)
SETUPLINE(4, 3)
SETUPLINE(5, 4)
SETUPLINE(6, 5)
SETUPLINE(7, 6)
SETUPLINE(8, 7)
SETUPLINE(9, 8)

void
DisplayPlot::setZoomerColor(QColor c) {
  d_zoomer->setRubberBandPen(c);
  d_zoomer->setTrackerPen(c);
}

QColor
DisplayPlot::getZoomerColor() const {
  return d_zoomer->rubberBandPen().color();
}

void
DisplayPlot::setPaletteColor(QColor c) {
  QPalette palette;
  palette.setColor(canvas()->backgroundRole(), c);
  canvas()->setPalette(palette);
}

QColor
DisplayPlot::getPaletteColor() const {
  return canvas()->palette().color(canvas()->backgroundRole());
}

void
DisplayPlot::setAxisLabelFontSize(int axisId, int fs) {
  QwtText axis_title = QwtText(axisWidget(axisId)->title());
  QFont font = QFont(axis_title.font());
  font.setPointSize(fs);
  axis_title.setFont(font);
  axisWidget(axisId)->setTitle(axis_title);
}

int
DisplayPlot::getAxisLabelFontSize(int axisId) const {
  return axisWidget(axisId)->title().font().pointSize();
}

void
DisplayPlot::setYaxisLabelFontSize(int fs) {
  setAxisLabelFontSize(QwtPlot::yLeft, fs);
}

int
DisplayPlot::getYaxisLabelFontSize() const {
  int fs = getAxisLabelFontSize(QwtPlot::yLeft);
  return fs;
}

void
DisplayPlot::setXaxisLabelFontSize(int fs) {
  setAxisLabelFontSize(QwtPlot::xBottom, fs);
}

int
DisplayPlot::getXaxisLabelFontSize() const {
  int fs = getAxisLabelFontSize(QwtPlot::xBottom);
  return fs;
}

void
DisplayPlot::setAxesLabelFontSize(int fs) {
  setAxisLabelFontSize(QwtPlot::yLeft, fs);
  setAxisLabelFontSize(QwtPlot::xBottom, fs);
}

int
DisplayPlot::getAxesLabelFontSize() const {
  // Returns 0 if all axes do not have the same font size.
  int fs = getAxisLabelFontSize(QwtPlot::yLeft);
  if (getAxisLabelFontSize(QwtPlot::xBottom) != fs)
    return 0;
  return fs;
}

void
DisplayPlot::setLineWidth(int which, int width)
{
  if(which < d_nplots) {
    // Set the new line width
    QPen pen(d_plot_curve[which]->pen());
    pen.setWidth(width);
    d_plot_curve[which]->setPen(pen);

    // Scale the marker size proportionally
#if QWT_VERSION < 0x060000
    QwtSymbol sym = (QwtSymbol)d_plot_curve[which]->symbol();
    sym.setSize(7+10*log10(1.0*width), 7+10*log10(1.0*width));
    d_plot_curve[which]->setSymbol(sym);
#else
    QwtSymbol *sym = (QwtSymbol*)d_plot_curve[which]->symbol();
    if(sym) {
      sym->setSize(7+10*log10(1.0*width), 7+10*log10(1.0*width));
      d_plot_curve[which]->setSymbol(sym);
    }
#endif
  }
}

int
DisplayPlot::getLineWidth(int which) const {
  if (which < d_nplots) {
    return d_plot_curve[which]->pen().width();
  }
  else {
    return 0;
  }
}

void
DisplayPlot::setLineStyle(int which, Qt::PenStyle style)
{
  if(which < d_nplots) {
    QPen pen(d_plot_curve[which]->pen());
    pen.setStyle(style);
    d_plot_curve[which]->setPen(pen);
  }
}

const Qt::PenStyle
DisplayPlot::getLineStyle(int which) const
{
  if(which < d_nplots) {
    return d_plot_curve[which]->pen().style();
  }
  else {
    return Qt::SolidLine;
  }
}

void
DisplayPlot::setLineMarker(int which, QwtSymbol::Style marker)
{
  if(which < d_nplots) {
#if QWT_VERSION < 0x060000
    QwtSymbol sym = (QwtSymbol)d_plot_curve[which]->symbol();
    QPen pen(d_plot_curve[which]->pen());
    QBrush brush(pen.color());
    sym.setStyle(marker);
    sym.setPen(pen);
    sym.setBrush(brush);
    d_plot_curve[which]->setSymbol(sym);
#else
    QwtSymbol *sym = (QwtSymbol*)d_plot_curve[which]->symbol();
    if(sym) {
      sym->setStyle(marker);
      d_plot_curve[which]->setSymbol(sym);
    }
#endif
  }
}

const QwtSymbol::Style
DisplayPlot::getLineMarker(int which) const
{
  if(which < d_nplots) {
#if QWT_VERSION < 0x060000
    QwtSymbol sym = (QwtSymbol)d_plot_curve[which]->symbol();
    return sym.style();
#else
    QwtSymbol *sym = (QwtSymbol*)d_plot_curve[which]->symbol();
    return sym->style();
#endif
  }
  else {
    return QwtSymbol::NoSymbol;
  }
}

void
DisplayPlot::setMarkerAlpha(int which, int alpha)
{
  if (which < d_nplots) {
    // Get the pen color
    QPen pen(d_plot_curve[which]->pen());
    QColor color = pen.color();

    // Set new alpha and update pen
    color.setAlpha(alpha);
    pen.setColor(color);
    d_plot_curve[which]->setPen(pen);

    // And set the new color for the markers
#if QWT_VERSION < 0x060000
    QwtSymbol sym = (QwtSymbol)d_plot_curve[which]->symbol();
    setLineMarker(which, sym.style());
#else
    QwtSymbol *sym = (QwtSymbol*)d_plot_curve[which]->symbol();
    if(sym) {
      sym->setColor(color);
      sym->setPen(pen);
      d_plot_curve[which]->setSymbol(sym);
    }
#endif
  }
}

int
DisplayPlot::getMarkerAlpha(int which) const
{
  if(which < d_nplots) {
    return d_plot_curve[which]->pen().color().alpha();
  }
  else {
    return 0;
  }
}

void
DisplayPlot::setStop(bool on)
{
  d_stop = on;
}

void
DisplayPlot::resizeSlot( QSize *s )
{
  // -10 is to spare some room for the legend and x-axis label
  resize(s->width()-10, s->height()-10);
}

void DisplayPlot::legendEntryChecked(QwtPlotItem* plotItem, bool on)
{
  plotItem->setVisible(!on);
  replot();
}

void DisplayPlot::legendEntryChecked(const QVariant &plotItem, bool on, int index)
{
#if QWT_VERSION < 0x060100
  std::runtime_error("DisplayPlot::legendEntryChecked with QVariant not enabled in this version of QWT.\n");
#else
  QwtPlotItem *p = infoToItem(plotItem);
  legendEntryChecked(p, on);
#endif /* QWT_VERSION < 0x060100 */
}

void
DisplayPlot::onPickerPointSelected(const QwtDoublePoint & p)
{
  QPointF point = p;
  //fprintf(stderr,"onPickerPointSelected %f %f\n", point.x(), point.y());
  emit plotPointSelected(point);
}

void
DisplayPlot::onPickerPointSelected6(const QPointF & p)
{
  QPointF point = p;
  //fprintf(stderr,"onPickerPointSelected %f %f\n", point.x(), point.y());
  emit plotPointSelected(point);
}

void DisplayPlot::zoomBaseUpdate()
{
	d_zoomer->setZoomBase(true);
}

void DisplayPlot::AddAxisOffset(int axisPos, int axisIdx, double offset)
{
	double min;
	double max;
	double ptsPerDiv;

	switch (axisPos) {
	case QwtPlot::yLeft:
		ptsPerDiv = vertAxes[axisIdx]->ptsPerDiv();
		min = d_yAxisMin * ptsPerDiv;
		max = d_yAxisMax * ptsPerDiv;
		break;
	case QwtPlot::xBottom:
		ptsPerDiv = horizAxis->ptsPerDiv();
		min = d_xAxisMin * ptsPerDiv;
		max = d_xAxisMax * ptsPerDiv;
		break;
	}

	QwtAxisId axisId(axisPos, axisIdx);

	setAxisScale(axisId, min + offset, max + offset,
			axisStepSize(axisId));
}

void DisplayPlot::setVertOffset(double offset, int axisIdx)
{
	AddAxisOffset(QwtPlot::yLeft, axisIdx, offset);
	vertAxes[axisIdx]->setOffset(offset);
}

double DisplayPlot::VertOffset(int axisIdx)
{
	return vertAxes[axisIdx]->offset();
}

void DisplayPlot::setHorizOffset(double offset)
{
	AddAxisOffset(QwtPlot::xBottom, 0, offset);
	horizAxis->setOffset(offset);
}

double DisplayPlot::HorizOffset()
{
	return horizAxis->offset();
}

void DisplayPlot::setVertUnitsPerDiv(double upd, int axisIdx)
{
	double min, max;
	double ptsPerDiv = vertAxes[axisIdx]->ptsPerDiv();
	double offset = vertAxes[axisIdx]->offset();

	if (ptsPerDiv != upd) {
		vertAxes[axisIdx]->setPtsPerDiv(upd);
		min = (d_yAxisMin * upd) + offset;
		max = (d_yAxisMax * upd) + offset;
		setAxisScale(QwtAxisId(QwtPlot::yLeft, axisIdx), min, max, upd);
		emit vertScaleDivisionChanged(upd);
	}
}

double DisplayPlot::VertUnitsPerDiv(int axisIdx)
{
	return vertAxes[axisIdx]->ptsPerDiv();
}

void DisplayPlot::setHorizUnitsPerDiv(double upd)
{
	double min, max;
	double ptsPerDiv = horizAxis->ptsPerDiv();
	double offset = horizAxis->offset();

	if (ptsPerDiv != upd) {
		horizAxis->setPtsPerDiv(upd);
		min = (d_xAxisMin * upd) + offset;
		max = (d_xAxisMax * upd) + offset;
		setAxisScale(QwtPlot::xBottom, min, max, upd);
		emit horizScaleDivisionChanged(upd);
	}
}

void DisplayPlot::setActiveVertAxis(unsigned int axisIdx)
{
	int numAxes = this->axesCount(QwtPlot::yLeft);

	if (axisIdx >= numAxes)
		return;

	if (d_usingLeftAxisScales) {
		for (int i = 0; i < numAxes; i++) {
			this->setAxisVisible(QwtAxisId(QwtPlot::yLeft, i),
					(i == axisIdx));
			this->vertAxes[i]->setMouseGesturesEnabled((i == axisIdx));
		}
	}

	d_activeVertAxis = axisIdx;
}

int DisplayPlot::activeVertAxis()
{
	return d_activeVertAxis;
}

double DisplayPlot::HorizUnitsPerDiv()
{
	return horizAxis->ptsPerDiv();
}

void DisplayPlot::DetachCurve(unsigned int curveIdx)
{
	if (curveIdx < this->d_plot_curve.size()) {
		this->d_plot_curve[curveIdx]->detach();
	}
}

void DisplayPlot::AttachCurve(unsigned int curveIdx)
{
	if (curveIdx < this->d_plot_curve.size()) {
		this->d_plot_curve[curveIdx]->attach((QwtPlot *)this);
	}
}

QwtPlotCurve * DisplayPlot::Curve(unsigned int curveIdx)
{
	QwtPlotCurve *curve = NULL;

	if (curveIdx < this->d_plot_curve.size())
		curve = d_plot_curve[curveIdx];

	return curve;
}

void DisplayPlot::setMinXaxisDivision(double minDivison)
{
	this->d_hScaleDivisions.setLower(minDivison);
}

double DisplayPlot::minXaxisDivision()
{
	return this->d_hScaleDivisions.lower();
}

void DisplayPlot::setMaxXaxisDivision(double maxDivison)
{
	this->d_hScaleDivisions.setUpper(maxDivison);
}

double DisplayPlot::maxXaxisDivision()
{
	return this->d_hScaleDivisions.upper();
}

void DisplayPlot::setMinYaxisDivision(double minDivison)
{
	this->d_vScaleDivisions.setLower(minDivison);
}

double DisplayPlot::minYaxisDivision()
{
	return this->d_vScaleDivisions.lower();
}

void DisplayPlot::setMaxYaxisDivision(double maxDivision)
{
	this->d_vScaleDivisions.setUpper(maxDivision);
}

double DisplayPlot::maxYaxisDivision()
{
	return this->d_vScaleDivisions.upper();
}

void DisplayPlot::onHorizAxisOffsetDecrease()
{
	double offset = this->HorizOffset();
	double scale = this->HorizUnitsPerDiv();

	offset -= scale / xAxisNumDiv();

	// a value very close to 0.0 is explicitely set to 0.0
	if ( qwtFuzzyCompare( offset, 0.0, scale / xAxisNumDiv() ) == 0 )
		offset = 0;

	this->setHorizOffset(offset);
	this->replot();
	emit horizScaleOffsetChanged(offset);
}

void DisplayPlot::onHorizAxisOffsetIncrease()
{
	double offset = this->HorizOffset();
	double scale = this->HorizUnitsPerDiv();

	offset += scale / xAxisNumDiv();

	// a value very close to 0.0 is explicitely set to 0.0
	if ( qwtFuzzyCompare( offset, 0.0, scale / xAxisNumDiv() ) == 0 )
		offset = 0;

	this->setHorizOffset(offset);
	this->replot();
	emit horizScaleOffsetChanged(offset);
}

void DisplayPlot::onVertAxisOffsetDecrease()
{
	OscAdjuster *osc_adj = dynamic_cast<OscAdjuster *>(QObject::sender());

	if (!osc_adj)
		return;

	double offset = this->VertOffset(osc_adj->axisId().id);
	double scale = this->VertUnitsPerDiv(osc_adj->axisId().id);

	offset -= scale / yAxisNumDiv();
	// a value very close to 0.0 is explicitely set to 0.0
	if ( qwtFuzzyCompare( offset, 0.0, scale / yAxisNumDiv() ) == 0 )
		offset = 0;

	this->setVertOffset(offset, osc_adj->axisId().id);
	this->replot();
	emit vertScaleOffsetChanged(offset);
}

void DisplayPlot::onVertAxisOffsetIncrease()
{
	OscAdjuster *osc_adj = dynamic_cast<OscAdjuster *>(QObject::sender());

	if (!osc_adj)
		return;

	double offset = this->VertOffset(osc_adj->axisId().id);
	double scale = this->VertUnitsPerDiv(osc_adj->axisId().id);

	offset += scale / yAxisNumDiv();

	// a value very close to 0.0 is explicitely set to 0.0
	if ( qwtFuzzyCompare( offset, 0.0, scale / yAxisNumDiv() ) == 0 )
		offset = 0;

	this->setVertOffset(offset, osc_adj->axisId().id);
	this->replot();

	emit vertScaleOffsetChanged(offset);
}

void DisplayPlot::_onXbottomAxisWidgetScaleDivChanged()
{
	QwtScaleWidget *axis_widget = dynamic_cast<QwtScaleWidget *>(QObject::sender());
	OscScaleDraw *scale_draw = dynamic_cast<OscScaleDraw *>(axis_widget->scaleDraw());

	if (scale_draw) {
		scale_draw->updateMetrics();
		scale_draw->invalidateTheCache();
		axis_widget->update();
	}
}

void DisplayPlot::_onYleftAxisWidgetScaleDivChanged()
{
	QwtScaleWidget *axis_widget = dynamic_cast<QwtScaleWidget *>(QObject::sender());
	OscScaleDraw *scale_draw = dynamic_cast<OscScaleDraw *>(axis_widget->scaleDraw());

	if (scale_draw) {
		scale_draw->updateMetrics();
		scale_draw->invalidateTheCache();
		axis_widget->update();
	}
}

void DisplayPlot::horizAxisScaleIncrease()
{
	double div = HorizUnitsPerDiv();
	double newDiv = d_hScaleDivisions.getNumberAfter(div);

	if (newDiv != div) {
		setHorizUnitsPerDiv(newDiv);
		replot();
	}
}

void DisplayPlot::horizAxisScaleDecrease()
{
	double div = HorizUnitsPerDiv();
	double newDiv = d_hScaleDivisions.getNumberBefore(div);

	if (newDiv != div) {
		setHorizUnitsPerDiv(newDiv);
		replot();
	}
}

void DisplayPlot::vertAxisScaleIncrease()
{
	OscAdjuster *osc_adj = dynamic_cast<OscAdjuster *>(QObject::sender());

	if (!osc_adj)
		return;

	double div = VertUnitsPerDiv(osc_adj->axisId().id);
	double newDiv = d_vScaleDivisions.getNumberAfter(div);

	if (newDiv != div) {
		setVertUnitsPerDiv(newDiv, osc_adj->axisId().id);
		replot();
	}
}

void DisplayPlot::vertAxisScaleDecrease()
{
	OscAdjuster *osc_adj = dynamic_cast<OscAdjuster *>(QObject::sender());

	if (!osc_adj)
		return;

	double div = VertUnitsPerDiv(osc_adj->axisId().id);
	double newDiv = d_vScaleDivisions.getNumberBefore(div);

	if (newDiv != div) {
		setVertUnitsPerDiv(newDiv, osc_adj->axisId().id);
		replot();
	}
}

void DisplayPlot::removeLeftVertAxis(unsigned int axis)
{
	const unsigned int numAxis = vertAxes.size();

	if (axis >= numAxis)
		return;

	setAxesCount(QwtPlot::yLeft, numAxis - 1);

	for (unsigned int i = axis + 1; i < numAxis; i++)
		vertAxes[i]->axis().id = i - 1;

	delete vertAxes[axis];
	vertAxes.erase(vertAxes.begin() + axis, vertAxes.begin() + axis + 1);

	for (unsigned int i = axis; i < numAxis - 1; i++) {
		double ptsPerDiv = vertAxes[i]->ptsPerDiv();
		double offset = vertAxes[i]->offset();

		setAxisScale(QwtAxisId(QwtPlot::yLeft, i),
				(d_yAxisMin * ptsPerDiv) + offset,
				(d_yAxisMax * ptsPerDiv) + offset,
				ptsPerDiv);
	}
}

void DisplayPlot::setLeftVertAxesCount(int count)
{
	setAxesCount(QwtPlot::yLeft, count);

	const int numAxis = vertAxes.size();

	for (int i = count; i < numAxis; i++) {
		delete vertAxes[i];
	}

	vertAxes.resize(count);
	for (int i = numAxis; i < count; i++) {
		vertAxes[i] = new PlotAxisConfiguration(QwtPlot::yLeft, i, this);
		configureAxis(QwtPlot::yLeft, i);
		this->setAxisVisible(QwtAxisId(QwtPlot::yLeft, i),
			d_usingLeftAxisScales);
		connect(axisWidget(vertAxes[i]->axis()), SIGNAL(scaleDivChanged()),
			      this, SLOT(_onYleftAxisWidgetScaleDivChanged()));
	}
}

int DisplayPlot::leftVertAxesCount()
{
	return vertAxes.size();
}

void DisplayPlot::setUsingLeftAxisScales(bool on)
{
	d_usingLeftAxisScales = on;
}

bool DisplayPlot::usingLeftAxisScales()
{
	return d_usingLeftAxisScales;
}

void DisplayPlot::configureAxis(int axisPos, int axisIdx)
{
	QwtAxisId axis(axisPos, axisIdx);

	// Use a custom Scale Engine to keep the grid fixed
	OscScaleEngine *scaleEngine = new OscScaleEngine();
	this->setAxisScaleEngine(axis, (QwtScaleEngine *)scaleEngine);

	// Use a custom Scale Draw to control the drawing of axis values
	OscScaleDraw *scaleDraw = new OscScaleDraw();
	this->setAxisScaleDraw(axis, scaleDraw);
}

void DisplayPlot::bottomHorizAxisInit()
{
	horizAxis = new PlotAxisConfiguration(QwtPlot::xBottom, 0, this);
	horizAxis->setMouseGesturesEnabled(true);
	configureAxis(QwtPlot::xBottom, 0);
	connect(axisWidget(horizAxis->axis()), SIGNAL(scaleDivChanged()),
		      this, SLOT(_onXbottomAxisWidgetScaleDivChanged()));
}

QwtScaleDiv adiscope::getEdgelessScaleDiv(const QwtScaleDiv& from_scaleDiv)
{
	double lowerBound;
	double upperBound;
	QList<double> minorTicks;
	QList<double> mediumTicks;
	QList<double> majorTicks;

	lowerBound = from_scaleDiv.lowerBound();
	upperBound = from_scaleDiv.upperBound();
	minorTicks = from_scaleDiv.ticks(QwtScaleDiv::MinorTick);
	mediumTicks = from_scaleDiv.ticks(QwtScaleDiv::MediumTick);
	majorTicks = from_scaleDiv.ticks(QwtScaleDiv::MajorTick);
	majorTicks.erase(majorTicks.begin());
	majorTicks.erase(majorTicks.end() - 1);
	return QwtScaleDiv(lowerBound, upperBound, minorTicks, mediumTicks, majorTicks);
}

unsigned int DisplayPlot::xAxisNumDiv()
{
	return d_xAxisNumDiv;
}

void DisplayPlot::setXaxisNumDiv(unsigned int num)
{
	if (d_xAxisNumDiv != num) {
		d_xAxisNumDiv = num;
		d_xAxisMin = -(double)num / 2;
		d_xAxisMax = (double)num / 2;
	}
}

unsigned int DisplayPlot::yAxisNumDiv()
{
	return d_yAxisNumDiv;
}

void DisplayPlot::setYaxisNumDiv(unsigned int num)
{
	if (d_yAxisNumDiv != num) {
		d_yAxisNumDiv = num;
		d_yAxisMin = -(double)num / 2;
		d_yAxisMax = (double)num / 2;
	}
}