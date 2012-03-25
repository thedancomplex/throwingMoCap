close all
clear all

%% Global Data
%[h,d]=hdrload('moCapCapture/SampleClient/xyzGlobal_UH.pts');
[h,d]=hdrload('moCapCapture/SampleClient/xyzGlobal.pts');
s = size(d);
dd = d(s(1),:);
%% Local Data
%[H,D]=hdrload('moCapCapture/SampleClient/rpyLocal_UH.pts');
[H,D]=hdrload('moCapCapture/SampleClient/rpyLocal.pts');
S = size(D);
DD = D(S(1),:);
figure

AZ = [];
EL = [];
F = {};
for( i = 1:s(1) )
    DD = D(i,:);
    dd = d(i,:);
    getMoCapPlot(dd,DD);
    if (i == 1)
        input('Set Desired View the press ENTER');
        [AZ,EL] = VIEW();
        view(AZ,EL);
    else
        view(AZ,EL);
    end
    
    F{i} = getframe(gcf);
    
    if( i < s(1))
        clf;
    end
end

for ( i = 1: s(1) )
    mF(i) = F{i};
end

movie(mF, 1, 100);
disp('play movie')
%movie2avi(mF, 'test.avi', 'FPS', 100)
%movie(F)




