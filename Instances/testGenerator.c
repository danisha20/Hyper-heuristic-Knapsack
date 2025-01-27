

/* ======================================================================
	     GENERATOR2.c, David Pisinger   nov 1997
   ====================================================================== */

/* This is a test generator from the paper:
 *
 *   S. Martello, D. Pisinger, P. Toth
 *   Dynamic programming and tight bounds for the 0-1 knapsack problem
 *   submitted
 *
 * The current code generates randomly generated instances and
 * writes them to a file. Different capacities are considered to
 * ensure proper testing.
 *
 * The code has been tested on a hp9000/735, and conforms with the
 * ANSI-C standard apart from some of the timing routines (which may
 * be removed). To compile the code use:
 *
 *   cc -Aa -O -o gen2 gen2.c -lm
 *
 * The code is run by issuing the command
 *
 *   gen2 n r type i S
 *
 * where n: number of items,
 *       r: range of coefficients,
 *       type: 1=uncorrelated, 2=weakly corr, 3=strongly corr,
 *             4=inverse str.corr, 5=almost str.corr, 6=subset-sum,
 *             7=even-odd subset-sum, 8=even-odd knapsack,
 *             9=uncorrelated, similar weights,
 *             11=Avis subset-sum, 12=Avis knapsack, 13=collapsing KP,
 *             14=bounded strongly corr, 15=No small weights
 *       i: instance no
 *       S: number of tests in series (typically 1000)
 * output will be written to the file "test.in".
 *
 * Please do not re-distribute. A new copy can be obtained by contacting
 * the author at the adress below. Errors and questions are refered to:
 *
 *   David Pisinger, associate professor
 *   DIKU, University of Copenhagen,
 *   Universitetsparken 1,
 *   DK-2100 Copenhagen.
 *   e-mail: pisinger@diku.dk
 *   fax: +45 35 32 14 01
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
/*#include <values.h>*/
#include <string.h>
#include <malloc.h>


/* ======================================================================
				     macros
   ====================================================================== */

#define srand(x)     srand48x(x)
#define randm(x)     (lrand48x() % (x))
#define NO(f,i)      ((int) ((i+1)-f))
typedef long long (*funcptr) (const void *, const void *);
#define TRUE  1
#define FALSE 0


/* ======================================================================
				 type declarations
   ====================================================================== */

typedef long long   boolean; /* boolean variables */
typedef long long itype;   /* item profits and weights */
typedef long long  stype;   /* sum of pofit or weight */

/* item */
typedef struct {
  itype   p;     /* profit */
  itype   w;     /* weight */
  boolean x;     /* solution variable */
} item;


/* =======================================================================
                                random
   ======================================================================= */

/* to generate the same instances as at HP9000 - UNIX, */
/* here follows C-versions of SRAND48, and LRAND48.  */

unsigned long _h48, _l48;

void srand48x(long long s)
{
  _h48 = s;
  _l48 = 0x330E;
}

long long lrand48x(void)
{
  _h48 = (_h48 * 0xDEECE66D) + (_l48 * 0x5DEEC);
  _l48 = _l48 * 0xE66D + 0xB;
  _h48 = _h48 + (_l48 >> 16);
  _l48 = _l48 & 0xFFFF;
  return (_h48 >> 1);
}


/* ======================================================================
                                 error
   ====================================================================== */

void error(char *str, ...)
{
  va_list args;

  va_start(args, str);
  vprintf(str, args); printf("\n");
  va_end(args);
  printf("STOP !!!\n\n");
  exit(-1);
}


/* ======================================================================
				  palloc
   ====================================================================== */

void pfree(void *p)
{
  if (p == NULL) error("freeing null");
  free(p);
}


void * palloc(size_t no, size_t each)
{
  long size;
  long long *p;

  size = no * (long long) each;
  if (size == 0) size = 1;
  if (size != (size_t) size) error("alloc too big %lld", size);
  p = malloc(size);
  if (p == NULL) error("no memory size %ld", size);
  return p;
}


/* ======================================================================
                                showitems
   ====================================================================== */


int num_case, num_type;
void showitems(item *f, item *l, stype c)
{
  item *i;
  stype ps, ws;
  FILE *out;
  char numcase[20];
  char numtype[20];
  int num_cop = num_case;
  int num_cop2 = num_type;
  itoa(num_cop, numcase, 10);
  itoa(num_cop2, numtype, 10);
  char prefix[] = "martello_";
  char underscore[] = "_";
  char suffix[] = ".kp";
  strcat(prefix, numtype);
  strcat(prefix, underscore);
  strcat(prefix, numcase);
  strcat(prefix, suffix);

  out = fopen(prefix, "w");
  if (out == NULL) error("no file");
  fprintf(out, "%d, %lld\n", NO(f, l), c);
  for (i = f; i <= l; i++) {
    fprintf(out, "%lld, %lld\n", i->w, i->p);
  }
  fclose(out);
}


/* ======================================================================
                                makecol
   ====================================================================== */

long long icomp(item *a, item *b) { return b->w - a->w; }

stype makecol(item *fitem, item *litem, itype rp, itype rw, stype b, int m)
{
  register item *i, *k, *f, *l;
  register stype psum, wsum, csum;
  long long h, n, nn;

  nn = NO(fitem,litem);
  if (m > nn/2) m = nn/2;
  litem = fitem + nn/2 + m - 1; /* only m big items */

  f = fitem; l = litem;
  psum = 0; wsum = 0; csum = 0;
  n = nn / 2;
  k = f + n;

  for (i = f; i != k; i++) {
    i->p = randm(rp) + 1;
    i->w = randm(rw) + 1;
    i->x = 0;
    psum += i->p;
    wsum += i->w;
  }

  for (i = k, h = 1; i <= l; i++, h++) {
    i->w = randm(b);
    i->p = 0;
    i->x = 0;
    if (h > m) i->w = 0;
    csum += i->w;
  }
  qsort(k, n, sizeof(item), (funcptr) icomp);

  /* convert the problem */
  for (i = f; i != k; i++) {
    i->p += psum;
    i->w += wsum;

    if(i->p <= 0) i->p = 1;
    if(i->w <= 0) i->w = 1;
  }
  for (i = k, h = n + 1; i <= l; i++, h++) {
    i->p  = (3*n + 1 - h) * psum;
    i->w  = (4*n - h) * wsum - i->w;
    if(i->p <= 0) i->p = 1;
    if(i->w <= 0) i->w = 1;
  }
  /* printf("psum %ld, wsum %ld, csum %ld\n", psum, wsum, csum); */
  return 3*n*wsum;
}


/* ======================================================================
				maketest
   ====================================================================== */

stype maketest(item *f, item *l, long long r, int type, long long v, long long int S)
{
  register item *i, *j;
  unsigned long long wsum, psum, c;
  long long r1;
  long long n, m, k, h;

  wsum = 0; psum = 0;
  r1 = r / 10;
  n = NO(f,l);
  for (i = f, h = 1; i <= l; i++, h++) {

    i->w = randm(r) + 1;
    switch (type) {
      case  1: i->p = randm(r) + 1; /* uncorrelated */
               if (i->p <= 0) i->p = 1;
               break;
      case  2: i->p = randm(2*r1+1) + i->w - r1; /* weakly corr */
               if (i->p <= 0) i->p = 1;
               break;
      case  3: i->p = i->w + r1;   /* strongly corr */
               break;
      case  4: i->p = i->w; /* inverse strongly corr */
               i->w = i->p + r1;
               break;
      case  5: i->p = i->w + r1 + randm(2*r/1000+1) - r/1000; /* alm str.corr */
               if (i->p <= 0) i->p = 1;
               break;
      case  6: i->p = i->w; /* subset sum */
               break;
      case  7: i->w = 2*((i->w + 1)/2); /* even-odd */
               i->p = i->w;
               break;
      case  8: i->w = 2*((i->w + 1)/2); /* even-odd knapsack */
               i->p = i->w + r1;
               break;
      case  9: i->p = i->w; /* uncorrelated, similar weights */
               i->w = randm(r1) + 100* r;
               break;
      case 11: i->w = n*(n+1) + h; /* Avis subset sum */
               i->p = i->w;
               break;
      case 12: i->w = n*(n+1) + h; /* Avis knapsack */
               i->p = randm(1000);
               break;
      case 13: i->p = i->w = 0;  /* collapsing KP is generated separatly */
               break;
      case 14: i->p = i->w + r1; /* bounded strongly corr */
               m = randm(10)+1;
               for (k = 1, j = i; m != 0; k=2*k) {
                 if (j > l) break;
                 if (k > m) k = m;
                 j->p = k * i->p; j->w = k * i->w;
                 wsum += j->w; psum += j->p;
                 m -= k; j++;
               }
               i = j-1;
               wsum -= i->w; psum -= i->p; /* avoid double addition */
               break;
      case 15: i->w = randm(r/2) + r/2; /* No small weights */
               i->p = randm(2*r1+1) + i->w - r1;
               if (i->p <= 0) i->p = 1;
               break;

      default: error("undefined problem type");
    }

    if (i->p <= 0) i->p = 1;

    if (i->w <= 0) i->w = 1;
    wsum += i->w; psum += i->p;
  }
  c = (v * (long long) wsum) / (S + 1);
  for (i = f; i <= l; i++) if (i->w > c) c = i->w;
  printf("test %d, %d: wsum %lld, psum %lld, cap %lld\n", num_type, num_case, wsum, psum, c);
  switch (type) {
    case  1: return c;
    case  2: return c;
    case  3: return c;
    case  4: return c;
    case  5: return c;
    case  6: return c;
    case  7: return 2*(c/2) + 1;
    case  8: return 2*(c/2) + 1;
    case  9: return c;
    case 11: return n*(n+1) * ((n-1)/2) + ((n*(n-1))/2);
    case 12: return n*(n+1) * ((n-1)/2) + ((n*(n-1))/2);
    case 13: return makecol(f, l, 300, 1000, 10000, 100);
    case 14: return c;
    case 15: return c;

    default: error("undefined capacity type");
  }
}


/* ======================================================================
				main
   ====================================================================== */

void main(int argc, char *argv[])
{
  item *f, *l;
  long long n, r, type, i, S;
  stype c;

  /*
  if (argc == 6) {
    n = atoi(argv[1]);
    r = atoi(argv[2]);
    type = atoi(argv[3]);
    i = atoi(argv[4]);
    S = atoi(argv[5]);
    printf("generator2 %d %d %d %d %d\n", n, r, type, i, S);
  } else {
    printf("generator2\n");
    printf("n = ");
    scanf("%d", &n);
    printf("r = ");
    scanf("%d", &r);
    printf("t = ");
    scanf("%d", &type);
    printf("i = ");
    scanf("%d", &i);
    printf("S = ");
    scanf("%d", &S);
  }
  */

    srand(time(NULL));

  int n_arr[5] = {50, 400, 2000, 5000, 10000};
  for(num_case = 0; num_case < 50; num_case++) {
    for(num_type = 1; num_type <= 9; num_type++) {
      n = n_arr[num_case/10];
      r = n;
      f = palloc(n, sizeof(item));
      l = f + n-1;
      c = maketest(f, l, r, num_type, 1, 1);
      showitems(f, l, c);
      pfree(f);
    }
    for(num_type = 10; num_type <= 14; num_type++) {
      n = n_arr[num_case/10];
      r = n;
      f = palloc(n, sizeof(item));
      l = f + n-1;
      c = maketest(f, l, r, num_type+1, 1, 1);
      showitems(f, l, c);
      pfree(f);
    }
  }
}

