close all
clear all

[h,d]=hdrload('Client-output.pts');
s = size(d);
dd = d(s(1),:);
SL = dd(1:3);
EL = dd(4:6);
SR = dd(7:9);
ER = dd(10:12);

HiL = dd(13:15);
HiR = dd(16:18);
KL = dd(19:21);
KR = dd(22:24);

FL = dd(25:27);
FR = dd(28:30);

UH = dd(31:33);
USL = dd(34:36);
USR = dd(37:39);
HL = dd(40:42);
HR = dd(43:45);
H = dd(46:48);
C = dd(49:51);
UC = dd(52:54);
FLe = dd(55:57);
FRe = dd(58:60);

pr = [SL; EL; SR; ER; HiL; HiR; KL; KR; FL; FR; UH; USL; USR; HL; HR; H; C; UC; FLe; FRe];


figure
tmp =   [SL(1), SL(2),  SL(3);
        EL(1),  EL(2),  EL(3);
        HL(1),  HL(2),  HL(3)];

plot3(tmp(:,1), tmp(:,2), tmp(:,3), 'r');
hold on

tmp =   [SL(1), SL(2),  SL(3);
        SR(1), SR(2), SR(3);
        ER(1),  ER(2),  ER(3);
        HR(1),  HR(2),  HR(3)];
plot3(tmp(:,1), tmp(:,2), tmp(:,3), 'g');

tmp =   [HiL(1),    HiL(2),     HiL(3);
        KL(1),      KL(2),      KL(3);
        FL(1),      FL(2),      FL(3); 
        FLe(1),     FLe(2),     FLe(3)];
        plot3(tmp(:,1), tmp(:,2), tmp(:,3), 'k');

tmp =   [HiL(1), HiL(2), HiL(3);
        HiR(1),     HiR(2),     HiR(3);
        KR(1),      KR(2),      KR(3);
        FR(1),      FR(2),      FR(3);
        FRe(1),     FRe(2),     FRe(3)];
        plot3(tmp(:,1), tmp(:,2), tmp(:,3), 'c');   
 
tmp =   [UH(1),     UH(2),      UH(3);
        C(1),       C(2),       C(3);
        UC(1),      UC(2),      UC(3);
        USL(1),     USL(2),     USL(3);
        H(1),       H(2),       H(3)];
        plot3(tmp(:,1), tmp(:,2), tmp(:,3), 'm'); 

plot3(pr(:,1), pr(:,2), pr(:,3),'o')
axis([-2000 2000 -100 2000 -2000 2000])
xlabel('X axis (mm)');
ylabel('Y axis (mm)');
zlabel('Z axis (mm)');
grid on
shg
