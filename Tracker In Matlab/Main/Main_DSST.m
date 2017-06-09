%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%                  ����������DSST
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
close all;clear all;clc;
addpath('../MaltabLibrary_qw','../MaltabLibrary_qw/Fhog_qw','../DSST');%�����һ��Ŀ¼���ļ���1,2,n
sequence = 'D:\ImageData\Coke';%����ͼƬ·��
[params,im] = Load_image(sequence);%��ȡԴ�ļ�picture %%% 
%     [paraams,Image16] = InitCap16(sequence);%��ȡԴ�ļ�cap16
%ask the user for the video
params.video_path = [sequence '/img/'];
% ground_truth = [params.init_pos,params.target_sz];

[positions, fps] = dsst(params);%Ŀ�����

%% ���㾫��
[distance_precision, PASCAL_precision, average_center_location_error] = ...
    compute_performance_measures(positions, ground_truth);

fprintf('Center Location Error: %.3g pixels\nDistance Precision: %.3g %%\nOverlap Precision: %.3g %%\nSpeed: %.3g fps\n', ...
    average_center_location_error, 100*distance_precision, 100*PASCAL_precision, fps);
%%
%%
% �㷨����
% 
% Input: 
% ����ͼ��patch It 
% ��һ֡��λ��Pt?1�ͳ߶�St?1 
% λ��ģ��Atranst?1��Btanst?1�ͳ߶�ģ��Ascalet?1��Bscalet?1 
% Output: 
% ���Ƶ�Ŀ��λ��Pt�ͳ߶�St 
% ����λ��Atranst��Btranst�ͳ߶�ģ��Ascalet��Bscalet
% ����, 
% λ�������� 
% 1.����ģ����ǰһ֡��λ�ã��ڵ�ǰ֡�а���ǰһ֡Ŀ��߶ȵ�2����С��ȡһ������Ztrans 
% 2.����Ztrans��Atranst?1��Btanst?1�����ݹ�ʽ(6)����ytrans 
% 3.����max(ytrans)���õ�Ŀ���µ�λ��Pt 
% �߶������� 
% 4.��Ŀ�굱ǰ��λ��Ϊ���ģ���ȡ33�ֲ�ͬ�߶ȵ�����Ztrans 
% 5.����Ztrans��Atranst?1��Btanst?��ʽ(6)�����yscale 
% 6.����max(yscale)���õ�Ŀ��׼ȷ�ĳ߶�St
% ģ�͸��£� 
% 7.��ȡ����ftrans��fscale 
% 8.����λ��ģ��Atranst��Btranst 
% 9.���³߶�ģ��Ascalet��Bscalet