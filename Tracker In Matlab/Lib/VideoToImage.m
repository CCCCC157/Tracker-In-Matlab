%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%���ߣ�qw
%E-mail��1406820937@qq.com
%�������ǽ���Ƶ�ļ���ȡΪͼƬ���У������෴��ͼƬ����תΪ��Ƶ
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
close all;clear all;clc;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% ��Ƶת��ΪͼƬ
Video_path = 'F:\WOT\';%����·������
videoFile = [Video_path 'worldoftanks 2017-06-20 16-01-58-187.avi'];%��Ƶ�ļ���
img_path = [Video_path 'img'];%ͼƬ����·��
videoData = VideoReader(videoFile);%��ȡ��Ƶ�ļ�
start_frame = 560;%��ʼ֡
end_frame = 2000;%videoData.NumberOfFrames;%����֡CurrentTime
for i = start_frame : end_frame
    videoframe = read(videoData,i);
%     imshow(videoframe);
%     text(5, 18, strcat('#',num2str(i)), 'Color','y', 'FontWeight','bold', 'FontSize',20);
    imwrite(videoframe,fullfile(img_path,[num2str(i,'%06d') '.jpg']));
    disp(i);
end
disp('��Ƶת��ΪͼƬ���');
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% %% ͼƬת��Ϊ��Ƶ
% %framesPath :ͼ����������·����ͬʱҪ��֤ͼ���С��ͬ
% framesPath = 'D:\ImageData\David_qw\img\';
% %videoName:  ��ʾ��Ҫ��������Ƶ�ļ�������
% videoName = 'David_video';
% %quality:    ������Ƶ������ 0-100
% quality = 100;
% %Compressed: ѹ�����ͣ� 'Indeo3'��Ĭ�ϣ�, 'Indeo5', 'Cinepak', 'MSVC', 'RLE' or 'None'
% % Compressed = 'Motion JPEG AVI';
% %FrameRate: ֡��
% FrameRate = 50;
% if(exist('videoName','file'))
%     delete videoName.avi
% end
% %������Ƶ�Ĳ����趨
% aviobj = VideoWriter(videoName);  %����һ��avi��Ƶ�ļ����󣬿�ʼʱ��Ϊ��
% aviobj.Quality=quality;
% aviobj.FrameRate=FrameRate;
% % aviobj.VideoCompressionMethod=Compressed;
% %����ͼƬ
% % ��ȡ����ͼ��֡
% dir_content = dir(framesPath);
% % skip '.' and '..' from the count
% n_imgs = length(dir_content) - 2;
% img_files = cell(n_imgs, 1);
% for ii = 1:n_imgs
%     img_files{ii} = dir_content(ii+2).name;%imag_files�洢����ͼ��֡�ļ���
% end
% %startFrame ,endFrame ;��ʾ����һ֡��ʼ����һ֡����
% startFrame = 1;
% endFrame = n_imgs;
% open(aviobj);
% for i=startFrame:n_imgs   %
%     frames=imread([framesPath,img_files{i}]);
%     writeVideo(aviobj,frames);
%     disp(i);
% end
% close(aviobj); % �رմ�����Ƶ
% disp('ͼƬת��Ϊ��Ƶ���');
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%