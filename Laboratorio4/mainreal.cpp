#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iomanip>

using namespace cv;
using namespace std;
namespace fs = filesystem;

Mat histograma(const Mat& img) {
    Mat hist = Mat::zeros(256, 1, CV_32F); 
    int ancho = img.cols, alto = img.rows;

    for (int y = 0; y<alto; y++) {
        for (int x = 0; x<ancho; x++) {
            uchar pixel = img.at<uchar>(y, x);
            hist.at<float>(pixel, 0) += 1;
        }
    }

    return hist;
}

void calcularMaxMin(const cv::Mat& mat, float& minVal, float& maxVal) {
    CV_Assert(mat.type() == CV_32F && mat.cols == 1); 
    minVal = FLT_MAX;
    maxVal = -FLT_MAX;
    for (int i = 0; i < mat.rows; ++i) {
        float val = mat.at<float>(i);
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    }
}


void normalizacion(cv::Mat& hist, float minVal, float maxVal) {
    const float epsilon = 1e-5f; 
    float minH, maxH;
    calcularMaxMin(hist, minH, maxH);
    float scale = (maxVal - minVal) / (maxH - minH + epsilon);
    float* ptr = hist.ptr<float>();
    for (int i = 0; i < hist.rows; ++i) {
        ptr[i] = (ptr[i] - minH) * scale + minVal;
    }
}

Mat ecualizacion(const Mat& img) {
    Mat hist = histograma(img);
    Mat cdf = hist.clone();
    for (int i = 1; i < 256; ++i)
        cdf.at<float>(i) += cdf.at<float>(i - 1);
    cdf /= (img.rows * img.cols);
    cdf *= 255;
    Mat ecualizada = img.clone();
    for (int i = 0; i < img.rows; ++i)
        for (int j = 0; j < img.cols; ++j)
            ecualizada.at<uchar>(i, j) = static_cast<uchar>(cdf.at<float>(img.at<uchar>(i, j)));
    return ecualizada;
}

Mat binarizacion(const Mat& img, int thresh, uchar maxValue = 255) {
    Mat binarizado = img.clone();
    for (int i = 0; i < img.rows; ++i)
        for (int j = 0; j < img.cols; ++j)
            binarizado.at<uchar>(i, j) = (img.at<uchar>(i, j) > thresh) ? maxValue : 0;
    return binarizado;
}

void guardarHistogramaIMG(const Mat& hist, const fs::path& direccion) {
    int ancho = 512, alto = 400;
    int bin_w = cvRound((double)ancho / hist.rows);
    Mat histImage(alto, ancho, CV_8UC1, Scalar(255));
    Mat normHist = hist.clone();
    normalizacion(normHist, 0, histImage.rows);

    for (int i = 1; i < hist.rows; ++i) {
        line(histImage,
             Point(bin_w * (i - 1), alto - cvRound(normHist.at<float>(i - 1))),
             Point(bin_w * i, alto - cvRound(normHist.at<float>(i))),
             Scalar(0), 2);
    }
    imwrite(direccion.string(), histImage);
}

void guardarHistogramaTXT(const Mat& hist, const fs::path& dirTXT) {
    ofstream txtOut(dirTXT);
    for (int i = 0; i < hist.rows; ++i)
        txtOut << i << "," << hist.at<float>(i) << "\n";
    txtOut.close();
}

void ecualizar_guardar(const Mat& img, const fs::path& direccion) {
    Mat ecualizada = ecualizacion(img);
    imwrite((direccion / "imagen_ecualizada.jpg").string(), ecualizada);

    Mat histograma_ecualizado = histograma(ecualizada);
    guardarHistogramaTXT(histograma_ecualizado, direccion / "frecuencias_ecualizada.txt");
    guardarHistogramaIMG(histograma_ecualizado, direccion / "histograma_ecualizado.png");
}

void binarizar_guardar(const Mat& ecualizada, const fs::path& direccion, int umbral) {
    Mat binarizada = binarizacion(ecualizada, umbral);
    imwrite((direccion / "imagen_binarizada.jpg").string(), binarizada);
}


int main() {
    string inputFolder = "Imgs";
    string outputFolder = "Resultados";
    fs::create_directories(outputFolder);
    int imgCount = 0;

    cout << "\n=====================\n";
    cout << "  LABORATORIO 4\n";
    cout << "=====================\n\n";


    for (const auto& entry : fs::directory_iterator(inputFolder)) {
        if (!entry.is_regular_file()) continue;


        // CREAR DIRECTORIO PARA CADA IMAGEN
        string path = entry.path().string();
        imgCount++;
        ostringstream folderName;
        folderName << "img_" << setfill('0') << setw(3) << imgCount;
        fs::path imgdireccion = fs::path(outputFolder) / folderName.str();
        fs::create_directories(imgdireccion);

	    cout << "▶ Procesando imagen " << imgCount << ": " << path << "\n";

        // LEER IMAGEN EN GRISES
        Mat img = imread(path, IMREAD_GRAYSCALE);
        imwrite((imgdireccion / "imagen.jpg").string(), img);
        cout << "  [+] Imagen cargada y guardada.\n";

        // Histograma original
        Mat hist = histograma(img);
        guardarHistogramaTXT(hist, imgdireccion / "frecuencias.txt");
        cout << "  [+] Vector de frecuencia guardado.\n";
        guardarHistogramaIMG(hist, imgdireccion / "histograma.png");
        cout << "  [+] Histograma guardado.\n";

        // Ecualización
        Mat ecualizada = ecualizacion(img);
        imwrite((imgdireccion / "imagen_ecualizada.jpg").string(), ecualizada);
        cout << "  [+] Imagen ecualizada guardado.\n";

        Mat histograma_ecualizado = histograma(ecualizada);
        guardarHistogramaTXT(histograma_ecualizado, imgdireccion / "frecuencias_ecualizada.txt");
        cout << "  [+] Vector de frecuencias ecualizado guardado.\n";
        guardarHistogramaIMG(histograma_ecualizado, imgdireccion / "histograma_ecualizado.png");
        cout << "  [+] Histograma ecualizado guardado.\n";

        // Binarización
        int umbral = 128;
        bool aprove = false;
        cout<<imgdireccion<<endl;
        do{
            cout<< "Seleccione umbral para esta imagen: ";cin>>umbral;
                binarizar_guardar(ecualizada, imgdireccion, umbral);
            cout<< "Conforme? ";cin>>aprove;
        }while(!aprove);
        cout << "  [+] Imagen binarizada guardada.\n";

        cout << "✅ Imagen procesada completamente.\n\n";
    }

    cout << "\n✅ Proceso completado. Total de imágenes: " << imgCount << endl;
    return 0;
}
