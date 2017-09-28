#include "osc_measure_panel_controller.h"
#include "ui_measure_panel.h"

#include <QSizePolicy>

using namespace adiscope;

MeasurePanelController::MeasurePanelController(QWidget *parent):
	pressedLeft(false),
	continuousLeft(false),
	pressedRight(false),
	continuousRight(false)
{
	measurements_panel = new Ui::MeasurementsPanel();
	panel = new QWidget(parent);
	measurements_panel->setupUi(panel);
	panel->hide();

	QSizePolicy sp_retainBtnLeft = measurements_panel->scrollLeftBtn->sizePolicy();
	sp_retainBtnLeft.setRetainSizeWhenHidden(true);
	measurements_panel->scrollLeftBtn->setSizePolicy(sp_retainBtnLeft);
	QSizePolicy sp_retainBtnRight = measurements_panel->scrollRightBtn->sizePolicy();
	sp_retainBtnRight.setRetainSizeWhenHidden(true);
	measurements_panel->scrollRightBtn->setSizePolicy(sp_retainBtnRight);

	measurements_panel->scrollRightBtn->setVisible(false);
	measurements_panel->scrollLeftBtn->setVisible(false);

	timerLeft = new QTimer(this);
	connect(timerLeft, &QTimer::timeout, [=](){
		if (pressedLeft && !continuousLeft){
			measurements_panel->scrollArea->continuosLeftScroll();
			continuousLeft = true;
			timerLeft->stop();
		}
	});

	connect(measurements_panel->scrollLeftBtn, &QPushButton::pressed, [=](){
		pressedLeft = true;
		timerLeft->start(200);
	});
	connect(measurements_panel->scrollLeftBtn, &QPushButton::released, [=](){
		pressedLeft = false;
		if (continuousLeft){
			measurements_panel->scrollArea->continuosLeftScroll(true);
			continuousLeft = false;
		} else {
			measurements_panel->scrollArea->leftScrollButton();
		}
	});


	timerRight = new QTimer(this);
	connect(timerRight, &QTimer::timeout, [=](){
		if (pressedRight && !continuousRight){
			measurements_panel->scrollArea->continuosRightScroll();
			continuousRight = true;
			timerRight->stop();
		}
	});


	connect(measurements_panel->scrollRightBtn, &QPushButton::pressed, [=](){
		pressedRight = true;
		timerRight->start(200);
	});
	connect(measurements_panel->scrollRightBtn, &QPushButton::released, [=](){
		pressedRight = false;
		if (continuousRight){
			measurements_panel->scrollArea->continuosRightScroll(true);
			continuousRight = false;
		} else {
			measurements_panel->scrollArea->rightScrollButton();
		}
	});


	connect(measurements_panel->scrollArea, &OscCustomScrollArea::setVisibleLeftBtn,
		[=](bool disabled){
		measurements_panel->scrollLeftBtn->setVisible(!disabled);
	});
	connect(measurements_panel->scrollArea, &OscCustomScrollArea::setVisibleRightBtn,
		[=](bool disabled){
		measurements_panel->scrollRightBtn->setVisible(!disabled);
	});
}

Ui::MeasurementsPanel* MeasurePanelController::getPanelGUI()
{
	return measurements_panel;
}

QWidget* MeasurePanelController::getPanel()
{
	return panel;
}
