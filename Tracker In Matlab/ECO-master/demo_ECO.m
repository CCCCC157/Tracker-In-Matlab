
% This demo script runs the ECO tracker with deep features on the
% included "Crossing" video.
close all;clearvars;clc;
% Add paths
setup_paths();
startframe = 1;
sub_flag = 0;%  '1'--ȡ�ָ�����ͼ��     '0'--���ָ�ͼ��

% ��ȡ��Ƶ�ļ�
video_path = 'F:\testwot\worldoftanks 2017-06-23 12-45-32-856.avi';%��Ƶ
[seq] = load_video_info_qw(video_path,startframe,sub_flag);%��Ƶ
% % Run ECO
results = testing_ECO(seq);%��ȡ��Ƶ

% ��ȡͼƬ�ļ�
% video_path = 'F:\testwot\121B_623_1317\';%ͼƬ
% [seq] = load_video_info_qw_picture(video_path,startframe);%��ͼƬ���ö�ȡgroundtruth�ļ�����ȡ��ʼ��λ
% % Run ECO
% results = testing_ECO_picture(seq);%��ȡͼƬ