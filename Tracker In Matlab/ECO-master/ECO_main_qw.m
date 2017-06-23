
% This demo script runs the ECO tracker
% qw 2017-6-23

close all;clearvars;clc;
% Add paths
setup_paths();

startframe = 1;         %  ��ʼ֡
choose     = 'fhog';    %  'fhog'--��ͨ����       'cnn'--���ѧϰ����
orign_file = 1;         %  '1 '--Դ�ļ�Ϊ��Ƶ     '0 '--Դ�ļ�ΪͼƬ  
sub_flag   = 0;         %  '1 '--ȡ�ָ�����ͼ��   '0 '--���ָ�ͼ��

%% ��ȡ��Ƶ�ļ�
if orign_file==1
    video_path = 'F:\testwot\WorldOfTanks 2017-06-21 13-00-30-988.avi';%��Ƶ
    [seq] = load_video_info_qw(video_path,startframe,sub_flag);%��Ƶ
else
%% ��ȡͼƬ�ļ�
    video_path = 'F:\testwot\59huangmo_2';%ͼƬ
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
