close all
clear all
%% Add paths
addpath('recordAces');

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
mName   =   {};
mDeg    =   [];
for( i = 1:s(1) )
    DD = D(i,:);
    dd = d(i,:);
    [mName, mDegT] = getMoCapPlot(dd,DD);
    mDeg(i,:) = mDegT;
    
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


%% record to aces
recordAces(mName, mDeg, 'test1');



%% play movie
for ( i = 1: s(1) )
    mF(i) = F{i};
end
movie(mF, 1, 100);
disp('play movie')
%movie2avi(mF, 'test.avi', 'FPS', 100)
%movie(F)




