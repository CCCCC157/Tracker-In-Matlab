function [Image] = loading_cap(fp,ImgHeight,ImgWidth)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%���ߣ�qw
%E-mail��1406820937@qq.com
%�ӳ����������ζ�ȡcap16�ļ�
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    Image = zerose(ImgHeight,ImgWidth);
    imgfile = fread(fp,ImgHeight*ImgWidth*2);
    for x=1:ImgHeight
        for y=1:ImgWidth
            Image(x,y) = imgfile((x-1)*ImgWidth*2 + 2*(y-1)+2)*256 + imgfile((x-1)*ImgWidth*2 + 2*(y-1)+1);
        end
    end
end