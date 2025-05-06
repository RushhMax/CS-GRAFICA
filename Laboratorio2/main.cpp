#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Mat rotar90GradosHorario(const Mat& imagen) {
    int filas = imagen.rows;
    int cols = imagen.cols;
    int canales = imagen.channels();

    Mat rotada(cols, filas, imagen.type());

    for (int y = 0; y < filas; y++) {
        for (int x = 0; x < cols; x++) {
            if (canales == 1) {
                rotada.at<uchar>(x, filas - 1 - y) = imagen.at<uchar>(y, x);
            } else {
                rotada.at<Vec3b>(x, filas - 1 - y) = imagen.at<Vec3b>(y, x);
            }
        }
    }

    return rotada;
}

int main() {
    Mat imagen = imread("C:/Users/Rushh/Universidad/Graphics/Laboratorio2/img.jpg");

    imshow("Imagen Original", imagen);
    //waitKey(0);
    ///destroyAllWindows();

    cout<<"Nro de rotaciones? ";
    int n;cin>>n;

    n = n % 4;

    for (int i = 0; i < n; i++) {
        imagen = rotar90GradosHorario(imagen);
    }

    imshow("Imagen Rotada", imagen);
    waitKey(0);
    destroyAllWindows();

    return 0;
}
