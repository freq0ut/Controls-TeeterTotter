% Zack Goyetche
% Open loop compensator simulations
% Controls II Lab

clc;
clear all;

%UNITS: Distance: millimeters
%       Voltage: Volts

%CONSTANTS-----------------------------------------------------------------
Ka = 0.27399;                   % Actuator gain
T = 0.018;                      % sample period of 18 ms

%S DOMAIN OL TRANSFER FUNCTION (Gs)----------------------------------------
Gs_num = [17.34^2];             % wn^2
Gs_den = [1 0.6936 17.34^2];    % s^2 + 2*z*wn*s + wn^2
Gs = tf(Gs_num,Gs_den)  % Gs transfer function

%Z DOMAIN OL TRANSFER FUNCTION (Gz * ZOH)----------------------------------
Gz = c2d(Gs, T, 'zoh'); % Gz transfer function with ZOH (plant)
[Gz_num, Gz_den] = tfdata(Gz,'v'); % num and den of Gz 
poleLocations = 0.69; % Change pole locations to a reasonable place on
                      % the real axis.

%Z DOMAIN OL COMPENSATOR (Fz)----------------------------------------------
Fz_num = Gz_den;            % numerator of the compensator Fz 
                            % (zeros should cancel plant's poles)                    
Fz_den = [1 -poleLocations*2 poleLocations^2]; % denominator of the comp
Fz = tf(Fz_num,Fz_den,T)    % Fz transfer function (compensator)
Fz_DCgain = dcgain(Fz)      % DC gain of Fz
K_Fz = 1/Fz_DCgain          % this is the gain to bring Fz to unity

%Z DOMAIN OL COMPENSATOR + PLANT (FGz)-------------------------------------
FGz = Gz*Fz                 % FzGz together (plant and compensator)
[FGz_num, FGz_den] = tfdata(FGz, 'v'); % break out num and den of FGz       
FGz_DCgain = dcgain(FGz)    % find DC gain of FGz so that we can bring to unity
K = 1/FGz_DCgain            % this is the gain required to bring FGz to unity

%PLOTS---------------------------------------------------------------------
input = 10; % this controls the step input
figure(1)
    subplot(1,2,1)
        dstep(input*K.*FGz_num, FGz_den, 50)
        title('Compensated System Step Response')
    subplot(1,2,2)
        dstep(input*K_Fz.*Fz_num, Fz_den, 50)
        title('OL Compensator Step Response')
