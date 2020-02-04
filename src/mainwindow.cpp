#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>

#include <cstdio>
#include <cstdlib>

#include <QLabel>
#include <QSlider>

#define BACKLIGHT_DIR "/sys/class/backlight/"

/*function... might want it in some class?*/
int getdir(std::string dir, std::vector<std::string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == nullptr) {
        return errno;
    }

    while ((dirp = readdir(dp)) != nullptr) {
        files.push_back(std::string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}



void MainWindow::brightnessChanged(int value) {
    QObject* obj = sender();
	QWidget* slider = qobject_cast<QWidget *>(obj);
	QString name = slider->objectName();
	std::string filename = name.toUtf8().constData();
	filename = BACKLIGHT_DIR + filename + "/brightness";

	FILE *fp = fopen(filename.c_str(), "w+");
	if(fp == NULL) {
		std::cout << "Write error: " << filename << std::endl;
		return;
	}
	fprintf(fp, "%d", value);
	fclose(fp);
}


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	std::vector<std::string> files = std::vector<std::string>();
	getdir(BACKLIGHT_DIR, files);

	ui->setupUi(this);

	setWindowTitle("Brightness control");

	for (unsigned int i = 2; i < files.size(); i++) {
		QSlider *slider = new QSlider(Qt::Horizontal);
		QLabel *label = new QLabel();
		QString qstr = QString::fromStdString(files[i]);
		label->setText(qstr);
		slider->setObjectName(qstr);

		std::string maxFilename = BACKLIGHT_DIR + files[i] + "/max_brightness";
		std::string curFilename = BACKLIGHT_DIR + files[i] + "/brightness";
		std::string actFilename = BACKLIGHT_DIR + files[i] + "/actual_brightness";

		int actBrightness, maxBrightness, curBrightness;

		FILE *fp = std::fopen(actFilename.c_str(), "r");
		fscanf(fp, "%d", &actBrightness);
		fclose(fp);

		// actual brightness is used to detect when the monitor doesnt exist
		if(actBrightness<0) continue;

		fp = std::fopen(maxFilename.c_str(), "r");
		fscanf(fp, "%d", &maxBrightness);
		fclose(fp);
		fp = std::fopen(curFilename.c_str(), "r");
		fscanf(fp, "%d", &curBrightness);
		fclose(fp);

		connect(slider, SIGNAL(valueChanged(int)), this, SLOT(brightnessChanged(int)));

		slider->setMaximum(maxBrightness);
		slider->setSliderPosition(curBrightness);

		ui->mainWidget->layout()->addWidget(label);
		ui->mainWidget->layout()->addWidget(slider);
    }

}

MainWindow::~MainWindow()
{
	delete ui;
}

