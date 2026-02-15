
% Script for testing the stress implementation
% The script works by letting the user set up 
% various "cases" which run as a batch file.
% The output from the cases is used for initializing
% the comparison data in the unit test code.
% The unit test code can refer back to this file
% by the "case number".

format long g

disp('Case 1')

E  = 1000.0;
nu = 0.49;
D = [0.001,0.001,0.0001]';
k = planestress(E,nu);
s = k*D

