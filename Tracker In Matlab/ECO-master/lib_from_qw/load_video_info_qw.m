function [seq] = load_video_info_qw(video_path,startframe)
    
    videoData = VideoReader(video_path);%��ȡ��Ƶ�ļ�
    seq.len = videoData.NumberOfFrames;%����֡CurrentTime
    seq.startframe = startframe;
    seq.videoData = videoData;
    pos = [videoData.Height/2,videoData.Width/2];
    
    videoframe = read(videoData,startframe);
    im = get_subimg(videoframe,pos);
    [~,seq.init_rect] = imcrop(im);%�ָ�ͼ��
    



% ground_truth = dlmread([video_path '/groundtruth_rect.txt']);%
% 
% seq.init_rect = ground_truth(1,:);
% sequence_path = [video_path,'/'];
% 
% seq.len = numel(img_files);
% seq.s_frames = cellstr(img_files);
% seq.path = [video_path,'/img/'];
% seq.startframe = startframe;
end

