#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include </home/alu/Escritorio/SE-p3/ej6-3/cxxtimer.hpp>  // Timers (Cambiar directorio si es necesario, compilar con --std=c++11 o superior)

#define MAX_LIFE     5    // Vidas maximas
#define MIN_DISTANCE 50   // Distancia minima entre centros de masas
#define MIN_TAIL     5    // Longitud inicial de la cola

using namespace std;
using namespace cv;
using namespace chrono;
using namespace cxxtimer;

void displayStartText(const Mat &frame);
bool checkTail(const vector<Point2i> &tail, const Point2i &pointBall);
void getBall(Mat &frame, Timer &timer, bool &flagTimer,  Timer &timerPoint, bool &nextPoint, unsigned &typePoint, const unsigned &radius, 
             Point2i &pointBall, const Point2i &pointHead, const unsigned &dcom, const vector<Point2i> tail);
void getTail(vector<Point2i> &tail, const Point2i &pointHead, Mat &frame, const unsigned &lengthTail);
void drawTail(Mat &frame, const vector<Point2i> &tail);
unsigned getDistanceBetweenCOM(Mat &frame, const Point2i &pointBall, Point2i &pointHead, vector<Point2i> &tail, const unsigned &lengthTail);
void evaluateDistanceBetweenCOM(Mat &frame, const unsigned &dcom, bool &nextPoint, const unsigned &typePoint, unsigned &score, unsigned &life, 
                                const Point2i &pointBall, unsigned &lengthTail);
void evaluateTail(const vector<Point2i> &tail, const Point2i &pointBall, unsigned &life);
void displayScreenText(const Mat &frame, const unsigned &life, const unsigned &score);
void displayGAMEOVERText(const Mat &frame, const unsigned &score);

int main(int argc, const char* argv[]) 
{
    VideoCapture cap(0); 
    
    // Comprobamos si la camara se abrio con exito
    if(!cap.isOpened())
    {
        cout << "Error opening video stream or file" << endl;
        return -1;
    }

    // Inicilizacion de variables
    bool start = false;              // Muestra el mensaje de inicio
    unsigned score = 0;              // Puntuacion
    unsigned life = MAX_LIFE;        // Vidas
    unsigned radius = 15;            // Radio de bola
    Point2i pointHead;               // Posicion del jugador (cabeza de la serpiente/posicion actual del jugador)
    Point2i pointBall;               // Posicion de la bola
    unsigned typePoint;              // Tipo del punto
    srand(time(NULL));               // Semilla
    cxxtimer::Timer timer;           // Tiempo entre puntos
    cxxtimer::Timer timerPoint;      // Duracion de los puntos en pantalla
    bool flagTimer = false;          // Controla el flag de timer
    unsigned dcom = 0;               // Distancia entre centro de masas
    bool nextPoint = false;          // Comprueba si el punto ha sido obtenido
    vector<Point2i> tail;            // Cola de la serpiente
    unsigned lengthTail = MIN_TAIL;  // Longitud de la cola de la serpiente

    while(1)
    {
        Mat frame;
        // Capturamos frame a frame
        cap >> frame;
        // Solucionamos el efecto espejo
        flip(frame, frame, 1);
        // Si el frame esta vacio, se acaba el programa
        if (frame.empty())
            break;
        
        if (not start)
        {
            // Mostramos el frame original con el texto de inicio
            displayStartText(frame);
            // Presiona SCAPE en el teclado para empezar la partida
            if ((char)waitKey(25) == 32)
                start = true;
        }
        else
        {
            if (life > 0)
            {
                // Obtenemos la siguiente bola y la mostramos por pantalla durante cierto tiempo, dado con timers
                getBall(frame, timer, flagTimer, timerPoint, nextPoint, typePoint, radius, pointBall, pointHead, dcom, tail);

                // Hallamos la distancia entre los centros de masas de la cabeza de la serpiente y la nueva bola, y la evaluamos
                dcom = getDistanceBetweenCOM(frame, pointBall, pointHead, tail, lengthTail);
                evaluateDistanceBetweenCOM(frame, dcom, nextPoint, typePoint, score, life, pointBall, lengthTail);

                // Comprobamos si la cola no se cruza con ningun punto, si es asi, el jugador pierde una vida
                evaluateTail(tail, pointBall, life);

                // Mostramos el frame original con el texto del juego
                displayScreenText(frame, life, score);
            }
            else
                // Mostramos el frame original con el texto de GAME OVER
                displayGAMEOVERText(frame, score);
        }
        // Mostramos el nuevo frame
        imshow("Frame", frame);
        
        // Presiona ESC en el teclado para finalizar el programa
        if ((char)waitKey(25) == 27)
            break;
    }

    // Cuando todo acaba, sacamos el objeto de la capura de video
    cap.release();
    // Cerramos todos los frames
    destroyAllWindows();
} 

void displayStartText(const Mat &frame)
{
    putText(frame, "REGLAS", Point(210,100), FONT_HERSHEY_DUPLEX, 2, Scalar(0,0,255), 2);
    putText(frame, "1. El jugador solamente es detectado como objeto naranja", Point(80,150), FONT_HERSHEY_DUPLEX, 0.5, Scalar(0,0,255), 1);
    putText(frame, "2. Preferiblemente es el uso de una naranja (fruta)", Point(80,170), FONT_HERSHEY_DUPLEX, 0.5, Scalar(0,0,255), 1);
    putText(frame, "3. Intentar que no haya otro objeto naranja en pantalla", Point(80,190), FONT_HERSHEY_DUPLEX, 0.5, Scalar(0,0,255), 1);
    putText(frame, "4. Las bolas verdes dan 1 punto", Point(80,210), FONT_HERSHEY_DUPLEX, 0.5, Scalar(0,0,255), 1);
    putText(frame, "5. Las bolas rojas quitan 1 vida", Point(80,230), FONT_HERSHEY_DUPLEX, 0.5, Scalar(0,0,255), 1);
    putText(frame, "6. Si la cola se cruza con una bola verde, obtienes 1 punto", Point(80,250), FONT_HERSHEY_DUPLEX, 0.5, Scalar(0,0,255), 1);
    putText(frame, "7. Si la cola se cruza con una bola roja, pierdes 1 vida", Point(80,270), FONT_HERSHEY_DUPLEX, 0.5, Scalar(0,0,255), 1);
    putText(frame, "8. La cola crece a medida que obtienes puntos", Point(80,290), FONT_HERSHEY_DUPLEX, 0.5, Scalar(0,0,255), 1);
    putText(frame, "9. Si te quedas sin vidas, se acaba la partida", Point(80,310), FONT_HERSHEY_DUPLEX, 0.5, Scalar(0,0,255), 1);
    putText(frame, "10. Intentar conseguir el maximo numero de puntos", Point(80,330), FONT_HERSHEY_DUPLEX, 0.5, Scalar(0,0,255), 1);
    putText(frame, "SCAPE: empezar", Point(180,400), FONT_HERSHEY_DUPLEX, 1, Scalar(0,0,255), 2);
}

// Comprueba que el siguiente punto no coincida con alguno de la cola
bool checkTail(const vector<Point2i> &tail, const Point2i &pointBall)
{
    bool check = false;

    for (unsigned i = 0 ; i < tail.size() and not check ; i++)  // Si ya ha coincidido con uno, no seguimos recorriendo la cola
    {
        if (sqrt(pow(tail[i].x - pointBall.x, 2)+pow(tail[i].y - pointBall.y, 2)) <= MIN_DISTANCE)
            check = true;
    }

    return check;
}

void getBall(Mat &frame, Timer &timer, bool &flagTimer,  Timer &timerPoint, bool &nextPoint, unsigned &typePoint, const unsigned &radius, 
             Point2i &pointBall, const Point2i &pointHead, const unsigned &dcom, const vector<Point2i> tail)
{
    if (not flagTimer)
    {
        timer.start();
        if (timer.count<seconds>() >= 2)  // Cada 2 segundos 
        {   
            do{
                 // Elegimos el siguiente punto para que no coincida con el punto donde se encuentra el jugador actualmente
                 // y tampoco puede coincidir con los puntos de la cola que hay actualmente en pantalla
                pointBall = Point2i((2*radius)+rand()%(frame.cols-(2*radius)), (2*radius)+rand()%(frame.rows-(2*radius))); 
            }while(sqrt(pow(pointHead.x - pointBall.x, 2)+pow(pointHead.y - pointBall.y, 2)) <= MIN_DISTANCE and checkTail(tail, pointBall));
            typePoint = 1+rand()%2;  // Seleccionamos de que tipo sera
            nextPoint = true;        // Podemos obtener dicho punto
            flagTimer = true;        // Podemos pasar al siguiente timer (duracion del punto en pantalla)
            timer.reset(); 
        }
    }
    else
    {
        timerPoint.start();
        if (timerPoint.count<milliseconds>() <= 750 and nextPoint)  // Durante 3/4 segundos
        {
            // Dibujamos el objeto seleccionado, el cual puede ser:
            switch(typePoint)
            {
                case 1:
                    circle(frame, pointBall, radius, Scalar(0,255,0), CV_FILLED);  // Circulo verde
                    break;
                case 2:
                    circle(frame, pointBall, radius, Scalar(0,0,255), CV_FILLED);  // Circulo rojo
                    break;
            }
        }
        else
        {
            timerPoint.reset();
            flagTimer = false;
        }
    }
}

void getTail(vector<Point2i> &tail, const Point2i &pointHead, Mat &frame, const unsigned &lengthTail)
{
    if (tail.size() <= lengthTail)  // Mientras no se llegue al maximo dado
        tail.push_back(pointHead);  // Guardamos los puntos visitados por el jugador (cola)
    if (tail.size() == lengthTail)  // Cuando llegamos al maximo hasta el momento
        tail.erase(tail.begin());   // Eliminamos el primer punto guardado (punto mas alejado de la cabeza)

    drawTail(frame, tail);          // Mostramos la trayectoria de la cola
}

void drawTail(Mat &frame, const vector<Point2i> &tail)
{
    for (unsigned i = 1 ; i < tail.size() ; i++)
        line(frame, tail[i-1], tail[i], Scalar(255,0,0), 5, CV_FILLED);  // Unimos los puntos 
}

unsigned getDistanceBetweenCOM(Mat &frame, const Point2i &pointBall, Point2i &pointHead, vector<Point2i> &tail, const unsigned &lengthTail)
{
    unsigned dcom = 0;

    // Inicilizacion de variables
    Mat hsv, mask, canny;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Point2i totalPoints = Point2i(0, 0);
    unsigned nPoints = 0;

    // Convertimos la imagen RGB a HSV
    cvtColor(frame, hsv, COLOR_BGR2HSV);
    
    // Obtenemos unicamente naranjas (rango de naranjas bajos y altos)
    inRange(hsv, Scalar(10,140,90), Scalar(20,200,160), mask);

    // Reducimos el ruido
    GaussianBlur(mask, mask, Size(3, 3), 0);			
    morphologyEx(mask, mask, MORPH_OPEN, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));	
    
    // Detectamos bordes con Canny
    Canny(mask, canny, 50, 150);
    // Buscamos dichos contornos
    findContours(canny, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

    // Obtenemos los momentos
    vector<Moments> mu(contours.size());
    for (unsigned i = 0; i < contours.size(); i++)
        mu[i] = moments(contours[i], false);     
    // Obtenemos los centros de masas de los objetos
    for (unsigned i = 0; i < contours.size(); i++)
    {
        if (mu[i].m00 > 0)
        {
            totalPoints.x += mu[i].m10/mu[i].m00;
            totalPoints.y += mu[i].m01/mu[i].m00;
            nPoints++;
        }
    }
    if (nPoints > 0)
    {
        pointHead.x = totalPoints.x/nPoints;
        pointHead.y = totalPoints.y/nPoints;
        // Dibujamos un circulo azul en el centro de masas medio del objeto (cabeza de la serpiente)
        circle(frame, pointHead, 7, Scalar(255,0,0), CV_FILLED);  
        // Obtenemos la cola de la serpiente
        getTail(tail, pointHead, frame, lengthTail);
        // Distancia entre los centros de masas (distancia euclidea)
        dcom = sqrt(pow(pointHead.x - pointBall.x, 2) +  pow(pointHead.y - pointBall.y, 2)); 
    }
    else
    {
        pointHead = Point2i(-1,-1);
        tail.clear();  // Si la serpiente no tiene cabeza (punto final cola), no hay cola
    }

    return dcom;
} 

void evaluateDistanceBetweenCOM(Mat &frame, const unsigned &dcom, bool &nextPoint, const unsigned &typePoint, unsigned &score, unsigned &life, 
                                const Point2i &pointBall, unsigned &lengthTail)
{
    if (dcom <= MIN_DISTANCE and dcom != 0 and nextPoint)  // Si hemos obtenido algun punto
    {
        Timer timer;
        timer.start();
        if (typePoint == 1)       // Circulo verde obtenido
        { 
            if (timer.count<milliseconds>() <= 250)  // Durante 1/4 segundos
                drawMarker(frame, pointBall,  Scalar(0, 0, 0), MARKER_CROSS, 20, 2);  // Marcamos con una cruz el punto
            else
                timer.reset();
            score++;
            lengthTail = lengthTail + MIN_TAIL;  // La cola crece
        }
        else if (typePoint == 2)  // Circulo rojo obtenido
        {
            if (timer.count<milliseconds>() <= 250)  // Durante medio segundo
                drawMarker(frame, pointBall,  Scalar(0, 0, 0), MARKER_CROSS, 20, 2);  // Marcamos con una cruz el punto
            else
                timer.reset();
            life--;
        }
        nextPoint = false;  // Reiniciamos dicha condicion
    }
}

void evaluateTail(const vector<Point2i> &tail, const Point2i &pointBall, unsigned &life)
{
    for (unsigned i = 0 ; i < tail.size() ; i++)  
    {
        if (tail[i] == pointBall)  // Si la cola se cruza con alguna bola, el jugador pierde una vida
            life--;
    }
}

void displayScreenText(const Mat &frame, const unsigned &life, const unsigned &score)
{
    // Mostramos los marcadores
    putText(frame, "SNAKE", Point(frame.cols/2-70,frame.rows-440), FONT_HERSHEY_DUPLEX, 1, Scalar(64,64,64), 2);
    putText(frame, "(ESC:salir)", Point(frame.cols/2-57,frame.rows-425), FONT_HERSHEY_DUPLEX, 0.45, Scalar(64,64,64), 1.5);
    // Mostramos las estadisticas hasta el momento
    char s1[50];
    sprintf(s1, "Vidas: %d", life);
    putText(frame, s1, Point(frame.cols-610,frame.rows-440), FONT_HERSHEY_DUPLEX, 0.8, Scalar(64,64,64), 2);
    char s2[50];
    sprintf(s2, "Puntuacion: %d", score);
    putText(frame, s2, Point(frame.cols-210,frame.rows-440), FONT_HERSHEY_DUPLEX, 0.8, Scalar(64,64,64), 2);
}

void displayGAMEOVERText(const Mat &frame, const unsigned &score)
{
    // Mostramos los marcadores
    putText(frame, "GAME OVER", Point(130,200), FONT_HERSHEY_DUPLEX, 2.2, Scalar(0,0,255), 2);
    putText(frame, "ESC:salir", Point(100,250), FONT_HERSHEY_DUPLEX, 1, Scalar(0,0,255), 2);
    // Mostramos las estadisticas finales
    char s[50];
    sprintf(s, "Puntuacion: %d", score);
    putText(frame, s, Point(330,250), FONT_HERSHEY_DUPLEX, 1, Scalar(0,0,255), 2);
}
