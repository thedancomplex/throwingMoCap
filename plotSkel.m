close all
clear all

%% Global Data
[h,d]=hdrload('moCapCapture/SampleClient/xyzGlobal_UH.pts');
s = size(d);
dd = d(s(1),:);
%% Local Data
[H,D]=hdrload('moCapCapture/SampleClient/rpyLocal_UH.pts');
S = size(D);
DD = D(S(1),:);
figure

getMoCapPlot(dd,DD);




