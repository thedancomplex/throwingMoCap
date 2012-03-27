%% function to create a plot
function [ mName, mDeg ] = getMoCapPlot( dd, DD )

%function [ mName, mDeg ] = getMoCapPlot( dd, DD )
% Send;
%   dd = global fram
%   DD = local frame
%
% Return:
%   mName   =    Name of the joint in aces format
%   mDeg    =    the value in rad that the joint will be set to




%% find and print on global frame

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
Nk  = dd(61:63);

pr = [SL; EL; SR; ER; HiL; HiR; KL; KR; FL; FR; UH; USL; USR; HL; HR; H; C; UC; FLe; FRe ; Nk];



tmp =   [SL(1), SL(2),  SL(3);
        EL(1),  EL(2),  EL(3);
        HL(1),  HL(2),  HL(3)];

plot3(tmp(:,1), tmp(:,2), tmp(:,3), 'g');
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
        plot3(tmp(:,1), tmp(:,2), tmp(:,3), 'g');

tmp =   [HiL(1), HiL(2), HiL(3);
        HiR(1),     HiR(2),     HiR(3);
        KR(1),      KR(2),      KR(3);
        FR(1),      FR(2),      FR(3);
        FRe(1),     FRe(2),     FRe(3)];
        plot3(tmp(:,1), tmp(:,2), tmp(:,3), 'g');   
 
tmp =   [UH(1),     UH(2),      UH(3);
        C(1),       C(2),       C(3);
        UC(1),      UC(2),      UC(3);
        USL(1),     USL(2),     USL(3);
        Nk(1),      Nk(2),      Nk(3);
        H(1),       H(2),       H(3)];
        plot3(tmp(:,1), tmp(:,2), tmp(:,3), 'g'); 

plot3(pr(:,1), pr(:,2), pr(:,3),'o')
hold on
plot3(Nk(1), Nk(2), Nk(3),'g+');
axis([-2000 2000 -100 2000 -1000 1000])
xlabel('X axis (mm)');
ylabel('Y axis (mm)');
zlabel('Z axis (mm)');
grid on
shg



%% Find and print Local Frame with ref to the first point of UH (Upper Hip)


UH_L    =   DD(71:77);      % upper hip
Ch_L    =   DD(113:119);      % Chest

%% Plot Upper hip to Chest
L1      =   UH_L(7);
R1      =   (UH_L(1));
P1      =   (UH_L(2));
Y1      =   (UH_L(3));

L2      =   Ch_L(7);
R2      =   (Ch_L(1));
P2      =   (Ch_L(2));
Y2      =   (Ch_L(3));

r1 = [ R1 0 0 ; 0 P1 0 ; 0 0 Y1];

r2 = [ R2 0 0 ; 0 P2 0 ; 0 0 Y2];

% Start at a known world point of the Upper Hip
x1 = UH(1);
y1 = UH(2);
z1 = UH(3);
t1 = [ x1; y1; z1];

%% Upper hip to chest, note Upper hip is known from the world corordinates.
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
    
jd = DD(71:77);
%% --------------------------------
Tf_Orig_to_UpperHip = T1*T0; 
xyz_Orig_to_UpperHip = getD(jd,'xyz','y',1);
%% --------------------------------
Tf = T2*T1*T0;

jd = DD(113:119);
%% --------------------------------
Tf_UpperHip_to_Chest = T2;
xyz_UpperHip_to_Chest = getD(jd,'zxy','y',1);
%% --------------------------------



    


%% Chest to Upper Chest
jd = DD(120:126);
T4 = getT(jd);

%% --------------------------------
Tf_Chest_to_UpperChest = T4;
xyz_Chest_to_UpperChest = getD(jd,'yzx','y',1);
%% --------------------------------



%% Upper Chest to LeftShoulder
T4 = getT(DD(78:84));

%% --------------------------------
Tf_UpperChest_to_LeftShoulder = T4;
%% --------------------------------

%% LeftShoulder to Left Upper Arm
T4 = getT(DD(1:7));

%% --------------------------------
Tf_LeftShoulder_to_LeftUpperArm = T4;
%% --------------------------------


%% Left Upper Arm to Left Elbow
T4 = getT(DD(8:14));

%% --------------------------------
Tf_LeftUpperArm_to_LeftElbow = T4;
%% --------------------------------


%% Left Elbow to Left Hand
T4 = getT(DD(92:98));

%% --------------------------------
Tf_LeftElbow_to_LeftHand = T4;
%% --------------------------------

%% Upper Chest to Right Shoulder
 
T4 = getT(DD(85:91));

%% --------------------------------
Tf_UpperChest_to_Right_Shoulder = T4;
%% --------------------------------



%% Right Shoulder to Right Upper Arm

T4 = getT(DD(15:21));

%% --------------------------------
Tf_Right_Shoulder_to_Right_Upper_Arm = T4;
%% --------------------------------



%% Right Upper Arm to Right Elbow

T4 = getT(DD(22:28));

%% --------------------------------
Tf_Right_Upper_Arm_to_Right_Elbow = T4;
%% --------------------------------
%% Right Elbow to Right Hand

T4 = getT(DD(99:105));

%% --------------------------------
Tf_Right_Elbow_to_Right_Hand = T4;
%% --------------------------------

%% Origin to Left Thigh

T4 = getT(DD(29:35));


%% --------------------------------
Tf_Orig_to_LeftThigh = T4;
%% --------------------------------


%% Left Thigh to Left Shin

T4 = getT(DD(43:49));


%% --------------------------------
Tf_LeftThigh_to_LeftShin = T4;
%% --------------------------------

%% Left Shin to Left Foot

T4 = getT(DD(57:63));


%% --------------------------------
Tf_LeftShin_to_LeftFoot = T4;
%% --------------------------------

%% Left Foot to Left Foot End

T4 = getT(DD(127:133));


%% --------------------------------
Tf_LeftFoot_to_LeftFootEnd = T4;
%% --------------------------------

%% Origin to Right Thigh

T4 = getT(DD(36:42));


%% --------------------------------
Tf_Orig_to_RightThigh = T4;
%% --------------------------------

%% Right Thigh to Right Shin

T4 = getT(DD(50:56));


%% --------------------------------
Tf_RightThigh_to_RightShin = T4;
%% --------------------------------

%% Right Shin to Right Foot

T4 = getT(DD(64:70));


%% --------------------------------
Tf_RightShin_to_RightFoot = T4;
%% --------------------------------

%% Right Foot to Right Foot End

T4 = getT(DD(134:140));


%% --------------------------------
Tf_RightFoot_to_RightFootEnd = T4;
%% --------------------------------


%% Upper Chest to Neck
T4 = getT(DD(141:147));

%% --------------------------------
Tf_RightUpperChest_to_Neck = T4;
%% --------------------------------

%% Neck to Head
T4 = getT(DD(106:112));

%% --------------------------------
Tf_Neck_to_Head = T4;
%% --------------------------------

%% Upper Hip to Head
L4 = [ UH(1), UH(2), UH(3) ];
T = Tf_UpperHip_to_Chest * Tf_Orig_to_UpperHip;
L4 = [[T(1,4), T(2,4), T(3,4)]; L4 ];
T = Tf_Chest_to_UpperChest * T;
L4 = [[T(1,4), T(2,4), T(3,4)]; L4 ];
T = Tf_RightUpperChest_to_Neck * T;
L4 = [[T(1,4), T(2,4), T(3,4)]; L4 ];
T = Tf_Neck_to_Head * T;
L4 = [[T(1,4), T(2,4), T(3,4)]; L4 ];

plot3(L4(:,1), L4(:,2), L4(:,3),'r','LineWidth',2);






%% Upper Hip to Right Foot
L3 = [ UH(1), UH(2), UH(3) ];
T = Tf_Orig_to_RightThigh * Tf_Orig_to_UpperHip;
L3 = [[T(1,4), T(2,4), T(3,4)]; L3 ];
T = Tf_RightThigh_to_RightShin * T;
L3 = [[T(1,4), T(2,4), T(3,4)]; L3 ];
T = Tf_RightShin_to_RightFoot * T;
L3 = [[T(1,4), T(2,4), T(3,4)]; L3 ];
T = Tf_RightFoot_to_RightFootEnd * T;
L3 = [[T(1,4), T(2,4), T(3,4)]; L3 ];

plot3(L3(:,1), L3(:,2), L3(:,3),'r','LineWidth',3);



%% Upper Hip to Left Foot
L2 = [ UH(1), UH(2), UH(3) ];
T = Tf_Orig_to_LeftThigh * Tf_Orig_to_UpperHip;
L2 = [[T(1,4), T(2,4), T(3,4)]; L2 ];
T = Tf_LeftThigh_to_LeftShin * T;
L2 = [[T(1,4), T(2,4), T(3,4)]; L2 ];
T = Tf_LeftShin_to_LeftFoot * T;
L2 = [[T(1,4), T(2,4), T(3,4)]; L2 ];
T = Tf_LeftFoot_to_LeftFootEnd * T;
L2 = [[T(1,4), T(2,4), T(3,4)]; L2 ];
plot3(L2(:,1), L2(:,2), L2(:,3),'r','LineWidth',3);

% Tf_Orig_to_LeftThigh



%% Upper Hip to Right Hand
L1 = [ UH(1), UH(2), UH(3) ];
T = Tf_UpperHip_to_Chest * Tf_Orig_to_UpperHip;
L1 = [[T(1,4), T(2,4), T(3,4)]; L1 ];
T = Tf_Chest_to_UpperChest * T;
L1 = [[T(1,4), T(2,4), T(3,4)]; L1 ];
T = Tf_UpperChest_to_Right_Shoulder * T;
L1 = [[T(1,4), T(2,4), T(3,4)]; L1 ];
T = Tf_Right_Shoulder_to_Right_Upper_Arm * T;
L1 = [[T(1,4), T(2,4), T(3,4)]; L1 ];
T = Tf_Right_Upper_Arm_to_Right_Elbow * T;
L1 = [[T(1,4), T(2,4), T(3,4)]; L1 ];
T = Tf_Right_Elbow_to_Right_Hand * T;
L1 = [[T(1,4), T(2,4), T(3,4)]; L1 ];

%% Upper Hip to Right Hand
% Tf_Orig_to_UpperHip
% Tf_UpperHip_to_Chest
% Tf_Chest_to_UpperChest
% Tf_UpperChest_to_Right_Shoulder
plot3(L1(:,1), L1(:,2), L1(:,3),'r','LineWidth',3);


%% Upper Hip to Left Hand
L = [ UH(1), UH(2), UH(3) ];
T = Tf_UpperHip_to_Chest * Tf_Orig_to_UpperHip;
L = [[T(1,4), T(2,4), T(3,4)]; L ];
T = Tf_Chest_to_UpperChest * T;
L = [[T(1,4), T(2,4), T(3,4)]; L ];
T = Tf_UpperChest_to_LeftShoulder * T;
L = [[T(1,4), T(2,4), T(3,4)]; L ];
T = Tf_LeftShoulder_to_LeftUpperArm * T;
L = [[T(1,4), T(2,4), T(3,4)]; L ];
T = Tf_LeftUpperArm_to_LeftElbow * T;
L = [[T(1,4), T(2,4), T(3,4)]; L ];
T = Tf_LeftElbow_to_LeftHand*T;
L = [[T(1,4), T(2,4), T(3,4)]; L ];
plot3(L(:,1), L(:,2), L(:,3),'r','LineWidth',3);


%% Upper Hip to Left Hand
% Tf_Orig_to_UpperHip
% Tf_UpperHip_to_Chest
% Tf_Chest_to_UpperChest
% Tf_UpperChest_to_LeftShoulder
% Tf_LeftShoulder_to_LeftUpperArm
% Tf_LeftUpperArm_to_LeftElbow
% Tf_LeftElbow_to_LeftHand


%% plot rot and length stuff

%% Hip to Left Arm
%L = [ UH(1), UH(2), UH(3) ];
a = xyz_Orig_to_UpperHip;
L = [a(1), a(2), a(3)];
a = xyz_UpperHip_to_Chest;
Lt = [ a' ; L];
L  = [sum(Lt) ; L]; 
a = xyz_Chest_to_UpperChest;
Lt = [ a' ; L];
L  = [sum(Lt) ; L]; 
plot3(L(:,1), L(:,2), L(:,3),'b','LineWidth',4);

%% Set values for the return

%% set the names
mName = {'RSP', 'RSR', 'RSY', 'LSP', 'LSR', 'LSY' };

mDeg = [];
for( i = 1:length(mName))
    mDeg(i) = 0;
end

%% LSP
t = Tf_LeftUpperArm_to_LeftElbow * Tf_LeftShoulder_to_LeftUpperArm;
tr = t(1,1);
tp = t(2,2);
ty = t(3,3);
%i = min(strcmp(mName,'LSP'));
mDeg(4) = tr;
%disp(num2str(t))




end