%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%���ߣ�qw
%E-mail��1406820937@qq.com
%�������ǽ���Ƶһ�����Ӵ��ڽ�ȡ��������Ϊ��Ƶ�ļ�
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
close all;clearvars;clc;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Video_path = 'F:\testwot\121B_623_1317\';%����·������
videoFile = [Video_path 'worldoftanks 2017-06-23 12-49-13-324.avi'];%��Ƶ�ļ���
videoData = VideoReader(videoFile);%��ȡ��Ƶ�ļ�
start_frame = 1;%��ʼ֡
end_frame = 100;%videoData.NumberOfFrames;%����֡CurrentTime
pos = [videoData.Height/2,videoData.Width/2];

out_videoName = 'ourtput_video';
writerObj=VideoWriter(out_videoName);  %// ����һ����Ƶ�ļ������涯��
writerObj.Quality=100;
writerObj.FrameRate=60;
open(writerObj);                    %// �򿪸���Ƶ�ļ�
figure
for frames = start_frame : 1 : end_frame
    videoframe = read(videoData,frames);
    im = get_subimg(videoframe,pos);
    imshow(im);
    writeVideo(writerObj,im);
    disp(frames);
end 
close(writerObj); %// �ر���Ƶ�ļ����
disp('��Ƶ��ȡ��С���');
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%