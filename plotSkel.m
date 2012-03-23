close all
clear all

%% Global Data
[h,d]=hdrload('moCapCapture/SampleClient/xyzGlobal.pts');
s = size(d);

%% find and print on global frame
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

%% Local Data
[H,D]=hdrload('moCapCapture/SampleClient/rpyLocal.pts');
S = size(D);

%% Find and print Local Frame with ref to the first point of UH (Upper Hip)
DD = D(S(1),:);

UH_L    =   DD(71:77);      % upper hip
Ch_L    =   DD(113:119);      % Chest

%% Plot Upper hip to Chest
L1      =   UH_L(7);
R1      =   deg2rad(UH_L(1));
P1      =   deg2rad(UH_L(2));
Y1      =   deg2rad(UH_L(3));

L2      =   Ch_L(7);
R2      =   deg2rad(Ch_L(1));
P2      =   deg2rad(Ch_L(2));
Y2      =   deg2rad(Ch_L(3));

r1 = [ R1 0 0 ; 0 P1 0 ; 0 0 Y1];

r2 = [ R2 0 0 ; 0 P2 0 ; 0 0 Y2];

% Start at a known world point of the Upper Hip
x1 = UH(1);
y1 = UH(2);
z1 = UH(3);
t1 = [ x1; y1; z1];

% next point is the chest
x2 = Ch_L(4);
y2 = Ch_L(5);
z2 = Ch_L(6);

t2 = [ x2; y2; z2];

%r2 = [ 0,0,0;0,0,0;0,0,0 ];
%r0 = [ 0 0 0 ; 0 0 0; 0 0 0];
T1 = [ r1, t1; 0 0 0 1];
T2 = [ r2, t2; 0 0 0 1];

T0 = [  0 0 0 0 ;
        0 0 0 0 ;
        0 0 0 0 ;
        0 0 0 1];
    
Tf = T2*T1*T0;


Lf = [  Tf(1,4),    Tf(2,4),    Tf(3,4);
        x1,         y1,         z1];
    
plot3(Lf(:,1), Lf(:,2), Lf(:,3),'LineWidth',6);

% Chest to Upper Chest
UC_L = DD(120:126);
x3  =   UC_L(4);
y3  =   UC_L(5);
z3  =   UC_L(6);

R3  =   deg2rad(UC_L(1));
P3  =   deg2rad(UC_L(2));
Y3  =   deg2rad(UC_L(3));

r3  = [ R3 0 0 ; 0 P3 0 ; 0 0 Y3 ];
t3  = [ x3 ; y3 ; z3 ];

T3  = [r3, t3; 0 0 0 1];

Tf = T3*Tf;

Lf = [[Tf(1,4),    Tf(2,4),    Tf(3,4)];Lf];
plot3(Lf(:,1), Lf(:,2), Lf(:,3),'g','LineWidth',3);




UH_a    = UH_L(1:3)*180/pi;

%disp(num2str(UH_a));



