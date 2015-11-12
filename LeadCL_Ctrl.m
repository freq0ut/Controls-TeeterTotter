% Zack Goyetche
% Lead compensator closed loop control simulations
% Controls II Lab

clc;
clear all;

%UNITS: Distance: millimeters
%       Voltage: Volts

%CONSTANTS-----------------------------------------------------------------
Ka = 0.27399;   % Actuator gain
Ks = 0.06339;   % Sensor gain
Kcomp = 267;    % Compensator gain that brings poles close to real axis
a = 0.934;      % Zero location
T = 0.018       % sample period of 18 ms
 
%S-DOMAIN OPEN LOOP PLANT TRANSFER FUNCTION--------------------------------
Gs_num = [17.34^2];             % wn^2
Gs_den = [1 0.6936 17.34^2];    % s^2 + 2*z*wn*s + wn^2
Gs = tf(Gs_num,Gs_den)          % Gs transfer function

%Z DOMAIN OL TRANSFER FUNCTION + ZOH---------------------------------------
Gz = c2d(Gs, T, 'zoh') % Gz transfer function with ZOH (plant)
%dcgain(Gz)             %The gain should be unity.
[Gz_num, Gz_den] = tfdata(Gz,'v'); % num and den of Gz

%Z DOMAIN LEAD COMP--------------------------------------------------------
Dz_num = [1 -a];    % numerator of the compensator Dz 
                    % (a is the locaiton of zero)                  
Dz_den = [1 0];     % denominator of the compensator Dz
Dz = tf(Dz_num,Dz_den,T); % Dz transfer function (lead compensator)

%Z DOMAIN OPENLOOP FORWARD PATH--------------------------------------------
OL_FPath = Gz*Dz*Ka*Ks; % DzGz together (plant, Ka, and compensator)

%Z DOMAIN CLOSED LOOP CONTROLLER-------------------------------------------
CL_Ctlr_num = [Dz_num*Ka*Ks*Kcomp];

%Z DOMAIN CLOSED LOOP CONTROLLER AND PLANT---------------------------------
CL_Ctlr_Plant_num = [conv(Gz_num, Dz_num)*Ka*Ks*Kcomp];
CL_den = [(conv(Dz_den, Gz_den) + conv(Gz_num, Dz_num)*Ka*Ks*Kcomp)];
CL_Ctlr = tf(CL_Ctlr_num, CL_den, T)    % closed-loop controller
CLz = tf(CL_Ctlr_Plant_num, CL_den, T)  % closed-loop controller + Plant

%PLOTS---------------------------------------------------------------------
figure(1)
    subplot(2,2,1)
        rlocus(OL_FPath)
        title('OL Forward Path RL')
    subplot(2,2,2)
        step(Gz)
        title('OL Plant Step Response')
    subplot(2,2,3)
        step(CL_Ctlr)
        title('Closed Loop Controller Step Response')
    subplot(2,2,4)
        step(CLz)
        title('Closed Loop System Step Response')
    

figure(2)
    subplot(1,2,1)
        pzmap(CL_Ctlr)
        title('Closed Loop Lead Comp. PZ Map')
    subplot(1,2,2)
        pzmap(CLz)
        title('Closed Loop System PZ Map')
    