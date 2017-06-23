function [img_out] = get_subimg(im,pos,sub_flag)
    %��ȡͼ��Ϊ��posΪ����out_w*out_h��С����ͼ
    if sub_flag==1
        out_w = 720;out_h = 576;[w,h,~] = size(im);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        if out_w>w  || out_h>h
            error('size of input image is too small');
            return;
        end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        img_out = im(pos(1)-out_h/2:pos(1)+out_h/2,pos(2)-out_w/2:pos(2)+out_w/2,:);
    else
        img_out = im;
    end
end