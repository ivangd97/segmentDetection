#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <imgviewer.h>

#include <QtWidgets/QFileDialog>


/**
 * P3 - Segment Detection
 * Ivan González
 * Borja Alberto Tirado Galán
 *
 *
 */

using namespace cv;

namespace Ui {
    class MainWindow;
}

typedef struct{
    Point point;
    float valor;
} punto;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    //Interfaz principal de usuario
    Ui::MainWindow *ui;

    QTimer timer;

    VideoCapture *cap;
    ImgViewer *visorS, *visorD, *visorHistoS, *visorHistoD;
    Mat colorImage, grayImage, destColorImage, destGrayImage;
    bool winSelected;
    Rect imageWindow;
    Mat corners; //Mat de esquinas
    /*
     * cornerList[0] = Point
     * cornerList[1] = Valor de Point */
    std::vector<punto> cornerList;



public slots:
    void compute();
    void start_stop_capture(bool start);
    void change_color_gray(bool color);
    void selectWindow(QPointF p, int w, int h);
    void deselectWindow();
    void loadFromFile();
    void saveToFile();
    void cornerDetection();
    void printCorners();
};


#endif // MAINWINDOW_H
