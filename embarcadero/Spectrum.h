//---------------------------------------------------------------------------

#ifndef SpectrumH
#define SpectrumH

#include "Spline.h"
#include "Types.h"

//---------------------------------------------------------------------------
class TSpectrum
{
private:
	FILE *logfile;

	int Nbars,Nx;
	double Xmin,Xmax,*X;
	double M,D,S,dX;
	bool MeshSet;
	bool SpectrumReady,AvReady,DispReady,SqrReady,WidthReady,EnvelopeReady;
	TSpectrumBar *Spectrum;
	void GetBoundaries();
	void ResetStatistics();

	void MakeSpectrum();
	void MakeEnvelope();
	
public:
	__fastcall TSpectrum();
	__fastcall ~TSpectrum();
	
	void SetMesh(double *X0,int Nb,int Ny);
	TSpectrumBar *GetSpectrum(bool Smooth=false);

    double GetAverage();
	double GetDispersion();
	double GetSquareDeviation();
	double GetWidth();
};

//---------------------------------------------------------------------------
#endif
