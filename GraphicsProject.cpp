#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

using namespace std;

#define WIDTH 1200
#define HEIGHT 600
#define PI 3.14159265358979323846
#define raysN 500
#define objectsN 3

double circleX = 300, circleY = 300, circleR = 40;
double lastCircleX = 300, lastCircleY = 300;
bool dragging = false;

struct Circle
{
    double x, y;
    double r;
};

struct Ray
{
    double xStart, yStart;
    double angle;
};

void drawCircle(struct Circle circle)
{
    glBegin(GL_POLYGON); //starts drawing a filled circle
    for (int i = 0; i < 360; i++)
    {
        float theta = i * PI / 180; //degrees to radians
        glVertex2f(circle.x + circle.r * cos(theta), circle.y + circle.r * sin(theta)); //calculates x and y coords
    }
    glEnd(); //ends shape
}

void generateRay(struct Circle circle, struct Ray rays[raysN])
{
    for (int i = 0; i < raysN; i++)
    {
        double angle = ((double)i / raysN) * 2 * PI;
        rays[i].xStart = circle.x;
        rays[i].yStart = circle.y;
        rays[i].angle = angle;
        cout << "Angle = " << angle << endl;
    }
}

void drawRay(struct Ray rays[raysN], struct Circle objects[])
{
    static bool lineWidthSet = false;
    if (!lineWidthSet) {
        glLineWidth(10);
        lineWidthSet = true;
    }

    const int maxBounces = 3; // Set how many reflections you want

    for (int i = 0; i < raysN; i++)
    {
        Ray ray = rays[i];
        double step = 1;
        double xDraw = ray.xStart, yDraw = ray.yStart;
        double dx = cos(ray.angle);
        double dy = sin(ray.angle);

        int currentBounce = 0;
        bool continueRay = true;

        while (continueRay && currentBounce <= maxBounces)
        {
            int endOfScreen = 0, objectHit = 0;
            double xStart = xDraw, yStart = yDraw;

            // Choose color depending on bounce
            if (currentBounce == 0)
                glColor3f(1.0, 0.8, 0.2); // Primary ray (yellow)
            else if (currentBounce == 1)
                glColor3f(1, 1, 1); // 1st reflection (blue)
            else if (currentBounce == 2)
                glColor3f(0.8, 0.2, 1.0); // 2nd reflection (purple)
            else
                glColor3f(0.2, 1.0, 0.8); // 3rd+ reflection (cyan)

            glBegin(GL_POINTS);
            while (!endOfScreen && !objectHit)
            {
                xDraw = xDraw + step * dx;
                yDraw = yDraw + step * dy;
                glVertex2f(xDraw, yDraw);

                if (xDraw < 0 || xDraw > WIDTH || yDraw < 0 || yDraw > HEIGHT)
                {
                    endOfScreen = 1;
                }

                for (int j = 0; j < objectsN; j++)
                {
                    double radiusSquared = pow(objects[j].r, 2);
                    double distanceSquared = pow(xDraw - objects[j].x, 2) + pow(yDraw - objects[j].y, 2);
                    if (distanceSquared < radiusSquared)
                    {
                        objectHit = 1;
                    }
                }
            }
            glEnd();

            if (objectHit)
            {
                // Find which object was hit
                int collidedObject = -1;
                for (int j = 0; j < objectsN; j++)
                {
                    double radiusSquared = pow(objects[j].r, 2);
                    double distanceSquared = pow(xDraw - objects[j].x, 2) + pow(yDraw - objects[j].y, 2);
                    if (distanceSquared < radiusSquared)
                    {
                        collidedObject = j;
                        break;
                    }
                }

                if (collidedObject != -1)
                {
                    // Calculate reflection
                    double nx = (xDraw - objects[collidedObject].x) / objects[collidedObject].r;
                    double ny = (yDraw - objects[collidedObject].y) / objects[collidedObject].r;
                    double normalLength = sqrt(nx * nx + ny * ny);
                    nx /= normalLength;
                    ny /= normalLength;

                    double dot = dx * nx + dy * ny;
                    dx = dx - 2 * dot * nx;
                    dy = dy - 2 * dot * ny;

                    // Set new starting point slightly forward to avoid getting stuck
                    xDraw = xDraw + dx * step;
                    yDraw = yDraw + dy * step;
                }
            }
            else
            {
                continueRay = false; // No more collisions, stop
            }

            if (endOfScreen)
            {
                continueRay = false; // Out of bounds, stop
            }

            currentBounce++;
        }
    }
}




Circle shadow = { 700, 300, 80 };
double speed = 5;

void updateShadow(int value)
{
    // Update shadow's position continuously every frame
    shadow.y += speed;

    // Only change direction if needed
    if (shadow.y - shadow.r <= 0 || shadow.y + shadow.r >= HEIGHT) {
        speed = -speed;
    }

    // Only request redisplay if the position has actually changed
    glutPostRedisplay();
    glutTimerFunc(16, updateShadow, 0); // Update every 16ms (60 FPS)
}

bool needToGenerateRays(double lightX, double lightY, double lastLightX, double lastLightY, double threshold = 2.0)
{
    // Check if the light has moved significantly (you can tweak the threshold)
    return (abs(lightX - lastLightX) > threshold || abs(lightY - lastLightY) > threshold);
}

void Display()
{
    static Ray rays[raysN];
    Circle light = { circleX, circleY, circleR };
    Circle shadow2 = { 900, 525, 40 };
    Circle shadow3 = { 900, 120, 40 };
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0, 1.0, 1.0); // white
    drawCircle(light);
    drawCircle(shadow);
    drawCircle(shadow2);
    drawCircle(shadow3);
    Circle shadows[objectsN];
    shadows[0] = shadow;
    shadows[1] = shadow2;
    shadows[2] = shadow3;
    // Only generate rays if the light circle has moved significantly
    if (needToGenerateRays(light.x, light.y, lastCircleX, lastCircleY)) {
        generateRay(light, rays);
        lastCircleX = light.x;
        lastCircleY = light.y;
    }

    glColor4f(1.0, 0.8, 0.2, 0.2); // ray color
    drawRay(rays, shadows);

    glFlush();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Get window size dynamically
        int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
        int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

        // Get OpenGL viewport coordinates
        double modelview[16], projection[16];
        int viewport[4];
        double glX, glY, glZ;

        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
        glGetDoublev(GL_PROJECTION_MATRIX, projection);
        glGetIntegerv(GL_VIEWPORT, viewport);

        // Convert mouse coordinates to OpenGL coordinates
        gluUnProject(x, HEIGHT - y, 0, modelview, projection, viewport, &glX, &glY, &glZ);

        // Check if click is inside the circle
        float dx = glX - circleX;
        float dy = glY - circleY;
        if (dx * dx + dy * dy <= circleR * circleR) {
            dragging = true;
        }
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        dragging = false;
    }
}

void motion(int x, int y)
{
    if (dragging) {
        int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
        int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

        double modelview[16], projection[16];
        int viewport[4];
        double glX, glY, glZ;

        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
        glGetDoublev(GL_PROJECTION_MATRIX, projection);
        glGetIntegerv(GL_VIEWPORT, viewport);

        gluUnProject(x, HEIGHT - y, 0, modelview, projection, viewport, &glX, &glY, &glZ);

        if (glX != circleX || glY != circleY) {  // Only update and redraw if position changed
            circleX = glX;
            circleY = glY;
            //glutPostRedisplay();  // Request redraw
        }
    }
}

void init()
{
    glClearColor(0.0, 0.0, 0.0, 1.0); //background black
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);

    // Get screen width and height
    int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);

    // Calculate window position (center the window)
    int windowPosX = (screenWidth - WIDTH) / 2;
    int windowPosY = (screenHeight - HEIGHT) / 2;
    glutCreateWindow("Raytracing");
    glutPositionWindow(windowPosX, windowPosY);

    init();
    glutDisplayFunc(Display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(16, updateShadow, 0); // Update every 16ms (60 FPS)
    glutMainLoop();
    return 0;
}
