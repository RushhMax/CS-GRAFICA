#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Mat rotarImg(const Mat& img) {
    Mat rotada(img.cols, img.rows, img.type());

    for (int y = 0; y < filas; y++) {
        for (int x = 0; x < cols; x++) {
            rotada.at<Vec3b>(x, filas - 1 - y) = img.at<Vec3b>(y, x);
        }
    }

    return rotada;
}

int main() {
    Mat img = imread("C:/Users/Rushh/Universidad/Graphics/Laboratorio2/img.jpg");

    imshow(img);
    //waitKey(0);
    ///destroyAllWindows();

    cout<<"Nro de rotaciones? ";
    int n;cin>>n;
    n=n%4;

    for(int i=0;i<n;i++) {
        img = rotarImg(img);
    }

    imshow(img);
    imwrite("Rotacion_"+to_string(n)+".jpg", img);    
    waitKey(0);
    destroyAllWindows();

    return 0;
}
