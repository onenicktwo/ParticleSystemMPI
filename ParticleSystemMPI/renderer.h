
#ifndef RENDERER_H
#define RENDERER_H

void display(void);
void reshape(int w, int h);
void timer(int value);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void mouseMotion(int x, int y); 
void synchronizeProcesses(void);
void cleanupAndExit(void);


#endif