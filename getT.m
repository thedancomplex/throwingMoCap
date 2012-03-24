function [ T ] = getT( jnt )


x4  =   jnt(4);
y4  =   jnt(5);
z4  =   jnt(6);

R4  =   deg2rad(jnt(1));
P4  =   deg2rad(jnt(2));
Y4  =   deg2rad(jnt(3));

r4  = [ R4 0 0 ; 0 P4 0 ; 0 0 Y4 ];
t4  = [ x4 ; y4 ; z4 ];

T  = [r4, t4; 0 0 0 1];
end