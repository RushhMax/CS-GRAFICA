#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

void mostrar(const Mat& mapaColor) {
    int width = 256;
    int height = 50; 
    Mat ver(height, width, CV_8UC3);

    for (int i = 0; i < 256; ++i) {
        Vec3b color = mapaColor.at<Vec3b>(0, i);
        for (int j = 0; j < height; ++j) {
            ver.at<Vec3b>(j, i) = color; 
        }
    }

    imshow("Mapa de Colores", ver);
    imwrite("Mapadecolor.jpeg", ver);
    
    waitKey(0);
    destroyAllWindows();
}

Mat transformar(const Mat& img, const Mat& mapaColor) {
    Mat imgRGB = img.clone();
    Mat resultado(imgRGB.size(), imgRGB.type());

    for (int y = 0; y < imgRGB.rows; ++y) {
        for (int x = 0; x < imgRGB.cols; ++x) {
            Vec3b pixel = imgRGB.at<Vec3b>(y, x);

            uchar b = pixel[0];
            uchar g = pixel[1];
            uchar r = pixel[2];

            Vec3b nuevoColor = Vec3b(
                mapaColor.at<Vec3b>(0, b)[0],  
                mapaColor.at<Vec3b>(0, g)[1], 
                mapaColor.at<Vec3b>(0, r)[2]   
            );

            resultado.at<Vec3b>(y, x) = nuevoColor;
        }
    }

    return resultado;
}


int main() {
    Mat img = imread("Gray.jpg");

    imshow("Original", img);
    waitKey(0);
    destroyAllWindows();

    Mat mapaColor(1, 256, CV_8UC3);

    for (int i = 0; i < 256; ++i) {
        uchar r,g,b;

        //r = rand() % 256;  
        //g = rand() % 256; 
        //b = rand() % 256; 

        if (i < 64) {
            r = 192+i; g = 66; b = 255-i;         
        } else if (i < 128) {
            r = 0; g = i; b = 255; 
        } else if (i < 192) {
            r = 0; g = 255; b = i; 
        } else {
            r = 255 ; g = 15 + (i/2); b = 143 + (i); 
            
        }

        mapaColor.at<Vec3b>(0, i) = Vec3b(b, g, r);
    }

    mostrar(mapaColor);

    Mat colored;
    colored = transformar(img, mapaColor);

    imshow("Mapa de color personalizado", colored);
    
    waitKey(0);
    destroyAllWindows();

    imwrite("NuevaIMG.jpg", colored);    

    return 0;
}


//g++ rotar.cpp -o rotar `pkg-config --cflags --libs opencv4`
