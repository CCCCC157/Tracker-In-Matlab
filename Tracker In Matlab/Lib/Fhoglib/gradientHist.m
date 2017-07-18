%% �����ݶ�ֱ��ͼ
function R = gradientHist(M,O,h,w,binSize,nOrients,softBin,full)
%gradientHist(M, O, h, w, binSize, nOrients * 2, softBin, true);
    hb = floor(h/binSize);wb = floor(w/binSize);nb = hb*wb;w0 = wb*binSize;h0 = hb*binSize;
    sInv = 1/binSize;sInv2 = 1/binSize/binSize;R = zeros(1,wb*hb*nOrients*3/2);
    for x=1:w0
        [M0,M1,O0,O1] = gradQuantize(M,O,x,h,nb,h0,sInv2,nOrients,full,softBin>=0);%����O��M
        if softBin==-1  %%����softBinΪ�������������Բ�ֵ           
            %����ԭ����c���԰汾�±��0��ʼ��216����matlab�±��1��ʼ���ʻᳬ�硣
                if x==1 ,init=0.5*sInv-0.5;xb=init;end%�˴��иĶ���init=-0.5*sInv;%Դ��0.5*sInv-0.5
                if xb>=0,hasLf=1;else hasLf=0;end
                if hasLf,xb0=floor(xb);else xb0=-1;end
                if xb0<wb-1,hasRt=1;else hasRt=0;end
                xd=xb-xb0;xb=xb+sInv;yb=init;base = 0;
                %�궨��
                expression='yd=yb-yb0;yb=yb+sInv;base=xb0*hb+yb0;xyd=xd*yd;ms(1)=1-xd-yd+xyd;ms(2)=yd-xyd;ms(3)=xd-xyd;ms(4)=xyd;';
                
                %leading rows,no top bin
                for y=1:floor(binSize/2)
                    yb0=-1;
                    eval(expression);
                    current=base+1;
                    if hasLf
                        R(current+O0(y)+1)=R(current+O0(y)+1)+ms(2)*M0(y);
                        R(current+O1(y)+1)=R(current+O1(y)+1)+ms(2)*M1(y);
                    end
                    if hasRt
                        R(current+O0(y)+hb+1)=R(current+O0(y)+hb+1)+ms(4)*M0(y);
                        R(current+O1(y)+hb+1)=R(current+O1(y)+hb+1)+ms(4)*M1(y);
                    end
                end
                y=y+1;
                %main rows,has top and bottom bins
                    while(1)
                        yb0=floor(yb);
                        if yb0>=hb-1,break;end
                        eval(expression);
                        try
                            if hasLf %Դ������sse��������λ��ˣ�������뵱ǰλ�ã�δʹ��sse���ٵĿ����˺����������
                                current=base+O0(y)+1;
%                                 if current>=213
%                                     current=213;
%                                 end
                                R(current)=R(current)+ms(1)*M0(y);
                                R(current+1)=R(current+1)+ms(2)*M0(y);
                                R(current+2)=R(current+2)+0*M0(y);
                                R(current+3)=R(current+3)+0*M0(y);%out of range
    %                             current=base+O0(y);R(current)=R(current)+0*M0(y)+0*M0(y)+ms(2)*M0(y)+ms(1)*M0(y);%by qw
                            end
                            if hasRt
                                current=base+O0(y)+hb+1;
%                                 if current>213
%                                     current=213;
%                                 end
                                R(current)=R(current)+ms(3)*M0(y);
                                R(current+1)=R(current+1)+ms(4)*M0(y);
                                R(current+2)=R(current+2)+0*M0(y);
                                R(current+3)=R(current+3)+0*M0(y);
    %                             current=base+O0(y)+hb;R(current)=R(current)+0*M0(y)+0*M0(y)+ms(4)*M0(y)+ms(3)*M0(y);%by qw
                            end
                        catch
                            disp('error');
                        end
                        y=y+1;
                    end
                    
                %final rows,no bottom bin
                for z=y:h0 %z�൱�ڴ�y���ŵ���ֱ��h0
                    yb0=floor(yb);eval(expression);
                    current=base;
                    if hasLf
                        R(current+O0(z))=R(current+O0(z))+ms(1)*M0(z);
                        R(current+O1(z))=R(current+O1(z))+ms(1)*M1(z);
                    end
                    if hasRt
                        R(current+O0(z)+hb)=R(current+O0(z)+hb)+ms(3)*M0(z);
                        R(current+O1(z)+hb)=R(current+O1(z)+hb)+ms(3)*M1(z);
                    end
                end
                
        end   %�����Բ�ֵ��ĩβ
    end%��ѭ��x��ĩβ
    %normalize boundary bins which only get 7/8 of weight of interior bins
    %��һ���߽�bin��ֻ���ڲ�bin��7/8Ȩ��
    if mod(softBin,2)~=0
        for o=1:nOrients
            x=1;for y=1:hb,R((o-1)*nb+(x-1)*hb+y)=R((o-1)*nb+(x-1)*hb+y)*8.0/7.0;end
            y=1;for x=1:wb,R((o-1)*nb+(x-1)*hb+y)=R((o-1)*nb+(x-1)*hb+y)*8.0/7.0;end
            x=wb;for y=1:hb,R((o-1)*nb+(x-1)*hb+y)=R((o-1)*nb+(x-1)*hb+y)*8.0/7.0;end
            y=hb;for x=1:wb,R((o-1)*nb+(x-1)*hb+y)=R((o-1)*nb+(x-1)*hb+y)*8.0/7.0;end
        end       
    end
end