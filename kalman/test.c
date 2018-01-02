#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "./libfixmath/fix16.h"

typedef struct fastKalmanI_s {
    fix16_t q;       //process noise covariance
    fix16_t r;       //measurement noise covariance
    fix16_t p;       //estimation error covariance matrix
    fix16_t k;       //kalman gain
    fix16_t x;       //state
    fix16_t lastX;   //previous state
    fix16_t fixed1;   
} fastKalmanI_t;

//Fast two-state Kalman
void fastKalmanInitI(fastKalmanI_t *filter, fix16_t q, fix16_t r, fix16_t p)
{
	filter->q     = q;
	filter->r     = r;
	filter->p     = p;
    // printf("INT: q: %f r: %f\n", fix16_to_float(filter->q), fix16_to_float(filter->r));
	filter->x     = fix16_from_int(0);   //set intial value, can be zero if unknown
	filter->lastX = fix16_from_int(0);   //set intial value, can be zero if unknown
	filter->k     = fix16_from_int(0);   //kalman gain, will be between 0-1
	filter->fixed1= fix16_from_int(1);   
}

fix16_t fastKalmanUpdateI(fastKalmanI_t *filter, fix16_t input)
{
    //project the state ahead using acceleration
    filter->x += (filter->x - filter->lastX);

    //update last state
    filter->lastX = filter->x;

	//prediction update
	filter->p = filter->p + filter->q;

	//measurement update
    // filter->k = (filter->p) / (filter->p + filter->r);
    // int div = filter->p + filter->r;//divisor
    // float d = filter->p/(float)div;

    //multiply J * 1000000
    //i is remainder
    //add i to J.
    // printf("INT: remainder: %i\n", i);
	filter->k = fix16_sdiv(filter->p, fix16_sadd(filter->p, filter->r));

    // filter->k = (d * 1000000);
    // printf("INT: k: %f\n", filter->k);
    printf("INT: k: %f\n", fix16_to_float(filter->k));


	filter->x += filter->k * (input - filter->x);
    // printf("INT: x: %fix16_t\n", filter->x);
    
	filter->p = (filter->fixed1 - filter->k) * (filter->p);
    // printf("INT: p: %f\n", filter->p);

    // printf("INT -- k: %i, q: %i, r: %i, p: %i, x: %i\n", filter->k, filter->q, filter->r, filter->p, filter->x );
    // static int xxx = 0;
    // xxx++;
    // if(xxx == 100)
    //     while(1);

    return filter->x;
}


typedef struct fastKalmanF_s {
    float q;       //process noise covariance
    float r;       //measurement noise covariance
    float p;       //estimation error covariance matrix
    float k;       //kalman gain
    float x;       //state
    float lastX;   //previous state
} fastKalmanF_t;

//Fast two-state Kalman
void fastKalmanInitF(fastKalmanF_t *filter, float q, float r, float p, float intialValue)
{
	filter->q     = q * 0.001f; //add multiplier to make tuning easier
	filter->r     = r;    //add multiplier to make tuning easier
    // printf("FLT: q: %f r: %f\n", filter->q, filter->r);
	filter->p     = p;    //add multiplier to make tuning easier
	filter->x     = intialValue;   //set intial value, can be zero if unknown
	filter->lastX = intialValue;   //set intial value, can be zero if unknown
	filter->k     = 0.0f;          //kalman gain,  
}

float fastKalmanUpdateF(fastKalmanF_t *filter, float input)
{

    //project the state ahead using acceleration
    filter->x += (filter->x - filter->lastX);

    //update last state
    filter->lastX = filter->x;

	//prediction update
	filter->p = filter->p + filter->q;

	//measurement update
	filter->k = filter->p / (filter->p + filter->r);
    printf("FLT: k: %f\n", filter->k);

	filter->x += filter->k * (input - filter->x);
    // printf("FLT: x: %f\n", filter->x);

	filter->p = (1.0f - filter->k) * filter->p;
    // printf("FLT: p: %f\n", filter->p);

    // printf("FLT -- k: %f, q: %f, r: %f, p: %f, x: %f\n", filter->k,  filter->q, filter->r, filter->p, (filter->x) );

    return filter->x;
}

const char* getfield(char* line, int num)
{
    const char* tok;
    for (tok = strtok(line, ",");
            tok && *tok;
            tok = strtok(NULL, ",\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}

int main()
{
    int cleanGyro;
    int rawGyro;

    FILE* stream = fopen("32data.csv", "r");

    char line[1024];

/*
set filter_type1=0
yaw quick = 25.0f
yaw rap = 88.0f
*/

    fastKalmanI_t fastKalmanInt;
    fastKalmanF_t fastKalmanFloat;

    //32768 multiplier is 1 << 15
    //	filter->q     = q * 0.000001f; //add multiplier to make tuning easier
    //filter->r     = r * 0.001f; 
    
    //fastKalmanInitF(&fastKalmanFloat, 25.0f, 88.0f, 0.0f, 0.0f);
    fastKalmanInitF(&fastKalmanFloat, 25.0f, 88.0f, 40.0f, 0.0f);
    //0.8192
    //32768
    fastKalmanInitI(&fastKalmanInt, fix16_from_float(25.001), fix16_from_int(88), fix16_from_int(40));

    volatile int retInt;
    volatile int retFloat;
    // fix16_t deg = fix16_from_float(16.4);
    while (fgets(line, 1024, stream))
    {
        
        char* tmp = strdup(line);
        
        // cleanGyro = atoi( getfield(tmp, 3) );
        rawGyro = atoi( getfield(tmp, 4) );
        
        tmp = strdup(line);
        // NOTE strtok clobbers tmp
        free(tmp);

        retFloat = fastKalmanUpdateF(&fastKalmanFloat, (rawGyro/16.4f));
        retInt = fastKalmanUpdateI(&fastKalmanInt, fix16_sdiv(fix16_from_float(rawGyro), fix16_from_float(16.4f)))

        //printf("raw: %i, clean: %i, float: %i, int: %i\n", rawGyro, cleanGyro, (int)(retFloat*16.4), retInt );

    }

}