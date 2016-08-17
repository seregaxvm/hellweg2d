//---------------------------------------------------------------------------


#pragma hdrstop

#include "BeamSolver.h"
//---------------------------------------------------------------------------
__fastcall TBeamSolver::TBeamSolver(AnsiString _Path)
{
	Path=_Path;
	Initialize();
}
//---------------------------------------------------------------------------
__fastcall TBeamSolver::TBeamSolver()
{
	Initialize();
}
//---------------------------------------------------------------------------
__fastcall TBeamSolver::~TBeamSolver()
{
	delete[] Cells;
	delete[] Structure;

	for (int i=0;i<Npoints;i++)
		delete Beam[i];
	delete[] Beam;

	delete[] Par;
	for (int i=0;i<Ncoef;i++)
		delete[] K[i];
	delete[] K;

	if (SmartProgress!=NULL)
		delete SmartProgress;

	delete InputStrings;
	delete ParsedStrings;

	//fclose(logFile);
}
//---------------------------------------------------------------------------
void TBeamSolver::Initialize()
{
	MaxCells=500;
	Nmesh=20;
	Kernel=0.9;
	SplineType=LSPLINE;
	Nstat=100;
	Ngraph=500;
	Nbars=100;
	Nav=10;
	Smooth=0.95;
	Npoints=1;

	Np=1;
	Nlim=-1;

	Magnetized=false;

	LoadIniConstants();

	DataReady=false;
	Stop=false;

	K=new TIntegration*[Ncoef];
	for (int i=0;i<Ncoef;i++)
		K[i]=new TIntegration[Np];

	Par=new TIntParameters[Ncoef];

	Cells = new TCell[1];

	Np_beam=1;
	Beam=new TBeam*[Npoints];
	for (int i=0;i<Npoints;i++)
		Beam[i]=new TBeam(1);

	Structure=new TStructure[Npoints];

	InputStrings=new TStringList;
	ParsedStrings=new TStringList;

	SmartProgress=NULL;
}
//---------------------------------------------------------------------------
void TBeamSolver::SaveToFile(AnsiString& Fname)
{
	FILE *F;
	F=fopen(Fname.c_str(),"wb");

	fwrite(&Npoints,sizeof(int),1,F);
	fwrite(&Np,sizeof(int),1,F);
	fwrite(&Nbars,sizeof(int),1,F);

	fwrite(Structure,sizeof(TStructure),Npoints,F);

	for(int i=0;i<Npoints;i++){
		fwrite(&(Beam[i]->lmb),sizeof(double),1,F);
		fwrite(&(Beam[i]->h),sizeof(double),1,F);
		fwrite(&(Beam[i]->Ib),sizeof(double),1,F);
		fwrite(&(Beam[i]->I0),sizeof(double),1,F);
		fwrite(Beam[i]->Particle,sizeof(TParticle),Np,F);
	}

	fclose(F);
}
//---------------------------------------------------------------------------
bool TBeamSolver::LoadFromFile(AnsiString& Fname)
{
	FILE *F;
	F=fopen(Fname.c_str(),"rb");
	bool Success;

	delete[] Structure;

	for (int i=0;i<Np_beam;i++)
		delete Beam[i];
	delete[] Beam;

	Beam=new TBeam*[Npoints];
	for (int i=0;i<Npoints;i++)
		Beam[i]=new TBeam(Np);
	Np_beam=Npoints;

	try{
		fread(&Npoints,sizeof(int),1,F);
		fread(&Np,sizeof(int),1,F);
		fread(&Nbars,sizeof(int),1,F);

		Structure=new TStructure[Npoints];
		Beam=new TBeam*[Npoints];
		for (int i=0;i<Npoints;i++)
			Beam[i]=new TBeam(Np);

		fread(Structure,sizeof(TStructure),Npoints,F);
	  //	fread(Beam,sizeof(TBeam),Npoints,F);


		for (int i=0;i<Npoints;i++){
			fread(&(Beam[i]->lmb),sizeof(double),1,F);
			fread(&(Beam[i]->h),sizeof(double),1,F);
			fread(&(Beam[i]->Ib),sizeof(double),1,F);
			fread(&(Beam[i]->I0),sizeof(double),1,F);
			fread(Beam[i]->Particle,sizeof(TParticle),Np,F);
		}
		Success=true;
	} catch (...){
		Success=false;
	}

	fclose(F);
	return Success;
}
//---------------------------------------------------------------------------
void TBeamSolver::AssignSolverPanel(TObject *SolverPanel)
{
	SmartProgress=new TSmartProgress(static_cast <TWinControl *>(SolverPanel));
}
//---------------------------------------------------------------------------
void TBeamSolver::LoadIniConstants()
{
	TIniFile *UserIni;
	int t;
	double stat;

	UserIni=new TIniFile(Path+"\\hellweg.ini");
	MaxCells=UserIni->ReadInteger("OTHER","Maximum Cells",MaxCells);
	Nmesh=UserIni->ReadInteger("NUMERIC","Number of Mesh Points",Nmesh);
	Kernel=UserIni->ReadFloat("Beam","Percent Of Particles in Kernel",Kernel);
	Kernel/=100;
	
	t=UserIni->ReadInteger("NUMERIC","Spline Interpolation",t);
	switch (t) {
		case (0):{
			SplineType=LSPLINE;
			break;
		}
		case (1):{
			SplineType=CSPLINE;
			break;
		}
		case (2):{
			SplineType=SSPLINE;
			break;
		}
	}

	stat=UserIni->ReadFloat("NUMERIC","Statistics Error",stat);
	if (stat<1e-6)
		stat=1e-6;
	if (stat>25)
		stat=25;
	Nstat=round(100.0/stat);
	AngErr=UserIni->ReadFloat("NUMERIC","Angle Error",AngErr);
	Smooth=UserIni->ReadFloat("NUMERIC","Smoothing",Smooth);
	Ngraph=UserIni->ReadInteger("OTHER","Chart Points",Nbars);
	Nbars=UserIni->ReadInteger("NUMERIC","Hystogram Bars",Ngraph);
	Nav=UserIni->ReadInteger("NUMERIC","Averaging Points",Nav);
}
//---------------------------------------------------------------------------
int TBeamSolver::GetNumberOfPoints()
{
	return Npoints;
}
/*//---------------------------------------------------------------------------
double TBeamSolver::GetWaveLength()
{
	return lmb;
} */
//---------------------------------------------------------------------------
int TBeamSolver::GetMeshPoints()
{
	return Nmesh;
}
//---------------------------------------------------------------------------
int TBeamSolver::GetNumberOfParticles()
{
	return Np;
}
//---------------------------------------------------------------------------
int TBeamSolver::GetNumberOfChartPoints()
{
	return Ngraph;
}
//---------------------------------------------------------------------------
int TBeamSolver::GetNumberOfBars()
{
	return Nbars;
}
//---------------------------------------------------------------------------
int TBeamSolver::GetNumberOfCells()
{
	return Ncells;
}
//---------------------------------------------------------------------------
double TBeamSolver::GetFrequency()
{
	return F0;
}
//---------------------------------------------------------------------------
double TBeamSolver::GetPower()
{
	return P0;
}
//---------------------------------------------------------------------------
double TBeamSolver::GetInputCurrent()
{
	return I0;
}
//---------------------------------------------------------------------------
double TBeamSolver::GetSolenoidField()
{
	return B0;
}
//---------------------------------------------------------------------------
double TBeamSolver::GetSolenoidPosition()
{
	return Zmag;
}
//---------------------------------------------------------------------------
double TBeamSolver::GetSolenoidLength()
{
	return Lmag;
}
/*//---------------------------------------------------------------------------
double TBeamSolver::GetMode(int *N,int *M)
{
	if (M!=NULL)
		*M=Mode_M;
	if (N!=NULL)
		*N=Mode_N;
	return Mode_N*pi/Mode_M;
}     */
//---------------------------------------------------------------------------
double TBeamSolver::GetInputAverageEnergy()
{
	return W0;
}
//---------------------------------------------------------------------------
double TBeamSolver::GetInputEnergyDeviation()
{
	return dW;
}
//---------------------------------------------------------------------------
double TBeamSolver::GetInputAveragePhase()
{
	return Phi0;
}
//---------------------------------------------------------------------------
double TBeamSolver::GetInputPhaseDeviation()
{
	return dPhi;
}
//---------------------------------------------------------------------------
double TBeamSolver::GetInputAlpha()
{
	return AlphaCS;
}
//---------------------------------------------------------------------------
double TBeamSolver::GetInputBetta()
{
	return BettaCS;
}
//---------------------------------------------------------------------------
double TBeamSolver::GetInputEpsilon()
{
	return EmittanceCS;
}
//---------------------------------------------------------------------------
bool TBeamSolver::IsCoulombAccounted()
{
	return Coulomb;
}
//---------------------------------------------------------------------------
bool TBeamSolver::IsWaveReversed()
{
	return Reverse;
}
//---------------------------------------------------------------------------
bool TBeamSolver::IsEnergyEquiprobable()
{
	return W_Eq;
}
//---------------------------------------------------------------------------
bool TBeamSolver::IsPhaseEquiprobable()
{
	return Phi_Eq;
}
//---------------------------------------------------------------------------
bool TBeamSolver::IsKeyWord(AnsiString& S)
{
	return S=="POWER" ||
		S=="SOLENOID" ||
		S=="BEAM" ||
		S=="CURRENT" ||
		S=="DRIFT" ||
		S=="CELL" ||
		S=="CELLS" ||
		S=="OPTIONS";
}
//---------------------------------------------------------------------------
TInputParameter TBeamSolver::Parse(AnsiString& S)
{
	TInputParameter P;
	if (S=="POWER")
		P=COUPLER;
	else if (S=="SOLENOID")
		P=SOLENOID;
	else if (S=="BEAM")
		P=BEAM;
	else if (S=="CURRENT")
		P=CURRENT;
	else if (S=="DRIFT")
		P=DRIFT;
	else if (S=="CELL")
		P=CELL;
	else if (S=="CELLS")
		P=CELLS;
	else if (S=="OPTIONS")
		P=OPTIONS;
	return  P;
}
//---------------------------------------------------------------------------
void TBeamSolver::GetDimensions(TCell& Cell)
{
	
	int Nbp=0,Nep=0;
	int Nar=0,Nab=0;
	int Mode=Cell.Mode;

	switch (Mode) {
		case 90:	Nbp=Nb12; Nep=Ne12;Nar=Nb23; Nab=Ne23; break;
		case 120:   Nbp=Nb23; Nep=Ne23;Nar=Nb23; Nab=Ne23; break;
		case 240:   Nbp=Nb43; Nep=Ne43;Nar=Nb23; Nab=Ne43; break;
		default: return;
	}

	double *Xo,*Yo,*Xi,*Yi;
	
	Xo=new double[Nep];
	Yo=new double[Nep];
	Xi=new double[Nbp];
	Yi=new double[Nbp];

	//Searching for a/lmb

  /*FILE *T;
	T=fopen("table.log","a");     */

	for (int i=0;i<Nbp;i++){
		for (int j=0;j<Nep;j++){
			switch (Mode) {
				case 90:	Xo[j]=E12[Nbp-i-1][j]; Yo[j]=R12[Nbp-i-1][j]; break;
				case 120:   Xo[j]=E23[Nbp-i-1][j]; Yo[j]=R23[Nbp-i-1][j]; break;
				case 240:   Xo[j]=E43[Nbp-i-1][j]; Yo[j]=R43[Nbp-i-1][j]; break;
				default: return;
			}
		}
		TSpline Spline;
		Spline.SoftBoundaries=false;
		Spline.MakeLinearSpline(Xo,Yo,Nep);
		Yi[i]=Spline.Interpolate(Cell.ELP);
	   //	Xi[i]=mode90?B12[i]:B23[i];
		switch (Mode) {
			case 90:	Xi[i]=B12[i];break;
			case 120:   Xi[i]=B23[i];break;
			case 240:   Xi[i]=B43[i];break;
			default: return;
		}
	}

	TSpline rSpline;
	rSpline.SoftBoundaries=false;
	rSpline.MakeLinearSpline(Xi,Yi,Nbp);
	Cell.AkL=rSpline.Interpolate(Cell.betta);

	delete[] Xo;
	delete[] Yo;
	delete[] Xi;
	delete[] Yi;

	Xo=new double[Nar];
	Yo=new double[Nar];
	Xi=new double[Nab];
	Yi=new double[Nab];

	for (int i=0;i<Nab;i++){
		for (int j=0;j<Nar;j++){
			switch (Mode) {
				case 90:	Xo[j]=AR[j]; Yo[j]=A12[i][j]; break;
				case 120:   Xo[j]=AR[j]; Yo[j]=A23[i][j]; break;
				case 240:   Xo[j]=AR43[j]; Yo[j]=A43[i][j]; break;
				default: return;
			}

			/*Xo[j]=AR[j];
			Yo[j]=mode90?A12[i][j]:A23[i][j]; */
		}
		TSpline Spline;
		Spline.SoftBoundaries=false;
		Spline.MakeLinearSpline(Xo,Yo,Nar);
		Yi[i]=Spline.Interpolate(Cell.AkL);
		//Xi[i]=AB[i];
		switch (Mode) {
			case 90:	Xi[i]=AB[i];break;
			case 120:   Xi[i]=AB[i];break;
			case 240:   Xi[i]=AB[i];break;
			default: return;
		}
	}

	TSpline aSpline;
	aSpline.SoftBoundaries=false;
	aSpline.MakeLinearSpline(Xi,Yi,Nab);
	Cell.AL32=1e-4*aSpline.Interpolate(Cell.betta);

	delete[] Xo;
	delete[] Yo;
	delete[] Xi;
	delete[] Yi;

  /*	fprintf(T,"%f %f %f %f\n",Cell.betta,Cell.ELP,Cell.AkL,Cell.AL32);
	fclose(T);  */
	//ShowMessage(E12[0][1]);

	
  /*	for (int i=0;i<Nbt;i++){
		akl[i]=solve(ELP(akl),ELP=E0,bf[i]);
	}
	akl(bf);
	y=interp(akl(bf),bf);    */


}
//---------------------------------------------------------------------------
TInputLine *TBeamSolver::ParseFile(int& N)
{
	TInputLine *Lines;
	char *FileName=InputFile.c_str();
	fstream fs(FileName);

	AnsiString S;
	TInputParameter P;
	char s[128];
	int i=-1,j=0;

	while (!fs.eof()){
		fs>>s;
		S=AnsiString(s);
		if(IsKeyWord(S))
			N++;
	}

	fs.clear();
	fs.seekg(ios::beg);

	Lines=new TInputLine[N];

		while (!fs.eof()){
			fs>>s;
			S=AnsiString(s);
			if (S=="END")
				break;
			if(IsKeyWord(S)){
				i++;
				P=Parse(S);
				Lines[i].P=P;
				Lines[i].N=0;
				j=0;
			} else {
				if (j==MaxParameters){
					i++;
					Lines[i].P=UNDEFINED;
					j=0;
				} else {
					Lines[i].S[j]=S;
					Lines[i].N++;
					j++;
				}
			}
		}

	fs.close();
	return Lines;
}
//---------------------------------------------------------------------------
TError TBeamSolver::ParseLines(TInputLine *Lines,int N,bool OnlyParameters)
{
	int Ni=0;
	float dF=0;

	Coulomb=false;
	Reverse=false;
	Magnetized=false;
	BeamType=RANDOM;
	FSolenoid=false;

	bool NewCell=true;
	AnsiString F,s;
	ParsedStrings->Clear();

	for (int k=0;k<N;k++){
		switch (Lines[k].P) {
			case SOLENOID:{
				if (Lines[k].N==3){
					B0=Lines[k].S[0].ToDouble();
					Lmag=Lines[k].S[1].ToDouble();
					Zmag=Lines[k].S[2].ToDouble();
					F="SOLENOID "+Lines[k].S[0]+"\t"+Lines[k].S[1]+"\t"+Lines[k].S[2];
					ParsedStrings->Add(F);
				}else if (Lines[k].N==1) {
					if (Lines[k].S[0]=="BFILE"){
						FSolenoid=true;
						F="SOLENOID BFILE";
						ParsedStrings->Add(F);
					}
				}else
					return ERR_SOLENOID;
				break;
			}
			case CELL:{
				if(OnlyParameters)
					break;
				if (Lines[k].N==3){
					Cells[Ni].Mode=Lines[k].S[0].ToDouble();
					Cells[Ni].betta=Lines[k].S[1].ToDouble();
					Cells[Ni].ELP=Lines[k].S[2].ToDouble();
					GetDimensions(Cells[Ni]);
					//Cells[Ni].AL32=table function;
					//Cells[Ni].AkL=table function;
				} else if (Lines[k].N==5){
					Cells[Ni].Mode=Lines[k].S[0].ToDouble();
					Cells[Ni].betta=Lines[k].S[1].ToDouble();
					Cells[Ni].ELP=Lines[k].S[2].ToDouble();
					Cells[Ni].AL32=Lines[k].S[3].ToDouble();
					Cells[Ni].AkL=Lines[k].S[4].ToDouble();
				}
				else{
					return ERR_CELL;
				}

				F="CELL "+Lines[k].S[0]+"\t"+s.FormatFloat("#0.000",Cells[Ni].betta)+"\t"+s.FormatFloat("#0.000",Cells[Ni].ELP)+"\t"+s.FormatFloat("#0.000000",Cells[Ni].AL32)+"\t"+s.FormatFloat("#0.000000",Cells[Ni].AkL);
				ParsedStrings->Add(F);

				Cells[Ni].F0=F0*1e6;
				Cells[Ni].P0=P0*1e6;
				Cells[Ni].dF=arc(dF);
				Cells[Ni].Drift=false;
				Cells[Ni].First=NewCell;
				NewCell=false;
				dF=0;

				Ni++;
				if (Nlim>-1 &&Ni>=Nlim)
					OnlyParameters=true;

				break;
			}
			case CELLS:{
				if(OnlyParameters)
					break;
				if (Lines[k].N==4){
					for (int j=0;j<Lines[k].S[0].ToInt();j++){
						Cells[Ni].Mode=Lines[k].S[1].ToDouble();
						Cells[Ni].betta=Lines[k].S[2].ToDouble();
						Cells[Ni].ELP=Lines[k].S[3].ToDouble(); \
						GetDimensions(Cells[Ni]);
						//Cells[Ni].AL32=table function;
						//Cells[Ni].AkL=table function;
						Cells[Ni].F0=F0*1e6;
						Cells[Ni].P0=P0*1e6;
						Cells[Ni].dF=arc(dF);
						Cells[Ni].Drift=false;
						Cells[Ni].First=NewCell;
						NewCell=false;
						dF=0;

						Ni++;
						if (Nlim>-1 &&Ni>=Nlim){
							OnlyParameters=true;
							break;
						}
					}
				} else if (Lines[k].N==6){
					for (int j=0;j<Lines[k].S[0].ToInt();j++){
						Cells[Ni].Mode=Lines[k].S[1].ToDouble();
						Cells[Ni].betta=Lines[k].S[2].ToDouble();
						Cells[Ni].ELP=Lines[k].S[3].ToDouble();
						Cells[Ni].AL32=Lines[k].S[4].ToDouble();
						Cells[Ni].AkL=Lines[k].S[5].ToDouble();

						Cells[Ni].F0=F0*1e6;
						Cells[Ni].P0=P0*1e6;
						Cells[Ni].dF=arc(dF);
						Cells[Ni].Drift=false;
						Cells[Ni].First=NewCell;
						NewCell=false;
						dF=0;

						Ni++;
						if (Nlim>-1 &&Ni>=Nlim){
							OnlyParameters=true;
							break;
						}
					}
				}
				else{
					return ERR_CELLS;
				}
				if (Ni>0){
					F="CELLS "+Lines[k].S[0]+"\t"+Lines[k].S[1]+"\t"+s.FormatFloat("#0.000",Cells[Ni-1].betta)+"\t"+s.FormatFloat("#0.000",Cells[Ni-1].ELP)+"\t"+s.FormatFloat("#0.000000",Cells[Ni-1].AL32)+"\t"+s.FormatFloat("#0.000000",Cells[Ni-1].AkL);
					ParsedStrings->Add(F);
				}

				break;
			}
			case BEAM:{
				if (Lines[k].N!=6){
					return ERR_BEAM;
				}
				Phi0=Lines[k].S[0].ToDouble();
				dPhi=Lines[k].S[1].ToDouble();
				Phi_Eq=(Lines[k].S[2]!="NORM");
				W0=Lines[k].S[3].ToDouble();
				dW=Lines[k].S[4].ToDouble();
				W_Eq=(Lines[k].S[5]!="NORM");

				F="BEAM "+Lines[k].S[0]+"\t"+Lines[k].S[1]+"\t"+Lines[k].S[2]+"\t"+Lines[k].S[3]+"\t"+Lines[k].S[4]+"\t"+Lines[k].S[5];
				ParsedStrings->Add(F);
				break;
			}
			case CURRENT:{
				if (Lines[k].N!=5 && Lines[k].N!=2){
					return ERR_CURRENT;
				}
				I0=Lines[k].S[0].ToDouble();
				F="CURRENT "+Lines[k].S[0]+"\t"+Lines[k].S[1]+"\t";//+Lines[k].S[2]+"\t";
				if (Lines[k].N==5 ){
					Np=Lines[k].S[1].ToInt();
					AlphaCS=Lines[k].S[2].ToDouble();
					BettaCS=Lines[k].S[3].ToDouble();
					EmittanceCS=Lines[k].S[4].ToDouble();
					F+=Lines[k].S[2]+"\t"+Lines[k].S[3]+"\t"+Lines[k].S[4];
				} else if (Lines[k].N==2){
					if (Lines[k].S[1]=="CST_X" || Lines[k].S[1]=="CST")
						BeamType=CST_X;
					else if (Lines[k].S[1]=="CST_Y")
						BeamType=CST_Y;
				   /*	else if (Lines[k].S[1]=="CST_R")
						BeamType=CST_R;   */
					else
						return ERR_CURRENT;
					if (BeamType!=RANDOM)
						F="CURRENT "+Lines[k].S[0]+Lines[k].S[1];
				}
				//F="CURRENT "+Lines[k].S[0]+"\t"+Lines[k].S[1]+"\t"+Lines[k].S[2]+"\t"+Lines[k].S[3]+"\t"+Lines[k].S[4];
				ParsedStrings->Add(F);
				break;
			}
			case OPTIONS:{
				F="OPTIONS ";
				for (int j=0;j<Lines[k].N;j++){
					if (Lines[k].S[j]=="COULOMB")
						Coulomb=true;
					if (Lines[k].S[j]=="REVERSE")
						Reverse=true;
					if (Lines[k].S[j]=="MAGNETIZED")
						Magnetized=true;
					F=F+"\t"+Lines[k].S[j];
				}
				ParsedStrings->Add(F);
				break;
			}
			case DRIFT:{
				if(OnlyParameters)
					break;
				if (Lines[k].N!=2){
					return ERR_DRIFT;
				}
				Cells[Ni].Drift=true;
				Cells[Ni].betta=Lines[k].S[0].ToDouble()/100;//D, cm
				Cells[Ni].AkL=Lines[k].S[1].ToDouble()/100;//Ra, cm
				Cells[Ni].ELP=0;
				Cells[Ni].AL32=0;
				Cells[Ni].First=true;
				Cells[Ni].F0=c;
				Cells[Ni].dF=arc(dF);
				dF=0;
				NewCell=true;

				F="DRIFT "+Lines[k].S[0]+"\t"+Lines[k].S[1];
				ParsedStrings->Add(F);

				Ni++;
				if (Nlim>-1 && Ni>=Nlim)
					OnlyParameters=true;
				break;
			}
			case COUPLER:{
				if (Lines[k].N==2){
					P0=Lines[k].S[0].ToDouble();
					F0=Lines[k].S[1].ToDouble();
					NewCell=true;
				}
				else if(Lines[k].N==3){
					P0=Lines[k].S[0].ToDouble();
					F0=Lines[k].S[1].ToDouble();
					dF=Lines[k].S[2].ToDouble();
					NewCell=true;
				}
				else{
					return ERR_COUPLER;
				}

				F="POWER "+Lines[k].S[0]+"\t"+Lines[k].S[1]+"\t"+s.FormatFloat("#0.00",dF);
				ParsedStrings->Add(F);

				break;
			}
		}
	}

	return ERR_NO;
}
//---------------------------------------------------------------------------
TError TBeamSolver::LoadData(int Nl)
{
	char *FileName=InputFile.c_str();
	LoadIniConstants();
	InputStrings->Clear();
	Nlim=Nl;

	DataReady=false;

	if (FileName=="")
		return ERR_NOFILE;
	if (!FileExists(FileName))
		return ERR_OPENFILE;

	delete[] Cells;

	AnsiString S;
	TInputLine *Lines;
	int i=-1,j=0,N=0;
	TError Parsed;

	Lines=ParseFile(N);
	//InputStrings->LoadFromFile(InputFile);

	Ncells=0;
	for (int k=0;k<N;k++){
		if (Lines[k].P==CELL)
			Ncells++;
		else if (Lines[k].P==CELLS)
			Ncells+=Lines[k].S[0].ToInt();
		else if (Lines[k].P==DRIFT)
			Ncells++;

	}
	if (Nlim>-1 && Ncells>=Nlim)
		Ncells=Nlim;
	Cells = new TCell[Ncells];

	Parsed=ParseLines(Lines,N);
	ParsedStrings->Add("END");
	InputStrings->AddStrings(ParsedStrings);
	
  	ParsedStrings->SaveToFile("PARSED.TXT");

	delete[] Lines;
	DataReady=true;
	return Parsed;
}
//---------------------------------------------------------------------------
TError TBeamSolver::MakeBuncher(TCell& iCell)
{
	char *FileName=InputFile.c_str();
	InputStrings->Clear();
	//LoadIniConstants();

	DataReady=false;
	TError Error;

	if (FileName=="")
		return ERR_NOFILE;
	if (!FileExists(FileName))
		return ERR_OPENFILE;

	delete[] Cells;

	AnsiString F,s;
	TInputLine *Lines;
	int i=-1,j=0,N=0;

	Lines=ParseFile(N);
	Error=ParseLines(Lines,N,true);

	if (F0*1e6!=iCell.F0 || P0*1e6!=iCell.P0){
		F0=iCell.F0*1e-6;
		P0=iCell.P0*1e-6;
		F="POWER "+s.FormatFloat("#0.00",P0)+"\t"+s.FormatFloat("#0.00",F0);
		ParsedStrings->Add(F);
	}

	ParsedStrings->Add("");

	double k1=0,k2=0,k3=0,k4=0,k5=0;
	double Am=iCell.ELP*sqrt(P0*1e6)/We0;
	k1=3.8e-3*(Power(10.8,Am)-1);
	k2=1.25*Am+2.25;
	k3=0.5*Am+0.15*sqrt(Am);
	k4=0.5*Am-0.15*sqrt(Am);
	k5=1/sqrt(1.25*sqrt(Am));

	double b=0,A=0,ksi=0,lmb=0,th=0;
	double b0=MeVToVelocity(W0);

	lmb=1e-6*c/F0;
	if (iCell.Mode==90)
		th=pi/2;
	else if (iCell.Mode==120)
		th=2*pi/3;

	Ncells=0;
	int iB=0;

	do {
		b=(2/pi)*(1-b0)*arctg(k1*Power(ksi,k2))+b0;
	   //	b=(2/pi)*arctg(0.25*sqr(10*ksi*lmb)+0.713);
		iB=10000*b;
		ksi+=b*th/(2*pi);
		Ncells++;
	} while (iB<9990);
	//} while (ksi*lmb<1.25);

	Cells=new TCell[Ncells];

	ksi=0;
	for (int i=0;i<Ncells;i++){
		ksi+=0.5*b*th/(2*pi);

		b=(2/pi)*(1-b0)*arctg(k1*Power(ksi,k2))+b0;
	   //	b=(2/pi)*arctg(0.25*sqr(10*ksi*lmb)+0.713);

		A=k3-k4*cos(pi*ksi/k5);
		//A=3.0e6*sin(0.11*sqr(10*ksi*lmb)+0.64);
	   /*
		if (ksi*lmb>0.3)
			A=3.00e6;     */

		if (ksi>k5)
			A=Am;

		ksi+=0.5*b*th/(2*pi);

		Cells[i].Mode=iCell.Mode;
		Cells[i].betta=(2/pi)*(1-b0)*arctg(k1*Power(ksi,k2))+b0;
		//Cells[i].betta=(2/pi)*arctg(0.25*sqr(10*ksi*lmb)+0.713);

		Cells[i].ELP=A*We0/sqrt(1e6*P0);
	   //	Cells[i].ELP=A*lmb/sqrt(1e6*P0);

		GetDimensions(Cells[i]);

		F="CELL "+s.FormatFloat("#0",iCell.Mode)+"\t"+s.FormatFloat("#0.000",Cells[i].betta)+"\t"+s.FormatFloat("#0.000",Cells[i].ELP)+"\t"+s.FormatFloat("#0.000000",Cells[i].AL32)+"\t"+s.FormatFloat("#0.000000",Cells[i].AkL);
		ParsedStrings->Add(F);

		Cells[i].F0=F0*1e6;
		Cells[i].P0=P0*1e6;
		Cells[i].dF=0;
		Cells[i].Drift=false;
		Cells[i].First=false;
	}
	Cells[0].First=true;

	ParsedStrings->Add("END");
	InputStrings->AddStrings(ParsedStrings);
	ParsedStrings->SaveToFile("PARSED.TXT");

	return Error;
}
//---------------------------------------------------------------------------
int TBeamSolver::ChangeCells(int N)
{
	TCell *iCells;
	iCells=new TCell[Ncells];

	for (int i=0;i<Ncells;i++)
		iCells[i]=Cells[i];

	delete[] Cells;

	int Nnew=Ncells<N?Ncells:N;
	int Nprev=Ncells;
	Ncells=N;

	Cells=new TCell[Ncells];
	
	for (int i=0;i<Nnew;i++)
    	Cells[i]=iCells[i];

	delete[] iCells;
	
	return Nprev;
}
//---------------------------------------------------------------------------
void TBeamSolver::AppendCells(TCell& iCell,int N)
{
	int Nprev=ChangeCells(Ncells+N);
	int i=InputStrings->Count-1;
	InputStrings->Delete(i);

	i=Nprev;
	Cells[i]=iCell;
	GetDimensions(Cells[i]);

	for (int j=Nprev;j<Ncells;j++)
		Cells[j]=Cells[i];

	AnsiString F,F1,s;
	F1=F=s.FormatFloat("#0",iCell.Mode)+"\t"+s.FormatFloat("#0.000",Cells[i].betta)+"\t"+s.FormatFloat("#0.000",Cells[i].ELP)+"\t"+s.FormatFloat("#0.000000",Cells[i].AL32)+"\t"+s.FormatFloat("#0.000000",Cells[i].AkL);
	if (N==1)
		F="CELL "+F1;
	else
		F="CELLS "+s.FormatFloat("#0",N)+F1;

	InputStrings->Add(F);
	InputStrings->Add("END");

}
//---------------------------------------------------------------------------
void TBeamSolver::AddCells(int N)
{
	TCell pCell=Cells[Ncells-1];
	AppendCells(pCell,N);
}
//---------------------------------------------------------------------------
TCell TBeamSolver::GetCell(int j)
{
	if (j<0)
		j=0;
	if (j>=Ncells)
		j=Ncells-1;

	return Cells[j];
}
//---------------------------------------------------------------------------
TCell TBeamSolver::LastCell()
{
	return GetCell(Ncells-1);
}
//---------------------------------------------------------------------------
void TBeamSolver::ChangeInputCurrent(double Ib)
{
	I0=Ib;
}
//---------------------------------------------------------------------------
double *TBeamSolver::SmoothInterpolation(double *x,double *X,double *Y,int Nbase,int Nint,double p0,double *W)
{
	TSpline *Spline;
	double *y;
	//y=new double[Nint];

	Spline=new TSpline;
	Spline->MakeSmoothSpline(X,Y,Nbase,p0,W);
	y=Spline->Interpolate(x,Nint);

	delete Spline;

	return y;
}
//---------------------------------------------------------------------------
double *TBeamSolver::SplineInterpolation(double *x,double *X,double *Y,int Nbase,int Nint)
{
	TSpline *Spline;
	double *y;
	y=new double[Nint];

	Spline=new TSpline;
	Spline->MakeCubicSpline(X,Y,Nbase);
	y=Spline->Interpolate(x,Nint);

	delete Spline;

	return y;
}
//---------------------------------------------------------------------------
double *TBeamSolver::LinearInterpolation(double *x,double *X,double *Y,int Nbase,int Nint)
{
	TSpline *Spline;
	double *y;
	y=new double[Nint];

	Spline=new TSpline;
	Spline->MakeLinearSpline(X,Y,Nbase);
	y=Spline->Interpolate(x,Nint);

	delete Spline;

	return y;
}
//---------------------------------------------------------------------------
int TBeamSolver::CreateGeometry()
{
	F0*=1e6;
	P0*=1e6;
	double theta=0;
	double *X_base,*B_base,*E_base,*Al_base,*A_base;
	double *X_int,*B_int,*E_int,*Al_int,*A_int;

	bool Solenoid_success=false;

	int Njmp=0;

	for(int i=0;i<Ncells;i++){
		if (Cells[i].First)
			Njmp++;
	}

	Npoints=Ncells*Nmesh+Njmp;
	TSplineType Spl;

	X_base = new double[Ncells]; X_int=new double[Npoints];
	B_base = new double[Ncells]; B_int=new double[Npoints];
	E_base = new double[Ncells]; E_int=new double[Npoints];
	Al_base = new double[Ncells]; Al_int=new double[Npoints];

	memset(Structure, 0, sizeof(Structure));
	delete[] Structure;

	double z=0,zm=0,D=0,x=0;

	Structure=new TStructure[Npoints];

	int k=0;
	for (int i=0;i<Ncells;i++){
		int Extra=0;

		if (i==Ncells-1)
			Extra=1;
		else if (Cells[i+1].First)
			Extra=1;

		if (Cells[i].First)
			z-=zm;

		double lmb=1;
		if (Cells[i].Drift){
			D=Cells[i].betta;
			bool isInput=false;
			for (int j=i;j<Ncells;j++){
				if (!Cells[j].Drift){
					Cells[i].betta=Cells[j].betta;
					lmb=c/Cells[j].F0;
					isInput=true;
					break;
				}
			}
			if (!isInput){
				for (int j=i;j>=0;j--){
					if (!Cells[j].Drift){
						Cells[i].betta=Cells[j].betta;
						lmb=c/Cells[j].F0;
						isInput=true;
						break;
					}
				}
			}
			if (!isInput){
				Cells[i].betta=1;
				lmb=1;
			}
		}else{
			lmb=c/Cells[i].F0;
			theta=Cells[i].Mode*pi/180;
			D=Cells[i].betta*lmb*theta/(2*pi);
		}
		x+=D/2;
		X_base[i]=x/lmb;
		x+=D/2;
		B_base[i]=Cells[i].betta;
		E_base[i]=Cells[i].ELP;
		Al_base[i]=Cells[i].AL32;
		zm=D/Nmesh;
		int k0=k;
		Structure[k].dF=Cells[i].dF;
		for (int j=0;j<Nmesh+Extra;j++){
			X_int[k]=z/lmb;
			Structure[k].ksi=z/lmb;
			Structure[k].lmb=lmb;
			Structure[k].P=Cells[i].P0;
            Structure[k].dF=0;
			Structure[k].drift=Cells[i].Drift;
			if (Cells[i].Drift)
				Structure[k].Ra=Cells[i].AkL/lmb;
			else
				Structure[k].Ra=Cells[i].AkL;//*lmb;
			Structure[k].jump=false;
			Structure[k].CellNumber=i;
			z+=zm;
			k++;
		}
		if (Cells[i].First)
			Structure[k0].jump=true;
	}




   //	int Njmp=0;
//	Structure[0].jump=true;

	double *Xo=NULL, *Bo=NULL, *Eo=NULL, *Ao=NULL;
	double *Xi=NULL, *Bi=NULL, *Ei=NULL, *Ai=NULL;
	int Ncls=0;
	int Npts=0;

	Njmp=0;
	int iJmp=0;

	bool EndOfBlock=false;

  /*	FILE *F;
	F=fopen("cells.log","w");*/

	for (int i=0;i<=Ncells;i++){
		if (i==Ncells)
			EndOfBlock=true;
		else if (Cells[i].First && i!=0)
			EndOfBlock=true;
		else
			EndOfBlock=false;

		if (EndOfBlock/*Cells[i].First && i!=0 || i==Ncells*/){
			Ncls=i-Njmp;
			Npts=Ncls*Nmesh+1;

			/*if (i!=Ncells)
				Structure[i*Nmesh].jump=true;*/

			Xo=new double[Ncls];
			Bo=new double[Ncls];
			Eo=new double[Ncls];
			Ao=new double[Ncls];
			Xi=new double[Npts];

			for (int j=0;j<Ncls;j++){
				Xo[j]=X_base[Njmp+j];
				Bo[j]=B_base[Njmp+j];
				Eo[j]=E_base[Njmp+j];
				Ao[j]=Al_base[Njmp+j];
			}

			for (int j=0;j<Npts;j++)
				Xi[j]=X_int[Njmp*Nmesh+iJmp+j];

			Spl=(Ncls<4)?LSPLINE:SplineType;

			if (Ncls==1)
				Spl=ZSPLINE;

			switch (Spl) {
				case ZSPLINE:{
					Bi=new double[Npts];
					Ei=new double[Npts];
					Ai=new double[Npts];
					for (int j=0;j<Npts;j++){
						Bi[j]=Bo[0];
						Ei[j]=Eo[0];
						Ai[j]=Ao[0];
					}
					break;
				}
				case(LSPLINE):{
					Bi=LinearInterpolation(Xi,Xo,Bo,Ncls,Npts);
					Ei=LinearInterpolation(Xi,Xo,Eo,Ncls,Npts);
					Ai=LinearInterpolation(Xi,Xo,Ao,Ncls,Npts);
					break;
				}
				case(CSPLINE):{
					Bi=SplineInterpolation(Xi,Xo,Bo,Ncls,Npts);
					Ei=SplineInterpolation(Xi,Xo,Eo,Ncls,Npts);
					Ai=SplineInterpolation(Xi,Xo,Ao,Ncls,Npts);
					break;
				}
				case(SSPLINE):{
					Bi=SmoothInterpolation(Xi,Xo,Bo,Ncls,Npts,Smooth);
					Ei=SmoothInterpolation(Xi,Xo,Eo,Ncls,Npts,Smooth);
					Ai=SmoothInterpolation(Xi,Xo,Ao,Ncls,Npts,Smooth);
					break;
				}
			}

			for (int j=0;j<Npts;j++){
				B_int[Njmp*Nmesh+iJmp+j]=Bi[j];
				E_int[Njmp*Nmesh+iJmp+j]=Ei[j];
				Al_int[Njmp*Nmesh+iJmp+j]=Ai[j];
			}


		/*	for (int i=0;i<Npts;i++){
				fprintf(F,"%i %f %f\n",Njmp*Nmesh+i,Xi[i],Ei[i]);
			}     */
			

			delete[] Xo;
			delete[] Xi;
			delete[] Bo;
			delete[] Bi;
			delete[] Eo;
			delete[] Ei;
			delete[] Ao;
			delete[] Ai;

			Njmp=i;
			iJmp++;
		}

	}
	// fclose(F);

	/*
	switch (Spl) {
		case(LSPLINE):{
			B_int=LinearInterpolation(X_int,X_base,B_base,Ncells,Npoints);
			E_int=LinearInterpolation(X_int,X_base,E_base,Ncells,Npoints);
			Al_int=LinearInterpolation(X_int,X_base,Al_base,Ncells,Npoints);
			break;
		}
		case(CSPLINE):{
			B_int=SplineInterpolation(X_int,X_base,B_base,Ncells,Npoints);
			E_int=SplineInterpolation(X_int,X_base,E_base,Ncells,Npoints);
			Al_int=SplineInterpolation(X_int,X_base,Al_base,Ncells,Npoints);
			break;
		}
		case(SSPLINE):{
			B_int=SmoothInterpolation(X_int,X_base,B_base,Ncells,Npoints,Smooth);
			E_int=SmoothInterpolation(X_int,X_base,E_base,Ncells,Npoints,Smooth);
			Al_int=SmoothInterpolation(X_int,X_base,Al_base,Ncells,Npoints,Smooth);
			break;
		}

	}      */

   /*	E_int[Npoints-1]=E_int[Npoints-2];
	B_int[Npoints-1]=B_int[Npoints-2];
	Al_int[Npoints-1]=Al_int[Npoints-2];*/

  /*	for (int i=0;i<Npoints;i++){
		Structure[i].Rp=E_int[i];
	}   */


	if (FSolenoid) {
		int NSol=0;
		double *Xz=NULL;
		double *Bz=NULL;

		NSol=GetSolenoidPoints();
		if(NSol<1){
			Zmag=0;
			B0=0;
			Lmag=0;
			FSolenoid=false;
		} else {
			Xz=new double[NSol];
			Bz=new double[NSol];

			ReadSolenoid(NSol,Xz,Bz);
			if (NSol==1) {
				Zmag=0;
				Lmag=Structure[Npoints-1].ksi*Structure[Npoints-1].lmb;
				B0=Bz[0];
				FSolenoid=false;
				delete[] Xz;
				delete[] Bz;
			} else{
				Xi=new double[Npoints];
				for (int i=0; i<Npoints; i++)
					Xi[i]=Structure[i].ksi*Structure[i].lmb;

				Bi=LinearInterpolation(Xi,Xz,Bz,NSol,Npoints);
				delete[] Xz;
				delete[] Bz;
				delete[] Xi;
			}
		}
	}

	for (int i=0;i<Npoints;i++){
		//int s=0;
		double lmb=1;
		lmb=Structure[i].lmb;
		if (B_int[i]<1)
			Structure[i].betta=B_int[i];
		else
			Structure[i].betta=0.999;

		Structure[i].E=E_int[i];
		Structure[i].A=Structure[i].P>0?E_int[i]*sqrt(Structure[i].P)/We0:0;
/*		if (i==20)
			s=1;*/
		Structure[i].Rp=sqr(E_int[i])/2;;
		Structure[i].B=Structure[i].Rp/(2*We0);
		Structure[i].alpha=Al_int[i]/(lmb*sqrt(lmb));
		if (FSolenoid) {
			Structure[i].B_ext=Bi[i]/10000;
		} else {
			if (Structure[i].ksi>=Zmag/lmb && Structure[i].ksi<=(Zmag+Lmag)/lmb)
				Structure[i].B_ext=B0;
			else
				Structure[i].B_ext=0;
		}
	}

	if (FSolenoid) {
		delete[] Bi;
	}

	delete[] X_base; delete[] X_int;
	delete[] B_base; delete[] B_int;
	delete[] E_base; delete[] E_int;
	delete[] Al_base; delete[] Al_int;

	return ERR_NO;
}
//---------------------------------------------------------------------------
int TBeamSolver::CreateBeam()
{
	double sx=0,sy=0,r=0;
	double b0=0,db=0;
	bool CST_success=0;

	//Npoints=Ncells*Nmesh;

	for (int i=0;i<Np_beam;i++){
		memset(Beam[i], 0, sizeof(Beam[i]));
		delete Beam[i];
	}
	memset(Beam, 0, sizeof(Beam));
	delete[] Beam;

	if (BeamType!=RANDOM){
		Np=Beam[0]->CountCSTParticles(BeamType);
		if(Np<0)
			return ERR_CURRENT;
	}

	Beam=new TBeam*[Npoints];
	for (int i=0;i<Npoints;i++){
		Beam[i]=new TBeam(Np);
		Beam[i]->SetBarsNumber(Nbars);
		Beam[i]->SetKernel(Kernel);
		Beam[i]->lmb=Structure[i].lmb;
		Beam[i]->I0=I0;
		Beam[i]->Reverse=Reverse;
		//Beam[i]->Cmag=c*Cmag/(Structure[i].lmb*We0); //Cmag = c*B*lmb/Wo * (1/lmb^2 from r normalization)
		for (int j=0;j<Np;j++){
			Beam[i]->Particle[j].lost=LIVE;
			Beam[i]->Particle[j].betta=0;
			Beam[i]->Particle[j].Bx=0;
			Beam[i]->Particle[j].phi=0;
			Beam[i]->Particle[j].Bth=0;
			Beam[i]->Particle[j].x=0;
			Beam[i]->Particle[j].x0=0;
			Beam[i]->Particle[j].Th=0;
		}
	}
	Beam[0]->Ib=I0;
	Np_beam=Npoints;

	
	for (int i=0;i<Np;i++)
		Beam[0]->Particle[i].z=Structure[0].ksi*Structure[0].lmb;

   /*	b0=MeVToVelocity(W0);
	db=MeVToVelocity(dW);   */
	if (W_Eq)
		Beam[0]->MakeEquiprobableDistribution(W0,dW,BETTA_PAR);
	else
		Beam[0]->MakeGaussDistribution(W0,dW,BETTA_PAR);

	for (int i=0;i<Np;i++)
		Beam[0]->Particle[i].betta=MeVToVelocity(Beam[0]->Particle[i].betta);

	if (Phi_Eq)
		Beam[0]->MakeEquiprobableDistribution(HellwegTypes::DegToRad(Phi0)-Structure[0].dF,HellwegTypes::DegToRad(dPhi),PHI_PAR);
	else
		Beam[0]->MakeGaussDistribution(HellwegTypes::DegToRad(Phi0)-Structure[0].dF,HellwegTypes::DegToRad(dPhi),PHI_PAR);

	if (BeamType==RANDOM)
		Beam[0]->MakeGaussEmittance(AlphaCS,BettaCS,EmittanceCS);
	else
		CST_success=Beam[0]->ReadCSTEmittance(BeamType);

	if (BeamType!=CST_Y)
		Beam[0]->MakeEquiprobableDistribution(pi,pi,TH_PAR);
	Beam[0]->MakeEquiprobableDistribution(0,0,BTH_PAR);

	for (int i=0;i<Npoints;i++){
		for (int j=0;j<Np;j++)
			Beam[i]->Particle[j].x0=Beam[0]->Particle[j].x;
	}

	if (!CST_success)
		return ERR_CURRENT;

	
 /*	for (int i=0;i<Np;i++){
		Beam[0]->Particle[i].x=0;//-0.001+0.002*i/(Np-1);
		//Beam[0]->Particle[i].phi=0;
	 	Beam[0]->Particle[i].Bx=0;
		Beam[0]->Particle[i].Th=0;
		Beam[0]->Particle[i].Bth=0;

	  //	Beam[0]->Particle[i].phi=DegToRad(-90+i);
	   //	Beam[0]->Particle[i].betta=MeVToVelocity(0.05);
	}           */
	return ERR_NO;
}
//---------------------------------------------------------------------------
int TBeamSolver::GetSolenoidPoints()
{
	int N=-1;
	fstream fs(Solenoid_File);
	char s[128];

	while (!fs.eof()){
		fs.getline(s, sizeof(s)) ;
		N++;
	}

	fs.close();

	return N;
}
//---------------------------------------------------------------------------
bool TBeamSolver::ReadSolenoid(int Nz,double *Z,double* B)
{
	fstream fs(Solenoid_File);
	float z=0,Bz=0;
	char s[128];
	AnsiString S;

	fs.getline(s, sizeof(s)) ;
	for (int i=0;i<Nz;i++){
		try {
			fs>>s;
			S=AnsiString(s);
			z=S.ToDouble();
			Z[i]=z;

			fs>>s;
			S=AnsiString(s);
			Bz=S.ToDouble();
			B[i]=Bz;
		} catch (...){
			fs.close();
			return false;
		}
	}

	fs.close();
	return true;
}
//---------------------------------------------------------------------------
void TBeamSolver::GetEllipticParameters(int Nknot, double& x0,double& y0, double& a,double& b,double& phi,double &Rx,double& Ry)
{
	Beam[Nknot]->GetEllipticParameters(x0,y0,a,b,phi,Rx,Ry);
}
//---------------------------------------------------------------------------
void TBeamSolver::GetCourantSneider(int Nknot, double& alpha,double& betta, double& epsilon)
{
	Beam[Nknot]->GetCourantSneider(alpha,betta,epsilon);
}
//---------------------------------------------------------------------------
TSpectrumBar *TBeamSolver::GetEnergySpectrum(int Nknot,double& Wav,double& dW)
{
	TSpectrumBar *Spectrum;
	Spectrum=GetEnergySpectrum(Nknot,false,Wav,dW);
	return Spectrum;
}
//---------------------------------------------------------------------------
TSpectrumBar *TBeamSolver::GetPhaseSpectrum(int Nknot,double& Fav,double& dF)
{
	TSpectrumBar *Spectrum;
	Spectrum=GetPhaseSpectrum(Nknot,false,Fav,dF);
 	return Spectrum;
}
//---------------------------------------------------------------------------
TSpectrumBar *TBeamSolver::GetEnergySpectrum(int Nknot,bool Env,double& Wav,double& dW)
{
	TSpectrumBar *Spectrum;
	Spectrum=Beam[Nknot]->GetEnergySpectrum(Env,Wav,dW);
	return Spectrum;
}
//---------------------------------------------------------------------------
TSpectrumBar *TBeamSolver::GetPhaseSpectrum(int Nknot,bool Env,double& Fav,double& dF)
{
	TSpectrumBar *Spectrum;
	Spectrum=Beam[Nknot]->GetPhaseSpectrum(Env,Fav,dF);
	return Spectrum;
}
//---------------------------------------------------------------------------
void TBeamSolver::GetBeamParameters(int Nknot,double *X,TBeamParameter Par)
{
	Beam[Nknot]->GetParameters(X,Par);
}
//---------------------------------------------------------------------------
double TBeamSolver::GetKernel()
{
	return Beam[0]->h;
}
//---------------------------------------------------------------------------
void TBeamSolver::GetStructureParameters(double *X,TStructureParameter Par)
{
	switch (Par) {
		case (KSI_PAR):{
			for (int i=0;i<Npoints;i++)
				X[i]=Structure[i].ksi;
			break;
		}
		case (Z_PAR):{
			for (int i=0;i<Npoints;i++)
				X[i]=Structure[i].ksi*Structure[i].lmb;
			break;
		}
		case (A_PAR):{
			for (int i=0;i<Npoints;i++)
				X[i]=Structure[i].A;
			break;
		}
		case (RP_PAR):{
			for (int i=0;i<Npoints;i++)
				X[i]=Structure[i].Rp;
			break;
		}
		case (B_PAR):{
			for (int i=0;i<Npoints;i++)
				X[i]=Structure[i].A*We0/Structure[i].lmb;
			break;
		}
		case (ALPHA_PAR):{
			for (int i=0;i<Npoints;i++)
				X[i]=Structure[i].alpha;
			break;
		}
		case (BETTA_F_PAR):{
			for (int i=0;i<Npoints;i++)
				X[i]=Structure[i].betta;
			break;
		}
		case (RA_PAR):{
			for (int i=0;i<Npoints;i++)
				X[i]=Structure[i].Ra*Structure[i].lmb;
			break;
		}
		case (B_EXT_PAR):{
			for (int i=0;i<Npoints;i++)
				X[i]=Structure[i].B_ext;
			break;
		}
		case (NUM_PAR):{
			for (int i=0;i<Npoints;i++)
				X[i]=Structure[i].CellNumber;
			break;
		}
	}
}
//---------------------------------------------------------------------------
void TBeamSolver::Abort()
{
	Stop=true;
}

//---------------------------------------------------------------------------
void TBeamSolver::Integrate(int Si, int Sj)
{
	double Rb=0,Lb=0,Fb=0,gamma=1,Mr=0,phic=0,Icur=0;
	TParticle *Particle=Beam[Si]->Particle;

	phic=Beam[Si]->iGetAveragePhase(Par[Sj],K[Sj]);
	Par[Sj].SumSin=0;
	Par[Sj].SumCos=0;
	Par[Sj].SumSin=Beam[Si]->SinSum(Par[Sj],K[Sj]);
	Par[Sj].SumCos=Beam[Si]->CosSum(Par[Sj],K[Sj]);

	gamma=Beam[Si]->iGetAverageEnergy(Par[Sj],K[Sj]);
	Par[Sj].gamma=gamma;

	Lb=Beam[Si]->iGetBeamLength(Par[Sj],K[Sj])/2;
  	Fb=Lb*2*pi/lmb;
	Rb=Beam[Si]->iGetBeamRadius(Par[Sj],K[Sj]);
	
	Icur=I;//*Lb/lmb;   //������ ����������� ����

	if (Rb==0)
		Mr=0;
	else
	 	Mr=FormFactor(gamma*Lb/Rb);

	Par[Sj].Bz_ext*=lmb*c/We0;
   //	Par[Sj].Bz_ext*=lmb/(myu0*We0);
	Par[Sj].Br_ext=0;
	if (Magnetized)
		Par[Sj].Cmag=Structure[0].B_ext*lmb*c/We0;
	else
        Par[Sj].Cmag=0;

	double phi=0,r=0;
	double Aqz=0,Aqr=0;

	Par[Sj].Aqz=new double[Np];
	Par[Sj].Aqr=new double[Np];

	for (int i=0;i<Np;i++){
		Par[Sj].Aqz[i]=0;
		Par[Sj].Aqr[i]=0;
		if (Particle[i].lost==LIVE){
			phi=Particle[i].phi+K[Sj][i].phi*Par[Sj].h;
			r=(Particle[i].x+K[Sj][i].r*Par[Sj].h)*lmb;
			double V=mod(sqr(Rb)*Lb);
			double z=(phi-phic)*lmb/(2*pi);
			
			if (Rb!=0 && Lb!=0 && Coulomb){
				//Par[Sj].Aqz=3*lmb*Mr*(phi-phic)*(Icur/Ia)/(sqr(gamma)*sqr(Rb)*Fb);
				Par[Sj].Aqz[i]=kFc*(3*Icur*lmb)*(Mr*z/V);  //E
				Par[Sj].Aqz[i]*=(lmb/We0);  //A
						
			  //	Par[Sj].Aqr=sqr(lmb)*(1-Mr)*r*lmb*(Icur/Ia)/(sqr(gamma)*sqr(Rb)*Lb);
				Par[Sj].Aqr[i]=kFc*(3*Icur*lmb/sqr(gamma))*(0.5*(1-Mr)*r/V);  //E
				Par[Sj].Aqr[i]*=(lmb/We0);  //A
			}
			//k1F[i]:=dF_dx(bv[j-1],Beam[i,5,j-1],A[j-1],Btmp,SinSum);
		}
	}

	Beam[Si]->Integrate(Par[Sj],K,Sj);
	delete[] Par[Sj].Aqz;
	delete[] Par[Sj].Aqr;
}
//---------------------------------------------------------------------------
void TBeamSolver::CountLiving(int Si)
{
	Nliv=Beam[Si]->GetLivingNumber();
	if (Nliv==0){
	  /*	FILE *F;
		F=fopen("beam.log","w");
		for (int i=Si;i<Npoints;i++){
			for (int j=0;j<Np;j++)
				fprintf(F,"%i ",Beam[i]->Particle[j].lost);
			fprintf(F,"\n");
		}
		fclose(F);   */
		ShowMessage("Beam Lost!");
		Stop=true;
		return;
	}
}
//---------------------------------------------------------------------------
void TBeamSolver::Step(int Si)
{
	bool drift=false;
	lmb=Structure[Si].lmb;
	Beam[Si]->lmb=lmb;
	CountLiving(Si);
	I=I0*Nliv/Np;
  /*	
	Rb=Beam[i]->GetBeamRadius();
	phi0=Beam[i]->GetAveragePhase();
	dphi=Beam[i]->GetPhaseLength();
	Lb=dphi*lmb/(2*pi);
	betta0=Beam[i]->GetAverageEnergy();

	w=Structure[i]->alpha*lmb;        */
	drift=(Structure[Si].drift);
	for (int i=0;i<4;i++)
		Par[i].drift=Structure[Si].drift;
	//Par[3].drift=Structure[Si+1].drift;

	dh=Structure[Si+1].ksi-Structure[Si].ksi;
	Par[0].h=0;
	Par[1].h=dh/2;
	Par[2].h=Par[1].h;
	Par[3].h=dh;

	double db=Structure[Si+1].betta-Structure[Si].betta;
	Par[0].bw=Structure[Si].betta;
	Par[1].bw=Structure[Si].betta+db/2;
	Par[2].bw=Par[1].bw;
	Par[3].bw=Structure[Si+1].betta;

	double dw=Structure[Si+1].alpha-Structure[Si].alpha;
	Par[0].w=Structure[Si].alpha*lmb;
	Par[1].w=(Structure[Si].alpha+dw/2)*lmb;
	Par[2].w=Par[1].w;
	Par[3].w=Structure[Si+1].alpha*lmb;

	double dE=Structure[Si+1].E-Structure[Si].E;
	Par[0].E=Structure[Si].E;
	Par[1].E=Structure[Si].E+dE/2;
	Par[2].E=Par[1].E;
	Par[3].E=Structure[Si+1].E;

	double dA=Structure[Si+1].A-Structure[Si].A;
	Par[0].A=Structure[Si].A;
	Par[1].A=Structure[Si].A;//+dA/2;
	Par[2].A=Par[1].A;
	Par[3].A=Structure[Si].A;

	double dB=Structure[Si+1].B-Structure[Si].B;
	Par[0].B=Structure[Si].B;
	Par[1].B=Structure[Si].B+dB/2;
	Par[2].B=Par[1].B;
	Par[3].B=Structure[Si+1].B;

   /*	for(int i=0;i<4;i++)
		Par[i].B*=I;  */

	double d2E=0;
	double d2A=0;
	double d2h=0;
	double dR=0;
	if (Structure[Si+1].Rp!=0 && Structure[Si].Rp!=0)
		dR=ln(Structure[Si+1].Rp)-ln(Structure[Si].Rp);
	double d2R=0;

	if (drift){
		for (int i=0;i<4;i++){
			Par[i].dL=0;
			Par[i].dA=0;
		}
	} else {
		if (Si==0 || (Si!=0 && Structure[Si].jump)){
			Par[0].dL=dE/(Structure[Si].E*dh);
			//Par[0].dL=dR/dh;
			Par[0].dA=dA/dh;
		}else{
			d2E=Structure[Si+1].E-Structure[Si-1].E;
			d2A=Structure[Si+1].A-Structure[Si-1].A;
			d2h=Structure[Si+1].ksi-Structure[Si-1].ksi;
			Par[0].dL=d2E/(Structure[Si].E*d2h);
		   //	d2R=ln(Structure[Si+1].Rp)-ln(Structure[Si-1].Rp);
		   //	Par[0].dL=d2R/d2h;
			Par[0].dA=d2A/d2h;
		}
		Par[1].dL=dE/((Structure[Si].E+dE/2)*dh);
		//Par[1].dL=dR/dh;
		Par[2].dL=Par[1].dL;

		Par[1].dA=dA/dh;
		Par[2].dA=Par[1].dA;

		if (Si==Npoints-2 || (Si<Npoints-2 && Structure[Si+2].jump)){
			Par[3].dL=dE/(Structure[Si+1].E*dh);
			//Par[3].dL=dR/dh;
			Par[3].dA=dA/dh;
		}else{
		   //	d2E=Structure[Si+2].E-Structure[Si].E;
			d2A=Structure[Si+2].A-Structure[Si].A;
			d2h=Structure[Si+2].ksi-Structure[Si].ksi;
			Par[3].dL=d2E/(Structure[Si+1].E*d2h);
			//d2R=ln(Structure[Si+2].Rp)-ln(Structure[Si].Rp);
			///Par[0].dL=d2R/d2h;
			Par[3].dA=d2A/d2h;
		}
	}

	double dBx=Structure[Si+1].B_ext-Structure[Si].B_ext;
	Par[0].Bz_ext=Structure[Si].B_ext;
	Par[1].Bz_ext=Structure[Si].B_ext+dBx/2;
	Par[2].Bz_ext=Par[1].Bz_ext;
	Par[3].Bz_ext=Structure[Si+1].B_ext;

	/*fprintf(logFile,"Phase Radius Betta\n");
	for (int i=0;i<Np;i++)
		fprintf(logFile,"%f %f %f\n",Beam[Si]->Particle[i].phi,Beam[Si]->Particle[i].x,Beam[Si]->Particle[i].betta);

	fclose(logFile);  */

	for (int j=0;j<Ncoef;j++)
		Integrate(Si,j);


	/*
		����� ����� ������
		�������� step
		������� ������������ �1-�4 �� ��������� ��������
		step(h)
	*/
}
//---------------------------------------------------------------------------
void TBeamSolver::Solve()
{
	if (SmartProgress==NULL){
		ShowMessage("System Message: ProgressBar not assigned! Code needs to be corrected");
		return;
	}
	SmartProgress->Reset(Npoints-1/*Np*/);

  //	logFile=fopen("beam.log","w");
 /*	for (int i=0;i<Np;i++){
	  //	fprintf(logFile,"%f %f\n",Beam[0]->Particle[i].x,Beam[0]->Particle[i].x/Structure[0].lmb);
		Beam[0]->Particle[i].x/=Structure[0].lmb;
	}                                             */
 //	fclose(logFile);
	
	for (int i=0;i<Ncoef;i++){
		memset(K[i], 0, sizeof(K[i]));
		delete[] K[i];
		K[i]=new TIntegration[Np];
	}
	K[0][0].A=Structure[0].A;
   //	Beam[0]->Particle[j].z=Structure[0].ksi*Structure[0].lmb;
   //
	for (int i=0;i<Npoints-1;i++){
		//for (int j=0;j<Np;j++){

			//if (i==0)
		   //	Nliv=Beam[i]->GetLivingNumber();

		if (!Structure[i+1].jump){
			Step(i);
			Structure[i+1].A=Structure[i].A+dh*(K[0][0].A+K[1][0].A+2*K[2][0].A+2*K[3][0].A)/6;
			//	fprintf(logFile,"%f %f %f %f %f\n",K[1][0].A,K[2][0].A,K[3][0].A,K[0][0].A,Structure[i+1].A);
			Beam[i]->Next(Beam[i+1],Par[3],K);
		} else {
			//Structure[i+1].A=Structure[i].A ;
			Beam[i]->Next(Beam[i+1]);
		}

		for (int j=0;j<Np;j++){
			if (Beam[i+1]->Particle[j].lost==LIVE && mod(Beam[i+1]->Particle[j].x)>=Structure[i+1].Ra)
				Beam[i+1]->Particle[j].lost=RADIUS_LOST;
			Beam[i+1]->Particle[j].z=Structure[i+1].ksi*Structure[i+1].lmb;
			Beam[i+1]->Particle[j].phi-=Structure[i+1].dF;
		}
		SmartProgress->operator ++();
		Application->ProcessMessages();
		if (Stop){
			Stop=false;
			ShowMessage("Solve Process Aborted!");
			SmartProgress->Reset();
			return;
		}
		for (int i=0;i<Ncoef;i++)
			memset(K[i], 0, sizeof(K[i]));
		//}
	}

   //	

	SmartProgress->SetPercent(100);
	SmartProgress->SetTime(0);
}
//---------------------------------------------------------------------------
TResult TBeamSolver::Output(AnsiString& FileName,TMemo *Memo)
{
	AnsiString Line,s;
	TStringList *OutputStrings;
	OutputStrings= new TStringList;
	TResult Result;
  /*	int Nliv=0;
	Nliv=Beam[Npoints-1]->GetLivingNumber();
					 */
	OutputStrings->Clear();
	OutputStrings->Add("==========================================");
	OutputStrings->Add("INPUT DATA");
	OutputStrings->Add("==========================================");

  /*	TStringList *InputStrings;
  	InputStrings= new TStringList;
	InputStrings->LoadFromFile(InputFile);    */
	OutputStrings->AddStrings(InputStrings);

	OutputStrings->Add("==========================================");
	OutputStrings->Add("RESULTS");
	OutputStrings->Add("==========================================");
	OutputStrings->Add("");

	double Ws=0;
   //	AnsiString s;
	int j=Npoints-1;
	double z=100*Structure[j].ksi*Structure[j].lmb;

	double W=0,dW=0;

	TSpectrumBar *WSpectrum=GetEnergySpectrum(j,true,W,Ws);
	if (W!=0)
		dW=100*Ws/W;
	else
		dW=100;
	double Wm=Beam[j]->GetMaxEnergy();
	double I=Beam[j]->Ib;
	double I0=Beam[0]->Ib;
	double kc=100.0*Beam[j]->GetLivingNumber()/Beam[0]->GetLivingNumber();
	double r=1e3*Beam[j]->GetBeamRadius();

	double F=0,dF=0;

	TSpectrumBar *FSpectrum=GetPhaseSpectrum(j,true,F,dF);
	double f=1e-6*c/Structure[j].lmb;
	double Ra=1e3*Structure[j].Ra*Structure[j].lmb;
	double P=W*I;

	double v=Structure[j].betta;
	double E=sqrt(2*Structure[j].Rp);
	double Pb=1e-6*sqr(Structure[j].A*We0/E);

	/*double Pw=P0;
	for(int i=1;i<Npoints;i++)
		Pw=Pw*exp(-2*(Structure[i].ksi-Structure[i-1].ksi)*Structure[i].alpha*Structure[i].lmb);  */
	double Pw=1e-6*P0-(P+Pb);

	double alpha=0,betta=0,eps=0;
   	GetCourantSneider(j,alpha,betta,eps);

	double A=0;
	int Na=j-Nmesh/2;
	if (Na>0)
		A=Structure[Na].A;

	Result.Length=z;
	Result.AverageEnergy=W;
	Result.MaximumEnergy=Wm;
	Result.EnergySpectrum=dW;
	Result.InputCurrent=1e3*I0;
	Result.BeamCurrent=1e3*I;
	Result.Captured=kc;
	Result.BeamRadius=r;
	Result.AveragePhase=F;
	Result.PhaseLength=dF;
	Result.BeamPower=P;
	Result.LoadPower=Pb;
	Result.Alpha=alpha;
	Result.Betta=100*betta;
	Result.Emittance=100*eps;
	Result.A=A;

	Line="Total Length = "+s.FormatFloat("#0.000",Result.Length)+" cm";
	OutputStrings->Add(Line);
	Line="Average Energy = "+s.FormatFloat("#0.000",Result.AverageEnergy)+" MeV";
	OutputStrings->Add(Line);
	Line="Maximum Energy = "+s.FormatFloat("#0.000",Result.MaximumEnergy)+" MeV";
	OutputStrings->Add(Line);
	Line="Energy Spectrum = "+s.FormatFloat("#0.00",Result.EnergySpectrum)+" %";
	OutputStrings->Add(Line);
	Line="Input Current = "+s.FormatFloat("#0.00",Result.InputCurrent)+" mA";
	OutputStrings->Add(Line);
	Line="Beam Current = "+s.FormatFloat("#0.00",Result.BeamCurrent)+" mA";
	OutputStrings->Add(Line);
	Line="Captured = "+s.FormatFloat("#0.00",Result.Captured)+" %";
	OutputStrings->Add(Line);
	Line="Beam Radius (RMS) = "+s.FormatFloat("#0.00",Result.BeamRadius)+" mm";
	OutputStrings->Add(Line);
	Line="Average Phase = "+s.FormatFloat("#0.00",Result.AveragePhase)+" deg";
	OutputStrings->Add(Line);
	Line="Phase Length = "+s.FormatFloat("#0.00",Result.PhaseLength)+" deg";
	OutputStrings->Add(Line);
	Line="Beam Power = "+s.FormatFloat("#0.0000",Result.BeamPower)+" MW";
	OutputStrings->Add(Line);
	Line="Load Power = "+s.FormatFloat("#0.0000",Result.LoadPower)+" MW";
	OutputStrings->Add(Line);
	Line="Loss Power = "+s.FormatFloat("#0.0000",Pw)+" MW";
	OutputStrings->Add(Line);
	OutputStrings->Add("Courant-Sneider Parameters:");
	Line="alpha= "+s.FormatFloat("#0.00000",Result.Alpha);
	OutputStrings->Add(Line);
	Line="betta = "+s.FormatFloat("#0.00000",Result.Betta)+" cm/rad";
	OutputStrings->Add(Line);
	Line="epsilon = "+s.FormatFloat("#0.000000",Result.Emittance)+" cm*rad";
	OutputStrings->Add(Line);
	OutputStrings->Add("==========================================");

	if (Memo!=NULL){
		Memo->Lines->AddStrings(OutputStrings);
	}
					   
	delete[] WSpectrum;
	delete[] FSpectrum;  
	OutputStrings->SaveToFile(FileName);
	delete OutputStrings;
   //	delete Strings;

	

	return Result;
}
//---------------------------------------------------------------------------
#pragma package(smart_init)
