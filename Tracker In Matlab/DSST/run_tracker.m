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
%%


% run_tracker.m

close all;
% clear all;

%choose the path to the videos (you'll be able to choose one with the GUI)
base_path = 'D:\Image Process\����FHOG�����汾ƽ̨����Ա�\DSST_code_MATLAB\code - ���������޸�\sequences';

%parameters according to the paper
params.padding = 1.0;         			% extra area surrounding the target%����Ŀ��Ķ�������
params.output_sigma_factor = 1/16;		% standard deviation for the desired translation filter output%����λ���˲�������ı�׼��
params.scale_sigma_factor = 1/4;        % standard deviation for the desired scale filter output%�����߶��˲�������ı�׼��
params.lambda = 1e-2;					% regularization weight (denoted "lambda" in the paper)%����Ȩ�أ������С�lamda����
params.learning_rate = 0.025;			% tracking model learning rate (denoted "eta" in the paper)%����ģ��ѧϰ�ʣ������С�eta����
params.number_of_scales = 33;           % number of scale levels (denoted "S" in the paper)%�߶ȵȼ�������Ϊ��S����
params.scale_step = 1.02;               % Scale increment factor (denoted "a" in the paper)%�߶��������ӣ���ʾ��a����
params.scale_model_max_area = 512;      % the maximum size of scale examples%���߶ȵ�����

params.visualization = 1;

%ask the user for the video
video_path = choose_video(base_path);
if isempty(video_path), return, end  %user cancelled
[img_files, pos, target_sz, ground_truth, video_path] = ...
	load_video_info(video_path);

params.init_pos = floor(pos) + floor(target_sz/2);
params.wsize = floor(target_sz);
params.img_files = img_files;
params.video_path = video_path;

[positions, fps] = dsst(params);%Ŀ�����

% calculate precisions%���㾫��
[distance_precision, PASCAL_precision, average_center_location_error] = ...
    compute_performance_measures(positions, ground_truth);

fprintf('Center Location Error: %.3g pixels\nDistance Precision: %.3g %%\nOverlap Precision: %.3g %%\nSpeed: %.3g fps\n', ...
    average_center_location_error, 100*distance_precision, 100*PASCAL_precision, fps);