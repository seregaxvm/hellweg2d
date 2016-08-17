//---------------------------------------------------------------------------

#ifndef ConstUnitH
#define ConstUnitH

#include <Graphics.hpp>

//PHYSICAL
const double c=2.99792458e8;
const double pi=3.1415926;
const double We0=0.5110034e6;
const double Ia=17068;//Alfen Current
const double eps0=8.85418782e-12;
const double myu0=1.25663706e-6;
const double kF=1/(4*pi*eps0);
const double kFc=1/(4*pi*eps0*c);

//OPTIMIZER
const double RS=0.61803399;
const double G=1-RS;
const int Nerr=1;
const int Nstep=10;
const double CurrentStep=100;  //mA
const double CurrentErr=2; //mA
const double Eerr=0.5;
const double Estep=10;

//NUMERICAL
const double MaxPhase=2*360;
const double MinPhase=-MaxPhase;
const int PointsNumber=1000;
const int SeparatrixNumber=20;
const double SpecLevel=0.5;

//COLORS
const TColor Line1_col_a=clRed;
const TColor Line1_col_b=clMaroon;
const TColor Line2_col_a=clBlue;
const TColor Line2_col_b=clNavy;
const TColor Particle_col=clRed;
const TColor Envelope_col=clBlue;

//FILES
const char *CST_FileX="cst_x.txt";
const char *CST_FileY="cst_y.txt";
const char *CST_FileR="cst_r.txt";
const char *Solenoid_File="BFIELD.txt";

//TABLES
const int Nb12=5;
const int Nb23=4;
const int Nb43=4;
const double B12[Nb12]={0.4,0.5,0.6,0.8,1.0};
const double B23[Nb23]={0.4,0.6,0.8,1.0};
const double B43[Nb43]={0.4,0.6,0.8,1.0};

const int Ne12=7;
const int Ne23=7;
//const int Ne23=5;
const int Ne43=5;

const double E12[Nb12][Ne12]=
{127,162,212,282,398,590,973,
98,131,180,250,358,543,902,
80,90,138,205,314,490,820,
42,69,115,174,265,424,729,
0,41,75,129,213,260,620};
const double E23[Nb23][Ne23]=
/*{318,349,396,472,679.4,
318,349,396,472,679.4,
318,349,396,472,679.4,
318,349,396,472,679.4}; */
{118,152,200,270,380,575,980,
105,134,180,250,364,557,937,
62,96,140,204,307,475,825,
47,57,95,149,227,365,618};
const double E43[Nb43][Ne43]=
{287.5,321,403,508,682,
287.5,321,403,508,682,
287.5,321,403,508,682,
287.5,321,403,508,682};

const double R12[Nb12][Ne12]=
{0.2164,0.1887,0.1634,0.1401,0.1182,0.0973,0.0772,
0.2242,0.1936,0.1664,0.1419,0.1192,0.0978,0.0774,
0.2361,0.2013,0.1713,0.1448,0.1208,0.0987,0.0778,
0.2437,0.2066,0.1748,0.1569,0.1220,0.0983,0.0781,
0.25,0.2131,0.1792,0.1498,0.1238,0.1002,0.0786};
const double R23[Nb23][Ne23]=
/*{0.062,0.061,0.06,0.059,0.058,
0.062,0.061,0.06,0.059,0.058,
0.062,0.061,0.06,0.059,0.058,
0.062,0.061,0.06,0.059,0.058};  */
{0.2158,0.1881,0.1629,0.1396,0.1179,0.0971,0.0771,
0.2234,0.1928,0.1658,0.1413,0.1187,0.0975,0.0773,
0.2344,0.1999,0.1702,0.1438,0.1202,0.0983,0.0786,
0.2495,0.2105,0.1774,0.1485,0.1228,0.0996,0.0782};
const double R43[Nb43][Ne43]=
{0.082,0.081,0.08,0.079,0.078,
0.082,0.081,0.08,0.079,0.078,
0.082,0.081,0.08,0.079,0.078,
0.082,0.081,0.08,0.079,0.078};


const int Nar23=5;
const int Nar43=5;
const int Nab23=4;

const double AR[Nar23]={0.10,0.12,0.14,0.16,0.18};
//const double AR[Nar43]={0.058,0.059,0.06,0.061,0.062};
const double AR43[Nar43]={0.078,0.079,0.08,0.081,0.082};
const double AB[Nab23]={0.4,0.6,0.8,1.0};

const double A12[Nab23][Nar23]=
{307.0,196.0,142.0,112.0,94.4,
139.0,74.4,52.1,37.6,29.1,
94.7,52.7,32.8,22.4,16.5,
80.0,42.0,25.4,16.8,12.0};
const double A23[Nab23][Nar23]=
/*{92.0,50.7,37.1,30.3,26.0,
92.0,50.7,37.1,30.3,26.0,
92.0,50.7,37.1,30.3,26.0,
92.0,50.7,37.1,30.3,26.0,}; */
{200.0,122.0,88.0,62.0,55.0,
120.0,68.0,40.0,27.0,22.0,
90.0,50.0,28.0,18.5,14.0,
80.0,40.0,24.0,15.0,11.0};   
const double A43[Nab23][Nar43]=
{57.4,32.96,22.13,16.35,13.11,
57.4,32.96,22.13,16.35,13.11,
57.4,32.96,22.13,16.35,13.11,
57.4,32.96,22.13,16.35,13.11};


 /*
enum TChartType {CH_BETTA,CH_A,CH_ELP,CH_B,CH_ALPHA,CH_RA,CH_BZ,
					CH_EMMITANCE,CH_PORTRAIT,CH_SECTION,CH_ENERGY,CH_PHASE,CH_ELLIPSE};
 */
//---------------------------------------------------------------------------
#endif
