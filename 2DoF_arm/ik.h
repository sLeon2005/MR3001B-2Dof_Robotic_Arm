#ifndef IK_H
#define IK_H

#ifndef PI
#define PI 3.14159265358979323846
#endif

// IK formulas
float theta1Rad(float x, float y, float link1, float link2);
float theta2Rad(float x, float y, float link1, float link2);
float theta1Deg(float x, float y, float link1, float link2);
float theta2Deg(float x, float y, float link1, float link2);

#endif