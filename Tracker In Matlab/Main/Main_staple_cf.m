%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%                  ����������StapleOnlyCf
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    close all;clear all;clc;
% ����ļ�·��
%     addpath(genpath(pwd));%��ӵ�ǰ�ļ��м����ļ���
    addpath('../Lib','../StapleOnlyCF');%�����һ��Ŀ¼���ļ���1,2,n
%     rmpath('./�ϼ�Ŀ¼�е��ļ���1');%ȥ��·����Ϊ���޸��ļ����Ȳ���������Matlab����Ϊ��Ҫ�ĵ�·������ʹ���У��ǽ�ֹ������
    sequence = 'D:\ImageData\Toy';%����ͼƬ·��
    [params,im] = Load_image(sequence);%��ȡԴ�ļ�picture %%% 
%     [paraams,Image16] = InitCap16(sequence);%��ȡԴ�ļ�cap16
%% staple-only-cf
    [params, bg_area, fg_area, area_resize_factor] = initializeAllAreas(im, params);
	% ��ʼ����������
    result = trackerMain_StapleOnlyCf(params, im, bg_area, area_resize_factor);
    %% the end ��ʾ���
    fclose('all');
    show_precision(result.pos, params.bb_VOT, sequence);%�������ֵ��groundtruth�걸����£�
    disp('��������������');