#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iomanip>


using namespace cv;
using namespace std;

namespace fs = filesystem;

Mat calcHistograma(const Mat& img) {
    Mat hist = Mat::zeros(256, 1, CV_32S); 
    int ancho = img.cols, alto = img.rows;

    for (int y = 0; y<alto; y++) {
        for (int x = 0; x<ancho; x++) {
            uchar pixel = img.at<uchar>(y, x);
            hist.at<int>(pixel, 0) += 1;
        }
    }

    return hist;
}



cv::Mat equalizeHistogramManual(const cv::Mat& img) {
    cv::Mat hist = calcHistogramManual(img);
    cv::Mat cdf = hist.clone();

    // Calcular CDF
    for (int i = 1; i < 256; ++i) {
        cdf.at<float>(i) += cdf.at<float>(i - 1);
    }

    // Normalizar CDF
    cdf /= (img.rows * img.cols);
    cdf *= 255;

    // Generar imagen ecualizada
    cv::Mat img_eq = img.clone();
    for (int i = 0; i < img.rows; ++i) {
        for (int j = 0; j < img.cols; ++j) {
            uchar pixel = img.at<uchar>(i, j);
            img_eq.at<uchar>(i, j) = static_cast<uchar>(cdf.at<float>(pixel));
        }
    }

    return img_eq;
}


cv::Mat thresholdManual(const cv::Mat& img, int thresh, uchar maxValue = 255) {
    cv::Mat binarized = img.clone();

    for (int i = 0; i < img.rows; ++i) {
        for (int j = 0; j < img.cols; ++j) {
            uchar pixel = img.at<uchar>(i, j);
            binarized.at<uchar>(i, j) = (pixel > thresh) ? maxValue : 0;
        }
    }

    return binarized;
}


// Función que ecualiza la imagen, guarda resultados en la carpeta destino
void equalizeHistogramAndSave(const cv::Mat& img, const fs::path& outputDir) {
    if (img.empty()) {
        std::cerr << "Imagen vacía para ecualizar.\n";
        return;
    }

    // Ecualizar histograma
    cv::Mat img_eq;
    cv::equalizeHist(img, img_eq);

    // Guardar imagen ecualizada
    cv::imwrite((outputDir / "imagen_ecualizada.jpg").string(), img_eq);

    // Calcular histograma ecualizado
    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange = {range};
    cv::Mat hist_eq;
    cv::calcHist(&img_eq, 1, 0, cv::Mat(), hist_eq, 1, &histSize, &histRange);

    // Guardar vector de frecuencias ecualizado
    std::ofstream txtOut(outputDir / "frecuencias_ecualizada.txt");
    for (int i = 0; i < histSize; ++i) {
        txtOut << i << "," << hist_eq.at<float>(i) << "\n";
    }
    txtOut.close();

    // Dibujar histograma ecualizado
    int hist_w = 512, hist_h = 400;
    int bin_w = cvRound((double)hist_w / histSize);
    cv::Mat histImage(hist_h, hist_w, CV_8UC1, cv::Scalar(255));
    cv::normalize(hist_eq, hist_eq, 0, histImage.rows, cv::NORM_MINMAX);

    for (int i = 1; i < histSize; ++i) {
        cv::line(histImage,
                 cv::Point(bin_w * (i - 1), hist_h - cvRound(hist_eq.at<float>(i - 1))),
                 cv::Point(bin_w * i, hist_h - cvRound(hist_eq.at<float>(i))),
                 cv::Scalar(0), 2);
    }

    cv::imwrite((outputDir / "histograma_ecualizado.png").string(), histImage);
}

// Función para binarizar imagen y guardar resultado
void binarizeImageAndSave(const cv::Mat& img_eq, const fs::path& outputDir, int thresholdValue) {
    if (img_eq.empty()) {
        std::cerr << "Imagen vacía para binarizar.\n";
        return;
    }

    cv::Mat img_bin;
    // Binarización simple: pixels > thresholdValue → 255, else 0
    cv::threshold(img_eq, img_bin, thresholdValue, 255, cv::THRESH_BINARY);

    cv::imwrite((outputDir / "imagen_binarizada.jpg").string(), img_bin);
}


int main() {
    string inputFolder = "Imgs";
    string outputFolder = "Resultados";
    fs::create_directories(outputFolder);

    int imgCount = 0;

    for (const auto& entry : fs::directory_iterator(inputFolder)) {
        if (!entry.is_regular_file()) continue;

        // Crear carpeta de salida para cada imagen
        string path = entry.path().string();
        imgCount++;
        ostringstream folderName;
        folderName<<"img_"<<setfill('0')<<setw(3)<<imgCount;
        fs::path imgOutputDir = fs::path(outputFolder) / folderName.str();
        fs::create_directories(imgOutputDir);

        // Cargar imagen y copiar a la nueva carpeta
        Mat img = imread(path, IMREAD_GRAYSCALE);
        imwrite((imgOutputDir / "imagen.jpg").string(), img);

        // HISTOGRAMA 
        int tam_histograma = 256;
        Mat hist = calcHistograma(img);
        //calcHist(&img, 1, 0, Mat(), hist, 1, &tam_histograma, &histrango);

        // IMAGEN HISTOGRAMA
        int hist_w = 512, hist_h = 400;
        int bin_w = cvRound((double)hist_w / tam_histograma);
        Mat histImage(hist_h, hist_w, CV_8UC1, Scalar(255));
        normalize(hist, hist, 0, histImage.rows, NORM_MINMAX);

        for (int i = 1; i < tam_histograma; ++i) {
            line(histImage,
                     Point(bin_w * (i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
                     Point(bin_w * i, hist_h - cvRound(hist.at<float>(i))),
                     Scalar(0), 2);
        }

        // GUARDAR HISTOGRAMA
        imwrite((imgOutputDir / "histograma.png").string(), histImage);

        // VECTOR DE FRECUENCIAS
        ofstream txtOut(imgOutputDir / "frecuencias.txt");
        for (int i = 0; i < tam_histograma; ++i) {
            txtOut<<i <<","<<hist.at<float>(i) <<"\n";
        }
        txtOut.close();

        // === LLAMADA A LA FUNCION DE ECUALIZACION ===
        equalizeHistogramAndSave(img, imgOutputDir);

        std::cout << "✅ Procesado y ecualizado: " << path << " → " << folderName.str() << "\n";

        cv::Mat img_eq = cv::imread((imgOutputDir / "imagen_ecualizada.jpg").string(), cv::IMREAD_GRAYSCALE);
        if (img_eq.empty()) {
            std::cerr << "No se pudo leer la imagen ecualizada para binarizar.\n";
        } else {
            // Ajusta aquí el umbral según lo que mejor funcione (por ejemplo, 128)
            int thresholdValue = 128;
            binarizeImageAndSave(img_eq, imgOutputDir, thresholdValue);
        }
        
    }

    cout<<"\n✅ Proceso completado. Total de imágenes: "<<imgCount <<endl;
    return 0;
}

