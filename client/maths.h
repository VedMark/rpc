/*
 * Please do not edit this file.
 * It was generated using stubgen.
 */

#ifndef _HS_H
#define _HS_H

class ClientRPC;

#ifdef __cplusplus
extern "C" {
#endif


#define MATH_PROG ((unsigned long) (0x20000001))

#define MATH_VER ((unsigned long) (1))
void math_prog_1(int proc, void *argBuff, size_t szArgBuff);

#define SQR ((unsigned long) (1))
extern float * sqr_1(float*, ClientRPC *);
extern float * sqr_1_svc(float*);
#define EXP ((unsigned long) (2))
extern float * exp_1(float*, ClientRPC *);
extern float * exp_1_svc(float*);
#define LOG10 ((unsigned long) (3))
extern float * log10_1(float*, ClientRPC *);
extern float * log10_1_svc(float*);


//#define MATH_VER ((unsigned long) (2))
void math_prog_2(int proc, void *argBuff, size_t szArgBuff);

//#define SQR ((unsigned long) (1))
extern float * sqr_2(float*, ClientRPC *);
extern float * sqr_2_svc(float*);
//#define EXP ((unsigned long) (2))
extern float * exp_2(float*, ClientRPC *);
extern float * exp_2_svc(float*);
//#define LOG10 ((unsigned long) (3))
extern float * log10_2(float*, ClientRPC *);
extern float * log10_2_svc(float*);
#define ABS ((unsigned long) (4))
extern float * abs_2(float*, ClientRPC *);
extern float * abs_2_svc(float*);


#ifdef __cplusplus
}
#endif

#endif // _HS_H
