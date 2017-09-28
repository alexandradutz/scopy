#ifndef OSC_MEASURE_PANEL_CONTROLLER_H
#define OSC_MEASURE_PANEL_CONTROLLER_H

#include "measurement_gui.h"
#include <QWidget>
#include <QDebug>
#include <QTimer>

namespace Ui{
	class MeasurementsPanel;
}

namespace adiscope {
class MeasurePanelController : public QObject
{
	Q_OBJECT
public:
	MeasurePanelController(QWidget *parent = 0);

	Ui::MeasurementsPanel* getPanelGUI();
	QWidget *getPanel();

	void statisticsEnabled(bool on);
	QWidget* getStatistics();
private:
	Ui::MeasurementsPanel *measurements_panel;
	QWidget *panel;
	bool pressedLeft, pressedRight;
	bool continuousLeft, continuousRight;
	QTimer *timerLeft, *timerRight;
};
}

#endif // OSC_MEASURE_PANEL_CONTROLLER_H
