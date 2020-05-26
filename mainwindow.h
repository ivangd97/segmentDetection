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
 * 26 Mayo 2020
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

struct puntoCompare {
    bool operator()(const punto a, const punto b) const {
        return a.valor > b.valor;
    }
};

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
    Mat canny_image; //Mat de canny
    Mat detected_edges;
    int threshold;
    int rho;
    float theta;

    /*
     * cornerList[0] = Point
     * cornerList[1] = Valor de Point */
    std::vector<punto> cornerList;
    // Vector de lineas
    std::vector<QLine> lineList;
    // Vector de puntos validos
    std::vector<Point> pCorte;
    //Vector de segmentos
    std::vector<QLine> segmentList;



public slots:
    void compute();
    void start_stop_capture(bool start);
    void change_color_gray(bool color);
    void selectWindow(QPointF p, int w, int h);
    void deselectWindow();
    void loadFromFile();
    void saveToFile();
    void printCorners();
    void cornerDetection();
    void edgesDetection();
    void linesDetection();
    void segmentDetection();
    void printLines();
    void printSegments();
};


#endif // MAINWINDOW_H
