%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%���ߣ�qw
%E-mail��1406820937@qq.com
%�ӳ������Ƕ�ȡͼƬ֡�����Ҷ�ȡgroundtruth����
%��ground_truthĿ����ʵλ��,img_pathͼƬ�ļ���,img_filesͼƬ�ļ����洢,imgDir�ļ�·��,��
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
function [params,im]=Load_image(imgDir)
%     %% Read params.txt
    params = readParams('params.txt');
	%% load video info
    sequence_path = [imgDir,'/'];%�ļ�·��
    img_path = [sequence_path 'img/'];
    start_frame = 1;
    %% Read files 
    params.bb_VOT = csvread([sequence_path 'groundtruth_rect.txt']);%��������ʵĿ��λ��
    if size(params.bb_VOT,2)==1
        region = params.bb_VOT(1:4);
    else
        region = params.bb_VOT(start_frame,:);%��ȡgroundtruth�ĵ�һ��4������
    end
    %%%%%%%%%%%%%%%%%%%%%%%%%
    % ��ȡ����ͼ��֡
    dir_content = dir([sequence_path 'img/']);
    % skip '.' and '..' from the count
    n_imgs = length(dir_content) - 2;
    img_files = cell(n_imgs, 1);
    for ii = 1:n_imgs
        img_files{ii} = dir_content(ii+2).name;
    end
    img_files(1:start_frame-1)=[];
    im = imread([img_path img_files{start_frame}]);%����һ��ͼ�����
    % �ж��Ƿ�Ҷ�ͼ�� ?
    if(size(im,3)==1)
        params.grayscale_sequence = true;
    end
    %% get position and boxsize ��ȡgroundtruth���� 
    params.img_files = img_files;%��ͼ��������������params
    params.img_path = img_path;%��ͼ��·������������params
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    if(numel(region)==4)%����ͼ�����ػ�����Ԫ�ظ���
        x = region(1);y = region(2);w = region(3);h = region(4);cx = x+w/2;cy = y+h/2;%
    else
        waring('wrong with groundtruth');
    end
    % init_pos �Ƿ�Χ�������
    params.init_pos = [cy cx];
    params.target_sz = round([h w]);%���������������������£
    params.totalframe = numel(params.img_files);
%     params.imgheight = h;
%     params.imgwidth = w;
%     params.fp = fp;
%     im = imread([img_path img_files{1}]);%��ȡĿ��֡
%     im= rgb2gray(im);%ת��Ϊ�Ҷ�ͼ
%     imshow(im);
end