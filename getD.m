function [ xyz ] = getD( jnt, ord, j, jd )
%% function [ xyz ] = getD( jnt, ord, j, jd )
% Send:
%   jnt     =   Joint data from rpyLocal file
%   ord     =   order of joints 'xyz' 'zxy' etc
%   j       =   joint axis 'x' 'y' 'z'
%   jd      =   joint direction -1 or 1

R  =   (jnt(1));
P  =   (jnt(2));
Y  =   (jnt(3));

L = jnt(7);     % length
% x = jnt(4);
% y = jnt(5); 
% z = jnt(6);

% 
% L = sqrt(x*x + y*y + z*z);

m = roMatrix(R, P, Y, ord);


dt = {'x','y','z'};
i = strcmp(dt, j);
i = min(find(i == 1));

b = [];
switch i
        case 1  % x
                b = [ L ; 0 ; 0 ] * jd;
        case 2 % y
                b = [ 0 ; L ; 0 ] * jd;
        case 3 % z
                b = [ 0 ; 0 ; L ] * jd;
end

    
xyz = m*b;
end