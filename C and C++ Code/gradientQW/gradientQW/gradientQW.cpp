/*******************************************************************************
* 此文件为不使用SEE加速的版本，未删去原始版本的if分支。运行过程会崩溃，可能缺少dll文件。
* 运行结果与matlab结果正确
* Autor:Tornado
* 2017-7-18
*******************************************************************************/
#include <malloc.h>
#include "wrappers.hpp"
#include <math.h>
#include "string.h"



#define PI 3.14159265f
//#define N_h 12
//#define N_w 16
//#define BINSIZE  4 
//#define NORIENTS  9
//#define SOFTBIN  -1


//float H[(N_h / BINSIZE)*(N_w / BINSIZE) * 32] = { 0 }, M[N_h * N_w] = { 0 }, O[N_h * N_w] = { 0 }, H_out[32][N_w / BINSIZE][N_h / BINSIZE] = { 0 };

float* acosTable(void);
void gradMag(float *I, float *M, float *O, int h, int w, int d, bool full);
void gradMagNorm(float *M, float *S, int h, int w, float norm);
void gradHist(float *M, float *O, float *H, int h, int w, int bin, int nOrients, int softBin, bool full);
void hog(float *M, float *O, float *H, int h, int w, int binSize, int nOrients, int softBin, float clip);
void fhog(float *M, float *O, float *H, int h, int w, int binSize, int nOrients, int softBin, float clip);


/****************************************************************************/

//仅计算一列的x和y梯度
void grad1( float *I, float *Gx, float *Gy, int h, int w, int x ) { 
  int y, y1; float *Ip, *In, r; 
  // compute column of Gx
  Ip=I-h; In=I+h; r=.5f;
  if(x==0) { r=1; Ip+=h; } else if(x==w-1) { r=1; In-=h; }
  if( h<4 || h%4>0 || (size_t(I)&15) || (size_t(Gx)&15) ) 
  {
    for( y=0; y<h; y++ ) *Gx++=(*In++-*Ip++)*r;//Gx=G(x+1)-G(x-1)
  }
  else 
  {  
    for(y=0; y<h; y++) Gx[y]=(In[y]-Ip[y])*r;
  };

  // compute column of Gy b
  #define GRADY(r) *Gy++=(*In++-*Ip++)*r;
  Ip=I; In=Ip+1;

  // GRADY(1); Ip--; for(y=1; y<h-1; y++) GRADY(.5f); In--; GRADY(1);
 /* y1=((~((size_t) Gy) + 1) & 15)/4;
  if(y1==0) y1=4; 
  if(y1>h-1) y1=h-1;
  GRADY(1); Ip--; for(y=1; y<y1; y++) GRADY(.5f);*/

  y1=h-1;
  GRADY(1); Ip--; for(y=1; y<y1; y++) GRADY(.5f);
  for(; y+4<h-1; y++) 
	  Gy[y]=(In[y]-Ip[y])*0.5f;

  for(; y<h-1; y++) GRADY(.5f); In--; GRADY(1);
#undef GRADY
}

// compute x and y gradients at each location (uses sse)在每个位置计算x和y梯度
void grad2(float *I, float *Gx, float *Gy, int h, int w, int d) {
	int o, x, c, a = w*h; for (c = 0; c<d; c++) for (x = 0; x<w; x++) {
		o = c*a + x*h; grad1(I + o, Gx + o, Gy + o, h, w, x);
	}
}

// build lookup table a[] s.t. a[x*n]~=acos(x) for x in [-1,1]
float* acosTable() {
  const int n=10000, b=10; int i;
  static float a[10000*2+10*2];
  static bool init=false;
  float *a1=a+n+b; if( init ) return a1;
  for( i=-n-b; i<-n; i++ )   a1[i]=PI;
  for( i=-n; i<n; i++ )      a1[i]=float(acos(i/float(n)));
  for( i=n; i<n+b; i++ )     a1[i]=0;
  for( i=-n-b; i<n/10; i++ ) if( a1[i] > PI-1e-6f ) a1[i]=PI-1e-6f;
  init=true; return a1;
}
// compute gradient magnitude and orientation at each location (uses sse)在每个位置计算梯度的大小和方向
void gradMag(float *I, float *M, float *O, int h, int w, int d, bool full) {
	int x, y, y1, c, h4 ;
  float *acost = acosTable(), acMult=10000.0f;
  // allocate memory for storing one column of output (padded so h4%4==0)分配内存来存储一列的输出
  h4=(h%4==0) ? h : h-(h%4)+4; //h4如果不是4的整数倍就补足，添加到4的整数倍
  
	//float M2[N_h] = { 0 }, Gx[N_h] = { 0 }, Gy[N_h] = { 0 };//此处有修改，将动态分配变成了静态
	int s=d*h4*sizeof(float);//此处有修改，注销了此句
	float *Gx, *Gy, *M2; 
	M2 = (float*)alMalloc(s, 16);
	Gx = (float*)alMalloc(s, 16);
	Gy = (float*)alMalloc(s, 16);

  // 计算每个列的梯度大小和方向
  for( x=0; x<w; x++ ) 
  {
    //计算梯度（GX、Gy）的最大幅值的平方（M2），本循环中也计算了梯度方向（O）
    for(c=0; c<d; c++) 
	{
      grad1( I+x*h+c*w*h, Gx+c*h4, Gy+c*h4, h, w, x );

	  for( y=0; y<h4; y++ ) 
	  {
        y1=h4*c+y;	
        M2[y1]=(Gx[y1]*Gx[y1]+Gy[y1]*Gy[y1]);//求Gx和Gy的平方和 M2

        if( c==0 ) continue; 
		M2[y] = M2[y1] > M2[y] ? M2[y1] : M2[y];
		Gx[y] = M2[y1] > M2[y] ? Gx[y1] : Gx[y];
		Gy[y] = M2[y1] > M2[y] ? Gy[y1] : Gy[y];//取较大的值
	  };
	};

    //计算梯度幅值M和范数Gx
	  for( y=0; y<h4; y++ ) //----因为这段代码的版本所以cce存在小的差异
	  { 
		   float m =  1.0f/sqrt((float)M2[y]);
		   m = m < 1e10f ? m : 1e10f;  
		   M2[y] = 1.0f/m; //此处M2内容变成了sqrt（Gx2+Gy2）
		   /*----------------
			float m = 1.0f/sqrt((float)M2[y]);
			if (m < 1e10f)
				M2[y] = (float)sqrt((float)M2[y]);
			else {
				M2[y] = 1e-10f;
				m = 1e10f;
			};----------------*/

		   if(O) Gx[y] = (Gx[y]*m)*acMult; //acMult=10000.0f
	 
		   if(O) {
			   //bitwise AND on floats按位与
			   float zero = -0.f; 
			   unsigned char *pGy = 0;
			   unsigned char *pZero = 0;
			   unsigned char *pGand = 0;
			   float Gand = 0.f;	  
			   pGy   = reinterpret_cast<unsigned char *>(&Gy[y]);
			   pZero = reinterpret_cast<unsigned char *>(&zero);
			   pGand = reinterpret_cast<unsigned char *>(&Gand);
			   for (int i=0; i<4; i++)
			   {
				   *pGand = (*pGy & *pZero);
				   pGand++;
				   pGy++;
				   pZero++;
			   };

			   //bitwise XOR on floats按位异或
			   unsigned char *pGx = 0;
			   unsigned char *pGxor = 0;
			   float Gxor = 0;
			   pGx   = reinterpret_cast<unsigned char *>(&Gx[y]);
			   pGand = reinterpret_cast<unsigned char *>(&Gand);
			   pGxor = reinterpret_cast<unsigned char *>(&Gxor);
			   for (int i=0; i<4; i++)
			   {
				   *pGxor = (*pGx ^ *pGand);
				   pGxor++;
				   pGx++;
				   pGand++;
			   };
			   Gx[y] = Gxor;
		   };
    };

    memcpy( M+x*h, M2, h*sizeof(float) );//将M2的值复制到 M 中
    //计算和存储梯度方向（O）通过查找表
    if( O!=0 ) for( y=0; y<h; y++ ) 
		O[x*h+y] = acost[(int)Gx[y]];//O存储的是归一化后的梯度方向
    if( O!=0 && full ) 
	{
      y1=((~size_t(O+x*h)+1)&15)/4; y=0;
      for( ; y<y1; y++ ) O[y+x*h]+=(Gy[y]<0)*PI; //如果Gy小于0则将O相应位置方向值+180度，否则不作变换

	  for( ; y<h-4; y++ )  
		  O[y+x*h]+=(Gy[y]<0)*PI;
		 // if (Gy[y] < 0) {
		 //	  O[y+x*h] += PI;
		 // }
		  //else O[y+x*h]++;
		  //else O[y+x*h]++;
      for( ; y<h; y++) O[y+x*h]+=(Gy[y]<0)*PI;//此处不懂：y为何要从0~y1~h-4~h分成三段来判断
	};
  };
}


// 在每个位置归一化梯度幅度
void gradMagNorm(float *M, float *S, int h, int w, float norm) {

	int i=0, n=h*w;
	int n4=n/4;

	for(; i<n; i++) M[i] /= (S[i] + norm); //i*=4; 

	for(; i<n; i++) M[i] /= (S[i] + norm);
}

// 辅助，量化O和M到O0、O1和M0、M1
void gradQuantize( float *O, float *M, int *O0, int *O1, float *M0, float *M1,
				  int nb, int n, float norm, int nOrients, bool full, bool interpolate )//整个文件里full参数代表方向bin数，如果为1则取0-2PI，0则取0-PI
{//	s=(float)bin, hb=h/bin, wb=w/bin, h0=hb*bin, w0=wb*bin, nb=wb*hb;   
	//norm=1/s/s; n=h0; interpolate=false
	//假设所有*输出*矩阵是4字节对齐
	int i,o0,o1; float  od, m,o;
	//float o[4] = { 0 };int o0[4] = { 0 }, o1[4] = { 0 };

	const float oMult=(float)nOrients/(full?2*PI:PI); const int oMax=nOrients*nb;//oMult=9/2PI或9/PI

	if( interpolate ) //本程序中为false
		for( i=0; i<=n-4; i+=1 ) 
		{
			o = O[i] * oMult; o0=(int) o; od= o - (float)o0;
			o0 *= nb; o0 = (oMax > o0) ? o0 : 0; O0[i]=o0;
			o1=o0 + nb; o1 = (oMax > o1) ? o1 : 0; O1[i]=o1;
			m=M[i]*norm; M1[i]= od * m; M0[i]= m - M1[i]; 
		}
	else 
		for( i=0; i<=n-4; i+=1 ) 
		{
			//修改
			//o[0] = O[i] * oMult; o[1] = O[i+1] * oMult; o[2] = O[i+2] * oMult; o[3] = O[i+3] * oMult
			//o0[0] = (int)(o[0] + 0.5f); o0[1] = (int)(o[1] + 0.5f); o0[2] = (int)(o[2] + 0.5f); o0[3] = (int)(o[3] + 0.5f);
			//
			o=O[i]*oMult;    //oMult = 9/2PI
			o0=(int)(o+0.5f); 
			o0 *= nb;
			o0 = (oMax > o0) ? o0 : 0; 
			O0[i]=o0; 
			M0[i]=M[i]*norm; 	
			M1[i]=0.0f; O1[i]=0;
		}  
		// 不使用sse计算跟踪位置
	//interpolate为false，因为softBin>=0为假//oftBin >= 0使用临近两个bin线性插值，softBin<0则取最近的bin（本程序中为 - 1）
		if( interpolate ) 
			for( i; i<n; i++ ) 
			{		
				o=O[i]*oMult; 
				o0=(int) o; 
				od=o-o0;    //od幅值量化后误差
				o0*=nb; 
				if(o0>=oMax) o0=0; 
				O0[i]=o0;          //O = wb*hb*(取整(O*9/2PI))
				o1=o0+nb; 
				if(o1==oMax) 
					o1=0; 
				O1[i]=o1;             //O1 = wb*hb*(取整(O*9/2PI))+wb*hb
				m=M[i]*norm; 
				M1[i]=od*m; M0[i]=m-M1[i]; //M1为梯度幅度M乘以权值后再乘以od（norm*M*od），M0=norm*M-（norm*M*od）。
			} 
		else 
			for( i; i<n; i++ ) 
			{                 
				o=O[i]*oMult;				//oMult = 9/2PI
				o0=(int) (o+.5f);
				o0*=nb; 
				if(o0>=oMax) 
					o0=0; 
				O0[i]=o0;				//O = wb*hb*((O*9/2PI)+0.5)
				M0[i]=M[i]*norm; //M0为梯度幅度M乘以权值后
				M1[i]=0; O1[i]=0; //M1和O1都是0
			}
}

// 计算nOrients个梯度直方图每个bin x bin块像素
void gradHist(float *M, float *O, float *H, int h, int w,
  int bin, int nOrients, int softBin, bool full )
  //H维度[wb*hb*9*3];bin=4;nOrients=2*9;softBin=-1;full=1

  //softBin表示选择梯度幅值加权形式，softBin>=0使用临近两个bin线性插值，softBin<0则取最近的bin（本程序中为-1）。
  //在每个w*h的方向通道的每一binsize * binsize大小的像素，决定了空间的分级。如果“softbin”是奇数像素
  //可以影响多个bin（使用双线性插值），否则每个像素只对一个方向bin有贡献。 经过这一步过后的结果为
  //floor([h / binSize w / binSize nOrients])维大小的特征图，代表了每个图层区域中的梯度直方图。
{
  const int hb=h/bin, wb=w/bin, h0=hb*bin, w0=wb*bin, nb=wb*hb;
  const float s=(float)bin, sInv=1/s, sInv2=1/s/s;
  float *H0, *H1; int x, y; float  xb, init;
  
  //int   O0[N_h] = { 0 }, O1[N_h] = { 0 };
  //float M0[N_h] = { 0 }, M1[N_h] = { 0 };//此处有修改，将动态分配变为固定值
  float *M0, *M1; int *O0, *O1;
  O0 = (int*)alMalloc(h*sizeof(int), 16); M0 = (float*)alMalloc(h*sizeof(float), 16);
  O1 = (int*)alMalloc(h*sizeof(int), 16); M1 = (float*)alMalloc(h*sizeof(float), 16);

  // 主循环
  for( x=0; x<w0; x++ ) // 在整个列计算目标方向bin--非常快
  {
    gradQuantize(O+x*h,M+x*h,O0,O1,M0,M1,nb,h0,sInv2,nOrients,full,softBin>=0);//量化O和M

    if( softBin<0 && softBin%2==0 )//表明softBin为负偶数，不采用插值 
	{
      H1=H+(x/bin)*hb;
      #define GH H1[O0[y]]+=M0[y]; y++;
      if( bin==1 )      for(y=0; y<h0;) { GH; H1++; }
      else if( bin==2 ) for(y=0; y<h0;) { GH; GH; H1++; }
      else if( bin==3 ) for(y=0; y<h0;) { GH; GH; GH; H1++; }
      else if( bin==4 ) for(y=0; y<h0;) { GH; GH; GH; GH; H1++; }
      else for( y=0; y<h0;) { for( int y1=0; y1<bin; y1++ ) { GH; } H1++; }
      #undef GH

    } 
	else if( softBin%2==0 || bin==1 )//n 表明softBin为大于零偶数，采用线性插值 
	{ 
      H1=H+(x/bin)*hb;
      #define GH H1[O0[y]]+=M0[y]; H1[O1[y]]+=M1[y]; y++;
      if( bin==1 )      for(y=0; y<h0;) { GH; H1++; }
      else if( bin==2 ) for(y=0; y<h0;) { GH; GH; H1++; }
      else if( bin==3 ) for(y=0; y<h0;) { GH; GH; GH; H1++; }
      else if( bin==4 ) for(y=0; y<h0;) { GH; GH; GH; GH; H1++; }
      else for( y=0; y<h0;) { for( int y1=0; y1<bin; y1++ ) { GH; } H1++; }
      #undef GH
	}
	else //采用三线性插值插值 //表明softBin为奇数
	{ 
		float ms[4], xyd, yb, xd, yd;
		bool hasLf, hasRt; int xb0, yb0;
		if (x == 0) { init = (0 + .5f)*sInv - 0.5f; xb = init; }   //初始化xb = init = 0.5*(1/4)-0.5=-0.375
		hasLf = xb >= 0; xb0 = hasLf ? (int)xb : -1; hasRt = xb0 < wb - 1;//hasLf=0，xb0=-1；hasRt=1；
		xd = xb - xb0; xb += sInv; yb = init; y = 0;//xd=xb+1=0.625;xb=-0.125;yb=-0.375;y=0;
		// 为了代码简洁而设置的宏
		#define GHinit yd=yb-yb0; yb+=sInv; H0=H+xb0*hb+yb0; xyd=xd*yd; \
			ms[0]=1-xd-yd+xyd; ms[1]=yd-xyd; ms[2]=xd-xyd; ms[3]=xyd;
		
		// 首先的一行，没有顶bin
		for( ; y<bin/2; y++ ) 
		{
			yb0=-1; GHinit;
			if(hasLf) { H0[O0[y]+1]+=ms[1]*M0[y]; H0[O1[y]+1]+=ms[1]*M1[y]; }
			if(hasRt) { H0[O0[y]+hb+1]+=ms[3]*M0[y]; H0[O1[y]+hb+1]+=ms[3]*M1[y]; }
		}

		//主行，有顶部和底部的bin，使用SSE的小加速
		if( softBin<0 )//此if语句中修改了 *H1 += ms[0]*M0[y]四句语句加权相加的顺序，其他分支还未修改
			for( ; ; y++ ) 
			{
				yb0 = (int) yb; if(yb0>=hb-1) break; GHinit; 
				if(hasLf) 
				{ 
					H1 = H0+O0[y]; *H1 += ms[0]*M0[y]; *(H1+1) += ms[1]*M0[y]; 
					*(H1+2) += 0*M0[y]; *(H1+3) += 0*M0[y];	
				}
				if(hasRt) 
				{ 
					H1 = H0+O0[y]+hb; *H1 += ms[2]*M0[y]; *(H1+1) += ms[3]*M0[y];
					*(H1+2) += 0*M0[y]; 
					*(H1+3) += 0*M0[y];							
				}
			} 
		else 
			for( ; ; y++ ) 
			{
				yb0 = (int) yb; if(yb0>=hb-1) break; GHinit;
				if(hasLf) 
				{ 
					H1 = H0+O0[y]; *H1 += 0*M0[y]; *(H1+1) += 0*M0[y];
					*(H1+2) += ms[1]*M0[y]; *(H1+3) += ms[0]*M0[y];	
					H1 = H0+O1[y]; *H1 += 0*M1[y]; *(H1+1) += 0*M1[y];
					*(H1+2) += ms[1]*M1[y]; *(H1+3) += ms[0]*M1[y];		 		
				}
				if(hasRt) 
				{ 
					H1 = H0+O0[y]+hb; *H1 += 0*M0[y]; *(H1+1) += 0*M0[y];
					*(H1+2) += ms[3]*M0[y]; *(H1+3) += ms[2]*M0[y];	
					H1 = H0+O1[y]+hb; *H1 += 0*M1[y]; *(H1+1) += 0*M1[y];
					*(H1+2) += ms[3]*M1[y]; *(H1+3) += ms[2]*M1[y];		
				}
			}

		//最后一行，没有底部bin
		for( ; y<h0; y++ ) 
		{
			yb0 = (int) yb; GHinit;
			if(hasLf) { H0[O0[y]]+=ms[0]*M0[y]; H0[O1[y]]+=ms[0]*M1[y]; }
			if(hasRt) { H0[O0[y]+hb]+=ms[2]*M0[y]; H0[O1[y]+hb]+=ms[2]*M1[y]; }
		}
		#undef GHinit
		#undef GH
    }				//三线插值else的末尾
  }					//主循环for的末尾

  // 归一化边界bin，只有内部bin的7/8权重
  if( softBin%2!=0 ) for( int o=0; o<nOrients; o++ ) 
  {
    x=0; for( y=0; y<hb; y++ ) H[o*nb+x*hb+y]*=8.f/7.f;
    y=0; for( x=0; x<wb; x++ ) H[o*nb+x*hb+y]*=8.f/7.f;
    x=wb-1; for( y=0; y<hb; y++ ) H[o*nb+x*hb+y]*=8.f/7.f;
    y=hb-1; for( x=0; x<wb; x++ ) H[o*nb+x*hb+y]*=8.f/7.f;
  }
}

/******************************************************************************/

// HOG helper: compute 2x2 block normalization values (padded by 1 pixel)HOG帮手：计算2x2块归一化值（用1像素）
float* hogNormMatrix(float *H, int nOrients, int hb, int wb, int bin) {
	float *N1, *n; int o, x, y, dx, dy, hb1 = hb + 1, wb1 = wb + 1;
	float eps = 1e-4f / 4 / bin / bin / bin / bin; // 精确的反向等式
	
	float *N;
	N = (float*)wrCalloc(hb1*wb1, sizeof(float));//此处有修改，去掉了动态分配

	N1 = N + hb1 + 1;
	for (o = 0; o<nOrients; o++) for (x = 0; x<wb; x++) for (y = 0; y<hb; y++)
		N1[x*hb1 + y] += H[o*wb*hb + x*hb + y] * H[o*wb*hb + x*hb + y];
	for (x = 0; x<wb - 1; x++) for (y = 0; y<hb - 1; y++) {
		n = N1 + x*hb1 + y; *n = 1 / float(sqrt(n[0] + n[1] + n[hb1] + n[hb1 + 1] + eps));
	}
	x = 0;     dx = 1; dy = 1; y = 0;						N[x*hb1 + y] = N[(x + dx)*hb1 + y + dy];
	x = 0;     dx = 1; dy = 0; for (y = 0; y<hb1; y++)		N[x*hb1 + y] = N[(x + dx)*hb1 + y + dy];
	x = 0;     dx = 1; dy = -1; y = hb1 - 1;				N[x*hb1 + y] = N[(x + dx)*hb1 + y + dy];
	x = wb1 - 1; dx = -1; dy = 1; y = 0;					N[x*hb1 + y] = N[(x + dx)*hb1 + y + dy];
	x = wb1 - 1; dx = -1; dy = 0; for (y = 0; y<hb1; y++)	N[x*hb1 + y] = N[(x + dx)*hb1 + y + dy];
	x = wb1 - 1; dx = -1; dy = -1; y = hb1 - 1;             N[x*hb1 + y] = N[(x + dx)*hb1 + y + dy];
	y = 0;     dx = 0; dy = 1; for (x = 0; x<wb1; x++)		N[x*hb1 + y] = N[(x + dx)*hb1 + y + dy];
	y = hb1 - 1; dx = 0; dy = -1; for (x = 0; x<wb1; x++)	N[x*hb1 + y] = N[(x + dx)*hb1 + y + dy];
	return N;
}

// HOG helper: compute HOG or FHOG channels HOG辅助：计算HOG或FHOG的多个通道
void hogChannels(float *H, const float *R, const float *N,
	int hb, int wb, int nOrients, float clip, int type)
{
#define GETT(blk) t=R1[y]*N1[y-(blk)]; if(t>clip) t=clip; c++;
	const float r = .2357f; int o, x, y, c; float t;
	const int nb = wb*hb, nbo = nOrients*nb, hb1 = hb + 1;
	for (o = 0; o<nOrients; o++) for (x = 0; x<wb; x++) {
		const float *R1 = R + o*nb + x*hb, *N1 = N + x*hb1 + hb1 + 1;
		float *H1 = (type <= 1) ? (H + o*nb + x*hb) : (H + x*hb);
		if (type == 0) for (y = 0; y<hb; y++) {
			// store each orientation and normalization (nOrients*4 channels) 存储每个方向与归一化（nOrients×4通道）
			c = -1; GETT(0); H1[c*nbo + y] = t; GETT(1); H1[c*nbo + y] = t;
			GETT(hb1); H1[c*nbo + y] = t; GETT(hb1 + 1); H1[c*nbo + y] = t;
		}
		else if (type == 1) for (y = 0; y<hb; y++) {
			// sum across all normalizations (nOrients channels) 求所有的归一化值得和（nOrients通道）
			c = -1; GETT(0); H1[y] += t*.5f; GETT(1); H1[y] += t*.5f;
			GETT(hb1); H1[y] += t*.5f; GETT(hb1 + 1); H1[y] += t*.5f;
		}
		else if (type == 2) for (y = 0; y<hb; y++) {
			// sum across all orientations (4 channels) 求所有方向的总和（4个通道）
			c = -1; GETT(0); H1[c*nb + y] += t*r; GETT(1); H1[c*nb + y] += t*r;
			GETT(hb1); H1[c*nb + y] += t*r; GETT(hb1 + 1); H1[c*nb + y] += t*r;
		}
	}
#undef GETT
}

// compute HOG features
void hog(float *M, float *O, float *H, int h, int w, int binSize,
	int nOrients, int softBin, bool full, float clip)
{
	//float R[(N_h / BINSIZE)*(N_w / BINSIZE) * NORIENTS] = { 0 };
	//float N[(N_h / BINSIZE + 1)*(N_w / BINSIZE + 1)] = { 0 };
	float *N, *R;
	const int hb = h / binSize, wb = w / binSize, nb = hb*wb;
	R = (float*)wrCalloc(wb*hb*nOrients, sizeof(float));
	
	//compute unnormalized gradient histograms 计算非归一化的梯度直方图
	gradHist(M, O, R, h, w, binSize, nOrients, softBin, full);
	// compute block normalization values 计算块归一化值
	N = hogNormMatrix( R, nOrients, hb, wb, binSize);
	// perform four normalizations per spatial block 每个空间块执行四次归一化
	hogChannels(H, R, N, hb, wb, nOrients, clip, 0);
}

// compute FHOG features
void fhog(float *M, float *O, float *H, int h, int w, int binSize,
	int nOrients, int softBin, float clip)
{
	const int hb = h / binSize, wb = w / binSize, nb = hb*wb, nbo = nb*nOrients;
	int o, x;

	float *N, *R1, *R2;
	R1 = (float*)wrCalloc(wb*hb*nOrients * 2, sizeof(float));
	R2 = (float*)wrCalloc(wb*hb*nOrients, sizeof(float));
	//float R1[(N_h / BINSIZE)*(N_w / BINSIZE) * NORIENTS * 2] = { 0 };
	//float R2[(N_h / BINSIZE)*(N_w / BINSIZE) * NORIENTS] = { 0 };
	//float N[(N_h / BINSIZE + 1)*(N_w / BINSIZE + 1)] = { 0 };//此处有修改，去掉了动态内存分配
	
	gradHist(M, O, R1, h, w, binSize, nOrients * 2, softBin, true);// 计算非归一化对比度敏感的直方图
	// 计算非归一化对比度不敏感的直方图

	for (o = 0; o < nOrients; o++)
	{
		for (x = 0; x<nb; x++)
			R2[o*nb + x] = R1[o*nb + x] + R1[(o + nOrients)*nb + x];
	}	
	// 计算块归一化值
	N = hogNormMatrix(R2, nOrients, hb, wb, binSize);
	//N = hogNormMatrix(R2, nOrients, hb, wb, binSize);
	// 归一化直方图和纹理通道
	hogChannels(H + nbo * 0, R1, N, hb, wb, nOrients * 2, clip, 1);
	hogChannels(H + nbo * 2, R2, N, hb, wb, nOrients * 1, clip, 1);
	hogChannels(H + nbo * 3, R1, N, hb, wb, nOrients * 2, clip, 2);
	
}
///******************************************************************************/

#ifdef MATLAB_MEX_FILE
// Create [hxwxd] mxArray array, initialize to 0 if c=true
mxArray* mxCreateMatrix3(int h, int w, int d, mxClassID id, bool c, void **I){
	const int dims[3] = { h, w, d }, n = h*w*d; int b; mxArray* M;
	if (id == mxINT32_CLASS) b = sizeof(int);
	else if (id == mxDOUBLE_CLASS) b = sizeof(double);
	else if (id == mxSINGLE_CLASS) b = sizeof(float);
	else mexErrMsgTxt("Unknown mxClassID.");
	*I = c ? mxCalloc(n, b) : mxMalloc(n*b);
	M = mxCreateNumericMatrix(0, 0, id, mxREAL);
	mxSetData(M, *I); mxSetDimensions(M, dims, 3); return M;
}

// Check inputs and outputs to mex, retrieve first input I
void checkArgs(int nl, mxArray *pl[], int nr, const mxArray *pr[], int nl0,
	int nl1, int nr0, int nr1, int *h, int *w, int *d, mxClassID id, void **I)
{
	const int *dims; int nDims;
	if (nl<nl0 || nl>nl1) mexErrMsgTxt("Incorrect number of outputs.");
	if (nr<nr0 || nr>nr1) mexErrMsgTxt("Incorrect number of inputs.");
	nDims = mxGetNumberOfDimensions(pr[0]); dims = mxGetDimensions(pr[0]);
	*h = dims[0]; *w = dims[1]; *d = (nDims == 2) ? 1 : dims[2]; *I = mxGetPr(pr[0]);
	if (nDims != 2 && nDims != 3) mexErrMsgTxt("I must be a 2D or 3D array.");
	if (mxGetClassID(pr[0]) != id) mexErrMsgTxt("I has incorrect type.");
}

// [Gx,Gy] = grad2(I) - see gradient2.m
void mGrad2(int nl, mxArray *pl[], int nr, const mxArray *pr[]) {
	int h, w, d; float *I, *Gx, *Gy;
	checkArgs(nl, pl, nr, pr, 1, 2, 1, 1, &h, &w, &d, mxSINGLE_CLASS, (void**)&I);
	if (h<2 || w<2) mexErrMsgTxt("I must be at least 2x2.");
	pl[0] = mxCreateMatrix3(h, w, d, mxSINGLE_CLASS, 0, (void**)&Gx);
	pl[1] = mxCreateMatrix3(h, w, d, mxSINGLE_CLASS, 0, (void**)&Gy);
	grad2(I, Gx, Gy, h, w, d);
}

// [M,O] = gradMag( I, channel, full ) - see gradientMag.m
void mGradMag(int nl, mxArray *pl[], int nr, const mxArray *pr[]) {
	int h, w, d, c, full; float *I, *M, *O = 0;
	checkArgs(nl, pl, nr, pr, 1, 2, 3, 3, &h, &w, &d, mxSINGLE_CLASS, (void**)&I);
	if (h<2 || w<2) mexErrMsgTxt("I must be at least 2x2.");
	c = (int)mxGetScalar(pr[1]); full = (int)mxGetScalar(pr[2]);
	if (c>0 && c <= d) { I += h*w*(c - 1); d = 1; }
	pl[0] = mxCreateMatrix3(h, w, 1, mxSINGLE_CLASS, 0, (void**)&M);
	if (nl >= 2) pl[1] = mxCreateMatrix3(h, w, 1, mxSINGLE_CLASS, 0, (void**)&O);
	gradMag(I, M, O, h, w, d, full>0);
}

// gradMagNorm( M, S, norm ) - operates on M - see gradientMag.m
void mGradMagNorm(int nl, mxArray *pl[], int nr, const mxArray *pr[]) {
	int h, w, d; float *M, *S, norm;
	checkArgs(nl, pl, nr, pr, 0, 0, 3, 3, &h, &w, &d, mxSINGLE_CLASS, (void**)&M);
	if (mxGetM(pr[1]) != h || mxGetN(pr[1]) != w || d != 1 ||
		mxGetClassID(pr[1]) != mxSINGLE_CLASS) mexErrMsgTxt("M or S is bad.");
	S = (float*)mxGetPr(pr[1]); norm = (float)mxGetScalar(pr[2]);
	gradMagNorm(M, S, h, w, norm);
}

// H=gradHist(M,O,[...]) - see gradientHist.m
void mGradHist(int nl, mxArray *pl[], int nr, const mxArray *pr[]) {
	int h, w, d, hb, wb, nChns, binSize, nOrients, softBin, useHog;
	bool full; float *M, *O, *H, clipHog;
	checkArgs(nl, pl, nr, pr, 1, 3, 2, 8, &h, &w, &d, mxSINGLE_CLASS, (void**)&M);
	O = (float*)mxGetPr(pr[1]);
	if (mxGetM(pr[1]) != h || mxGetN(pr[1]) != w || d != 1 ||
		mxGetClassID(pr[1]) != mxSINGLE_CLASS) mexErrMsgTxt("M or O is bad.");
	binSize = (nr >= 3) ? (int)mxGetScalar(pr[2]) : 8;
	nOrients = (nr >= 4) ? (int)mxGetScalar(pr[3]) : 9;
	softBin = (nr >= 5) ? (int)mxGetScalar(pr[4]) : 1;
	useHog = (nr >= 6) ? (int)mxGetScalar(pr[5]) : 0;
	clipHog = (nr >= 7) ? (float)mxGetScalar(pr[6]) : 0.2f;
	full = (nr >= 8) ? (bool)(mxGetScalar(pr[7])>0) : false;
	hb = h / binSize; wb = w / binSize;
	nChns = useHog == 0 ? nOrients : (useHog == 1 ? nOrients * 4 : nOrients * 3 + 5);
	pl[0] = mxCreateMatrix3(hb, wb, nChns, mxSINGLE_CLASS, 1, (void**)&H);
	if (nOrients == 0) return;
	if (useHog == 0) {
		gradHist(M, O, H, h, w, binSize, nOrients, softBin, full);
	}
	else if (useHog == 1) {
		hog(M, O, H, h, w, binSize, nOrients, softBin, full, clipHog);
	}
	else {
		fhog(M, O, H, h, w, binSize, nOrients, softBin, clipHog);
	}
}

// inteface to various gradient functions (see corresponding Matlab functions)
void mexFunction(int nl, mxArray *pl[], int nr, const mxArray *pr[]) {
	int f; char action[1024]; f = mxGetString(pr[0], action, 1024); nr--; pr++;
	if (f) mexErrMsgTxt("Failed to get action.");
	else if (!strcmp(action, "gradient2")) mGrad2(nl, pl, nr, pr);
	else if (!strcmp(action, "gradientMag")) mGradMag(nl, pl, nr, pr);
	else if (!strcmp(action, "gradientMagNorm")) mGradMagNorm(nl, pl, nr, pr);
	else if (!strcmp(action, "gradientHist")) mGradHist(nl, pl, nr, pr);
	else mexErrMsgTxt("Invalid action.");
}
#endif