#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;


VideoCapture video;
uchar estructura[] = { 255, 0, 255, 0, 0, 0, 255, 0, 255 };
Mat estructurante(3, 3, CV_8UC1, estructura); 

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

void drawline(Mat& frame, Point p1, Point p2) {
	line(frame, p1, p2, Scalar(255, 0, 0), 2);
}

Point identifyPoint(const Mat& frame) {
    int sumaX = 0, sumaY = 0, contador = 0;
    for (int i = 0; i < frame.rows; i++) {
        for (int j = 0; j < frame.cols; j++) {
            Vec3b pixel = frame.at<Vec3b>(i, j);
            if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0) {
                sumaX += j;
                sumaY += i;
                contador++;
            }
        }
    }

    if (contador <= 100) return Point(-1, -1);
    //cout << "SumaX " << sumaX << " sumaY " << sumaY << endl;
	return Point(sumaX / contador, sumaY / contador);
}

int main() {
	// 1 Capurar video desde webcam o archivo   
    cout << "\n========================================\n";
    cout << "           LABORATORIO 5 - CS GRAFICA    \n";
    cout << "        Detección y Trayectoria de Objeto\n";
    cout << "========================================\n\n";
    bool flag = false;
    cout << ">> ¿Cómo deseas capturar el video?\n";
    cout << "    [0] En vivo (Webcam)\n";
    cout << "    [1] Desde archivo (Upload)\n";
    cout << "    Selección: ";
    cin >> flag;

    if(flag) {
        cout << "\n Cargando video desde archivo...\n";
        video.open("C:/Users/rushe/Documents/Universidad/S7/Graphics/CS-GRAFICA/Lab5/video1.mp4");
		//video.open("C:/Users/rushe/Pictures/Camera Roll/video4.mp4");
        if (!video.isOpened()) {
            cout << " Error: No se pudo abrir el video desde archivo.\n";
            return -1;
        }
        else {
            cout << " Video cargado correctamente.\n";
        }
    }
    else {
        cout << "\n Iniciando captura en vivo desde webcam...\n";
        cout << "   Presiona [ESC] para finalizar la grabación.\n";
        video.open(0);
    }

    cout << "\n Preparando el procesamiento de video...\n";
    Mat frame;
    Size frameSize;
    vector<Point> camino; // Almacenar puntos de trayectoria
	Point prevPoint(-1, -1); // Punto previo para dibujar la trayectoria
    int maxX = 0, maxY = 0;

    video.read(frame);
    frameSize = frame.size();  // Tamaño del video original

    if (frame.empty()) {
        cout << "Error! No se pudo capturar el frame. /n";
        return -1;
    }
    frameSize = frame.size();  

    while(true) {
        video.read(frame);
        if (frame.empty()) {cout << "Error! No se pudo capturar el frame. /n"; break;}

		imshow("Original", frame);

        // 2 Prepocesamiento de la imagen
		// Escala de grises y binarización
		escalaGris(frame);
		int umbral = 100; 
		my_binarization(frame, umbral);

        // 3 Detección de movimiento
		// dilatacion y erosión
		my_Dilate(frame);
        my_Erode(frame);

        cvtColor(frame, frame, COLOR_GRAY2BGR);
		// 4 deteción de centro de objeto
		Point currentPoint = identifyPoint(frame);

        cout << "-----------------------------\n";
        cout << " Seguimiento de objeto\n";
        cout << "   ▪ Punto anterior: (" << prevPoint.x << ", " << prevPoint.y << ")\n";
        cout << "   ▪ Punto actual  : (" << currentPoint.x << ", " << currentPoint.y << ")\n";
        cout << "-----------------------------\n";


        if(prevPoint.x == -1 && prevPoint.y == -1) { prevPoint = currentPoint; }
        if (currentPoint.x != -1 && currentPoint.y != -1) {
            camino.push_back(currentPoint);
            if (currentPoint.x > maxX) maxX = currentPoint.x;
            if (currentPoint.y > maxY) maxY = currentPoint.y;
            drawline(frame, prevPoint, currentPoint);
            prevPoint = currentPoint;
        }
		
        imshow("Video", frame);
        if (waitKey(30) == 27) break; // Presionar ESC para terminar
    }

    cout << "\n Finalizando captura...\n";
    cout << "  Dimensiones del video original: " << frameSize.width << " x " << frameSize.height << "\n";
    cout << "  Generando trayectoria completa...\n";


    Mat trajImage(frameSize,CV_8UC3, Scalar(0, 0, 0));
    for (size_t i = 1; i < camino.size(); i++) {
        cout << camino[i-1] << endl;
        line(trajImage, camino[i - 1], camino[i], Scalar(0, 255, 0), 2);
    }

    imshow("Trayectoria final", trajImage);
    waitKey(0);

    cout << "\n Proceso completado.\n";
    cout << "   La trayectoria ha sido generada y mostrada.\n";
    cout << "   Cierra la ventana para salir.\n";
    cout << "\n Gracias por usar el sistema de seguimiento.\n";

    video.release();
    destroyAllWindows();
    return 0;
}
