/* { dg-do compile { target *-*-linux* } } */
/* { dg-options "-O2 -mandroid" } */

long double
foo (long double x)
{
  return x * x;
}

/* { dg-final { scan-assembler-not "fldt" } } */
