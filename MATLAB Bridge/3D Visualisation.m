%          You have to send data According to:
%          qw(\t)qx(\t)qy(\t)qz(\t)(\n)

function yprSphere()

close all; clear; clc;
fprintf('waiting for connection..')
buttonEnd=false;
%SERIAL: Creat Port
%s = serial('COM20')
%Open Port 
%fopen(s);
%closeS=onCleanup(@() sclose(s));

s=tcpip('localhost', 4014,'NetworkRole','client');
set(s,'InputBufferSize',30000);
%Creat Port
%Open Port 
fopen(s);
closeS=onCleanup(@() fclose(s));

a(1,:)=fscanf(s,'%f');
a(1,:)=fscanf(s,'%f');
yaw_l = a(1,2);
pitch_l = a(1,3);
roll_l = a(1,4);


hbutton=uicontrol(gcf,'style','pushbutton',...
                                 'string','End',...
                                 'callback',@Button_Callback...
                                );




h=prismg(-3,-10,-1,6,20,2);
Rt=makehgtform('zrotate',(yaw_l)/180*pi,'yrotate',(pitch_l)/180*pi,'xrotate',(roll_l)/180*pi);
set(h, 'Matrix', Rt);       
axis([-20 20 -20 20 -20 20]) 
title('Pendulum')
xlabel('x'); ylabel('y'); zlabel('z');


while buttonEnd==false
    
    %Read Data;
    a(1,:)=fscanf(s,'%f');
    yaw = a(1,2);
    pitch = a(1,3);
    roll = a(1,4);
    %Visualize Data On Sphere Or any Other Objects
    %[x,y,z] = sphere;h = surf(x,y,z);axis('square'); 


%     t = hgtransform;
%     set(h, 'Parent', t);
%     h = t;
    %Rotate Object
    
    %Rt=makehgtform('xrotate',roll-roll_c,'yrotate',pitch-pitch_c,'zrotate',yaw-yaw_c);
    Rt=makehgtform('zrotate',wrap180(yaw-yaw_l)/180*pi*-1,'yrotate',-1*wrap180(pitch-pitch_l)/180*pi,'xrotate',wrap180(roll-roll_l)/180*pi);
  
    % Rt=makehgtform('zrotate',-1*wrap180(yaw)/180*pi,'yrotate',-1*wrap180(pitch)/180*pi,'xrotate',wrap180(roll)/180*pi);
    
    set(h,'Matrix',Rt);

%     Xt=makehgtform('xrotate',(roll-roll_l)/180*pi);
%     Yt=makehgtform('yrotate',(pitch-pitch_l)/180*pi);
%     Zt=makehgtform('zrotate',(yaw-yaw_l)/180*pi);
%     set(h, 'Matrix', Xt);
%     set(h, 'Matrix', Yt);
%     set(h, 'Matrix', Zt);
%     
    %disp([(roll-roll_l)/180*pi (pitch-pitch_l)/180*pi (yaw-yaw_l)/180*pi]);
    %disp([yaw pitch roll]);
    disp([wrap180(yaw-yaw_l) wrap180(pitch-pitch_l) wrap180(roll-roll_l)]);
    
    
    
%     
%     rotate(h,[1,0,0],(roll-roll_c))
%     rotate(h,[0,1,0],(pitch-pitch_c))
%     rotate(h,[0,0,1],(yaw-yaw_c))

    drawnow
end

function r=wrap180(x)
    if (x<-180)
        r=x+360;
    elseif (x>180)
        r=x-360;
    else 
        r=x;
    end
end    
   
   
function h = prismg(x, y, z, w, l, h)
    [X Y Z] = prism_faces(x, y, z, w, l, h);

    faces(1, :) = [4 2 1 3];
    faces(2, :) = [4 2 1 3] + 4;
    faces(3, :) = [4 2 6 8];
    faces(4, :) = [4 2 6 8] - 1;
    faces(5, :) = [1 2 6 5];
    faces(6, :) = [1 2 6 5] + 2;

    for i = 1:size(faces, 1)
        h(i) = fill3(X(faces(i, :)), Y(faces(i, :)), Z(faces(i, :)),'r'); hold on;
    end

    set(h(3),'FaceColor', 'w');
    
    % Conjoin all prism faces into one object.
    t = hgtransform;
    set(h, 'Parent', t);
    h = t;
end
 
% Compute the points on the edge of a prism at
% location (x, y, z) with width w, length l, and height h.
function [X Y Z] = prism_faces(x, y, z, w, l, h)
    X = [x x x x x+w x+w x+w x+w];
    Y = [y y y+l y+l y y y+l y+l];
    Z = [z z+h z z+h z z+h z z+h];
end



function Button_Callback(src,evt)
    fclose(s);
    close(gcf);
    buttonEnd=true;
end

function sclose(t)
    fclose(t);
end



end