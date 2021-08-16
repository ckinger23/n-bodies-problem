/*
 *  UPC NBody
 *  Carter King
 *  Dr. Larkins
 *  Parallel
 *  3/18/19
 */


#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "upc_relaxed.h"
//#include "upc_strict.h"
#define N 10000
#define G 6.67e-11
#define TIMESTEP 0.25
#define NSTEPS 10
shared int globalarray[THREADS];

/*
 * body data structure
 */
struct body_s {
  double x;
  double y;
  double z;
  double dx;
  double dy;
  double dz;
  double mass;
};
typedef struct body_s body_t;

shared [N / THREADS] body_t bodies[N];
shared [N / THREADS] body_t next[N];


/*
 * init - give the planets initial values for position, velocity, mass
 */


void init(void) {
  for (int i=0; i<N; i++) {
    bodies[i].x = 100.0 * (i + 0.1);
    bodies[i].y = 200.0 * (i + 0.1);
    bodies[i].z = 300.0 * (i + 0.1);
    bodies[i].dx = i + 400.0;
    bodies[i].dy = i + 500.0;
    bodies[i].dz = i + 600.0;
    bodies[i].mass = 10e6 * (i + 100.2);
  }
}


/*
 * dist - determine the distance between two bodies
 * @param dx - distance in the x dimension
 * @param dy - distance in the y dimension
 * @param dz - distance in the z dimension
 * @return distance 
 */


double dist(double dx, double dy, double dz) {
  return sqrt((dx*dx) + (dy*dy) + (dz*dz));;
}


/*
 * computeforce - compute the superposed forces on one body
 *   @param me     - the body to compute forces on at time t
 *   @param nextme - the body at time t+1
 */


void computeforce(shared [N / THREADS] body_t *me, shared [N / THREADS] body_t *nextme) {
  double d, f;        // distance, force
  double dx, dy, dz;  // position deltas
  double fx, fy, fz;  // force components
  double ax, ay, az;  // acceleration components

  fx = fy = fz = 0.0;

  // for every other body relative to me
  for (int i=0; i<N; i++) {

    // compute the distances in each dimension
    dx = me->x - bodies[i].x;
    dy = me->y - bodies[i].y;
    dz = me->z - bodies[i].z;

    // compute the distance magnitude
    d = dist(dx, dy, dz);

    // skip over ourselves (d==0)
    if (d != 0) {

      // F = G m1 m2 / r^2
      f = (G * me->mass * bodies[i].mass) / (d * d);

      // compute force components in each dimension
      fx += (f * dx) / d;
      fy += (f * dy) / d;
      fz += (f * dz) / d;
    }
  }

  // acc = force / mass (F=ma)
  ax = fx / me->mass;
  ay = fy / me->mass;
  az = fz / me->mass;

  // update the body velocity at time t+1
  nextme->dx = me->dx + (TIMESTEP * ax);
  nextme->dy = me->dy + (TIMESTEP * ay);
  nextme->dz = me->dz + (TIMESTEP * az);

  // update the body position at t+1
  nextme->x = me->x + (TIMESTEP * me->dx);
  nextme->y = me->y + (TIMESTEP * me->dy);
  nextme->z = me->z + (TIMESTEP * me->dz);

  // copy over the mass
  nextme->mass = me-> mass;
}


/*
 *  print_body - prints a body for debugging
 *    @param b - body to print
 */


void print_body(body_t *b) {
  printf("x: %7.3f y: %7.3f z: %7.3f dx: %7.3f dy: %7.3f dz: %7.3f\n",
      b->x, b->y, b->z, b->dx, b->dy, b->dz);
}


/*
 *  eprintf - error printing wrapper (only prints once)
 *    @param format printf-like format string
 *    @param ... arguments to printf format string
 *    @return number of bytes written to stderr
 */


int eprintf(const char *format, ...) {
  va_list ap;
  int ret;

  if (MYTHREAD == 0) {
    va_start(ap, format);
    ret = vfprintf(stdout, format, ap);
    va_end(ap);
    return ret;
  }
  else
    return 0;
}


/*
 * get_wctime - returns wall clock time as double
 *   @return double representation of wall clock time
 */


double get_wctime(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec + 1E-6 * tv.tv_usec);
}


/**
 * main
 */


int main(int argc, char **argv) {
  double startProcess, ts_start;
  body_t localBlock [N / THREADS];
  body_t currBlock [N / THREDS];
    
  eprintf("beginning N-body simulation of %d bodies with %d processes.\n", N, THREADS);
  
  setbuf(stdout, NULL);

  init();

  startProcess = get_wctime();

  upc_barrier;

  for (int ts = 0; ts < NSTEPS; ts++){
    ts_start = get_wctime();    
    for(int i = 0; i < THREADS + 1; i++){
      upc_memget(&localBlock, &bodies[i * (N / THREADS)], (N / THREADS) * sizeof(body_t));
      upc_barrier;
      computeforce(&localBlock[],
      upc_memput
    }
  }

/*
  upc_barrier;
  // for each time step
  for(int ts = 0; ts < NSTEPS; ts++) {
    ts_start = get_wctime();
    
    upc_forall (int i=0; i< N; i++; i) {
      computeforce(&bodies[i], &next[i]);
    }

    upc_barrier;

    //copy the t+1 state to be the new time t
    upc_forall(int i =0; i < N; i++; i) {
      upc_memcpy(&bodies[i], &next[i], sizeof(body_t));
    }

    upc_barrier;

    eprintf("timestep %d complete: %7.3f ms\n", ts, (get_wctime()-ts_start)*1000);
  }
  eprintf("simulation complete: %9.3f ms\n", (get_wctime() - startProcess)*1000);
  upc_barrier;

  sleep(1);
*/


  return 0;
}
