//---------------------------------------------------------------------------


#pragma hdrstop

#include "Matrix.h"

using namespace HellwegMatrix;
//---------------------------------------------------------------------------
__fastcall TMatrix::TMatrix(int N0,int M0)
{
	N=N0;
	M=M0;

	A=CreateArray();

	K=(N<M)?N:M;
	Piv=new int[K];

  //	logfile=fopen("matrix.log","w");
}
//---------------------------------------------------------------------------
__fastcall TMatrix::~TMatrix()
{
	DeleteArray(A);
//	DeleteArray(X);
//	DeleteArray(U);

	delete[] Piv;
   //	fclose(logfile);
}
//---------------------------------------------------------------------------
void TMatrix::ChangeDim(int N0,int M0)
{
	DeleteArray(A);
	delete[] Piv;

	N=N0;
	M=M0;

	A=CreateArray();

	K=(N<M)?N:M;
	Piv=new int[K];
	for (int i=0;i<K;i++)
		Piv[i]=0;
}
//---------------------------------------------------------------------------
void TMatrix::DeleteArray(double **B,int N0,int M0)
{
	if (N0==0)
		N0=N;
	if (M0==0)
		M0=M;

	if (B!=NULL){
		for (int i=0;i<N0;i++)
			if (B[i]!=NULL){
				delete[] B[i];
				B[i]=NULL;
			}
		delete[] B;
		B=NULL;
	}
}
//---------------------------------------------------------------------------
double **TMatrix::CreateArray(int N0,int M0)
{
	double **B;

	if (N0==0)
		N0=N;
	if (M0==0)
		M0=M;

	B=new double*[N0];
	for (int i=0;i<N0;i++){
		B[i]=new double[M0];
		for(int j=0;j<M0;j++)
			B[i][j]=0;
	}

	return B;
}
//---------------------------------------------------------------------------
double **TMatrix::CloneArray(double **B)
{
	double **C;
	C=CreateArray();

	for (int i=0;i<N;i++)
		for(int j=0;j<M;j++)
			C[i][j]=B[i][j];

	return C;		

}
//---------------------------------------------------------------------------
void TMatrix::CopyArray(double **B,double **C)
{
	for (int i=0;i<N;i++)
		for(int j=0;j<M;j++)
			B[i][j]=C[i][j];
}
//---------------------------------------------------------------------------
void TMatrix::Dim(int *N0,int *M0)
{
	*N0=N;
	*M0=M;
}
//---------------------------------------------------------------------------
void TMatrix::SetElement(int I,int J,double x)
{
	if ((I>=0) && (I<N) && (J>=0) &&(J<M))
		A[I][J]=x;
}
//---------------------------------------------------------------------------
double TMatrix::GetElement(int I,int J)
{
	double x;

	if ((I>=0) && (I<N) && (J>=0) &&(J<M))
		x=A[I][J];
	else
		x=0;
	
	return x; 
}
//---------------------------------------------------------------------------
void TMatrix::SetPivot(int I,double x)
{
	if ((I>=0) && (I<K))
		Piv[I]=x;
}
//---------------------------------------------------------------------------
double TMatrix::GetPivot(int I)
{
	double x;

	if ((I>=0) && (I<K))
		x=Piv[I];
	else
		x=0;
	
	return x; 
}
//---------------------------------------------------------------------------
TMatrix TMatrix::operator+(TMatrix& B)
{
	int N0,M0;
	B.Dim(&N0,&M0);


	TMatrix C(N,M);
	if (N0!=N && M0!=M)
		for (int i=0;i<N;i++){
			for (int j=0;j<M;j++)
				C.SetElement(i,j,A[i][j]);
	}else{
		for (int i=0;i<N;i++)
			for (int j=0;j<M;j++)
				C.SetElement(i,j,A[i][j]+B.GetElement(i,j));
	}

	return C;

}
//---------------------------------------------------------------------------
TMatrix TMatrix::operator*(TMatrix& B)
{
	int N0,M0;
	double x,b;
	B.Dim(&N0,&M0);

	TMatrix C(N,M0);
	if (N0!=M)
		for (int i=0;i<N;i++){
			for (int j=0;j<M;j++)
				C.Ones();
	}else{
		for (int i=0;i<N;i++){
			for (int j=0;j<M0;j++){
				x=0;
				for (int k=0;k<M;k++){
					b=B.GetElement(k,j);
					x+=A[i][k]*b;
				}
				C.SetElement(i,j,x);
			}
		}
	}

	return C;
}
//---------------------------------------------------------------------------
TMatrix &TMatrix::operator*=(TMatrix& B)
{
	int N0,M0;

	double x,b,**C;
	B.Dim(&N0,&M0);

	C=CreateArray(N,M0);

	if (N0!=M)
		return *this;
	else{
		for (int i=0;i<N;i++){
			for (int j=0;j<M0;j++){
				x=0;
				for (int k=0;k<M;k++){
					b=B.GetElement(k,j);
					x+=A[i][k]*b;
				}
				C[i][j]=x;
			}
		}
		ChangeDim(N,M0);
		CopyArray(A,C);
	}

	DeleteArray(C,N,M0);

	return *this;
}
//---------------------------------------------------------------------------
TMatrix &TMatrix::operator=(TMatrix& B)
{
	int N0,M0;

	if(this == &B)
		return *this;
	B.Dim(&N0,&M0);
	ChangeDim(N0,M0);
	for (int i=0;i<N0;i++)
		for (int j=0;j<M0;j++)
			A[i][j]=B.GetElement(i,j);

	return *this;
}
//---------------------------------------------------------------------------
void TMatrix::Transpose()
{
//	TMatrix C(M,N);
	double **B;
	B=CloneArray(A);
	ChangeDim(M,N);

	for (int i=0;i<N;i++)
		for (int j=0;j<M;j++)
			A[i][j]=B[j][i];
			//C.SetElement(i,j,A[j][i]);

	DeleteArray(B,M,N);
 //	return C;
}
//---------------------------------------------------------------------------
void TMatrix::Decomposite_LU()
{

	int i,j;
	int p;
	double s,x,*T;
	int L=(N>M)?N:M;
	T=new double[L];
	
	for (int i=0;i<L;i++)
		T[i]=0;

/*	L=CreateArray();
	U=CreateArray();*/
	X=CloneArray(A);

	j=0;

	while (j<K){
		p=j;
		i=j+1;
		while (i<M){
			if (abs(X[i][j])>abs(X[p][j]))
				p=i;
			i++;
		}
		Piv[j]=p;
		if (X[p][j]!=0){
			if (p!=j){
				for (int k=0;k<N;k++)
					T[k]=X[j][k];
				for (int k=0;k<N;k++)
					X[j][k]=X[p][k];;
				for (int k=0;k<N;k++)
					X[p][k]=T[k];
			   /*	for (int k=0;k<N;k++){
					X[j][k]=A[p][k];
					X[p][k]=A[j][k];
				}  */
			}
			if (j<M-1){
				p=j+1;
				s=1/X[j][j];
				for (int k=p;k<M;k++)
					X[k][j]*=s;
			}
		}
		if (j<K){
			p=j+1;
			i=j+1;
			while (i<M){
				s=X[i][j];
				for (int k=p;k<N;k++)
					X[i][k]-=s*X[j][k];
				i++;
			}
		}
		j++;
	}

/*
	i=0;
	while (i<M){
		j=0;
		while (j<K){
			if (i==j)
				L[i][j]=1;
			else if (j<i)
				L[i][j]= X[i][j];
			j++;
		}
		i++;
	}
	i=0;
	while (i<K){
		j=1;
		while (i<N){
			if (j>=i)
				U[i][j]= X[i][j];
			j++;
		}
		i++;
	} */
	delete[] T;

}
//---------------------------------------------------------------------------
void TMatrix::InverseUpper()
{
	double *T;
	double x,v;
	int i,jm;
	T=new double[N];
	for (int i=0;i<N;i++)
		T[i]=0;

	int j=0;
	while (j<N){
		if (X[j][j]!=0){
			X[j][j]=1/X[j][j];
			x=-X[j][j];
		}
		if (j>0){
			jm=j-1;
			for (int k=0;k<=jm;k++)
				T[k]=X[k][j];
			i=0;
			while (i<=j-1){
				if (i<j-1){
					v=0;
					for (int k=i+1;k<=jm+1;k++)
						v+=X[i][k]*T[k];
				}else
					v=0;
				X[i][j]=v+X[i][i]*T[i];
				i++;
			}
			for (int k=0;k<jm+1;k++)
				X[k][j]=x*X[k][j];
		}
		j++;
	}

	delete[] T;
}
//---------------------------------------------------------------------------
void TMatrix::InverseLower()
{
	double *T;
	double x,v;
	int i,jp,jn;
	T=new double[N];
	for (int i=0;i<N;i++)
		T[i]=0;

	int j=N-1;

	while (j>=0){
		if (X[j][j]!=0){
			X[j][j]=1/X[j][j];
			x=-X[j][j];
		}
		if (j<N-1){
			jn=N-j-1;
			jp=j+1;
			for (int k=jp;k<N;k++)
				T[k]=X[k][j];
			i=j+1;
			while (i<N){
				if (i>j+1){
					v=0;
					for (int k=jp;k<i;k++)
						v+=X[i][k]*T[k];
				}else
					v=0;
				X[i][j]=v+X[i][i]*T[i];
				i++;
				for (int k=jp;k<N;k++)
					X[k][j]=x*X[k][j];
			}
            j--;
		}
	}

	delete[] T;
}
//---------------------------------------------------------------------------
void TMatrix::Inverse_LU()
{
	double *w;
	double v;
	int i,j,jp;

	if (IsSquare()){
		w=new double[N];
		for (int i=0;i<N;i++)
			w[i]=0;

		InverseUpper();

		j=N-1;
		while (j>=0){
			i=j+1;
			while (i<N){
				w[i]=X[i][j];
				X[i][j]=0;
				i++;
			}
			if (j<N){
				jp=j+1;
				i=0;
				while (i<N){
					v=0;
					for (int k=jp;k<N;k++)
						v+=X[i][k]*w[k];
					X[i][j]-=v;
					i++;
				}
			}
			j--;
		}
		j=N-2;
		while (j>=0){
			jp=Piv[j];
			if (jp!=j){
				for (int k=0;k<N;k++)
					w[k]=X[k][j];
				for (int k=0;k<N;k++)
					X[k][j]=X[k][jp];
				for (int k=0;k<N;k++)
					X[k][jp]=w[k];
			}
			j--;
		}
		delete[] w;
	}

}
//---------------------------------------------------------------------------
void TMatrix::Inverse()
{
	double x;
	if (IsSquare()){
		Decomposite_LU();
		Inverse_LU();
		CopyArray(A,X);
		DeleteArray(X);
	}
}
//---------------------------------------------------------------------------
bool TMatrix::IsSquare()
{
	return (N==M);
}
//---------------------------------------------------------------------------
bool TMatrix::IsDiagonal()
{
	double x=0;
	bool res;

	if (IsSquare()){
		for (int i=0;i<N;i++){
			for (int j=0;j<M;j++){
				if (i!=j)
					x+=A[i][j];
			}
		}
		res=(abs(x)<1e-10)?true:false;
	}else
		res=false;

 	return res;
}
//---------------------------------------------------------------------------
void TMatrix::Ones()
{
	for(int i=0;i<K;i++)
    	A[i][i]=1;
}
//---------------------------------------------------------------------------
void TMatrix::Zeros()
{
	for (int i=0;i<N;i++)
		for(int j=0;j<M;j++)
			A[i][j]=0;
}

//---------------------------------------------------------------------------

#pragma package(smart_init)
