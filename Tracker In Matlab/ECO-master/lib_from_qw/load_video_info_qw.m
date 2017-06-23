function [seq] = load_video_info_qw(video_path,startframe,sub_flag)
    
    videoData = VideoReader(video_path);%��ȡ��Ƶ�ļ�
    seq.len = videoData.NumberOfFrames;%����֡CurrentTime
    seq.startframe = startframe;
    seq.videoData = videoData;
    seq.sub_flag = sub_flag;
    pos = [videoData.Height/2,videoData.Width/2];
    
    videoframe = read(videoData,startframe);
    if sub_flag==1
        im = get_subimg(videoframe,pos);%��ȡ720*576�м�ͼ��
    else
        im = videoframe;
    end
    
    [~,seq.init_rect] = imcrop(im);%�ָ�ͼ��
    
end

