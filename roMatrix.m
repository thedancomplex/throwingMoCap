function [m] = roMatrix(r,p,y,d)

m = [];
dt = {'xyz','xzy','yxz','yzx','zxy','zyx'};

i = strcmp(dt,d);
i = min(find(i == 1));

R = [	1 	0 	0 ;
	0 	cos(r) 	-sin(r);
	0 	sin(r)	cos(r)];

P = [ cos(p) 	0	sin(p);
	0	1	0;
	-sin(p)	0	cos(p)];

Y = [ cos(y)	-sin(y)	0;
	sin(y)	cos(y)	0;
	0 	0	1];
%%
% x = R
% y = P
% z = Y

switch i
	case 1	% XYZ
		m = Y*P*R;
	case 2 % XZY
		m = P*Y*R;
	case 3 % yxz
		m = Y*R*P;
	case 4 % yzx
		m = R*Y*P;
	case 5 %zxy
		m = P*R*Y;
	case 6 %zyx
		m = R*P*Y;
end



