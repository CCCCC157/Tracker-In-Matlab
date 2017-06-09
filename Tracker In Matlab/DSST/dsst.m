function [positions, fps] = dsst(params)

% [positions, fps] = dsst(params)

% parameters
padding = params.padding;                         	%extra area surrounding the target%����Ŀ��Ķ�������
output_sigma_factor = params.output_sigma_factor;	%spatial bandwidth (proportional to target)%�ռ������Ŀ��ɱ�����%����λ���˲�������ı�׼��
lambda = params.lambda;                             % regularization weight (denoted "lambda" in the paper)%����Ȩ�أ������С�lamda����
learning_rate = params.learning_rate;               % tracking model learning rate (denoted "eta" in the paper)%����ģ��ѧϰ�ʣ������С�eta����
nScales = params.number_of_scales;                  % number of scale levels (denoted "S" in the paper)%�߶ȵȼ�������Ϊ��S����
scale_step = params.scale_step;                     % Scale increment factor (denoted "a" in the paper)%�߶��������ӣ���ʾ��a����
scale_sigma_factor = params.scale_sigma_factor;     % standard deviation for the desired scale filter output%�����߶��˲�������ı�׼��
scale_model_max_area = params.scale_model_max_area; % the maximum size of scale examples%���߶ȵ�����

video_path = params.video_path; %��Ƶ·��
img_files = params.img_files;   %�ļ�·��
pos = floor(params.init_pos);   %��ʼ��Ŀ��λ��
target_sz = floor(params.target_sz);%Ŀ���С

visualization = params.visualization;%���ӻ�����������1��

num_frames = numel(img_files);%p.totalframe;%ͼƬ֡������

init_target_sz = target_sz;%Ŀ���ʼ��С

% target size att scale = 1
base_target_sz = target_sz;

% window size, taking padding into account
sz = floor(base_target_sz * (1 + padding));

% desired translation filter output (gaussian shaped), bandwidth
% proportional to target size
output_sigma = sqrt(prod(base_target_sz)) * output_sigma_factor;
[rs, cs] = ndgrid((1:sz(1)) - floor(sz(1)/2), (1:sz(2)) - floor(sz(2)/2));
y = exp(-0.5 * (((rs.^2 + cs.^2) / output_sigma^2)));
yf = single(fft2(y));


% desired scale filter output (gaussian shaped), bandwidth proportional to
% number of scales
scale_sigma = nScales/sqrt(33) * scale_sigma_factor;
ss = (1:nScales) - ceil(nScales/2);
ys = exp(-0.5 * (ss.^2) / scale_sigma^2);
ysf = single(fft(ys));

% store pre-computed translation filter cosine window
cos_window = single(hann(sz(1)) * hann(sz(2))');

% store pre-computed scale filter cosine window
if mod(nScales,2) == 0
    scale_window = single(hann(nScales+1));
    scale_window = scale_window(2:end);
else
    scale_window = single(hann(nScales));
end;

% scale factors
ss = 1:nScales;
scaleFactors = scale_step.^(ceil(nScales/2) - ss);

% compute the resize dimensions used for feature extraction in the scale
% estimation
scale_model_factor = 1;
if prod(init_target_sz) > scale_model_max_area
    scale_model_factor = sqrt(scale_model_max_area/prod(init_target_sz));
end
scale_model_sz = floor(init_target_sz * scale_model_factor);

currentScaleFactor = 1;

% to calculate precision
positions = zeros(numel(img_files), 4);

% to calculate FPS
time = 0;

% find maximum and minimum scales
im = imread([video_path img_files{1}]); %%% im = loding_cap(p.fp,p.ImgHeight,p.ImgWidth);
min_scale_factor = scale_step ^ ceil(log(max(5 ./ sz)) / log(scale_step));
max_scale_factor = scale_step ^ floor(log(min([size(im,1) size(im,2)] ./ base_target_sz)) / log(scale_step));

for frame = 1:num_frames,
    %load image
    im = imread([video_path img_files{frame}]); %%% im = loding_cap(p.fp,p.ImgHeight,p.ImgWidth);

    tic;
    
    if frame > 1
        
        %��ȡ������������F
         %������ÿ�����ص����28ά�ں�����(1άԭʼ�Ҷ�+27άfhog)
         %���Զ�άhann����Ϊ����F
         %����λ������˲���
        % extract the test sample feature map for the translation filter
        xt = get_translation_sample(im, pos, sz, currentScaleFactor, cos_window);
        
        %������Ӧֵy=F-1{(A*Z)/(B+lambda)}
        % calculate the correlation response of the translation filter
        xtf = fft2(xt);
        response = real(ifft2(sum(hf_num .* xtf, 3) ./ (hf_den + lambda)));
        
        %�ҵ�max(y)�õ�Ŀ����λ��
        % find the maximum translation response
        [row, col] = find(response == max(response(:)), 1);
        
        % ����Ŀ��λ��
        % update the position
        pos = pos + round((-sz/2 + [row, col]) * currentScaleFactor);
        
        %��ÿ������resize�ɹ̶���С���ֱ���ȡ31άfhog������ÿ������������fhog��
        %������һ��������������33�����������������һάhann������Ϊ����F
        % ���ڳ߶�����˲���
        % extract the test sample feature map for the scale filter
        xs = get_scale_sample(im, pos, base_target_sz, currentScaleFactor * scaleFactors, scale_window, scale_model_sz);
        
        %�õ��߶ȱ任����Ӧ���ֵy=F-1{(A*Z)/(B+lambda)}
        % calculate the correlation response of the scale filter
        xsf = fft(xs,[],2);
        scale_response = real(ifft(sum(sf_num .* xsf, 1) ./ (sf_den + lambda)));
        
        %�ҵ�max(y)�õ���ǰ�ĳ߶�
        % find the maximum scale response
        recovered_scale = find(scale_response == max(scale_response(:)), 1);
        
         % ���µ�ǰ�߶�
        % update the scale
        currentScaleFactor = currentScaleFactor * scaleFactors(recovered_scale);
        if currentScaleFactor < min_scale_factor
            currentScaleFactor = min_scale_factor;
        elseif currentScaleFactor > max_scale_factor
            currentScaleFactor = max_scale_factor;
        end
    end
    %��ȡ����ѵ����������X
    %������ÿ�����ص����28ά�ں�����(1άԭʼ�Ҷ�+27άfhog)
    %���Զ�άhann����Ϊ����X
    %��ȡ��������λ������˲���
    % extract the training sample feature map for the translation filter
    xl = get_translation_sample(im, pos, sz, currentScaleFactor, cos_window);
    
    %��ȡ����A=GF;��ĸB=F*F;��ʱû��lambda
    % calculate the translation filter update
    xlf = fft2(xl);
    new_hf_num = bsxfun(@times, yf, conj(xlf));
    new_hf_den = sum(xlf .* conj(xlf), 3);
    
    %��ÿ������resize�ɹ̶���С���ֱ���ȡ31άfhog������ÿ������������fhog��
    %������һ��������������33�����������������һάhann������Ϊ����X
    % ��ȡ�������ڳ߶�����˲���
    % extract the training sample feature map for the scale filter
    xs = get_scale_sample(im, pos, base_target_sz, currentScaleFactor * scaleFactors, scale_window, scale_model_sz);
    
    %ͬ���Ļ�ȡ����A=GF;��ĸB=F*F;��ʱû��lambda
    % calculate the scale filter update
    xsf = fft(xs,[],2);
    new_sf_num = bsxfun(@times, ysf, conj(xsf));
    new_sf_den = sum(xsf .* conj(xsf), 1);
    
    
    if frame == 1
        % first frame, train with a single image
        hf_den = new_hf_den;
        hf_num = new_hf_num;
        
        sf_den = new_sf_den;
        sf_num = new_sf_num;
    else
        % subsequent frames, update the model
        hf_den = (1 - learning_rate) * hf_den + learning_rate * new_hf_den;
        hf_num = (1 - learning_rate) * hf_num + learning_rate * new_hf_num;
        sf_den = (1 - learning_rate) * sf_den + learning_rate * new_sf_den;
        sf_num = (1 - learning_rate) * sf_num + learning_rate * new_sf_num;
    end
    
    % calculate the new target size
    target_sz = floor(base_target_sz * currentScaleFactor);
    
    %save position
    positions(frame,:) = [pos target_sz];
    
    time = time + toc;
    
    
    %visualization
    if visualization == 1
        rect_position = [pos([2,1]) - target_sz([2,1])/2, target_sz([2,1])];
        if frame == 1,  %first frame, create GUI
            figure('Number','off', 'Name',['Tracker - ' video_path]);
            im_handle = imshow(im);%im_handle = imshow(uint8(im), 'Border','tight', 'InitialMag', 100 + 100 * (length(im) < 500));
            title('DSST');
            rect_handle = rectangle('Position',rect_position, 'EdgeColor','w');
            text_handle = text(5, 18, strcat('#',num2str(frame)), 'Color','w', 'FontWeight','bold', 'FontSize',15);
%             set(text_handle, 'color', [0 1 1]);
        else
            try  %subsequent frames, update GUI
                set(im_handle, 'CData', im)
                set(rect_handle, 'Position', rect_position)
                set(text_handle, 'string', strcat('#',num2str(frame)))
            catch
                return
            end
        end
        
        drawnow
%         pause
    end
end

fps = num_frames/time;