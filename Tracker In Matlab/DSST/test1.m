clc;close all;clear all;
im = imread('suv_2.jpg');
addpath('../Lib','../Lib/Fhoglib');%�����һ��Ŀ¼���ļ���1,2,n
im= rgb2gray(im);%ת��Ϊ�Ҷ�ͼ
im=single(im);
softBin = -1; useHog = 2; binSize=4; clip=.2; nOrients=9;
[M,O]=gradientMex('gradientMag',im,0,1);%
H = gradientMex('gradientHist',M,O,binSize,nOrients,softBin,useHog,clip);%

[M_qw,O_qw]=gradientMag(im,0,1);%new by qw
H_qw = fhog_features(im,M_qw,O_qw,binSize,nOrients,softBin,useHog,clip);

