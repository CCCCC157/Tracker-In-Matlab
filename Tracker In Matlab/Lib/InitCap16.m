function [params,Image16] = InitCap16(CapFile_Floader)
    if nargin<1,CapFile_Floader = 'C:Generated_CAP_File\';end
    [filename,pathname] = uigetfile([CapFile_Floader '*.cap16']);
    type =152;     %cap�ļ�������
    file = fullfile(filename,pathname);
    fp = fopen(file);
    TotalFrame = 1000;  %��֡��
    switch type
        case 152
            ImgHeight = 576;ImgWidth = 720; %152: 576*720
        case 140
            ImgHeight = 576;ImgWidth = 720; %152: 576*720
        case 127
            ImgHeight = 288;ImgWidth = 384; %152: 576*720
    end
    Image16 = loding_cap(fp,ImgHeight,ImgWidth);
    %%% Read params.txt
    params = readParams('params.txt');
    choose = 1;
    if choose
        cx = 358;cy = 283;w = 20;h = 16;
    else
        [sub_img,target] = imcrop(Image16);%�ָ�ͼ��
        cx = target(1)+target(3)/2;
        cy = target(2)+target(4)/2;
        w = target(3);h = target(4);
    end
    % init_pos �Ƿ�Χ�������
    params.init_pos = [cy cx];
    params.target_sz = round([h w]);%���������������������£
    params.totalframe = TotalFrame;
    params.imgheight = ImgHeight;
    params.imgwidth = ImgWidth;
    params.fp = fp;
end