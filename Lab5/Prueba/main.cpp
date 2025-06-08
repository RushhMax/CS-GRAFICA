#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;


uchar estructura[] = { 255, 0, 255, 0, 0, 0, 255, 0, 255 }; // Estructura de 3x3
Mat estructurante(3, 3, CV_8UC1, estructura); // inicializar

void printMat(const Mat &mat) {
    for (int i = 0; i < mat.rows; i++) {
        for (int j = 0; j < mat.cols; j++) {
            cout << (int)mat.at<uchar>(i, j) << " ";
        }
        cout << endl;
    }
}

bool esValido(const Mat& frame, int x, int y) {
    return (x >= 0 && x < frame.rows && y >= 0 && y < frame.cols);
}

void escalaGris(Mat &frame) {
    cvtColor(frame, frame, COLOR_BGR2GRAY);
}

void my_binarization(Mat &frame, int umbral) {
    Mat binarizado = frame.clone();
    for (int i = 0; i < frame.rows; i++) {
        for (int j = 0; j < frame.cols; j++) {
            if (frame.at<uchar>(i, j) > umbral) binarizado.at<uchar>(i, j) = 255;
			else binarizado.at<uchar>(i, j) = 0;
        }
    }
	frame = binarizado.clone();
}

bool fit(Mat frame, Mat estructurante, int x, int y) {
    for (int i = x; i < x + estructurante.rows; i++) {
        for(int j = y; j < y + estructurante.cols; j++) {
            if (i >= frame.rows || j >= frame.cols) return false;
            if(estructurante.at<uchar>(i - x, j - y) == 0 && frame.at<uchar>(i,j) != 0) {
                return false;
			}
        }
    }
    return true;
}

bool hit(Mat frame, Mat estructurante, int x, int y) {
    for (int i = x; i < x + estructurante.rows; i++) {
        for (int j = y; j < y + estructurante.cols; j++) {
            if (i >= frame.rows || j >= frame.cols) continue;
            if (estructurante.at<uchar>(i - x, j - y) == 0 && frame.at<uchar>(i, j) == 0) {
                return true;
            }
        }
    }
    return false;
}

void my_Erode(Mat &frame) {
    Mat eroded(frame.rows, frame.cols, CV_8UC1, 255);

    for (int i = 0; i < frame.rows; i++) {
        for (int j = 0; j < frame.cols; j++) {
            if (fit(frame, estructurante, i, j)) {
                eroded.at<uchar>(i + 1, j + 1) = 0;
            }
        }
    }
    frame = eroded.clone();
}

void dilating(Mat& frame, int x, int y) {
    for (int i = x; i < x + estructurante.rows; i++) {
        for (int j = y; j < y + estructurante.cols; j++) {
            if (estructurante.at<uchar>(i - x, j - y) == 0 && esValido(frame, i, j)) {
                frame.at<uchar>(i, j) = 0;
        
            }
        }
    }
}

void my_Dilate(Mat &frame) {
	Mat dilated = frame.clone();
    
    for (int i = 0; i < frame.rows; i++) {
        for (int j = 0; j < frame.cols; j++) {
            if (hit(frame, estructurante, i, j)) {
				dilating(dilated, i, j);
            }
        }
    }
	frame = dilated.clone();
}

VideoCapture video;

int main() {
	// 1 Capurar video desde webcam o archivo   
    cout << "INICIO\n";
    bool flag = false;
    cout << "Como desea capturar el video? LIVE (0) / UPLOAD (1)  \n"; cin >> flag;

    if(flag) {
        cout << "Subiendo video...\n";
		video.open("C:/Users/rushe/Pictures/Camera Roll/video2.mp4");
        if (!video.isOpened()) {
            cout << "Error! No se pudo abrir el video.\n";
            return -1;
		}
    }
    else {
		cout << "Capturando video en vivo.../n";
		cout << "Presione ESC para terminar la captura./n";
		video.open(0);
    }

    Mat frame;
    vector<Point> camino; // Almacenar puntos de trayectoria

    for (;;) {
        // 2 Prepocesamiento de la imagen
        // Escala de grises
        // Binarización
        video.read(frame);
        if (frame.empty()) {
            cout << "Error! No se pudo capturar el frame. /n";
            break;
        }

		// Escala de grises
		escalaGris(frame);
		// Binarización
		int umbral = 100; // Umbral para binarización
		my_binarization(frame, umbral);
        //threshold(frame, frame, umbral, 255, THRESH_BINARY);

        // 3 Detección de movimiento
        // erosión y dilatación
        //my_Dilate(frame);
		my_Erode(frame);
		my_Dilate(frame);
        //my_Erode(frame);
        imshow("Detección", frame);

        // 4 deteción de contornos

        if (waitKey(30) == 27) break; // Presionar ESC para terminar
    }

	cout << "Finalizando captura...\n";

    video.release();
    destroyAllWindows();

    // Mostrar trayectoria al final
    //Mat trajImage(480, 640, CV_8UC3, Scalar(0, 0, 0));
    //for (size_t i = 1; i < camino.size(); i++) {
    ///    line(trajImage, camino[i - 1], camino[i], Scalar(0, 255, 0), 2);
    //}

    //imshow("Trayectoria final", trajImage);
    //waitKey(0);
    return 0;
}
