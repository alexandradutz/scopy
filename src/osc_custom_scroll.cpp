#include "osc_custom_scroll.h"

#include <QApplication>
#include <QMouseEvent>
#include <QDebug>
#include <QScrollBar>
#include <QPointf>

using namespace adiscope;

OscCustomScrollArea::OscCustomScrollArea(QWidget *parent):
	QScrollArea(parent),
	inside(false),
	disableCursor(true)
{
	QScroller::grabGesture(this, QScroller::LeftMouseButtonGesture);

	scroll = QScroller::scroller(this);

	QScrollerProperties properties = QScroller::scroller(this)->scrollerProperties();

	QVariant overshootPolicy = QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootWhenScrollable);
	properties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, overshootPolicy);

	QScroller::scroller(this)->setScrollerProperties(properties);

	connect(scroll, &QScroller::stateChanged, [=](QScroller::State newstate){
		if (disableCursor)
				return;
		switch (newstate) {
		case QScroller::Inactive :
			if (inside)
				QApplication::setOverrideCursor(Qt::OpenHandCursor);
			break;
		case QScroller::Pressed :
			QApplication::setOverrideCursor(Qt::ClosedHandCursor);
			break;
		case QScroller::Dragging :
			QApplication::setOverrideCursor(Qt::ClosedHandCursor);
			break;
		case QScroller::Scrolling :
			if (inside)
				QApplication::setOverrideCursor(Qt::OpenHandCursor);
			break;
		default:
			break;
		}
	});

	bar = QAbstractScrollArea::horizontalScrollBar();

	connect(bar, &QScrollBar::rangeChanged, [=](){
		int value = bar->value();
		minValue = bar->minimum();
		maxValue = bar->maximum();
		if (!(maxValue - minValue)){
			Q_EMIT setVisibleLeftBtn(true);
			Q_EMIT setVisibleRightBtn(true);
			disableCursor = true;
		} else {
			disableCursor = false;
			if (value == minValue){
				Q_EMIT setVisibleLeftBtn(true);
				Q_EMIT setVisibleRightBtn(false);
			} else if (value == maxValue){
				Q_EMIT setVisibleLeftBtn(false);
				Q_EMIT setVisibleRightBtn(true);
			} else {
				Q_EMIT setVisibleLeftBtn(false);
				Q_EMIT setVisibleRightBtn(false);
			}
		}
	});
	connect(bar, &QScrollBar::valueChanged, [=](int value){
		if (!(maxValue - minValue)){
			Q_EMIT setVisibleLeftBtn(true);
			Q_EMIT setVisibleRightBtn(true);
			return;
		}

		if (value == minValue){
			Q_EMIT setVisibleLeftBtn(true);
			Q_EMIT setVisibleRightBtn(false);
		} else if (value == maxValue){
			Q_EMIT setVisibleLeftBtn(false);
			Q_EMIT setVisibleRightBtn(true);
		} else {
			Q_EMIT setVisibleLeftBtn(false);
			Q_EMIT setVisibleRightBtn(false);
		}

	});
}

void OscCustomScrollArea::enterEvent(QEvent *event)
{
	if (!disableCursor)
		QApplication::setOverrideCursor(Qt::OpenHandCursor);
	inside = true;
}

void OscCustomScrollArea::leaveEvent(QEvent *event)
{
	if (!disableCursor)
		QApplication::setOverrideCursor(Qt::ArrowCursor);
	inside = false;
}

void OscCustomScrollArea::leftScrollButton()
{
	int newValue = bar->value() - 50;
	newValue = newValue < 0 ? 0 : newValue;
	scroll->stop();
	bar->setValue(newValue);
}

void OscCustomScrollArea::rightScrollButton()
{
	int newValue = bar->value() + 50;
	newValue = newValue > maxValue ? maxValue : newValue;
	scroll->stop();
	bar->setValue(newValue);
}

void OscCustomScrollArea::continuosLeftScroll(bool cancel)
{
	if (!cancel){
		int value = bar->value();
		scroll->scrollTo(QPointF(0, 0), value * 6);
	} else {
		scroll->stop();
	}
}

void OscCustomScrollArea::continuosRightScroll(bool cancel)
{
	if (!cancel){
		int value = maxValue - bar->value();
		QRect rec = findChild<QWidget* >("scrollAreaWidgetContents")->rect();
		scroll->scrollTo(QPointF(rec.width(), 0), value * 6);
	} else {
		scroll->stop();
	}
}



