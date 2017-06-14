function H_out = pcafhog(H,pcasz,frame)
%���������H����pca���н�ά
%pcaszΪ���ά��
    [h,w,p] = size(H);
%     pcasz = 5;%��Ҫ���������ά��
    H_out = zeros(h,w,pcasz);
%     if frame==58
%         disp('��ͣ');
%     end
%     percent=0;
    for i=1:h
        feature = reshape(H(i,:,:),w,p);
        [coed,score,latent] = pca(feature);
        coef = coed';
        H_out(i,:,:) = score*coef(:,1:pcasz);
%         pareto(100*latent/sum(latent));%����matla��ͼ
%         percent = percent+100*(sum(latent(1:pcasz))/sum(latent));
%         fangcha=sqrt(std(latent));
    end
%     percent = percent/h;
end