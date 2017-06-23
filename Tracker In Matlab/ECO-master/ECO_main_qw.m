
% This demo script runs the ECO tracker
% qw 2017-6-23

close all;clearvars;clc;
% Add paths
setup_paths();
startframe = 1;

choose = 'fhog';%   'fhog'--�ֶ��������hand-crafted      'cnn'--���ѧϰ����deep features
orign_file = 1;%   '1'--Դ�ļ�Ϊ��Ƶ     '0'--Դ�ļ�ΪͼƬ  


%% ��ȡ��Ƶ�ļ�
if orign_file==1
    video_path = 'F:\testwot\worldoftanks 2017-06-23 12-45-32-856.avi';%��Ƶ
    [seq] = load_video_info_qw(video_path,startframe);%��Ƶ
else
%% ��ȡͼƬ�ļ�
    video_path = 'F:\testwot\121B_623_1317';%ͼƬ
    [seq] = load_video_info_qw_picture(video_path,startframe);%��ͼƬ���ö�ȡgroundtruth�ļ�����ȡ��ʼ��λ
end
%% Run ECO

switch choose
    case 'fhog'
        % Run ECO
        if orign_file==1
            results = testing_ECO_HC(seq);%��ȡ��Ƶ
        else
            results = testing_ECO_HC_picture(seq);%��ȡͼƬ
        end
    case 'cnn'
        if orign_file==1
            results = testing_ECO(seq);%��ȡ��Ƶ
        else
            results = testing_ECO_picture(seq);%��ȡͼƬ
        end
end
%%
