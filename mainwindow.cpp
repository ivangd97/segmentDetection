#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

/**
 * P3 - Segment Detection
 * Ivan González
 * Borja Alberto Tirado Galán
 *
 *
 */

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    cap = new VideoCapture(0);
    winSelected = false;

    //Inicializacion de imagenes
    colorImage.create(240, 320, CV_8UC3);
    grayImage.create(240, 320, CV_8UC1);
    destColorImage.create(240, 320, CV_8UC3);
    destGrayImage.create(240, 320, CV_8UC1);
    corners.create(240, 320, CV_8UC1);
    canny_image.create(240, 320, CV_8UC1);
    detected_edges.create(240, 320, CV_8UC1);

    visorS = new ImgViewer(&grayImage, ui->imageFrameS);
    visorD = new ImgViewer(&destGrayImage, ui->imageFrameD);

    connect(&timer, SIGNAL(timeout()), this, SLOT(compute()));
    connect(ui->captureButton, SIGNAL(clicked(bool)), this, SLOT(start_stop_capture(bool)));
    connect(ui->colorButton, SIGNAL(clicked(bool)), this, SLOT(change_color_gray(bool)));
    connect(visorS, SIGNAL(windowSelected(QPointF, int, int)), this, SLOT(selectWindow(QPointF, int, int)));
    connect(visorS, SIGNAL(pressEvent()), this, SLOT(deselectWindow()));

    connect(ui->loadButton, SIGNAL(pressed()), this, SLOT(loadFromFile()));

    //PILAR (14/05): cuidado con esto. Para ejecutar la detección de líneas es necesario ejecutar antes la detección de bordes.
    //               De la misma forma, para ejecutar la detección de segmentos, se necesitan esquinas y líneas.
    //               Todo esto debería gestionarse en el método "compute".
    connect(ui->showCorners_checkbox, SIGNAL(clicked()), this, SLOT(cornerDetection()));
    connect(ui->showCanny_checkbox, SIGNAL(clicked()), this, SLOT(edgesDetection()));
    connect(ui->showLines_checkbox, SIGNAL(clicked()), this, SLOT(linesDetection()));
    connect(ui->showSegments_checkbox, SIGNAL(clicked()), this, SLOT(segmentDetection()));

    timer.start(30);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete cap;
    delete visorS;
    delete visorD;
}

void MainWindow::compute()
{
    //Captura de imagen

    if (ui->captureButton->isChecked() && cap->isOpened())
    {
        *cap >> colorImage;

        cv::resize(colorImage, colorImage, Size(320, 240));
        cvtColor(colorImage, grayImage, COLOR_BGR2GRAY);
        cvtColor(colorImage, colorImage, COLOR_BGR2RGB);
    }

    // ________________Procesamiento__________________
    // Representar esquinas
    if (!cornerList.empty() && ui->showCorners_checkbox->isChecked())
        printCorners();

    // Representar lineas
    if (!lineList.empty() && ui->showLines_checkbox->isChecked())
        printLines();

    // Volver a la imagen original cuando se desactiva la opcion Canny
    if (!ui->showCanny_checkbox->isChecked())
    {
        grayImage.copyTo(destGrayImage);
    }

    if (winSelected)
    {
        visorS->drawSquare(QPointF(imageWindow.x + imageWindow.width / 2, imageWindow.y + imageWindow.height / 2), imageWindow.width, imageWindow.height, Qt::green);
    }
    visorS->update();
    visorD->update();
}

void MainWindow::start_stop_capture(bool start)
{
    if (start)
        ui->captureButton->setText("Stop capture");
    else
        ui->captureButton->setText("Start capture");
}

void MainWindow::change_color_gray(bool color)
{
    if (color)
    {
        ui->colorButton->setText("Gray image");
        visorS->setImage(&colorImage);
        visorD->setImage(&destColorImage);
    }
    else
    {
        ui->colorButton->setText("Color image");
        visorS->setImage(&grayImage);
        visorD->setImage(&destGrayImage);
    }
}

void MainWindow::selectWindow(QPointF p, int w, int h)
{
    QPointF pEnd;
    if (w > 0 && h > 0)
    {
        imageWindow.x = p.x() - w / 2;
        if (imageWindow.x < 0)
            imageWindow.x = 0;
        imageWindow.y = p.y() - h / 2;
        if (imageWindow.y < 0)
            imageWindow.y = 0;
        pEnd.setX(p.x() + w / 2);
        if (pEnd.x() >= 320)
            pEnd.setX(319);
        pEnd.setY(p.y() + h / 2);
        if (pEnd.y() >= 240)
            pEnd.setY(239);
        imageWindow.width = pEnd.x() - imageWindow.x;
        imageWindow.height = pEnd.y() - imageWindow.y;

        winSelected = true;
    }
}

void MainWindow::deselectWindow()
{
    winSelected = false;
}

void MainWindow::loadFromFile()
{
    disconnect(&timer, SIGNAL(timeout()), this, SLOT(compute()));

    Mat image;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), "/home", tr("Images (*.jpg *.png "
                                                                                  "*.jpeg *.gif);;All Files(*)"));
    image = cv::imread(fileName.toStdString());

    if (fileName.isEmpty())
        return;
    else
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return;
        }
        ui->captureButton->setChecked(false);
        ui->captureButton->setText("Start capture");
        cv::resize(image, colorImage, Size(320, 240));
        cvtColor(colorImage, colorImage, COLOR_BGR2RGB);
        cvtColor(colorImage, grayImage, COLOR_RGB2GRAY);

        if (ui->colorButton->isChecked())
            colorImage.copyTo(destColorImage);
        else
            grayImage.copyTo(destGrayImage);
        connect(&timer, SIGNAL(timeout()), this, SLOT(compute()));
    }
}

void MainWindow::saveToFile()
{
    disconnect(&timer, SIGNAL(timeout()), this, SLOT(compute()));
    Mat save_image;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image File"),
                                                    QString(),
                                                    tr("JPG (*.JPG) ; jpg (*.jpg); png (*.png); jpeg(*.jpeg); gif(*.gif); All Files (*)"));
    if (ui->colorButton->isChecked())
        cvtColor(destColorImage, save_image, COLOR_RGB2BGR);

    else
        cvtColor(destGrayImage, save_image, COLOR_GRAY2BGR);

    if (fileName.isEmpty())
        return;
    else
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::information(this, tr("Unable to open file"),
                                     file.errorString());
            return;
        }
    }
    cv::imwrite(fileName.toStdString(), save_image);

    connect(&timer, SIGNAL(timeout()), this, SLOT(compute()));
}

void MainWindow::cornerDetection()
{
    Mat dst;
    dst.create(240, 320, CV_32FC1);
    cornerList.clear();

    double harris_factor = ui->harrisFactor_box->value();
    int blockSize = ui->blockSize_box->value();
    float threshold = ui->threshold_box->value();

    dst = Mat::zeros(grayImage.size(), CV_32FC1);

    //Metodo que calcula las esquinas
    cv::cornerHarris(grayImage, dst, blockSize, 3, harris_factor);

    //Almacenamiento de las esquinas en una lista
    for (int x = 0; x < dst.cols; x++)
    {
        for (int y = 0; y < dst.rows; y++)
        {
            if (dst.at<float>(y, x) > threshold)
            {
                punto p;
                p.point = Point(x, y);
                p.valor = dst.at<float>(y, x);
                this->cornerList.push_back(p);
            }
        }
    }
    //Lista ordenada de esquinas
    std::sort(cornerList.begin(), cornerList.end(), puntoCompare());

    //Supresion del no maximo
    for (int i = 0; i < (int)cornerList.size(); i++)
    {
        for (int j = i + 1; j < (int)cornerList.size(); j++)
        {
            if (abs(cornerList[i].point.x - cornerList[j].point.x) < threshold &&
                abs(cornerList[i].point.y - cornerList[j].point.y) < threshold)
            {

                cornerList.erase(cornerList.begin() + j);
                j--;
            }
        }
    }

    dst.copyTo(destGrayImage);
}

void MainWindow::printCorners()
{
    for (size_t i = 0; i < cornerList.size(); i++)
    {
        visorD->drawEllipse(QPoint(cornerList[i].point.x, cornerList[i].point.y), 2, 2, Qt::red);
    }
}

// PILAR (14/05)[DONE]: necesitáis otra imagen para almacenar el resultado de Canny.
// PILAR (14/05): destGrayImage debe contener la imagen visible para el destino.
// PILAR (14/05)[DONE]: detected_edges debería ser atributo de clase
void MainWindow::edgesDetection()
{
    int lowThreshold = ui->minthreshold_box->value();
    int const maxThreshold = ui->maxthreshold_box->value();
    int kernel_size = 3;

    grayImage.copySize(destGrayImage);
    // Reduce noise with a kernel 3x3
    blur(destGrayImage, detected_edges, Size(3, 3));

    // Canny detector
    cv::Canny(detected_edges, canny_image, lowThreshold, maxThreshold, kernel_size);


    //PILAR (14/05)[DONE]: no estiendo porqué hacéis esta parte. El resultado (detección de bordes de Canny) debe estar en detected_edges (definida como atributo de la clase)
    // Using Canny's output as a mask, we display our result
    destGrayImage = Scalar::all(0);

    grayImage.copyTo(destGrayImage, canny_image);
}

void MainWindow::linesDetection()
{
    // PILAR (14/05)[DONE]: Deberían ser variables locales. rho y theta tienen que tener otro nombre
    int threshold_param = ui->linesthreshold_box->value();
    float rho_param  = ui->rhoresolution_box->value();
    float theta_param = ui->thetaresolution_box->value();
    pCorte.clear();
    lineList.clear();
    std::vector<Point>::iterator it;
    std::vector<Vec2f> lines;


    //PILAR (14/05): incluyo aquí la detección de bordes porque HoughLines tiene como entrada una imagen de bordes
    int lowThreshold = ui->minthreshold_box->value();
    int maxThreshold = ui->maxthreshold_box->value();
    cv::Canny(grayImage, detected_edges, lowThreshold, maxThreshold);

    //PILAR (14/05)[DONE]: no debe hacerse sobre destGrayImahe, sino sobre una imagen de bordes
    cv::HoughLines(detected_edges, lines, rho_param, theta_param, threshold_param);

    lineList.clear(); // PILAR (14/05)[DONE]: hay que generar la lista de nuevo en cada llamada
    for (size_t i = 0; i < lines.size(); i++)
    {
        float rho = lines[i][0];
        float theta = lines[i][1];
        Point pt1, pt2;

        pCorte.clear(); //PILAR (14/05)[DONE]: hay que generarla para cada línea

        pt1.x = 0;
        pt1.y = (int)rint(rho / sin(theta));
        if (pt1.y >= 0 && pt1.y < 240)// PILAR (14/05): la comprobación hay que hacerla sobre y. x vale 0, por lo que ya sabemos que está dentro de los límites
        {
            it = std::find(pCorte.begin(), pCorte.end(), pt1);
            if (it == pCorte.end()) // PILAR (14/05): la comprobación estaba al revés. Si no existe, la función find devuelve end().
                pCorte.push_back(pt1);
        }
        pt1.x = (int)rint(rho / cos(theta));
        pt1.y = 0;
        if (pt1.x >= 0 && pt1.x < 320)// PILAR (14/05): la comprobación hay que hacerla sobre x. y vale 0, por lo que ya sabemos que está dentro de los límites
        {
            it = std::find(pCorte.begin(), pCorte.end(), pt1);
            if (it == pCorte.end()) // PILAR (14/05): la comprobación estaba al revés. Si no existe, la función find devuelve end().
                pCorte.push_back(pt1);
        }

        pt2.x = 319;
        pt2.y = (int)rint((rho - 319*cos(theta)) / sin(theta));  //PILAR (14/05): la ecuación de la recta es rho = x*cos(theta) + y*sin(theta). No habéis despejado correctamente
        if (pt2.y >= 0 && pt2.y < 240) // PILAR (14/05): la comprobación hay que hacerla sobre y. x vale 319, por lo que ya sabemos que está dentro de los límites
        {
            it = std::find(pCorte.begin(), pCorte.end(), pt2);
            if (it == pCorte.end()) // PILAR (14/05): la comprobación estaba al revés. Si no existe, la función find devuelve end().
                pCorte.push_back(pt2);
        }
        pt2.x = (int)rint((rho - 239*sin(theta)) / cos(theta)); //PILAR (14/05): la ecuación de la recta es rho = x*cos(theta) + y*sin(theta). No habéis despejado correctamente
        pt2.y = 239;
        if (pt2.x >= 0 && pt2.x < 320) // PILAR (14/05): la comprobación hay que hacerla sobre x. y vale 239, por lo que ya sabemos que está dentro de los límites
        {
            it = std::find(pCorte.begin(), pCorte.end(), pt2);
            if (it == pCorte.end()) // PILAR (14/05): la comprobación estaba al revés. Si no existe, la función find devuelve end().
                pCorte.push_back(pt2);
        }

        // PILAR (14/05)[DONE]: los 2 puntos que definen la línea son los dos puntos almacenados en pCorte
        if(pCorte.size()==2)
            this->lineList.push_back(QLine(QPoint(pCorte[0].x, pCorte[0].y), QPoint(pCorte[1].x, pCorte[1].y)));
    }

}

void MainWindow::segmentDetection()
{
}

void MainWindow::printLines()
{
    for (size_t i = 0; i < lineList.size(); i++)
        visorD->drawLine(lineList[i], Qt::green, 3);
}
