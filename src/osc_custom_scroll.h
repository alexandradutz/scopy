#ifndef OSCCUSTOMSCROLL_H
#define OSCCUSTOMSCROLL_H

#include <QScrollArea>
#include <QEvent>
#include <QTimer>
#include <QScroller>

namespace adiscope {
class OscCustomScrollArea : public QScrollArea
{
	Q_OBJECT
public:
	OscCustomScrollArea(QWidget *parent = 0);

Q_SIGNALS:
	void setVisibleLeftBtn(bool disabled);
	void setVisibleRightBtn(bool disabled);

public Q_SLOTS:
	void enterEvent(QEvent *);
	void leaveEvent(QEvent *);
	void leftScrollButton();
	void rightScrollButton();
	void continuosLeftScroll(bool cancel = false);
	void continuosRightScroll(bool cancel = false);

private:
	QScroller *scroll;
	bool inside;
	QScrollBar *bar;
	int minValue, maxValue;
	bool disableCursor;
};
}

#endif // OSCCUSTOMSCROLL_H
