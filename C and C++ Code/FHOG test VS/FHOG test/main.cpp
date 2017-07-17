// Object tracking algorithm using matchTemplate  


#include <opencv2/opencv.hpp>  
#include "stdlib.h"
#include "malloc.h"
#include "string.h"
#include <math.h>
#include <stddef.h>
#include <fstream> //ͷ�ļ�


#define PI 3.14159265f
#define Nmax 100
#define N_h 12
#define N_w 16

using namespace cv;
using namespace std;

void fhog_test(float *Im, float *H1, float(*H2)[N_w/4][N_h/4], int h, int w);
float* acosTable(void);
void gradientMagnitude(float *I, float *M, float *O, int h, int w, int d, bool full);
void gradMagNormalization(float *M, float *S, int h, int w, float norm);
void gradientHist(float *M, float *O, float *H, int h, int w, int bin, int nOrients, int softBin, bool full);
void hog(float *M, float *O, float *H, int h, int w, int binSize, int nOrients, int softBin,  float clip);
void fhog(float *M, float *O, float *H, int h, int w, int binSize, int nOrients, int softBin, float clip);

float H[(N_h/4)*(N_w/4) * 32] = { 0 }, M[N_h * N_w] = { 0 }, O[N_h * N_w] = { 0 }, H_out[32][N_w/4][N_h/4] = { 0 };

int main(int argc, char * argv[])
{

	Mat src_mat = imread("suv_2.jpg");
	Mat gray_mat;
	//int h = src_mat.rows, w = src_mat.cols;
	int h = N_h, w = N_w;
	cvtColor(src_mat, gray_mat, CV_BGR2GRAY);
	float data[N_h][N_w] = { 0 };
	float data_zhuan[N_h*N_w] = { 0 };
	unsigned char* idata = (unsigned char*)gray_mat.data;
	for (int i = 0; i < h; i++)
		for (int j = 0; j < w; j++)
		{
			data[i][j] = (float)idata[i*w+j];
		}
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
		{
			data_zhuan[i*h+j] = data[j][i];
		}
	//imshow("image", img);
/****************************************************************************/
	gradientMagnitude(data_zhuan, M, O, h, w, 1, true);
	int binSize = 4; int nOrients = 9; int softBin = -1; float clip = 0.2f;
	int hb = h / binSize; int wb = w / binSize; int nChns = nOrients * 3 + 5;
	//hog(M, O, H, h, w, binSize, nOrients, softBin, clip);
	fhog(M, O, H, h, w, binSize, nOrients, softBin, clip);
	int i, j, k;
	//ofstream fout("H_qw.txt");//Ĭ��·���ǹ����ļ���
	for (i = 0; i < nChns; i++)
	{
		for (j = 0; j < wb; j++)
		{
			for (k = 0; k < hb; k++)
			{
				H_out[i][j][k] = H[i*hb*wb + j*hb + k];
				//fout << H2[i][j][k] << endl;
				//memcpy(H_out[k][j][i], H[i*hb*wb + j*hb + k], sizeof(float));
			}
		}
	}
	//fout.close();
	waitKey(0);
	//system("pause");
	return 0;
}

/****************************************************************************/

// compute x and y gradients for just one column ������һ�е�x��y�ݶ�
void grad1(float *I, float *Gx, float *Gy, int h, int w, int x) {
	int y, y1; float *Ip, *In, r;
	// compute column of Gx
	Ip = I - h; In = I + h; r = .5f;
	if (x == 0) { r = 1; Ip += h; }
	else if (x == w - 1) { r = 1; In -= h; }
	if (h<4 || h % 4>0) {													//�˴���Դ����ͬ
		for (y = 0; y<h; y++) *Gx++ = (*In++ - *Ip++)*r;//Gx=G(x+1)-G(x-1)
	}
	else {
		for (y = 0; y<h; y++) Gx[y] = (In[y] - Ip[y])*r;
	}

	// compute column of Gy b
#define GRADY(r) *Gy++=(*In++-*Ip++)*r;
	Ip = I; In = Ip + 1;

	// GRADY(1); Ip--; for(y=1; y<h-1; y++) GRADY(.5f); In--; GRADY(1);
	/* y1=((~((size_t) Gy) + 1) & 15)/4;
	if(y1==0) y1=4;
	if(y1>h-1) y1=h-1;
	GRADY(1); Ip--; for(y=1; y<y1; y++) GRADY(.5f);*/
	y1 = h - 1;
	GRADY(1); Ip--; for (y = 1; y<y1; y++) GRADY(.5f);
	for (; y + 4<h - 1; y++)
		Gy[y] = (In[y] - Ip[y])*0.5f;

	for (; y<h - 1; y++) GRADY(.5f); In--; GRADY(1);
#undef GRADY
}

// compute x and y gradients at each location (uses sse)��ÿ��λ�ü���x��y�ݶ�
void grad2(float *I, float *Gx, float *Gy, int h, int w, int d) {
	int o, x, c, a = w*h; for (c = 0; c<d; c++) for (x = 0; x<w; x++) {
		o = c*a + x*h; grad1(I + o, Gx + o, Gy + o, h, w, x);
	}
}

// build lookup table a[] s.t. a[x*n]~=acos(x) for x in [-1,1]
float* acosTable(void) {
	const int n = 10000, b = 10; int i;
	static float a[10000 * 2 + 10 * 2];
	static bool init = false;
	float *a1 = a + n + b;
	if (init)
		return a1;
	for (i = -n - b; i<-n; i++)
		a1[i] = PI;
	for (i = -n; i<n; i++)
		a1[i] = (float)acos(i / (float)n);
	for (i = n; i<n + b; i++)
		a1[i] = 0;
	for (i = -n - b; i<n / 10; i++)
		if (a1[i] > PI - 1e-6f)
			a1[i] = PI - 1e-6f;
	init = true; return a1;
}
// compute gradient magnitude and orientation at each location (uses sse)��ÿ��λ�ü����ݶȵĴ�С�ͷ���
void gradientMagnitude(float *I, float *M, float *O, int h, int w, int d, bool full) {
	int x, y, y1, c, h4;//s;
	float *acost = acosTable();
	float acMult = 10000.0f;
	// allocate memory for storing one column of output (padded so h4%4==0)�����ڴ����洢һ�е����
	h4 = (h % 4 == 0) ? h : h - (h % 4) + 4; //s = d*h4*sizeof(float);//h4�������4���������Ͳ��㣬��ӵ�4��������
	float M2[N_h] = { 0 }, Gx[N_h] = { 0 }, Gy[N_h] = { 0 };
	/*M2 = (float*)malloc(sizeof(float)*d*h4);Gx = (float*)malloc(sizeof(float)*d*h4);Gy = (float*)malloc(sizeof(float)*d*h4);
	memset(M2, 0, s);memset(Gx, 0, s);memset(Gy, 0, s);*/

	// compute gradient magnitude and orientation for each column ����ÿ���е��ݶȴ�С�ͷ���
	for (x = 0; x<w; x++) {
		// compute gradients (Gx, Gy) with maximum squared magnitude (M2)�����ݶȣ�GX��Gy��������ֵ��ƽ����M2������ѭ����Ҳ�������ݶȷ���O��
		for (c = 0; c<d; c++) {
			grad1(I + x*h + c*w*h, Gx + c*h4, Gy + c*h4, h, w, x);
			for (y = 0; y<h4; y++) {
				y1 = h4*c + y;
				M2[y1] = (Gx[y1] * Gx[y1] + Gy[y1] * Gy[y1]);//��Gx��Gy��ƽ���� M2
				if (c == 0) continue;
				M2[y] = M2[y1] > M2[y] ? M2[y1] : M2[y];
				Gx[y] = M2[y1] > M2[y] ? Gx[y1] : Gx[y];
				Gy[y] = M2[y1] > M2[y] ? Gy[y1] : Gy[y];//ȡ�ϴ��ֵ
			}

		}
		// compute gradient mangitude (M) and normalize Gx �����ݶȷ�ֵM�ͷ���Gx
		for (y = 0; y<h4; y++) {

			//----��Ϊ��δ���İ汾����cce����С�Ĳ���
			float m = 1.0f / sqrt((float)M2[y]);
			m = m < 1e10f ? m : 1e10f;
			M2[y] = 1.0f / m; //�˴�M2���ݱ����sqrt��Gx2+Gy2��
			//----------------
			if (O) Gx[y] = (Gx[y] * m)*acMult; //acMult=10000.0f

			if (O) {
				//bitwise AND on floats��λ��
				float zero = -0.f;
				unsigned char *pGy = 0;
				unsigned char *pZero = 0;
				unsigned char *pGand = 0;
				float Gand = 0.f;
				pGy = reinterpret_cast<unsigned char *>(&Gy[y]);
				pZero = reinterpret_cast<unsigned char *>(&zero);
				pGand = reinterpret_cast<unsigned char *>(&Gand);
				for (int i = 0; i<4; i++){
					*pGand = (*pGy & *pZero);
					pGand++;
					pGy++;
					pZero++;
				};

				//bitwise XOR on floats��λ���
				unsigned char *pGx = 0;
				unsigned char *pGxor = 0;
				float Gxor = 0;
				pGx = reinterpret_cast<unsigned char *>(&Gx[y]);
				pGand = reinterpret_cast<unsigned char *>(&Gand);
				pGxor = reinterpret_cast<unsigned char *>(&Gxor);
				for (int i = 0; i<4; i++){
					*pGxor = (*pGx ^ *pGand);
					pGxor++;
					pGx++;
					pGand++;
				};

				Gx[y] = Gxor;
			};
		};

		memcpy(M + x*h, M2, h*sizeof(float));//��M2��ֵ���Ƶ� M ��
		// compute and store gradient orientation (O) via table lookup ����ʹ洢�ݶȷ���O��ͨ�����ұ�
		if (O != 0) for (y = 0; y<h; y++)
			O[x*h + y] = acost[(int)Gx[y]];//O�洢���ǹ�һ������ݶȷ���
		if (O != 0 && full) {
			int y1 = (((~(int)(O + x*h)) + 1) & 15) / 4;
			//y1 = ((~size_t(O + x*h) + 1) & 15) / 4; 
			y = 0;
			for (; y<y1; y++) O[y + x*h] += (Gy[y]<0)*PI; //���GyС��0��O��Ӧλ�÷���ֵ+180�ȣ��������任
			for (; y<h - 4; y++)
				O[y + x*h] += (Gy[y]<0)*PI;
			for (; y<h; y++) O[y + x*h] += (Gy[y]<0)*PI;//�˴�������yΪ��Ҫ��0~y1~h-4~h�ֳ��������ж�
		}
	}
}

// helper for gradHist, quantize O and M into O0, O1 and M0, M1 gradhist ����������O��M��O0��O1��M0��M1
void gradQuantize(float *O, float *M, int *O0, int *O1, float *M0, float *M1,
	int nb, int n, float norm, int nOrients, bool full)//�����ļ���full����������bin�������Ϊ1��ȡ0-2PI��0��ȡ0-PI
{//	s=(float)bin, hb=h/bin, wb=w/bin, h0=hb*bin, w0=wb*bin, nb=wb*hb;   
	//norm=1/s/s; n=h0; interpolate=false
	// assumes all *OUTPUT* matrices are 4-byte aligned��������*���*������4�ֽڶ���
	int i, o0, o1; float o, od, m;
	const float oMult = (float)nOrients / (full ? 2 * PI : PI); const int oMax = nOrients*nb;//oMult=9/2PI��9/PI

	for (i = 0; i <= n - 4; i++) {
		o = O[i] * oMult;
		o0 = (int)(o + 0.5f);
		o0 *= nb;
		o0 = (oMax > o0) ? o0 : 0;
		O0[i] = o0;
		M0[i] = M[i] * norm;
		M1[i] = 0.0f; O1[i] = 0;
		}
	// compute trailing locations without sse ��ʹ��sse�������λ��
	//interpolateΪfalse����ΪsoftBin>=0Ϊ��//softBin >= 0ʹ���ٽ�����bin���Բ�ֵ��softBin<0��ȡ�����bin����������Ϊ - 1��
	for (i; i<n; i++) {
		o = O[i] * oMult;				//oMult = 9/2PI
		o0 = (int)(o + .5f);
		o0 *= nb;
		if (o0 >= oMax)
			o0 = 0;
		O0[i] = o0;				//O = wb*hb*((O*9/2PI)+0.5)
		M0[i] = M[i] * norm; //M0Ϊ�ݶȷ���M����Ȩֵ��
		M1[i] = 0; O1[i] = 0; //M1��O1����0
	}
}

// compute nOrients gradient histograms per bin x bin block of pixels ����nOrients���ݶ�ֱ��ͼÿ��bin x bin������
void gradientHist(float *M, float *O, float *H, int h, int w,
	int bin, int nOrients, int softBin, bool full)
	//Hά��[wb*hb*9*3];bin=4;nOrients=2*9;softBin=-1;full=1

	//softBin��ʾѡ���ݶȷ�ֵ��Ȩ��ʽ��softBin>=0ʹ���ٽ�����bin���Բ�ֵ��softBin<0��ȡ�����bin����������Ϊ-1����
	//��ÿ��w*h�ķ���ͨ����ÿһbinsize * binsize��С�����أ������˿ռ�ķּ��������softbin������������
	//����Ӱ����bin��ʹ��˫���Բ�ֵ��������ÿ������ֻ��һ������bin�й��ס� ������һ������Ľ��Ϊ
	//floor([h / binSize w / binSize nOrients])ά��С������ͼ��������ÿ��ͼ�������е��ݶ�ֱ��ͼ��
{
	const int hb = h / bin, wb = w / bin, h0 = hb*bin, w0 = wb*bin, nb = wb*hb;
	const float s = (float)bin, sInv = 1 / s, sInv2 = 1 / s / s;
	float *H0, *H1; int x, y; float  xb, init;
	int   O0[N_h] = { 0 }, O1[N_h] = { 0 };
	float M0[N_h] = { 0 }, M1[N_h] = { 0 };

	// main loop
	for (x = 0; x<w0; x++) {
		// compute target orientation bins for entire column - very fast �������м���Ŀ�귽��bin-�ǳ���
		gradQuantize(O + x*h, M + x*h, O0, O1, M0, M1, nb, h0, sInv2, nOrients, full);//����O��M
		if (softBin==-1) { // interpolate using trilinear interpolation ���������Բ�ֵ��ֵ //softBinΪ����
			float ms[4], xyd, yb, xd, yd;
			bool hasLf, hasRt; int xb0, yb0;
			if (x == 0) { init = (0 + .5f)*sInv - 0.5f; xb = init; }   //��ʼ��xb = init = 0.5*(1/4)-0.5=-0.375
			hasLf = xb >= 0; xb0 = hasLf ? (int)xb : -1; hasRt = xb0 < wb - 1;//hasLf=0��xb0=-1��hasRt=1��
			xd = xb - xb0; xb += sInv; yb = init; y = 0;//xd=xb+1=0.625;xb=-0.125;yb=-0.375;y=0;
			// macros for code conciseness�������
			#define GHinit yd=yb-yb0; yb+=sInv; H0=H+xb0*hb+yb0; xyd=xd*yd; \
			   ms[0]=1-xd-yd+xyd; ms[1]=yd-xyd; ms[2]=xd-xyd; ms[3]=xyd;
			// leading rows, no top bin���ȵ��У�û�ж�bin
			for (; y<bin / 2; y++) {
				yb0 = -1; GHinit;
				if (hasLf) { H0[O0[y] + 1] += ms[1] * M0[y]; H0[O1[y] + 1] += ms[1] * M1[y]; }
				if (hasRt) { H0[O0[y] + hb + 1] += ms[3] * M0[y]; H0[O1[y] + hb + 1] += ms[3] * M1[y]; }
			}

			// main rows, has top and bottom bins, use SSE for minor speedup���У��ж����͵ײ���bin��ʹ��SSE��С����
				for (;; y++) {
					yb0 = (int)yb; if (yb0 >= hb - 1) break; GHinit;
					if (hasLf) {
						H1 = H0 + O0[y]; 
						*H1 += 0 * M0[y]; *(H1 + 1) += 0 * M0[y];*(H1 + 2) += ms[1] * M0[y]; *(H1 + 3) += ms[0] * M0[y];
						//*H1 += (ms[0] * M0[y] + ms[1] * M0[y]);
					}
					if (hasRt) {
						H1 = H0 + O0[y] + hb;
						*H1 += 0 * M0[y]; *(H1 + 1) += 0 * M0[y];*(H1 + 2) += ms[3] * M0[y];*(H1 + 3) += ms[2] * M0[y];
						//*H1 += (ms[2] * M0[y] + ms[3] * M0[y]);
					}
				}
			// final rows, no bottom bin���һ�У�û�еײ�bin
			for (; y<h0; y++) {
				yb0 = (int)yb; GHinit;
				if (hasLf) { H0[O0[y]] += ms[0] * M0[y]; H0[O1[y]] += ms[0] * M1[y]; }
				if (hasRt) { H0[O0[y] + hb] += ms[2] * M0[y]; H0[O1[y] + hb] += ms[2] * M1[y]; }
			}
			#undef GHinit
		}//���߲�ֵelse��ĩβ
	}

	// normalize boundary bins which only get 7/8 of weight of interior bins��һ���߽�bin��ֻ���ڲ�bin��7/8Ȩ��
	int o;
	if (softBin % 2 != 0) for (o = 0; o<nOrients; o++) {
		x = 0; for (y = 0; y<hb; y++) H[o*nb + x*hb + y] *= 8.f / 7.f;
		y = 0; for (x = 0; x<wb; x++) H[o*nb + x*hb + y] *= 8.f / 7.f;
		x = wb - 1; for (y = 0; y<hb; y++) H[o*nb + x*hb + y] *= 8.f / 7.f;
		y = hb - 1; for (x = 0; x<wb; x++) H[o*nb + x*hb + y] *= 8.f / 7.f;
	}
}

/******************************************************************************/

// HOG helper: compute 2x2 block normalization values (padded by 1 pixel)HOG���֣�����2x2���һ��ֵ����1���أ�
float* hogNormMatrix(float *N ,float *H, int nOrients, int hb, int wb, int bin) {
	float *N1, *n; int o, x, y, dx, dy, hb1 = hb + 1, wb1 = wb + 1;
	float eps = 1e-4f / 4 / bin / bin / bin / bin; // precise backward equality��ȷ�ķ����ʽ
	//N = (float*)malloc(sizeof(float)*hb1*wb1);
	//memset(N, 0, hb1*wb1*sizeof(float));
	N1 = N + hb1 + 1;
	for (o = 0; o<nOrients; o++) for (x = 0; x<wb; x++) for (y = 0; y<hb; y++)
		N1[x*hb1 + y] += H[o*wb*hb + x*hb + y] * H[o*wb*hb + x*hb + y];
	for (x = 0; x<wb - 1; x++) for (y = 0; y<hb - 1; y++) {
		n = N1 + x*hb1 + y; *n = 1 / (float)sqrt(n[0] + n[1] + n[hb1] + n[hb1 + 1] + eps);
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

// HOG helper: compute HOG or FHOG channels HOG����������HOG��FHOG�Ķ��ͨ��
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
			// store each orientation and normalization (nOrients*4 channels) �洢ÿ���������һ����nOrients��4ͨ����
			c = -1; GETT(0); H1[c*nbo + y] = t; GETT(1); H1[c*nbo + y] = t;
			GETT(hb1); H1[c*nbo + y] = t; GETT(hb1 + 1); H1[c*nbo + y] = t;
		}
		else if (type == 1) for (y = 0; y<hb; y++) {
			// sum across all normalizations (nOrients channels) �����еĹ�һ��ֵ�úͣ�nOrientsͨ����
			c = -1; GETT(0); H1[y] += t*.5f; GETT(1); H1[y] += t*.5f;
			GETT(hb1); H1[y] += t*.5f; GETT(hb1 + 1); H1[y] += t*.5f;
		}
		else if (type == 2) for (y = 0; y<hb; y++) {
			// sum across all orientations (4 channels) �����з�����ܺͣ�4��ͨ����
			c = -1; GETT(0); H1[c*nb + y] += t*r; GETT(1); H1[c*nb + y] += t*r;
			GETT(hb1); H1[c*nb + y] += t*r; GETT(hb1 + 1); H1[c*nb + y] += t*r;
		}
	}
#undef GETT
}

// compute HOG features
void hog(float *M, float *O, float *H, int h, int w, int binSize,
	int nOrients, int softBin, float clip)
{
	const int hb = h / binSize, wb = w / binSize, nb = hb*wb;
	//compute unnormalized gradient histograms ����ǹ�һ�����ݶ�ֱ��ͼ
	float R[(N_h / 4)*(N_w / 4) * 9] = { 0 };
	gradientHist(M, O, R, h, w, binSize, nOrients, softBin, true);
	// compute block normalization values ������һ��ֵ
	float N[(N_h / 4 + 1)*(N_w / 4 + 1)] = { 0 };
	hogNormMatrix(N,R, nOrients, hb, wb, binSize);
	// perform four normalizations per spatial block ÿ���ռ��ִ���Ĵι�һ��
	hogChannels(H, R, N, hb, wb, nOrients, clip, 0);
}
// compute FHOG features
void fhog(float *M, float *O, float *H, int h, int w, int binSize,
	int nOrients, int softBin, float clip)
{
	const int hb = h / binSize, wb = w / binSize, nb = hb*wb, nbo = nb*nOrients;
	int o, x;
	// compute unnormalized constrast sensitive histograms	����ǹ�һ���Աȶ����е�ֱ��ͼ
	//R1 = (float*)malloc(sizeof(float)*wb*hb*nOrients * 2);//Ϊ�˲�������Ԫ�������ڴ棬����Ӧ����2
	//memset(R1, 0, wb*hb*nOrients * 2 * sizeof(float));
	float R1[(N_h / 4)*(N_w / 4) * 18] = { 0 };	
	float R2[(N_h / 4)*(N_w / 4) * 9 ] = { 0 };
	gradientHist(M, O, R1, h, w, binSize, nOrients * 2, softBin, true);
	// compute unnormalized contrast insensitive histograms	����ǹ�һ���ԱȶȲ����е�ֱ��ͼ
	//R2 = (float*)malloc(sizeof(float)*wb*hb*nOrients);
	//memset(R2, 0, wb*hb*nOrients * sizeof(float));
	for (o = 0; o<nOrients; o++)
		for (x = 0; x<nb; x++)
			R2[o*nb + x] = R1[o*nb + x] + R1[(o + nOrients)*nb + x];
	// compute block normalization values ������һ��ֵ
	float N[(N_h / 4 + 1)*(N_w / 4 + 1)] = { 0 };
	hogNormMatrix(N,R2, nOrients, hb, wb, binSize);
	// normalized histograms and texture channels ��һ��ֱ��ͼ������ͨ��
	hogChannels(H + nbo * 0, R1, N, hb, wb, nOrients * 2, clip, 1);
	hogChannels(H + nbo * 2, R2, N, hb, wb, nOrients * 1, clip, 1);
	hogChannels(H + nbo * 3, R1, N, hb, wb, nOrients * 2, clip, 2);
}
