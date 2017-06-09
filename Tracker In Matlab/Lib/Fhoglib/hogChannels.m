function H = hogChannels(H,n,tum, R1, N, hb, wb, nOrients, clip, type)
% ����HOG��FHOG�Ķ��ͨ��
% H = hogChannels(H + nbo * t, R1, N, hb, wb, nOrients * 2, clip, 1)
    %�궨��
%     expression='t=R1(y)*N(y-blk);if t>clip,t=clip;end ;c=c+1;';
    h=tum*n;r=0.2357;nb=wb*hb;nbo=nOrients*nb;hb1=hb+1;
    for o=1:nOrients
        for x=1:wb
            r1=(o-1)*nb+(x-1)*hb;n1=(x-1)*hb1+hb1+1;
            if type<=1,h1=h+(o-1)*nb+(x-1)*hb;else h1=h+(x-1)*hb;end
            if type==0
                for y=1:hb
                    %store each orientation and normalization (nOrients*4 channels) �洢ÿ���������һ����nOrients��4ͨ����
                    c=-1;
                    t=R1(r1+y)*N(n1+y-0);if t>clip,t=clip;end ;c=c+1;%expression
                    H(h1+(c-1)*nbo+y)=t;
                    t=R1(r1+y)*N(n1+y-1);if t>clip,t=clip;end ;c=c+1;%expression
                    H(h1+(c-1)*nbo+y)=t;
                    t=R1(r1+y)*N(n1+y-hb1);if t>clip,t=clip;end ;c=c+1;%expression
                    H(h1+(c-1)*nbo+y)=t;
                    t=R1(r1+y)*N(n1+y-(hb1+1));if t>clip,t=clip;end ;c=c+1;%expression
                    H(h1+(c-1)*nbo+y)=t;
                end
            else if type==1
                    for y=1:hb
                        %sum across all normalizations (nOrients channels) �����еĹ�һ��ֵ�úͣ�nOrientsͨ����
                        c=-1;
                        t=R1(r1+y)*N(n1+y-0);if t>clip,t=clip;end ;c=c+1;%expression
                        H(h1+y)=H(h1+y)+t*0.5;
                        t=R1(r1+y)*N(n1+y-1);if t>clip,t=clip;end ;c=c+1;%expression
                        H(h1+y)=H(h1+y)+t*0.5;
                        t=R1(r1+y)*N(n1+y-hb1);if t>clip,t=clip;end ;c=c+1;%expression
                        H(h1+y)=H(h1+y)+t*0.5;
                        t=R1(r1+y)*N(n1+y-(hb1+1));if t>clip,t=clip;end ;c=c+1;%expression
                        H(h1+y)=H(h1+y)+t*0.5;
                    end
            else if type==2
                    for y=1:hb
                       %sum across all orientations (4 channels) �����з�����ܺͣ�4��ͨ����
                        c=-1;
                        t=R1(r1+y)*N(n1+y-0);if t>clip,t=clip;end ;c=c+1;%expression
                        H(h1+(c-1)*nb+y)=H(h1+(c-1)*nb+y)+t*r;
                        t=R1(r1+y)*N(n1+y-1);if t>clip,t=clip;end ;c=c+1;%expression
                        H(h1+(c-1)*nb+y)=H(h1+(c-1)*nb+y)+t*r;
                        t=R1(r1+y)*N(n1+y-hb1);if t>clip,t=clip;end ;c=c+1;%expression
                        H(h1+(c-1)*nb+y)=H(h1+(c-1)*nb+y)+t*r;
                        t=R1(r1+y)*N(n1+y-(hb1+1));if t>clip,t=clip;end ;c=c+1;%expression
                        H(h1+(c-1)*nb+y)=H(h1+(c-1)*nb+y)+t*r;
                    end
                end
                end
            end    
        end
    end
end