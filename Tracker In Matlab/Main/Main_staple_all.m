%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%                  ����������StapleAll
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    close all;clear all;clc;
% ����ļ�·��
%     addpath(genpath(pwd));%��ӵ�ǰ�ļ��м����ļ���
    addpath('../Lib','../StapleAll');%�����һ��Ŀ¼���ļ���1,2,n
%     rmpath('./�ϼ�Ŀ¼�е��ļ���1');%ȥ��·����Ϊ���޸��ļ����Ȳ���������Matlab����Ϊ��Ҫ�ĵ�·������ʹ���У��ǽ�ֹ������
    sequence = 'D:\ImageData\Toy';%����ͼƬ·��
    [params,im] = Load_image(sequence);%��ȡԴ�ļ�picture %%% 
%     [paraams,Image16] = InitCap16(sequence);%��ȡԴ�ļ�cap16
	if params.visualization%Ϊ1ʱ��ͼ��0ʱ����ͼ
		params.videoPlayer = vision.VideoPlayer('Position', [100 100 [size(im,2), size(im,1)]+30]);
	end
%% staple-all
    [params, bg_area, fg_area, area_resize_factor] = initializeAllAreas(im, params);
	% ��ʼ����������
    result = trackerMain(params, im, bg_area, fg_area, area_resize_factor);
    %% the end ��ʾ���
    fclose('all');
%     show_precision(result.pos, params.bb_VOT, sequence);%�������ֵ��groundtruth�걸����£�
    disp('��������������');