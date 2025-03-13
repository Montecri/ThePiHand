// By Cristiano Monteiro <cristianomonteiro@gmail.com> - 20220313
// Based on:
// http://numbers.computation.free.fr/Constants/Algorithms/nthdigit.html
// http://numbers.computation.free.fr/Constants/Algorithms/pidec.cpp

/*
 * Changed by Cristiano Monteiro <cristianomonteiro@gmail.com> to use
 * the "Pi Hands" device as a "display"
 * Mar/2025
 */

#include <Arduino.h>
#include <Servo.h>
#include <stdlib.h>
#include <math.h>

#include <time.h>

Servo littleFinger;
Servo ringFinger;
Servo mediumFinger;
Servo indexFinger;
Servo thumbFinger;

int littleInitial = 10;
int ringInitial = 10;
int mediumInitial = 10;
int indexInitial = 10;
int thumbInitial = 10;

int littleFinal = 90;
int ringFinal = 90;
int mediumFinal = 90;
int indexFinal = 90;
int thumbFinal = 50;

int littlePos = littleInitial;
int ringPos = ringInitial;
int mediumPos = mediumInitial;
int indexPos = indexInitial;
int thumbPos = thumbInitial;

int littlePin = 13;
int ringPin = 12;
int mediumPin = 10;
int indexPin = 4;
int thumbPin = 6;

int delayCombinedHands = 250;

// ModInt should be a 64-bit long integer. It could be a double floating point type, provided
// adaptations of MulMod and SumMulMod functions are done
typedef int64_t ModInt;
ModInt _m;
double _invm;
long n;
String pd = "", last = "";

void InitializeModulo(ModInt m)
{
  _m = m;
  _invm = 1. / (double)m;
}

// Compute a*b modulo _m
inline ModInt MulMod(ModInt a, ModInt b)
{
  // classical trick to bypass the 64-bit limitation, when a*b does not fit into the ModInt type.
  // Works whenever a*b/_m is less than 2^52 (double type maximal precision)
  ModInt q = (ModInt)(_invm * (double)a * (double)b);
  return a * b - q * _m;
}

// Compute a*b+c*d modulo _m
inline ModInt SumMulMod(ModInt a, ModInt b, ModInt c, ModInt d)
{
  ModInt q = (ModInt)(_invm * ((double)a * (double)b + (double)c * (double)d));
  return a * b + c * d - q * _m;
}

// double MyTime()
// {
//   return ((double) clock())/ CLOCKS_PER_SEC;
// }

double FullDouble = 1024. * 1024. * 1024. * 1024. * 1024. * 8.; // 2^53

inline double easyround(double x)
{
  double y = x + FullDouble;
  y -= FullDouble;
  return y;
}

/* return g, A such that g=gcd(a,_m) and a*A=g mod _m  */
ModInt ExtendedGcd(ModInt a, ModInt &A)
{
  ModInt A0 = 1, A1 = 0;
  ModInt r0 = a, r1 = _m;

  while (r1 > 0.)
  {
    ModInt q = r0 / r1;

    ModInt tmp = A0 - q * A1;
    A0 = A1;
    A1 = tmp;

    tmp = r0 - q * r1;
    r0 = r1;
    r1 = tmp;
  }
  A = A0;
  return r0;
}

ModInt InvMod(ModInt a)
{
  ModInt A;
  a = a % _m;
  if (a < 0)
    a += _m;
  ModInt gcd = ExtendedGcd(a, A);
  // if (gcd!=1)
  // printf("pb, gcd should be 1\n");
  return A;
}

ModInt PowMod(ModInt a, long b)
{
  ModInt r, aa;

  r = 1;
  aa = a;
  while (1)
  {
    if (b & 1)
      r = MulMod(r, aa);
    b >>= 1;
    if (b == 0)
      break;
    aa = MulMod(aa, aa);
  }
  return r;
}

/* Compute sum_{j=0}^k binomial(n,j) mod m */
ModInt SumBinomialMod(long n, long k)
{
  // Optimisation : when k>n/2 we use the relation
  // sum_{j=0}^k binomial(n,j) =  2^n - sum_{j=0}^{n-k-1} binomial(n,j)
  //
  // Note : additionnal optimization, not afforded here, could be done when k is near n/2
  // using the identity sum_{j=0}^{n/2} = 2^(n-1) + 1/2 binomial(n,n/2). A global saving of
  // 20% or 25% could be obtained.
  if (k > n / 2)
  {
    ModInt s = PowMod(2, n) - SumBinomialMod(n, n - k - 1);
    if (s < 0)
      s += _m;
    return s;
  }
  //
  // Compute prime factors of _m which are smaller than k
  //
  const long NbMaxFactors = 20; // no more than 20 different prime factors for numbers <2^64
  long PrimeFactor[NbMaxFactors];
  long NbPrimeFactors = 0;
  ModInt mm = _m;
  // _m is odd, thus has only odd prime factors
  for (ModInt p = 3; p * p <= mm; p += 2)
  {
    if (mm % p == 0)
    {
      mm = mm / p;
      if (p <= k) // only prime factors <=k are needed
        PrimeFactor[NbPrimeFactors++] = p;
      while (mm % p == 0)
        mm = mm / p; // remove all powers of p in mm
    }
  }
  // last factor : if mm is not 1, mm is necessarily prime
  if (mm > 1 && mm <= k)
  {
    PrimeFactor[NbPrimeFactors++] = mm;
  }

  // BinomialExponent[i] will contain the power of PrimeFactor[i] in binomial
  long BinomialPower[NbMaxFactors];
  // NextDenom[i] and NextNum[i] will contain next multiples of PrimeFactor[i]
  long NextDenom[NbMaxFactors], NextNum[NbMaxFactors];
  for (long i = 0; i < NbPrimeFactors; i++)
  {
    BinomialPower[i] = 1;
    NextDenom[i] = PrimeFactor[i];
    NextNum[i] = PrimeFactor[i] * (n / PrimeFactor[i]);
  }

  ModInt BinomialNum0 = 1, BinomialDenom = 1;
  ModInt SumNum = 1;
  ModInt BinomialSecondary = 1;

  for (long j = 1; j <= k; j++)
  {
    // new binomial : b(n,j) = b(n,j-1) * (n-j+1) / j
    ModInt num = n - j + 1;
    ModInt denom = j;
    int BinomialSecondaryUpdate = 0;

    for (long i = 0; i < NbPrimeFactors; i++)
    {
      long p = PrimeFactor[i];
      // Test if p is a prime factor of num0
      if (NextNum[i] == n - j + 1)
      {
        BinomialSecondaryUpdate = 1;
        NextNum[i] -= p;
        BinomialPower[i] *= p;
        num /= p;
        while (num % p == 0)
        {
          BinomialPower[i] *= p;
          num /= p;
        }
      }
      // Test if p is a prime factor of denom0
      if (NextDenom[i] == j)
      {
        BinomialSecondaryUpdate = 1;
        NextDenom[i] += p;
        BinomialPower[i] /= p;
        denom /= p;
        while (denom % p == 0)
        {
          BinomialPower[i] /= p;
          denom /= p;
        }
      }
    }

    if (BinomialSecondaryUpdate)
    {
      BinomialSecondary = BinomialPower[0];
      for (long i = 1; i < NbPrimeFactors; i++)
        BinomialSecondary = MulMod(BinomialSecondary, BinomialPower[i]);
    }

    BinomialNum0 = MulMod(BinomialNum0, num);
    BinomialDenom = MulMod(BinomialDenom, denom);

    if (BinomialSecondary != 1)
    {
      SumNum = SumMulMod(SumNum, denom, BinomialNum0, BinomialSecondary);
    }
    else
    {
      SumNum = MulMod(SumNum, denom) + BinomialNum0;
    }
  }
  SumNum = MulMod(SumNum, InvMod(BinomialDenom));
  return SumNum;
}

/* return fractionnal part of 10^n*(a/b) */
double DigitsOfFraction(long n, ModInt a, ModInt b)
{
  InitializeModulo(b);
  ModInt pow = PowMod(10, n);
  ModInt c = MulMod(pow, a);
  return (double)c / (double)b;
}

/* return fractionnal part of 10^n*S, where S=4*sum_{k=0}^{m-1} (-1)^k/(2*k+1). m is even */
double DigitsOfSeries(long n, ModInt m)
{
  double x = 0.;
  for (ModInt k = 0; k < m; k += 2)
  {
    x += DigitsOfFraction(n, 4, 2 * k + 1) - DigitsOfFraction(n, 4, 2 * k + 3);
    x = x - easyround(x);
  }
  return x;
}

double DigitsOfPi(long n)
{
  double logn = log((double)n);
  long M = 2 * (long)(3. * n / logn / logn / logn);               // M is even
  long N = 1 + (long)((n + 15.) * log(10.) / (1. + log(2. * M))); // n >= N
  N += N % 2;                                                     // N should be even
  ModInt mmax = (ModInt)M * (ModInt)N + (ModInt)N;
  // printf("Parameters : M=%ld, N=%ld, M*N+M=%.0lf\n",M,N,(double)mmax);
  // double st = MyTime();
  double x = DigitsOfSeries(n, mmax);
  // printf("Series time : %.2lf\n",MyTime()-st);
  for (long k = 0.; k < N; k++)
  {
    ModInt m = (ModInt)2 * (ModInt)M * (ModInt)N + (ModInt)2 * (ModInt)k + 1;
    InitializeModulo(m);
    ModInt s = SumBinomialMod(N, k);
    s = MulMod(s, PowMod(5, N));
    s = MulMod(s, PowMod(10, n - N)); // n-N is always positive
    s = MulMod(s, 4);
    x += (2 * (k % 2) - 1) * (double)s / (double)m; // 2*(k%2)-1 = (-1)^(k-1)
    x = x - floor(x);
  }
  return x;
}

void zero()
{
  if (littlePos > littleInitial)
    littleFinger.write(littleInitial);
  if (ringPos > ringInitial)
    ringFinger.write(ringInitial);
  if (mediumPos > mediumInitial)
    mediumFinger.write(mediumInitial);
  if (indexPos > indexInitial)
    indexFinger.write(indexInitial);
  if (thumbPos > thumbInitial)
    thumbFinger.write(thumbInitial);

  // This is a trick: Detaching servos to avoid shaking while holding position up
  littleFinger.attach(littlePin);
  ringFinger.attach(ringPin);
  mediumFinger.attach(mediumPin);
  indexFinger.attach(indexPin);
  thumbFinger.attach(thumbPin);

  littlePos = littleInitial;
  ringPos = ringInitial;
  mediumPos = mediumInitial;
  indexPos = indexInitial;
  thumbPos = thumbInitial;
}

void one()
{
  zero();
  delay(delayCombinedHands);

  if (littlePos > littleInitial)
    littleFinger.write(littleInitial);
  if (ringPos > ringInitial)
    ringFinger.write(ringInitial);
  if (mediumPos > mediumInitial)
    mediumFinger.write(mediumInitial);
  if (indexPos < indexFinal)
    indexFinger.write(indexFinal);
  if (thumbPos > thumbInitial)
    thumbFinger.write(thumbInitial);

  littleFinger.attach(littlePin);
  ringFinger.attach(ringPin);
  mediumFinger.attach(mediumPin);
  indexFinger.attach(indexPin);
  thumbFinger.attach(thumbPin);

  littlePos = littleInitial;
  ringPos = ringInitial;
  mediumPos = mediumInitial;
  indexPos = indexFinal; // One
  thumbPos = thumbInitial;
}

void two()
{
  zero();
  delay(delayCombinedHands);

  if (littlePos > littleInitial)
    littleFinger.write(littleInitial);
  if (ringPos > ringInitial)
    ringFinger.write(ringInitial);
  if (mediumPos < mediumFinal)
    mediumFinger.write(mediumFinal);
  if (indexPos < indexFinal)
    indexFinger.write(indexFinal);
  if (thumbPos > thumbInitial)
    thumbFinger.write(thumbInitial);

  littleFinger.attach(littlePin);
  ringFinger.attach(ringPin);
  mediumFinger.attach(mediumPin);
  indexFinger.attach(indexPin);
  thumbFinger.attach(thumbPin);

  littlePos = littleInitial;
  ringPos = ringInitial;
  mediumPos = mediumFinal; // two
  indexPos = indexFinal;   // two
  thumbPos = thumbInitial;
}

void three()
{
  zero();
  delay(delayCombinedHands);

  if (littlePos > littleInitial)
    littleFinger.write(littleInitial);
  if (ringPos < ringFinal)
    ringFinger.write(ringFinal);
  if (mediumPos < mediumFinal)
    mediumFinger.write(mediumFinal);
  if (indexPos < indexFinal)
    indexFinger.write(indexFinal);
  if (thumbPos > thumbInitial)
    thumbFinger.write(thumbInitial);

  littleFinger.attach(littlePin);
  ringFinger.attach(ringPin);
  mediumFinger.attach(mediumPin);
  indexFinger.attach(indexPin);
  thumbFinger.attach(thumbPin);

  littlePos = littleInitial;
  ringPos = ringFinal;     // three
  mediumPos = mediumFinal; // three
  indexPos = indexFinal;   // three
  thumbPos = thumbInitial;
}

void four()
{
  zero();
  delay(delayCombinedHands);

  if (littlePos < littleFinal)
    littleFinger.write(littleFinal);
  if (ringPos < ringFinal)
    ringFinger.write(ringFinal);
  if (mediumPos < mediumFinal)
    mediumFinger.write(mediumFinal);
  if (indexPos < indexFinal)
    indexFinger.write(indexFinal);
  if (thumbPos > thumbInitial)
    thumbFinger.write(thumbInitial);

  littleFinger.attach(littlePin);
  ringFinger.attach(ringPin);
  mediumFinger.attach(mediumPin);
  indexFinger.attach(indexPin);
  thumbFinger.attach(thumbPin);

  littlePos = littleFinal; // four
  ringPos = ringFinal;     // four
  mediumPos = mediumFinal; // four
  indexPos = indexFinal;   // four
  thumbPos = thumbInitial;
}

void five()
{
  zero();
  delay(delayCombinedHands);

  if (littlePos < littleFinal)
    littleFinger.write(littleFinal);
  if (ringPos < ringFinal)
    ringFinger.write(ringFinal);
  if (mediumPos < mediumFinal)
    mediumFinger.write(mediumFinal);
  if (indexPos < indexFinal)
    indexFinger.write(indexFinal);
  if (thumbPos < thumbFinal)
    thumbFinger.write(thumbFinal);

  littleFinger.attach(littlePin);
  ringFinger.attach(ringPin);
  mediumFinger.attach(mediumPin);
  indexFinger.attach(indexPin);
  thumbFinger.attach(thumbPin);

  littlePos = littleFinal; // five
  ringPos = ringFinal;     // five
  mediumPos = mediumFinal; // five
  indexPos = indexFinal;   // five
  thumbPos = thumbFinal;   // five
}

void six()
{
  zero();
  delay(delayCombinedHands);
  five();
  delay(delayCombinedHands);
  zero();
  delay(delayCombinedHands);
  one();
}

void seven()
{
  zero();
  delay(delayCombinedHands);
  five();
  delay(delayCombinedHands);
  zero();
  delay(delayCombinedHands);
  two();
}

void eight()
{
  zero();
  delay(delayCombinedHands);
  five();
  delay(delayCombinedHands);
  zero();
  delay(delayCombinedHands);
  three();
}

void nine()
{
  zero();
  delay(delayCombinedHands);
  five();
  delay(delayCombinedHands);
  zero();
  delay(delayCombinedHands);
  four();
}

void count()
{
  delay(2000);

  Serial.println("Zero");
  zero();
  delay(2000);

  Serial.println("One");
  one();
  delay(2000);

  Serial.println("Two");
  two();
  delay(2000);

  Serial.println("Three");
  three();
  delay(2000);

  Serial.println("Four");
  four();
  delay(2000);

  Serial.println("Five");
  five();
  delay(2000);

  Serial.println("Six");
  six();
  delay(2000);

  Serial.println("Seven");
  seven();
  delay(2000);

  Serial.println("Eight");
  eight();
  delay(2000);

  Serial.println("Nine");
  nine();
  delay(2000);
}

void displayDigit(char a)
{
  switch (a)
  {
  case '0':
    Serial.println('0');
    zero();
    break;
  case '1':
    Serial.println('1');
    one();
    break;
  case '2':
    Serial.println('2');
    two();
    break;
  case '3':
    Serial.println('3');
    three();
    break;
  case '4':
    Serial.println('4');
    four();
    break;
  case '5':
    Serial.println('5');
    five();
    break;
  case '6':
    Serial.println('6');
    six();
    break;
  case '7':
    Serial.println('7');
    seven();
    break;
  case '8':
    Serial.println('8');
    eight();
    break;
  case '9':
    Serial.println('9');
    nine();
    break;
  default:
    // if nothing else matches, do the default
    // default is optional
    break;
  }
}

void turnOff()
{
  delay(40);

  littleFinger.detach();
  ringFinger.detach();
  mediumFinger.detach();
  indexFinger.detach();
  thumbFinger.detach();
}

void setup()
{
  // Algorithm will only calculate from digit n (n=50, but I added 4 more to keep screen fluid) onwards, need to hard code the first ones
  String FirstDigits = "31415926535897932384626433832795028841971693993751058209";

  Serial.begin(9600);

  littleFinger.attach(littlePin);
  ringFinger.attach(ringPin);
  mediumFinger.attach(mediumPin);
  indexFinger.attach(indexPin);
  thumbFinger.attach(thumbPin);

  delay(250);

  littleFinger.write(littlePos);
  ringFinger.write(ringPos);
  mediumFinger.write(mediumPos);
  indexFinger.write(indexPos);
  thumbFinger.write(thumbPos);

  turnOff();

  Serial.println("Starting...");
  delay(4000);

  while (FirstDigits.length() > 4)
  {
    // Serial.println(FirstDigits);

    // Serial.println(FirstDigits.charAt(0));
    displayDigit(FirstDigits.charAt(0));
    turnOff();
    delay(1500);

    FirstDigits = FirstDigits.substring(1);
    // delay(100);
  }

  // Serial.println(FirstDigits.charAt(0));

  n = 51;
  Serial.println("Setup finished");
}

void loop()
{
  // long int t1 = millis();
  double x = DigitsOfPi(n);
  double pow = 1.e9;
  double y = x * pow;
  // To be able to know exactly the digits of pi at position n, the
  // value (pow*x) should be not too close to an integer
  // Serial.println(y);
  while (pow > 10 && (y - floor(y) < 0.05 || y - floor(y) > 0.95))
  {
    pow /= 10.;
    y = x * pow;
  }
  // Serial.println("Digits of pi after n-th decimal digit : %.0lf\n",floor(y));

  if (last.charAt(1) == '0')
  {
    pd = last.substring(1);
    Serial.println("achei");
  }
  else
  {
    pd = String(y, 0);
  }

  last = pd;

  // Serial.println(pd);

  // Serial.println(pd.charAt(0));
  displayDigit(pd.charAt(0));
  turnOff();
  delay(1500);

  n++;
}